# template-project

Firmware template for HyperloopUPV pod control boards. Target: STM32H723ZGT6 (ARM Cortex-M7 @ 550 MHz).

## Architecture

- **ST-LIB** (`deps/ST-LIB`): hardware abstraction library (submodule). All peripheral access goes through it. Never use STM32 HAL directly.
- **`Board<>`** template: compile-time peripheral registration. All peripherals declared as `constexpr` globals and passed as template parameters.
- **Packet system**: inter-board communication via UDP/TCP. Headers generated from JSON schemas in `Core/Inc/Code_generation/JSON_ADE/`.
- **Examples** (`Core/Src/Examples/`): self-contained programs enabled via `-DEXAMPLE_[NAME]=ON`. Each has `TEST_0`, `TEST_1`... variants with separate `main()`.

## Build system

CMake + Ninja. Python venv at `virtual/`. Use `./hyper` CLI for all common tasks.

```bash
./hyper build main --preset simulator              # build simulator
./hyper build main --preset nucleo-debug           # build for Nucleo board
./hyper build adc --test 0 --preset nucleo-debug   # build ExampleADC TEST_0
./hyper run adc --test 0                           # flash + open UART
./hyper stlib build --preset simulator --run-tests # run CTest suite
./hyper doctor                                     # check tool dependencies
```

## Presets

| Preset | Hardware | Ethernet | Use when |
|--------|----------|----------|----------|
| `simulator` | Host PC | No | Tests, CI, fast iteration |
| `simulator-asan` | Host PC | No | Memory/UB sanitizer runs |
| `nucleo-debug` | Nucleo H7 | No | Debugging on Nucleo |
| `nucleo-release` | Nucleo H7 | No | Production on Nucleo |
| `nucleo-debug-eth` | Nucleo H7 | Yes | Ethernet debugging on Nucleo |
| `nucleo-relwithdebinfo-eth` | Nucleo H7 | Yes | Ethernet profiling on Nucleo |
| `board-debug` | Custom PCB | No | Debugging on custom board |
| `board-debug-eth-ksz8041` | Custom PCB | KSZ8041 | Ethernet with KSZ8041 PHY |
| `board-debug-eth-lan8700` | Custom PCB | LAN8700 | Ethernet with LAN8700 PHY |
| `board-release-*` | Custom PCB | Various | Production builds |

`nucleo` presets set `TARGET_NUCLEO=ON`, adjusting pin/LED mappings for the Nucleo dev board. `board` presets target the custom HyperloopUPV PCB. `relwithdebinfo` keeps debug symbols with optimizations for profiling.

## Code conventions

- C++23, **no exceptions** (`-fno-exceptions`), **no RTTI** (`-fno-rtti`)
- All hardware configs must be `constexpr` — declared at global/namespace scope, not inside functions
- Output variables (ADC values, sensor readings) must be `constinit float` or matching type
- `using namespace ST_LIB` is standard in firmware files
- Formatting: clang-format v17 (enforced by pre-commit hook)
- Python scripts: Ruff formatter/linter (enforced by pre-commit hook)

## Packet code generation

Generated from JSON schemas in `Core/Inc/Code_generation/JSON_ADE/boards/[BOARD]/`.

| File | Purpose |
|------|---------|
| `[BOARD].json` | Board ID, IP address, references to other files |
| `[BOARD]_measurements.json` | Variable definitions with types |
| `packets.json` | Data packets (outgoing telemetry) |
| `orders.json` | Order packets (incoming commands) |
| `sockets.json` | Socket definitions (ServerSocket, Socket, DatagramSocket) |

Measurement types: `bool`, `uint8`, `uint16`, `uint32`, `uint64`, `int8`, `int16`, `int32`, `int64`, `float`, `double`.

Generated output (gitignored, rebuilt on configure):
- `Core/Inc/Communications/Packets/DataPackets.hpp`
- `Core/Inc/Communications/Packets/OrderPackets.hpp`

Manual regeneration:
```bash
python3 Core/Inc/Code_generation/Generator.py TEST
```

CMake runs this automatically at configure time using the `BOARD_NAME` variable (default: `TEST`). Available boards: `Core/Inc/Code_generation/JSON_ADE/boards.json`.

## Examples pattern

Each example in `Core/Src/Examples/Example[Name].cpp`:
- Entire file wrapped in `#ifdef EXAMPLE_[NAME]` / `#endif`
- Multiple test variants: `#ifdef TEST_0` ... `#ifdef TEST_1` etc., each with its own `int main()`
- All configs declared as `constexpr` in anonymous namespace
- Board type: `using ExampleXBoard = ST_LIB::Board<cfg1, cfg2, ...>`
- Init sequence: `ExampleXBoard::init()` → `ExampleXBoard::instance_of<cfg>()`

Build and run an example:
```bash
./hyper build adc --test 0 --preset simulator       # compile only
./hyper build adc --test 1 --preset nucleo-debug    # for hardware
./hyper run adc --test 1 --uart                     # flash + UART monitor
```

## Testing

Tests live in `deps/ST-LIB/Tests/`. Require `simulator` preset (no hardware needed).

```bash
cmake --preset simulator
cmake --build --preset simulator
ctest --preset simulator-all          # run all tests
ctest --preset simulator-adc          # ADC tests only
ctest --preset simulator-all-asan     # with AddressSanitizer + UBSan
```

## Hard fault analysis

When the board crashes, fault data is saved to flash at `0x080C0000`. Analyze with:
```bash
./hyper hardfault-analysis
```
Requirements: STM32_Programmer_CLI installed, SWD debugger connected, **active debug session stopped**. ELF must be at `out/build/latest.elf` (copied there automatically on each board build).

## Reference documents

Downloaded via `./hyper doc`. Stored locally at `manuals/` (gitignored).

| Document | ID | URL |
|----------|----|-----|
| STM32H723 Reference Manual | RM0468 | https://www.st.com/resource/en/reference_manual/rm0468-stm32h723733-stm32h725735-and-stm32h730-value-line-advanced-armbased-32bit-mcus-stmicroelectronics.pdf |
| STM32H723ZG Datasheet | DS13313 | https://www.st.com/resource/en/datasheet/stm32h723zg.pdf |
| STM32H7 Programming Manual | PM0253 | https://www.st.com/resource/en/programming_manual/pm0253-stm32f7-series-and-stm32h7-series-cortexm7-processor-programming-manual-stmicroelectronics.pdf |
| STM32H7 HAL/LL User Manual | UM2217 | https://www.st.com/resource/en/user_manual/um2217-description-of-stm32h7-hal-and-lowlayer-drivers-stmicroelectronics.pdf |
| Nucleo-H723ZG Board Manual | UM2407 | https://www.st.com/resource/en/user_manual/um2407-stm32h7-nucleo144-boards-mb1364-stmicroelectronics.pdf |
| Cortex-M7 TRM | DDI0489F | https://developer.arm.com/documentation/ddi0489/f/DDI0489F_cortex_m7_trm.pdf |
| Cortex-M7 Generic User Guide | DUI0646C | https://developer.arm.com/documentation/dui0646/c/DUI0646C_cortex_m7_dgug.pdf |
| KSZ8041 PHY Datasheet | DS00002245B | https://ww1.microchip.com/downloads/en/DeviceDoc/00002245B.pdf |
| LAN8700 PHY Datasheet | DS00001927A | https://ww1.microchip.com/downloads/en/DeviceDoc/00001927A.pdf |
| LAN8742A PHY Datasheet | — | https://ww1.microchip.com/downloads/en/DeviceDoc/8742a.pdf |
| LTC6810-1/2 Battery Cell Monitor | — | https://www.analog.com/media/en/technical-documentation/data-sheets/LTC6810-1-6810-2.pdf |
| LTC6820 isoSPI Interface | — | https://www.analog.com/media/en/technical-documentation/data-sheets/ltc6820.pdf |
| Bender IR155-32xx Isolation Monitor | D00376 | https://www.bender.de/fileadmin/content/Products/d/e/IR155-3210-V004_D00376_D_XXEN.pdf |

## Key environment variables

| Variable | Purpose |
|----------|---------|
| `HYPER_DEFAULT_PRESET` | Skip `--preset` flag on every command |
| `HYPER_FLASH_METHOD` | `stlink` / `jlink` / `dfu` (auto-detected otherwise) |
| `HYPER_UART_PORT` | Serial port path (auto-detected otherwise) |
| `HYPER_UART_BAUD` | Baud rate (default: 115200) |
