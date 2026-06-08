# Template Project

Code prepared for sensor characterization. This code corresponds to [this ADJ](https://github.com/HyperloopUPV-H8/adj/tree/linear-sensor-characterization)

To use it, specify the pin to which the ADC you want to characterize is connected by setting the corresponding define at the beginning of the main code. Then compile the code and flash the board.

Connect to the control station using the indicated ADJ.

You have a command available where you can input the real value currently being measured by the ADC. Once you’ve entered at least two measurements, you’ll start seeing the slope and offset being updated with the corresponding characterization values. At the same time, you’ll be able to see the sensor reading adjusted according to the new characterization.

Once you’ve added as many measurements as you want, you can copy the final slope and offset. It’s best to take them directly from the logs, as they include more decimal places and are therefore more precise.
HyperloopUPV STM32 firmware template based on CMake + VSCode, using `deps/ST-LIB`.

## Quickstart

```sh
./hyper init
./hyper doctor
./hyper build main --preset simulator
./hyper stlib build --preset simulator --run-tests
```

## Hyper CLI

The repo includes a local helper CLI at `./hyper` for the common hardware flow:

```sh
./hyper examples list
./hyper build adc --test 1
./hyper run adc --test 1 --uart
./hyper uart
./hyper doctor
```

It wraps the existing repo scripts instead of replacing them, and also exposes a small ST-LIB namespace:

```sh
./hyper stlib build --preset simulator --run-tests
./hyper stlib sim-tests
```

Useful defaults can be pinned with environment variables:

- `HYPER_DEFAULT_PRESET`
- `HYPER_FLASH_METHOD`
- `HYPER_UART_PORT`
- `HYPER_UART_BAUD`
- `HYPER_UART_TOOL`

> [!NOTE]
To connect through UART (`./hyper uart`), it's recommended to install `tio` with your package manager.

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

```sh
./hyper build main --preset simulator
./hyper build main --preset nucleo-debug
./hyper build main --preset board-debug
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
./hyper build main --preset board-debug --board-name TEST
```

Generated packet headers such as `Core/Inc/Communications/Packets/DataPackets.hpp` and `Core/Inc/Communications/Packets/OrderPackets.hpp` are build outputs derived from the active `JSON_ADE` schema. They are intentionally gitignored and should not be edited or committed.
