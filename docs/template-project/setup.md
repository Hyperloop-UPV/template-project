# Template Project Setup

## 1. Prerequisites

- VSCode (recommended)
- CMake + Ninja
- Python 3

For MCU build/flash/debug:

- `STM32CubeCLT`
- `openocd` or `ST-LINK_gdbserver`
- `arm-none-eabi-gdb`

## 2. Quick Initialization

From the repository root:

```sh
./tools/init.sh
```

This command:

- creates `virtual/`
- installs Python dependencies from `requirements.txt`
- initializes template and `deps/ST-LIB` submodules

On Windows:

```bat
tools\init.bat
```

## 3. `BOARD_NAME` Configuration (codegen)

Code generation uses `BOARD_NAME` (CMake cache variable), and the value must exist in:

- `Core/Inc/Code_generation/JSON_ADE/boards.json`

Example:

```sh
cmake --preset board-debug -DBOARD_NAME=TEST
```

If not set, the default value is `TEST`.
