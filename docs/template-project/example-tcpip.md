# ExampleTCPIP

`ExampleTCPIP` is a hardware stress example for:

- `ServerSocket` (TCP server)
- `Socket` (TCP client from board to host)
- `DatagramSocket` (UDP)
- `Server` connection manager

It includes command/control, payload integrity checks (checksum), burst/saturation traffic, forced disconnect/reconnect and UDP probe/ack.

Control command IDs used by the host script:

- `CMD_RESET=1`
- `CMD_PING=2`
- `CMD_GET_STATS=3`
- `CMD_FORCE_DISCONNECT=4`
- `CMD_BURST_SERVER=5`
- `CMD_BURST_CLIENT=6`
- `CMD_FORCE_CLIENT_RECONNECT=7`
- `CMD_GET_HEALTH=8` (paged telemetry)
- `CMD_RESET_HEALTH=9`

## 1. Build

Build with Ethernet enabled and `EXAMPLE_TCPIP` defined.

Example (board + KSZ8041):

```sh
cmake --preset board-debug-eth-ksz8041 \
  -DBUILD_EXAMPLES=ON \
  -DCMAKE_CXX_FLAGS='-DEXAMPLE_TCPIP -DTCPIP_TEST_HOST_IP=192.168.1.9'
cmake --build --preset board-debug-eth-ksz8041
```

Example (nucleo + LAN8742):

```sh
cmake --preset nucleo-debug-eth \
  -DBUILD_EXAMPLES=ON \
  -DCMAKE_CXX_FLAGS='-DEXAMPLE_TCPIP -DTCPIP_TEST_HOST_IP=192.168.1.9'
cmake --build --preset nucleo-debug-eth
```

Notes:

- `TCPIP_TEST_HOST_IP` must be the IPv4 of your laptop on the same Ethernet segment.
- Defaults (if not overridden in compile flags):
  - `TCPIP_TEST_BOARD_IP="192.168.1.7"`
  - TCP server port: `40000`
  - TCP client local/remote ports: `40001` / `40002`
  - UDP local/remote ports: `40003` / `40004`

## 2. Flash and run

Flash as usual (`out/build/latest.elf`) and power the board.

One-shot automation (build + flash + ping + tests):

```sh
./tools/run_example_tcpip_nucleo.sh \
  --iface en6 \
  --board-ip 192.168.1.7 \
  --base-runs 1 \
  --aggr-runs 0
```

## 3. Run stress tests from laptop

```sh
./tools/run_example_tcpip_stress.sh --board-ip 192.168.1.7 --host-bind 192.168.1.9
```

Useful options:

- `--host-bind 0.0.0.0`
- `--tcp-server-port 40000`
- `--tcp-client-port 40002`
- `--udp-local-port 40003`
- `--udp-remote-port 40004`
- `--good-payloads 1200`
- `--bad-payloads 200`
- `--min-payload-rx-ratio 0.90`
- `--min-bad-detect-ratio 0.80`
- `--payload-interval-us 800` (set `0` for max blast / likely RX overrun testing)
- `--server-burst 800`
- `--client-burst 800`
- `--min-server-burst-ratio 0.95`
- `--min-client-burst-ratio 0.95`
- `--udp-count 300`
- `--strict-client-stream` (make `tcp_client_stream` a hard fail instead of warning)
- `--health-pages 6`
- `--reset-health`
- `--health-at-end`
- `--no-health-on-fail`

## 4. What the script validates

- TCP command/response path (`PING`)
- TCP payload integrity under load (good + bad checksum packets)
- Forced disconnect and reconnect
- TCP server burst stream reception
- UDP probe/ack response ratio and board-reported counters
- Board TCP client stream reception on host-side sink

## 5. Telemetry Pages (`CMD_GET_HEALTH`)

`CMD_GET_HEALTH` returns three values per page:

- `page 0`: `uptime_ms`, `loop_iterations`, `tcp_commands_rx`
- `page 1`: `tcp_payload_rx`, `tcp_payload_bad`, `tcp_responses_tx`
- `page 2`: `tcp_client_tx_ok`, `tcp_client_tx_fail`, `tcp_client_send_fail_streak_max`
- `page 3`: `tcp_server_recreate_count`, `tcp_client_recreate_count`, `tcp_client_reconnect_calls`
- `page 4`: `reason_last`, `reason_arg_last`, `reason_update_count`
- `page 5`: `server_burst_requested_max`, `client_burst_requested_max`, `tcp_client_not_connected_ticks`

Current reason codes:

- `0`: none/reset
- `1`: boot
- `2`: force-disconnect command
- `3`: server recreate
- `4`: client recreate by command
- `5`: client recreate watchdog
- `6`: client reconnect poll
- `7`: client send fail streak started

## 6. Quality Gate

Strict matrix (base + aggressive):

```sh
./tools/example_tcpip_quality_gate.sh \
  --board-ip 192.168.1.7 \
  --host-bind 192.168.1.9 \
  --base-runs 20 \
  --aggr-runs 5 \
  --health-at-end
```

This script stores all logs in `out/quality-gate/<timestamp>/`.
Useful flags:

- `--health-pages 6`
- `--health-at-end`
- `--stop-on-first-fail`

## 7. Soak Test

Long-running soak with automatic pass/fail summary:

```sh
./tools/example_tcpip_soak.sh \
  --board-ip 192.168.1.7 \
  --host-bind 192.168.1.9 \
  --duration-min 480 \
  --strict-client-stream \
  --health-pages 6 \
  --max-failures 1
```

This script stores all logs in `out/soak/<timestamp>/`.

## 8. Multi-Hour / Overnight Soak

Use the long-run wrapper to execute for hours and get final pass ratio + fail breakdown:

```sh
./tools/example_tcpip_soak_hours.sh \
  --board-ip 192.168.1.7 \
  --host-bind 192.168.1.9 \
  --hours 8 \
  --min-pass-ratio 0.90 \
  --baseline-pass-ratio 0.8475
```

Outputs:

- Run console/session log: `out/soak-hours/<timestamp>.log`
- Per-run logs from soak engine: `out/soak/<timestamp>/`

To leave it running in background:

```sh
nohup ./tools/example_tcpip_soak_hours.sh --board-ip 192.168.1.7 --host-bind 192.168.1.9 --hours 8 > out/soak-hours/latest.nohup.log 2>&1 &
```
