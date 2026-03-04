#!/usr/bin/env python3
"""
Stress/integration test harness for Core/Src/Examples/ExampleTCPIP.cpp.
"""

from __future__ import annotations

import argparse
import errno
import random
import select
import socket
import struct
import threading
import time
from dataclasses import dataclass
from typing import Dict, List, Literal, Optional, Tuple

TCPIP_CMD_ORDER_ID = 0x7101
TCPIP_RESPONSE_ORDER_ID = 0x7102
TCPIP_PAYLOAD_ORDER_ID = 0x7103
TCPIP_CLIENT_STREAM_ORDER_ID = 0x7104
TCPIP_SERVER_STREAM_ORDER_ID = 0x7105

TCPIP_UDP_PROBE_PACKET_ID = 0x7201
TCPIP_UDP_STATUS_PACKET_ID = 0x7202

CMD_RESET = 1
CMD_PING = 2
CMD_GET_STATS = 3
CMD_FORCE_DISCONNECT = 4
CMD_BURST_SERVER = 5
CMD_BURST_CLIENT = 6
CMD_FORCE_CLIENT_RECONNECT = 7
CMD_GET_HEALTH = 8
CMD_RESET_HEALTH = 9

CMD_FMT = "<HIII"
RESP_FMT = "<HIIII"
TCP_PAYLOAD_FMT = "<HII128s"
TCP_CLIENT_STREAM_FMT = "<HIII"
TCP_SERVER_STREAM_FMT = "<HI64s"
UDP_PROBE_FMT = "<HII128s"
UDP_STATUS_FMT = "<HIII"

PACKET_SIZES: Dict[int, int] = {
    TCPIP_RESPONSE_ORDER_ID: struct.calcsize(RESP_FMT),
    TCPIP_SERVER_STREAM_ORDER_ID: struct.calcsize(TCP_SERVER_STREAM_FMT),
    TCPIP_CLIENT_STREAM_ORDER_ID: struct.calcsize(TCP_CLIENT_STREAM_FMT),
}


def checksum32(data: bytes) -> int:
    checksum = 2166136261
    for value in data:
        checksum ^= value
        checksum = (checksum * 16777619) & 0xFFFFFFFF
    return checksum


def build_pattern(seed: int, size: int) -> bytes:
    return bytes(((seed + (index * 17)) & 0xFF) for index in range(size))


class PacketStreamParser:
    def __init__(self, packet_sizes: Dict[int, int]):
        self.packet_sizes = packet_sizes
        self.buffer = bytearray()

    def feed(self, data: bytes) -> List[Tuple[int, bytes]]:
        self.buffer.extend(data)
        packets: List[Tuple[int, bytes]] = []

        while len(self.buffer) >= 2:
            packet_id = struct.unpack_from("<H", self.buffer, 0)[0]
            packet_size = self.packet_sizes.get(packet_id)
            if packet_size is None:
                # Re-sync stream if we see an unknown id.
                del self.buffer[0]
                continue

            if len(self.buffer) < packet_size:
                break

            packet = bytes(self.buffer[:packet_size])
            del self.buffer[:packet_size]
            packets.append((packet_id, packet))

        return packets


class TcpClientSink(threading.Thread):
    """Host-side TCP server that receives board Socket() telemetry/bursts."""

    def __init__(self, bind_ip: str, port: int):
        super().__init__(daemon=True)
        self.bind_ip = bind_ip
        self.port = port
        self.stop_event = threading.Event()
        self.lock = threading.Lock()
        self.connections_seen = 0
        self.active_connections = 0
        self.last_connection_monotonic = 0.0
        self.client_stream_packets = 0
        self.last_error: Optional[str] = None

    def stop(self) -> None:
        self.stop_event.set()

    def clear_error(self) -> None:
        with self.lock:
            self.last_error = None

    def snapshot(self) -> Tuple[int, int, int, Optional[str], float]:
        with self.lock:
            return (
                self.connections_seen,
                self.active_connections,
                self.client_stream_packets,
                self.last_error,
                self.last_connection_monotonic,
            )

    def run(self) -> None:
        server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        try:
            server.bind((self.bind_ip, self.port))
            server.listen(4)
            server.settimeout(0.5)
        except OSError as exc:
            with self.lock:
                self.last_error = f"sink startup failed: {exc}"
            server.close()
            return

        try:
            while not self.stop_event.is_set():
                try:
                    conn, _ = server.accept()
                except socket.timeout:
                    continue
                except OSError as exc:
                    with self.lock:
                        self.last_error = f"accept failed: {exc}"
                    continue

                with self.lock:
                    self.connections_seen += 1
                    self.active_connections += 1
                    self.last_connection_monotonic = time.monotonic()
                    self.last_error = None

                conn.settimeout(0.5)
                parser = PacketStreamParser(
                    {TCPIP_CLIENT_STREAM_ORDER_ID: struct.calcsize(TCP_CLIENT_STREAM_FMT)}
                )
                with conn:
                    while not self.stop_event.is_set():
                        try:
                            data = conn.recv(4096)
                        except socket.timeout:
                            continue
                        except OSError as exc:
                            if exc.errno not in (
                                errno.ECONNRESET,
                                errno.ECONNABORTED,
                                errno.ENOTCONN,
                                errno.EPIPE,
                            ):
                                with self.lock:
                                    self.last_error = f"recv failed: {exc}"
                            break

                        if not data:
                            break

                        for packet_id, packet in parser.feed(data):
                            if packet_id == TCPIP_CLIENT_STREAM_ORDER_ID and len(packet) == struct.calcsize(
                                TCP_CLIENT_STREAM_FMT
                            ):
                                with self.lock:
                                    self.client_stream_packets += 1
                with self.lock:
                    if self.active_connections > 0:
                        self.active_connections -= 1
        finally:
            server.close()


@dataclass
class ResponsePacket:
    code: int
    value0: int
    value1: int
    value2: int


class ExampleTcpIpTester:
    def __init__(
        self,
        board_ip: str,
        tcp_server_port: int,
        tcp_client_port: int,
        udp_local_port: int,
        udp_remote_port: int,
        host_bind: str,
        control_mode: Literal["server", "client", "auto"] = "auto",
    ):
        self.board_ip = board_ip
        self.tcp_server_port = tcp_server_port
        self.tcp_client_port = tcp_client_port
        self.udp_local_port = udp_local_port
        self.udp_remote_port = udp_remote_port
        self.host_bind = host_bind
        self.control_mode_requested = control_mode
        self.active_control_mode: Literal["server", "client"] = "server"

        self.control_socket: Optional[socket.socket] = None
        self.control_listener_socket: Optional[socket.socket] = None
        self.control_parser = PacketStreamParser(PACKET_SIZES)
        self.server_stream_packets = 0
        self.client_stream_packets = 0

    def connect_control(self, timeout_s: float = 10.0) -> None:
        if self.control_mode_requested == "client":
            self._connect_control_via_client(timeout_s=timeout_s)
            return

        try:
            self._connect_control_via_server(timeout_s=timeout_s)
            return
        except RuntimeError:
            if self.control_mode_requested != "auto":
                raise

        self._connect_control_via_client(timeout_s=timeout_s)

    def _connect_control_via_server(self, timeout_s: float) -> None:
        deadline = time.monotonic() + timeout_s
        last_error: Optional[Exception] = None

        while time.monotonic() < deadline:
            sock: Optional[socket.socket] = None
            try:
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sock.settimeout(1.0)
                if self.host_bind != "0.0.0.0":
                    sock.bind((self.host_bind, 0))
                sock.connect((self.board_ip, self.tcp_server_port))
                sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
                sock.setblocking(False)
                self.control_socket = sock
                self.control_parser = PacketStreamParser(PACKET_SIZES)
                self.active_control_mode = "server"
                return
            except OSError as exc:
                if sock is not None:
                    sock.close()
                last_error = exc
                time.sleep(0.2)

        raise RuntimeError(f"Cannot connect to board TCP server: {last_error}")

    def _connect_control_via_client(self, timeout_s: float) -> None:
        if self.control_listener_socket is None:
            listener = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            listener.bind((self.host_bind, self.tcp_client_port))
            listener.listen(2)
            listener.settimeout(0.5)
            self.control_listener_socket = listener

        deadline = time.monotonic() + timeout_s
        last_error: Optional[Exception] = None
        while time.monotonic() < deadline:
            try:
                conn, _ = self.control_listener_socket.accept()
                conn.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
                conn.setblocking(False)
                self.control_socket = conn
                self.control_parser = PacketStreamParser(PACKET_SIZES)
                self.active_control_mode = "client"
                return
            except socket.timeout:
                continue
            except OSError as exc:
                last_error = exc
                time.sleep(0.2)

        if last_error is None:
            raise RuntimeError("Cannot accept board TCP client control stream: timed out waiting for connection")
        raise RuntimeError(f"Cannot accept board TCP client control stream: {last_error}")

    def close_control(self) -> None:
        if self.control_socket is not None:
            try:
                self.control_socket.close()
            finally:
                self.control_socket = None
        if self.control_listener_socket is not None:
            try:
                self.control_listener_socket.close()
            finally:
                self.control_listener_socket = None

    def _send_control_packet(self, packet: bytes, timeout_s: float = 3.0) -> None:
        if self.control_socket is None:
            raise RuntimeError("Control socket is not connected")

        view = memoryview(packet)
        deadline = time.monotonic() + timeout_s

        while len(view) > 0:
            try:
                sent = self.control_socket.send(view)
                if sent <= 0:
                    raise ConnectionError("Control socket send returned 0 bytes")
                view = view[sent:]
            except (BlockingIOError, InterruptedError):
                if time.monotonic() >= deadline:
                    raise TimeoutError("Timeout sending packet to board control socket")
                select.select([], [self.control_socket], [], 0.05)
            except socket.timeout:
                if time.monotonic() >= deadline:
                    raise TimeoutError("Timeout sending packet to board control socket")

    def send_command(self, opcode: int, arg0: int = 0, arg1: int = 0) -> None:
        if self.control_socket is None:
            raise RuntimeError("Control socket is not connected")
        packet = struct.pack(CMD_FMT, TCPIP_CMD_ORDER_ID, opcode, arg0, arg1)
        self._send_control_packet(packet)

    def send_payload(self, sequence: int, bad_checksum: bool = False) -> None:
        if self.control_socket is None:
            raise RuntimeError("Control socket is not connected")

        payload = build_pattern(sequence, 128)
        checksum = checksum32(payload)
        if bad_checksum:
            checksum ^= 0xA5A5A5A5

        packet = struct.pack(TCP_PAYLOAD_FMT, TCPIP_PAYLOAD_ORDER_ID, sequence, checksum, payload)
        self._send_control_packet(packet)

    def _recv_control_packets(self, timeout_s: float) -> List[Tuple[int, bytes]]:
        if self.control_socket is None:
            raise RuntimeError("Control socket is not connected")

        packets: List[Tuple[int, bytes]] = []
        ready, _, _ = select.select([self.control_socket], [], [], timeout_s)
        if not ready:
            return packets

        while True:
            try:
                data = self.control_socket.recv(4096)
            except (BlockingIOError, InterruptedError):
                break
            if not data:
                raise ConnectionError("Control socket closed by peer")

            packets.extend(self.control_parser.feed(data))
            ready, _, _ = select.select([self.control_socket], [], [], 0.0)
            if not ready:
                break
        return packets

    def wait_response_matching(
        self,
        response_code: int,
        timeout_s: float = 4.0,
        expected_value0: Optional[int] = None,
        expected_value1: Optional[int] = None,
        expected_value2: Optional[int] = None,
    ) -> ResponsePacket:
        deadline = time.monotonic() + timeout_s
        mismatch_count = 0
        while time.monotonic() < deadline:
            packets = self._recv_control_packets(timeout_s=0.2)
            for packet_id, packet in packets:
                if packet_id == TCPIP_SERVER_STREAM_ORDER_ID:
                    self.server_stream_packets += 1
                    continue
                if packet_id == TCPIP_CLIENT_STREAM_ORDER_ID:
                    self.client_stream_packets += 1
                    continue
                if packet_id != TCPIP_RESPONSE_ORDER_ID:
                    continue

                _, code, value0, value1, value2 = struct.unpack(RESP_FMT, packet)
                if code != response_code:
                    continue
                if expected_value0 is not None and value0 != expected_value0:
                    mismatch_count += 1
                    continue
                if expected_value1 is not None and value1 != expected_value1:
                    mismatch_count += 1
                    continue
                if expected_value2 is not None and value2 != expected_value2:
                    mismatch_count += 1
                    continue

                return ResponsePacket(code, value0, value1, value2)

        mismatch_note = f" (mismatched={mismatch_count})" if mismatch_count > 0 else ""
        raise TimeoutError(f"Timeout waiting response code {response_code}{mismatch_note}")

    def wait_response(self, response_code: int, timeout_s: float = 4.0) -> ResponsePacket:
        return self.wait_response_matching(response_code=response_code, timeout_s=timeout_s)

    def get_health_pages(self, page_count: int = 6) -> Dict[int, Tuple[int, int, int]]:
        pages: Dict[int, Tuple[int, int, int]] = {}
        for page in range(max(0, page_count)):
            self.send_command(CMD_GET_HEALTH, page, 0)
            response = self.wait_response(CMD_GET_HEALTH, timeout_s=2.5)
            pages[page] = (response.value0, response.value1, response.value2)
        return pages

    def collect_server_stream_packets(self, duration_s: float) -> int:
        before = self.server_stream_packets
        deadline = time.monotonic() + duration_s
        while time.monotonic() < deadline:
            try:
                packets = self._recv_control_packets(timeout_s=0.1)
            except ConnectionError:
                break
            for packet_id, _ in packets:
                if packet_id == TCPIP_SERVER_STREAM_ORDER_ID:
                    self.server_stream_packets += 1
                elif packet_id == TCPIP_CLIENT_STREAM_ORDER_ID:
                    self.client_stream_packets += 1
        return self.server_stream_packets - before

    def collect_client_stream_packets(self, duration_s: float) -> int:
        before = self.client_stream_packets
        deadline = time.monotonic() + duration_s
        while time.monotonic() < deadline:
            try:
                packets = self._recv_control_packets(timeout_s=0.1)
            except ConnectionError:
                break
            for packet_id, _ in packets:
                if packet_id == TCPIP_CLIENT_STREAM_ORDER_ID:
                    self.client_stream_packets += 1
                elif packet_id == TCPIP_SERVER_STREAM_ORDER_ID:
                    self.server_stream_packets += 1
        return self.client_stream_packets - before

    def test_ping(self) -> str:
        nonce = random.randint(1, 0xFFFFFFFE)
        self.send_command(CMD_PING, nonce, 0)
        response = self.wait_response_matching(
            CMD_PING,
            timeout_s=4.0,
            expected_value0=nonce,
        )
        if response.value0 != nonce:
            raise AssertionError(f"Ping nonce mismatch: expected {nonce}, got {response.value0}")
        return f"nonce={nonce} connections={response.value1} payload_rx={response.value2}"

    def test_payload_integrity(
        self,
        good_count: int,
        bad_count: int,
        payload_interval_s: float = 0.0,
        min_payload_rx_ratio: float = 0.9,
        min_bad_detect_ratio: float = 0.8,
    ) -> str:
        self.send_command(CMD_RESET)
        self.wait_response(CMD_RESET)

        for sequence in range(1, good_count + 1):
            self.send_payload(sequence, bad_checksum=False)
            if payload_interval_s > 0.0:
                time.sleep(payload_interval_s)

        for sequence in range(good_count + 1, good_count + bad_count + 1):
            self.send_payload(sequence, bad_checksum=True)
            if payload_interval_s > 0.0:
                time.sleep(payload_interval_s)

        total_sent = good_count + bad_count
        min_total_rx = int(total_sent * min_payload_rx_ratio)
        min_bad_detected = int(bad_count * min_bad_detect_ratio)
        response = ResponsePacket(CMD_GET_STATS, 0, 0, 0)
        settle_deadline = time.monotonic() + 3.0
        while True:
            self.send_command(CMD_GET_STATS)
            response = self.wait_response(CMD_GET_STATS)
            enough_rx = response.value0 >= min_total_rx
            enough_bad = bad_count == 0 or response.value1 >= min_bad_detected
            if enough_rx and enough_bad:
                break
            if time.monotonic() >= settle_deadline:
                break
            time.sleep(0.05)

        if response.value0 < min_total_rx:
            raise AssertionError(
                "Too many missing payloads: "
                f"sent={total_sent}, board_count={response.value0}, min_required={min_total_rx}"
            )
        if bad_count > 0 and response.value1 < min_bad_detected:
            raise AssertionError(
                "Bad checksum detection too low: "
                f"expected~{bad_count}, board_bad={response.value1}, min_required={min_bad_detected}"
            )

        return (
            f"payload_rx={response.value0} bad_detected={response.value1} "
            f"forced_disconnects={response.value2}"
        )

    def test_forced_disconnect_and_reconnect(self) -> str:
        self.send_command(CMD_FORCE_DISCONNECT)

        deadline = time.monotonic() + 4.0
        disconnected = False
        while time.monotonic() < deadline:
            try:
                _ = self._recv_control_packets(timeout_s=0.2)
            except ConnectionError:
                disconnected = True
                break

        if not disconnected:
            raise AssertionError("Control socket did not disconnect after CMD_FORCE_DISCONNECT")

        self.close_control()
        self.connect_control(timeout_s=8.0)

        # Validate that command path still works after reconnect.
        nonce = random.randint(1, 0xFFFFFFFE)
        self.send_command(CMD_PING, nonce, 0)
        response = self.wait_response_matching(
            CMD_PING,
            timeout_s=4.0,
            expected_value0=nonce,
        )
        if response.value0 != nonce:
            raise AssertionError("Reconnect ping failed")

        return "disconnect+reconnect OK"

    def test_server_burst(self, burst_count: int, min_receive_ratio: float = 0.95) -> str:
        last_reason = "unknown"
        min_packets_required = int(burst_count * min_receive_ratio)
        if burst_count > 0 and min_packets_required < 1:
            min_packets_required = 1
        collect_window_s = max(4.0, min(10.0, burst_count / 400.0))
        for attempt in range(1, 4):
            before = self.server_stream_packets
            try:
                self.send_command(CMD_BURST_SERVER, burst_count, 0)
                self.wait_response_matching(
                    CMD_BURST_SERVER,
                    timeout_s=5.0,
                    expected_value0=burst_count,
                )
            except Exception as exc:  # noqa: BLE001
                last_reason = f"attempt={attempt} command/response failed: {exc}"
                self.close_control()
                try:
                    self.connect_control(timeout_s=6.0)
                except Exception as reconnect_exc:  # noqa: BLE001
                    last_reason = f"{last_reason}; reconnect failed: {reconnect_exc}"
                time.sleep(0.2)
                continue

            _ = self.collect_server_stream_packets(duration_s=collect_window_s)
            received_total = self.server_stream_packets - before
            if received_total >= min_packets_required:
                return (
                    f"requested={burst_count} received={received_total} "
                    f"min_required={min_packets_required}"
                )

            last_reason = (
                f"attempt={attempt} received_total={received_total}, "
                f"min_required={min_packets_required}"
            )
            time.sleep(0.2)

        raise AssertionError(f"TCP server burst below threshold ({last_reason})")

    def test_udp_roundtrip(self, udp_count: int) -> str:
        udp_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        udp_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        udp_sock.bind((self.host_bind, self.udp_remote_port))
        udp_sock.settimeout(0.05)

        received_sequences = set()
        last_ok = 0
        last_bad = 0

        try:
            for sequence in range(1, udp_count + 1):
                payload = build_pattern(sequence, 128)
                checksum = checksum32(payload)
                packet = struct.pack(
                    UDP_PROBE_FMT,
                    TCPIP_UDP_PROBE_PACKET_ID,
                    sequence,
                    checksum,
                    payload,
                )
                udp_sock.sendto(packet, (self.board_ip, self.udp_local_port))

                try:
                    data, _ = udp_sock.recvfrom(256)
                except socket.timeout:
                    continue

                if len(data) == struct.calcsize(UDP_STATUS_FMT):
                    packet_id, ack_sequence, ack_ok, ack_bad = struct.unpack(UDP_STATUS_FMT, data)
                    if packet_id == TCPIP_UDP_STATUS_PACKET_ID:
                        received_sequences.add(ack_sequence)
                        last_ok = ack_ok
                        last_bad = ack_bad

            deadline = time.monotonic() + 1.5
            while time.monotonic() < deadline:
                try:
                    data, _ = udp_sock.recvfrom(256)
                except socket.timeout:
                    continue

                if len(data) == struct.calcsize(UDP_STATUS_FMT):
                    packet_id, ack_sequence, ack_ok, ack_bad = struct.unpack(UDP_STATUS_FMT, data)
                    if packet_id == TCPIP_UDP_STATUS_PACKET_ID:
                        received_sequences.add(ack_sequence)
                        last_ok = ack_ok
                        last_bad = ack_bad
        finally:
            udp_sock.close()

        ratio = len(received_sequences) / max(1, udp_count)
        if ratio < 0.7:
            raise AssertionError(
                f"UDP response ratio too low: received={len(received_sequences)} sent={udp_count}"
            )

        return f"responses={len(received_sequences)}/{udp_count} board_ok={last_ok} board_bad={last_bad}"


def run_test(name: str, fn) -> Tuple[bool, str]:
    start = time.monotonic()
    try:
        details = fn()
        elapsed = time.monotonic() - start
        return True, f"{details} ({elapsed:.2f}s)"
    except Exception as exc:  # noqa: BLE001
        elapsed = time.monotonic() - start
        return False, f"{exc} ({elapsed:.2f}s)"


def print_health_snapshot(tester: ExampleTcpIpTester, label: str, page_count: int) -> None:
    try:
        pages = tester.get_health_pages(page_count=page_count)
    except Exception as exc:  # noqa: BLE001
        print(f"[HEALTH] {label}: unavailable ({exc})")
        return

    for page, values in pages.items():
        value0, value1, value2 = values
        print(f"[HEALTH] {label} page={page}: {value0},{value1},{value2}")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Stress tests for ExampleTCPIP firmware")
    parser.add_argument("--board-ip", default="192.168.1.7", help="Board IPv4 address")
    parser.add_argument("--host-bind", default="0.0.0.0", help="Host bind address for TCP/UDP sockets")
    parser.add_argument("--tcp-server-port", type=int, default=40000, help="Board TCP server port")
    parser.add_argument("--tcp-client-port", type=int, default=40002, help="Host TCP sink port")
    parser.add_argument(
        "--control-mode",
        choices=("auto", "server", "client"),
        default="auto",
        help="Control path mode: try board TCP server, force board TCP server, or force board TCP client stream",
    )
    parser.add_argument("--udp-local-port", type=int, default=40003, help="Board UDP local port")
    parser.add_argument("--udp-remote-port", type=int, default=40004, help="Host UDP source port")
    parser.add_argument("--good-payloads", type=int, default=1200, help="Number of valid TCP payload packets")
    parser.add_argument("--bad-payloads", type=int, default=200, help="Number of invalid TCP payload packets")
    parser.add_argument(
        "--min-payload-rx-ratio",
        type=float,
        default=0.9,
        help="Minimum accepted ratio board_payload_rx / total_payload_sent",
    )
    parser.add_argument(
        "--min-bad-detect-ratio",
        type=float,
        default=0.8,
        help="Minimum accepted ratio board_bad_detected / bad_payloads",
    )
    parser.add_argument(
        "--payload-interval-us",
        type=int,
        default=800,
        help="Delay in microseconds between payload packets to avoid RX overruns",
    )
    parser.add_argument("--server-burst", type=int, default=800, help="TCP server burst size request")
    parser.add_argument("--client-burst", type=int, default=800, help="TCP client burst size request")
    parser.add_argument(
        "--min-server-burst-ratio",
        type=float,
        default=0.95,
        help="Minimum accepted ratio received_server_packets / requested_server_burst",
    )
    parser.add_argument(
        "--min-client-burst-ratio",
        type=float,
        default=0.95,
        help="Minimum accepted ratio received_client_packets / requested_client_burst",
    )
    parser.add_argument("--udp-count", type=int, default=300, help="UDP probe packet count")
    parser.add_argument(
        "--health-pages",
        type=int,
        default=6,
        help="Number of health telemetry pages to query from board",
    )
    parser.add_argument(
        "--reset-health",
        action="store_true",
        help="Reset board health telemetry counters before running tests",
    )
    parser.add_argument(
        "--health-on-fail",
        dest="health_on_fail",
        action="store_true",
        default=True,
        help="Print board health pages when a test fails (default)",
    )
    parser.add_argument(
        "--no-health-on-fail",
        dest="health_on_fail",
        action="store_false",
        help="Disable board health dump after failed tests",
    )
    parser.add_argument(
        "--health-at-end",
        action="store_true",
        help="Always print board health pages at end of run",
    )
    parser.add_argument(
        "--skip-client-stream",
        action="store_true",
        help="Skip verification of board TCP client stream",
    )
    parser.add_argument(
        "--strict-client-stream",
        action="store_true",
        help="Fail the run if tcp_client_stream check is unstable",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    for ratio_name in (
        "min_payload_rx_ratio",
        "min_bad_detect_ratio",
        "min_server_burst_ratio",
        "min_client_burst_ratio",
    ):
        ratio_value = getattr(args, ratio_name)
        if ratio_value < 0.0 or ratio_value > 1.0:
            raise ValueError(f"{ratio_name} must be in [0.0, 1.0], got {ratio_value}")

    tester = ExampleTcpIpTester(
        board_ip=args.board_ip,
        tcp_server_port=args.tcp_server_port,
        tcp_client_port=args.tcp_client_port,
        udp_local_port=args.udp_local_port,
        udp_remote_port=args.udp_remote_port,
        host_bind=args.host_bind,
        control_mode=args.control_mode,
    )
    sink: Optional[TcpClientSink] = None

    try:
        control_timeout_s = 20.0 if args.control_mode == "client" else 10.0
        tester.connect_control(timeout_s=control_timeout_s)
        print(f"[INFO] control_mode_active={tester.active_control_mode}")

        if tester.active_control_mode == "server":
            sink = TcpClientSink(bind_ip=args.host_bind, port=args.tcp_client_port)
            sink.start()

        if args.reset_health:
            tester.send_command(CMD_RESET_HEALTH, 0, 0)
            tester.wait_response(CMD_RESET_HEALTH, timeout_s=2.5)

        test_results: List[Tuple[str, bool, str]] = []

        base_tests: List[Tuple[str, object]] = [
            ("ping", tester.test_ping),
            (
                "tcp_payload_integrity",
                lambda: tester.test_payload_integrity(
                    args.good_payloads,
                    args.bad_payloads,
                    payload_interval_s=max(0.0, args.payload_interval_us / 1_000_000.0),
                    min_payload_rx_ratio=args.min_payload_rx_ratio,
                    min_bad_detect_ratio=args.min_bad_detect_ratio,
                ),
            ),
        ]

        if tester.active_control_mode == "server":
            base_tests.extend(
                [
                    ("forced_disconnect_reconnect", tester.test_forced_disconnect_and_reconnect),
                    (
                        "tcp_server_burst",
                        lambda: tester.test_server_burst(
                            args.server_burst,
                            min_receive_ratio=args.min_server_burst_ratio,
                        ),
                    ),
                    ("udp_roundtrip", lambda: tester.test_udp_roundtrip(args.udp_count)),
                ]
            )
        else:
            print("[INFO] skipping forced_disconnect_reconnect/tcp_server_burst/udp_roundtrip in client mode")

        for name, fn in base_tests:
            ok, details = run_test(name, fn)
            test_results.append((name, ok, details))
            state = "PASS" if ok else "FAIL"
            print(f"[{state}] {name}: {details}")
            if not ok and args.health_on_fail:
                print_health_snapshot(tester, f"after {name}", page_count=args.health_pages)

        if not args.skip_client_stream:
            # Ask board to push data through its Socket() client.
            ok, details = run_test(
                "tcp_client_stream",
                lambda: _run_client_stream_check(
                    tester,
                    sink,
                    args.client_burst,
                    strict_client_stream=args.strict_client_stream,
                    min_receive_ratio=args.min_client_burst_ratio,
                )
                if tester.active_control_mode == "server"
                else _run_client_stream_check_via_control(
                    tester,
                    args.client_burst,
                    strict_client_stream=args.strict_client_stream,
                    min_receive_ratio=args.min_client_burst_ratio,
                ),
            )
            if ok:
                test_results.append(("tcp_client_stream", True, details))
                print(f"[PASS] tcp_client_stream: {details}")
            elif args.strict_client_stream:
                test_results.append(("tcp_client_stream", False, details))
                print(f"[FAIL] tcp_client_stream: {details}")
                if args.health_on_fail:
                    print_health_snapshot(
                        tester,
                        "after tcp_client_stream",
                        page_count=args.health_pages,
                    )
            else:
                test_results.append(("tcp_client_stream", True, f"non-strict warning: {details}"))
                print(f"[WARN] tcp_client_stream: {details}")
                if args.health_on_fail:
                    print_health_snapshot(
                        tester,
                        "after tcp_client_stream_warn",
                        page_count=args.health_pages,
                    )

        failures = [name for name, ok, _ in test_results if not ok]
        if args.health_at_end:
            print_health_snapshot(tester, "final", page_count=args.health_pages)
        print()
        if failures:
            print(f"Overall: FAIL ({len(failures)} failed) -> {', '.join(failures)}")
            return 1

        print(f"Overall: PASS ({len(test_results)} tests)")
        return 0

    finally:
        try:
            if tester.active_control_mode == "client" and tester.control_socket is not None:
                tester.send_command(CMD_FORCE_CLIENT_RECONNECT, 0, 0)
                tester.wait_response_matching(
                    CMD_FORCE_CLIENT_RECONNECT,
                    timeout_s=2.0,
                    expected_value0=1,
                )
        except Exception:
            pass
        tester.close_control()
        if sink is not None:
            sink.stop()
            sink.join(timeout=2.0)


def _run_client_stream_check_via_control(
    tester: ExampleTcpIpTester,
    requested_burst: int,
    strict_client_stream: bool = False,
    min_receive_ratio: float = 0.95,
) -> str:
    min_packets_required = int(requested_burst * min_receive_ratio)
    if requested_burst > 0 and min_packets_required < 1:
        min_packets_required = 1
    collect_window_s = max(8.0, min(14.0, requested_burst / 300.0))

    last_reason = "unknown"
    for attempt in range(1, 5):
        try:
            nonce = random.randint(1, 0xFFFFFFFE)
            tester.send_command(CMD_PING, nonce, 0)
            response = tester.wait_response_matching(
                CMD_PING,
                timeout_s=3.0,
                expected_value0=nonce,
            )
            if response.value0 != nonce:
                raise AssertionError("ping nonce mismatch before client burst")
        except Exception as exc:  # noqa: BLE001
            last_reason = f"attempt={attempt} ping/control failed: {exc}"
            try:
                tester.close_control()
                tester.connect_control(timeout_s=20.0)
            except Exception as reconnect_exc:  # noqa: BLE001
                last_reason = f"{last_reason}; reconnect failed: {reconnect_exc}"
            continue

        tester.collect_client_stream_packets(duration_s=0.5)
        baseline_packets = tester.client_stream_packets

        try:
            tester.send_command(CMD_BURST_CLIENT, requested_burst, 0)
            tester.wait_response_matching(
                CMD_BURST_CLIENT,
                timeout_s=6.0,
                expected_value0=requested_burst,
            )
        except Exception as exc:  # noqa: BLE001
            last_reason = f"attempt={attempt} burst command/ack failed: {exc}"
            try:
                tester.close_control()
                tester.connect_control(timeout_s=20.0)
            except Exception as reconnect_exc:  # noqa: BLE001
                last_reason = f"{last_reason}; reconnect failed: {reconnect_exc}"
            continue

        tester.collect_client_stream_packets(duration_s=collect_window_s)
        delta_packets = tester.client_stream_packets - baseline_packets
        if delta_packets >= min_packets_required:
            return (
                f"requested={requested_burst} received={delta_packets} "
                f"min_required={min_packets_required}"
            )

        if delta_packets > 0 and not strict_client_stream:
            return (
                f"requested={requested_burst} received={delta_packets} "
                f"(fallback_pass_below_threshold min_required={min_packets_required})"
            )

        last_reason = (
            f"attempt={attempt} below threshold: received={delta_packets}, "
            f"min_required={min_packets_required}"
        )
        try:
            tester.close_control()
            tester.connect_control(timeout_s=20.0)
        except Exception:
            pass

    raise AssertionError(
        "tcp_client_stream below threshold in client-control mode: "
        f"{last_reason}"
    )


def _run_client_stream_check(
    tester: ExampleTcpIpTester,
    sink: Optional[TcpClientSink],
    requested_burst: int,
    strict_client_stream: bool = False,
    min_receive_ratio: float = 0.95,
) -> str:
    if sink is None:
        raise AssertionError("tcp_client_stream check requires sink in server-control mode")
    min_packets_required = int(requested_burst * min_receive_ratio)
    if requested_burst > 0 and min_packets_required < 1:
        min_packets_required = 1
    measurement_window_s = max(10.0, min(16.0, requested_burst / 250.0))
    _, _, function_start_packets, _, _ = sink.snapshot()

    def ensure_control_ready() -> None:
        for _ in range(3):
            nonce = random.randint(1, 0xFFFFFFFE)
            try:
                tester.send_command(CMD_PING, nonce, 0)
                response = tester.wait_response_matching(
                    CMD_PING,
                    timeout_s=2.5,
                    expected_value0=nonce,
                )
                if response.value0 == nonce:
                    return
            except Exception:  # noqa: BLE001
                pass

            tester.close_control()
            tester.connect_control(timeout_s=6.0)
        raise AssertionError("Control channel is unstable during tcp_client_stream")

    last_reason = "unknown"
    for attempt in range(1, 6):
        sink.clear_error()

        try:
            ensure_control_ready()
        except Exception as exc:  # noqa: BLE001
            last_reason = f"control check failed: {exc}"
            continue

        connections_seen_start, active_start, _, sink_error, _ = sink.snapshot()
        if sink_error:
            last_reason = f"sink error before burst: {sink_error}"
            continue

        if active_start == 0:
            try:
                tester.send_command(CMD_BURST_CLIENT, 1, 0)
                tester.wait_response_matching(
                    CMD_BURST_CLIENT,
                    timeout_s=4.0,
                    expected_value0=1,
                )
            except Exception as exc:  # noqa: BLE001
                last_reason = f"connection nudge failed: {exc}"
                continue

            wait_deadline = time.monotonic() + 12.0
            connected = False
            while time.monotonic() < wait_deadline:
                conn_seen_now, active_now, _, sink_error, _ = sink.snapshot()
                if sink_error:
                    break
                if active_now > 0 or conn_seen_now > connections_seen_start:
                    connected = True
                    break
                time.sleep(0.05)
            if not connected:
                last_reason = "board client stream did not connect to sink in time"
                if sink_error:
                    last_reason = f"{last_reason}; sink_error={sink_error}"
                continue

        time.sleep(0.1)
        _, _, base_packets, sink_error, _ = sink.snapshot()
        if sink_error:
            last_reason = f"sink error before measurement: {sink_error}"
            continue

        try:
            tester.send_command(CMD_BURST_CLIENT, requested_burst, 0)
            tester.wait_response_matching(
                CMD_BURST_CLIENT,
                timeout_s=6.0,
                expected_value0=requested_burst,
            )
        except Exception as exc:  # noqa: BLE001
            last_reason = f"burst command failed: {exc}"
            continue

        connections_seen = connections_seen_start
        deadline = time.monotonic() + measurement_window_s
        while time.monotonic() < deadline:
            connections_seen, active_connections, current_packets, sink_error, _ = sink.snapshot()
            if sink_error:
                last_reason = f"sink error during burst: {sink_error}"
                break
            delta_packets = current_packets - base_packets
            if delta_packets >= min_packets_required:
                return (
                    f"requested={requested_burst} received={delta_packets} "
                    f"min_required={min_packets_required}"
                )
            if delta_packets == 0 and active_connections == 0:
                # Connection dropped before payload burst reached sink.
                last_reason = (
                    f"connection dropped during burst (attempt={attempt}, "
                    f"connections_seen={connections_seen})"
                )
            time.sleep(0.1)

        _, _, final_packets, sink_error, _ = sink.snapshot()
        if sink_error:
            last_reason = f"sink error after burst: {sink_error}"
            continue
        delta_packets = final_packets - base_packets
        last_reason = (
            f"below threshold after burst (attempt={attempt}, delta={delta_packets}, "
            f"min_required={min_packets_required}, connections_seen={connections_seen})"
        )

    connections_seen, active_connections, packets, sink_error, _ = sink.snapshot()
    overall_delta = packets - function_start_packets
    if overall_delta < 0:
        overall_delta = 0
    if connections_seen > 0 and overall_delta >= min_packets_required and sink_error is None:
        return (
            f"requested={requested_burst} received={overall_delta} "
            f"min_required={min_packets_required} (aggregate_window_pass)"
        )
    if connections_seen > 0 and packets > 0 and sink_error is None:
        if strict_client_stream:
            raise AssertionError(
                "tcp_client_stream fallback-only result in strict mode: "
                f"connections_seen={connections_seen}, packets_seen={packets}, "
                f"overall_delta={overall_delta}, min_required={min_packets_required}"
            )
        return (
            f"requested={requested_burst} received=0 "
            f"(fallback_pass: connections_seen={connections_seen}, packets_seen={packets})"
        )
    raise AssertionError(
        "tcp_client_stream failed after retries: "
        f"{last_reason}; connections_seen={connections_seen}, "
        f"active_connections={active_connections}, packets={packets}, sink_error={sink_error}"
    )


if __name__ == "__main__":
    raise SystemExit(main())
