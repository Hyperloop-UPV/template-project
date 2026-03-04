#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import math
import random
import socket
import struct
import sys
import time
from pathlib import Path


TYPE_LAYOUT = {
    "bool": ("?", 1),
    "uint8": ("B", 1),
    "uint16": ("H", 2),
    "uint32": ("I", 4),
    "uint64": ("Q", 8),
    "int8": ("b", 1),
    "int16": ("h", 2),
    "int32": ("i", 4),
    "int64": ("q", 8),
    "float32": ("f", 4),
    "float64": ("d", 8),
    "enum": ("B", 1),
}


def repo_root() -> Path:
    return Path(__file__).resolve().parents[1]


class Schema:
    def __init__(self, board_name: str):
        json_root = repo_root() / "Core/Inc/Code_generation/JSON_ADE"
        boards = json.loads((json_root / "boards.json").read_text())
        if board_name not in boards:
            raise KeyError(f"Board {board_name} not found in boards.json")

        board_path = json_root / boards[board_name]
        self.board = json.loads(board_path.read_text())
        self.board_dir = board_path.parent
        self.board_name = board_name
        self.board_ip = self.board["board_ip"]

        self.measurements: dict[str, dict] = {}
        for measurement_file in self.board["measurements"]:
            for measurement in json.loads((self.board_dir / measurement_file).read_text()):
                self.measurements[measurement["id"]] = measurement

        self.orders: dict[str, dict] = {}
        self.packets: dict[str, dict] = {}
        self.packet_by_id: dict[int, dict] = {}
        self.order_by_id: dict[int, dict] = {}

        for definition_file in self.board["packets"]:
            for definition in json.loads((self.board_dir / definition_file).read_text()):
                table = self.orders if definition["type"] == "order" else self.packets
                by_id = self.order_by_id if definition["type"] == "order" else self.packet_by_id
                table[definition["name"]] = definition
                by_id[definition["id"]] = definition

        sockets = json.loads((self.board_dir / "sockets.json").read_text())
        server_sockets = [entry for entry in sockets if entry["type"] == "ServerSocket"]
        client_sockets = [entry for entry in sockets if entry["type"] == "Socket"]
        datagram_sockets = [entry for entry in sockets if entry["type"] == "DatagramSocket"]
        if not server_sockets:
            raise ValueError("No ServerSocket defined for TEST board")
        self.tcp_server_port = int(server_sockets[0]["port"])
        if not client_sockets:
            raise ValueError("No Socket defined for TEST board")
        if not datagram_sockets:
            raise ValueError("No DatagramSocket defined for TEST board")
        self.tcp_client_port = int(client_sockets[0]["remote_port"])
        self.udp_port = int(datagram_sockets[0]["port"])

    def field_layout(self, field_name: str) -> tuple[str, int]:
        measurement = self.measurements[field_name]
        return TYPE_LAYOUT[measurement["type"]]

    def packet_size(self, definition: dict) -> int:
        total = 2
        for field_name in definition.get("variables", []):
            total += self.field_layout(field_name)[1]
        return total

    def encode(self, definition: dict, values: dict[str, object]) -> bytes:
        payload = bytearray(struct.pack("<H", definition["id"]))
        for field_name in definition.get("variables", []):
            fmt, _ = self.field_layout(field_name)
            payload.extend(struct.pack("<" + fmt, values[field_name]))
        return bytes(payload)

    def decode_packet(self, raw: bytes) -> tuple[dict, dict[str, object]]:
        if len(raw) < 2:
            raise ValueError("Frame is too short")
        packet_id = struct.unpack_from("<H", raw, 0)[0]
        if packet_id not in self.packet_by_id:
            raise KeyError(f"Unknown packet id 0x{packet_id:04x}")
        definition = self.packet_by_id[packet_id]
        expected_size = self.packet_size(definition)
        if len(raw) != expected_size:
            raise ValueError(
                f"Packet {definition['name']} length mismatch: got {len(raw)}, expected {expected_size}"
            )

        offset = 2
        decoded: dict[str, object] = {}
        for field_name in definition.get("variables", []):
            fmt, size = self.field_layout(field_name)
            decoded[field_name] = struct.unpack_from("<" + fmt, raw, offset)[0]
            offset += size
        return definition, decoded


def send_chunked(
    conn: socket.socket,
    payload: bytes,
    rng: random.Random,
    max_chunk: int,
    max_gap_ms: int,
) -> None:
    offset = 0
    while offset < len(payload):
        chunk = min(rng.randint(1, max_chunk), len(payload) - offset)
        conn.sendall(payload[offset : offset + chunk])
        offset += chunk
        if offset < len(payload) and max_gap_ms > 0:
            time.sleep(rng.uniform(0.0, max_gap_ms / 1000.0))


def values_match(schema: Schema, expected: dict[str, object], actual: dict[str, object]) -> bool:
    for field_name, expected_value in expected.items():
        if field_name not in actual:
            return False
        measurement_type = schema.measurements[field_name]["type"]
        actual_value = actual[field_name]
        if measurement_type in ("float32", "float64"):
            if not math.isclose(
                float(actual_value),
                float(expected_value),
                rel_tol=1e-6,
                abs_tol=1e-5,
            ):
                return False
        else:
            if actual_value != expected_value:
                return False
    return True


def recv_matching_packet(
    udp_sock: socket.socket,
    schema: Schema,
    packet_name: str,
    predicate,
    timeout_s: float,
) -> tuple[dict, dict[str, object]]:
    deadline = time.monotonic() + timeout_s
    last_error: str | None = None
    while time.monotonic() < deadline:
        udp_sock.settimeout(max(0.1, deadline - time.monotonic()))
        try:
            raw, _ = udp_sock.recvfrom(2048)
        except socket.timeout:
            continue

        try:
            definition, decoded = schema.decode_packet(raw)
        except Exception as exc:  # noqa: BLE001
            last_error = str(exc)
            continue

        if definition["name"] != packet_name:
            continue
        if predicate(decoded):
            return definition, decoded
        last_error = f"{packet_name} received but predicate rejected payload {decoded}"

    if last_error is None:
        raise TimeoutError(f"Timed out waiting for {packet_name}")
    raise TimeoutError(f"Timed out waiting for {packet_name}: {last_error}")


def wait_for_accept(listener: socket.socket, timeout_s: float) -> socket.socket:
    listener.settimeout(timeout_s)
    conn, _ = listener.accept()
    conn.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
    return conn


def connect_with_retry(host: str, port: int, timeout_s: float, local_bind: str | None = None) -> socket.socket:
    deadline = time.monotonic() + timeout_s
    last_error: Exception | None = None
    while time.monotonic() < deadline:
        conn: socket.socket | None = None
        try:
            conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            conn.settimeout(1.0)
            if local_bind and local_bind != "0.0.0.0":
                conn.bind((local_bind, 0))
            conn.connect((host, port))
            conn.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
            return conn
        except OSError as exc:
            if conn is not None:
                conn.close()
            last_error = exc
            time.sleep(0.25)
    raise TimeoutError(f"Timed out connecting to {host}:{port}: {last_error}")


def random_small_profile(rng: random.Random) -> dict[str, object]:
    return {
        "enable_flag": bool(rng.getrandbits(1)),
        "small_counter": rng.randint(0, 255),
        "offset_value": rng.randint(-32000, 32000),
        "order_mode": rng.randint(0, 3),
    }


def random_large_profile(rng: random.Random) -> dict[str, object]:
    return {
        "window_size": rng.randint(0, 65535),
        "magic_value": rng.randint(0, 0xFFFFFFFF),
        "position_value": rng.randint(-1_000_000_000, 1_000_000_000),
        "ratio_value": round(rng.uniform(-500.0, 500.0), 3),
        "precise_value": round(rng.uniform(-10_000.0, 10_000.0), 6),
    }


def random_extremes(rng: random.Random) -> dict[str, object]:
    return {
        "trim_value": rng.randint(-128, 127),
        "energy_value": rng.randint(-(2**50), 2**50),
        "big_counter": rng.randint(0, 2**56),
    }


def random_probe(rng: random.Random, seq: int) -> dict[str, object]:
    return {
        "probe_seq": seq,
        "probe_toggle": bool(rng.getrandbits(1)),
        "probe_window": rng.randint(0, 65535),
        "probe_ratio": round(rng.uniform(-200.0, 200.0), 3),
        "probe_mode": rng.randint(0, 3),
    }


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Exercise ExamplePackets over TCP and UDP and verify packet/order parsing."
    )
    parser.add_argument("--board-name", default="TEST")
    parser.add_argument("--board-ip", default=None)
    parser.add_argument("--tcp-port", type=int, default=None)
    parser.add_argument("--udp-port", type=int, default=None)
    parser.add_argument("--client-port", type=int, default=None)
    parser.add_argument("--host-bind", default="0.0.0.0")
    parser.add_argument("--iterations", type=int, default=25)
    parser.add_argument("--timeout", type=float, default=8.0)
    parser.add_argument("--seed", type=int, default=12345)
    parser.add_argument("--chunk-max", type=int, default=7)
    parser.add_argument("--chunk-gap-ms", type=int, default=4)
    parser.add_argument("--connect-settle", type=float, default=0.5)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    schema = Schema(args.board_name)
    board_ip = args.board_ip or schema.board_ip
    tcp_port = args.tcp_port or schema.tcp_server_port
    udp_port = args.udp_port or schema.udp_port
    client_port = args.client_port or schema.tcp_client_port
    rng = random.Random(args.seed)

    udp_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    udp_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    udp_sock.bind((args.host_bind, udp_port))

    listener = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    listener.bind((args.host_bind, client_port))
    listener.listen(1)

    client_conn: socket.socket | None = None
    server_conn: socket.socket | None = None

    try:
        print(
            f"[INFO] Waiting for board TCP client on {args.host_bind}:{client_port} "
            f"and UDP on {args.host_bind}:{udp_port}"
        )
        client_conn = wait_for_accept(listener, args.timeout)
        print("[INFO] Board-initiated Socket connected")

        server_conn = connect_with_retry(board_ip, tcp_port, args.timeout, local_bind=args.host_bind)
        print(f"[INFO] Connected to board ServerSocket at {board_ip}:{tcp_port}")

        if args.connect_settle > 0:
            time.sleep(args.connect_settle)

        _, heartbeat = recv_matching_packet(
            udp_sock,
            schema,
            "heartbeat_snapshot",
            lambda _: True,
            args.timeout,
        )
        expected_order_count = int(heartbeat["tcp_order_count"])
        expected_udp_count = int(heartbeat["udp_parse_count"])
        current_state = int(heartbeat["mirror_state"])
        print(
            "[INFO] Baseline heartbeat "
            f"orders={expected_order_count} udp={expected_udp_count} state={current_state}"
        )

        for iteration in range(args.iterations):
            server_small = random_small_profile(rng)
            client_small = random_small_profile(rng)
            large_profile = random_large_profile(rng)
            extremes = random_extremes(rng)
            probe = random_probe(rng, expected_udp_count + iteration + 1)

            send_chunked(
                server_conn,
                schema.encode(schema.orders["set_small_profile"], server_small),
                rng,
                args.chunk_max,
                args.chunk_gap_ms,
            )
            expected_order_count += 1
            recv_matching_packet(
                udp_sock,
                schema,
                "order_mirror",
                lambda payload: values_match(
                    schema,
                    {
                        "tcp_order_count": expected_order_count,
                        "last_order_code": schema.orders["set_small_profile"]["id"],
                        "enable_flag": server_small["enable_flag"],
                        "small_counter": server_small["small_counter"],
                        "offset_value": server_small["offset_value"],
                        "mirror_mode": server_small["order_mode"],
                    },
                    payload,
                ),
                args.timeout,
            )

            server_burst = (
                schema.encode(schema.orders["set_large_profile"], large_profile)
                + schema.encode(schema.orders["set_extremes"], extremes)
            )
            send_chunked(
                server_conn,
                server_burst,
                rng,
                args.chunk_max,
                args.chunk_gap_ms,
            )
            expected_order_count += 2
            recv_matching_packet(
                udp_sock,
                schema,
                "numeric_mirror",
                lambda payload: values_match(schema, large_profile, payload),
                args.timeout,
            )
            recv_matching_packet(
                udp_sock,
                schema,
                "extremes_mirror",
                lambda payload: values_match(
                    schema,
                    {
                        "trim_value": extremes["trim_value"],
                        "energy_value": extremes["energy_value"],
                        "big_counter": extremes["big_counter"],
                        "mirror_state": current_state,
                    },
                    payload,
                ),
                args.timeout,
            )

            send_chunked(
                client_conn,
                schema.encode(schema.orders["set_small_profile"], client_small),
                rng,
                args.chunk_max,
                args.chunk_gap_ms,
            )
            expected_order_count += 1
            recv_matching_packet(
                udp_sock,
                schema,
                "order_mirror",
                lambda payload: values_match(
                    schema,
                    {
                        "tcp_order_count": expected_order_count,
                        "last_order_code": schema.orders["set_small_profile"]["id"],
                        "enable_flag": client_small["enable_flag"],
                        "small_counter": client_small["small_counter"],
                        "offset_value": client_small["offset_value"],
                        "mirror_mode": client_small["order_mode"],
                    },
                    payload,
                ),
                args.timeout,
            )

            if iteration % 2 == 0:
                next_state = rng.randint(0, 3)
                send_chunked(
                    client_conn,
                    schema.encode(
                        schema.orders["set_state_code"],
                        {"order_state": next_state},
                    ),
                    rng,
                    args.chunk_max,
                    args.chunk_gap_ms,
                )
                current_state = next_state
            else:
                send_chunked(
                    client_conn,
                    schema.encode(schema.orders["bump_state"], {}),
                    rng,
                    args.chunk_max,
                    args.chunk_gap_ms,
                )
                current_state = (current_state + 1) % 4
            expected_order_count += 1
            recv_matching_packet(
                udp_sock,
                schema,
                "heartbeat_snapshot",
                lambda payload: values_match(
                    schema,
                    {
                        "tcp_order_count": expected_order_count,
                        "mirror_state": current_state,
                    },
                    payload,
                ),
                args.timeout,
            )

            udp_sock.sendto(schema.encode(schema.packets["udp_probe"], probe), (board_ip, udp_port))
            expected_udp_count += 1
            recv_matching_packet(
                udp_sock,
                schema,
                "udp_probe_echo",
                lambda payload: values_match(
                    schema,
                    {
                        "udp_parse_count": expected_udp_count,
                        "probe_seq": probe["probe_seq"],
                        "probe_toggle": probe["probe_toggle"],
                        "probe_window": probe["probe_window"],
                        "probe_ratio": probe["probe_ratio"],
                        "probe_mode": probe["probe_mode"],
                    },
                    payload,
                ),
                args.timeout,
            )

            print(
                f"[PASS] iteration={iteration + 1}/{args.iterations} "
                f"orders={expected_order_count} udp={expected_udp_count}"
            )

        print(
            "[PASS] ExamplePackets parsing verified "
            f"across {args.iterations} iterations (TCP server, TCP client, UDP packet path)."
        )
        return 0
    except Exception as exc:  # noqa: BLE001
        print(f"[FAIL] {exc}", file=sys.stderr)
        return 1
    finally:
        if client_conn is not None:
            client_conn.close()
        if server_conn is not None:
            server_conn.close()
        listener.close()
        udp_sock.close()


if __name__ == "__main__":
    raise SystemExit(main())
