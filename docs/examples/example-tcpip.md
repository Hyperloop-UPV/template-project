# ExampleTCPIP

## Purpose

`ExampleTCPIP` is the full socket stress and robustness example.

It validates:

- `Server` / `ServerSocket` TCP server handling
- `Socket` TCP client handling
- `DatagramSocket` UDP handling
- request/response control traffic
- payload integrity under load
- forced disconnect and reconnect behavior
- burst streaming in both directions
- health telemetry collection during stress

## Build

Nucleo Ethernet build:

```sh
./tools/build-example.sh --example tcpip --preset nucleo-debug-eth --extra-cxx-flags "-DTCPIP_TEST_HOST_IP=192.168.1.9"
```

Equivalent macro selection:

- `EXAMPLE_TCPIP`
- optional compile-time overrides such as `TCPIP_TEST_HOST_IP`, `TCPIP_TEST_BOARD_IP`, and port macros

Default network values inside the example:

- Board IP: `192.168.1.7`
- TCP server port: `40000`
- TCP client local/remote: `40001` / `40002`
- UDP local/remote: `40003` / `40004`

## Runtime behavior

The example exposes:

- a TCP command/control channel
- a TCP payload ingestion path with checksums
- a board-to-host TCP client stream
- a UDP probe/ack path
- runtime health pages retrievable by command

Main command IDs:

- `CMD_RESET`
- `CMD_PING`
- `CMD_GET_STATS`
- `CMD_FORCE_DISCONNECT`
- `CMD_BURST_SERVER`
- `CMD_BURST_CLIENT`
- `CMD_FORCE_CLIENT_RECONNECT`
- `CMD_GET_HEALTH`
- `CMD_RESET_HEALTH`

## How to validate

Make sure the host-side board-link interface is configured on `192.168.1.9` before running the stress tools.

Base stress run:

```sh
./tools/run_example_tcpip_stress.sh --board-ip 192.168.1.7 --host-bind 192.168.1.9
```

Quality gate:

```sh
./tools/example_tcpip_quality_gate.sh --board-ip 192.168.1.7 --host-bind 192.168.1.9 --base-runs 20 --aggr-runs 5
```

Long soak:

```sh
./tools/example_tcpip_soak.sh --board-ip 192.168.1.7 --host-bind 192.168.1.9 --duration-min 120
```

Multi-hour soak:

```sh
./tools/example_tcpip_soak_hours.sh --board-ip 192.168.1.7 --host-bind 192.168.1.9 --hours 8 --min-pass-ratio 0.90
```

Use `--host-bind 192.168.1.9` when the USB/Ethernet link and the Wi‑Fi are both inside `192.168.1.x`. The stress tooling now binds the outbound control socket to that IP so the direct board link works without stealing the global route from Wi‑Fi.

## What the scripts validate

- control path ping/ack
- payload good/bad checksum accounting
- forced disconnect recovery
- TCP server burst throughput
- TCP client stream delivery to host
- UDP round-trip success ratio
- optional health-page inspection

## Expected result

- Base stress run ends with overall pass.
- Quality gate reaches the configured pass criteria.
- Soak pass ratio stays above the chosen threshold.
- No hard fault, silent packet corruption, or stuck reconnect state.

## Deep references

For the full command list, telemetry pages, and long-run scripts, see:

- [Detailed ExampleTCPIP guide](../template-project/example-tcpip.md)
