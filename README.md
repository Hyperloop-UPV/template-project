# Template Project

HyperloopUPV STM32 firmware template based on CMake + VSCode, using `deps/ST-LIB`.

## Quickstart

```sh
./tools/init.sh
cmake --preset simulator
cmake --build --preset simulator
ctest --preset simulator-all
```

## Documentation

- Template setup: [`docs/template-project/setup.md`](docs/template-project/setup.md)
- Build and debug: [`docs/template-project/build-debug.md`](docs/template-project/build-debug.md)
- Testing and quality: [`docs/template-project/testing.md`](docs/template-project/testing.md)
- Per-example guides: [`docs/examples/README.md`](docs/examples/README.md)
- TCP/IP hardware stress example: [`docs/template-project/example-tcpip.md`](docs/template-project/example-tcpip.md)
- ST-LIB docs (inside this repository): [`deps/ST-LIB/docs/setup.md`](deps/ST-LIB/docs/setup.md)

## Main Working Modes

- `simulator`: fast local development and tests.
- `nucleo-*` / `board-*`: hardware builds.

List all presets:

```sh
cmake --list-presets
```

## VSCode Debug

`launch.json` and `tasks.json` include debug flows for:

- OpenOCD
- ST-LINK
- simulator tests

Detailed guide:

- [`docs/template-project/build-debug.md`](docs/template-project/build-debug.md)

## `BOARD_NAME` (code generation)

Packet code generation uses `BOARD_NAME` (a key from JSON_ADE).

Example:

```sh
cmake --preset board-debug -DBOARD_NAME=TEST
```

Generated packet headers such as `Core/Inc/Communications/Packets/DataPackets.hpp` and `Core/Inc/Communications/Packets/OrderPackets.hpp` are build outputs derived from the active `JSON_ADE` schema. They are intentionally gitignored and should not be edited or committed.
