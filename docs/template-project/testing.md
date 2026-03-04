# Testing and Quality

## 1. Local Simulator Tests

```sh
cmake --preset simulator
cmake --build --preset simulator
ctest --preset simulator-all
```

Run only ADC tests:

```sh
ctest --preset simulator-adc
```

## 2. Tests with Sanitizers

```sh
cmake --preset simulator-asan
cmake --build --preset simulator-asan
ctest --preset simulator-all-asan
```

## 3. Formatting

This repository uses `pre-commit` and `clang-format`.

Install hooks:

```sh
pip install pre-commit
pre-commit install --install-hooks --hook-type pre-commit --hook-type pre-push
```

Run manually:

```sh
pre-commit run --all-files
```

## 4. GitHub Actions CI

- `Compile Checks`: builds MCU matrix (no simulator tests)
- `Run Simulator Tests`: runs tests using `simulator` preset
- `Format Checks`: validates formatting with `pre-commit`

## 5. TCP/IP Hardware Stress Tests

For Ethernet/socket stress testing on real hardware, see:

- [`docs/template-project/example-tcpip.md`](example-tcpip.md)
- Per-example quick guides: [`docs/examples/README.md`](../examples/README.md)
- One-shot Nucleo flow: `./tools/run_example_tcpip_nucleo.sh`
- Run strict matrix gate: `./tools/example_tcpip_quality_gate.sh`
- Run long soak: `./tools/example_tcpip_soak.sh`
- Run multi-hour soak + ratio summary: `./tools/example_tcpip_soak_hours.sh`

## 6. Packet / Order Parser Validation on Hardware

For generated `OrderPackets` / `DataPackets` validation on real hardware, see:

- [`docs/examples/example-packets.md`](../examples/example-packets.md)
- Host-side checker: `./tools/example_packets_check.py --board-ip 192.168.1.7 --host-bind 192.168.1.9`

`Core/Inc/Communications/Packets/DataPackets.hpp` and `Core/Inc/Communications/Packets/OrderPackets.hpp` are generated from the active `JSON_ADE` schema during build. They are intentionally gitignored and should not be committed.
