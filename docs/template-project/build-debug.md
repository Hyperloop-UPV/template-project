# Build and Debug

## 1. Main CMake Presets

- `simulator`
- `simulator-asan`
- `nucleo-*`
- `board-*`

List all presets:

```sh
cmake --list-presets
```

## 2. Build for Simulator

```sh
cmake --preset simulator
cmake --build --preset simulator
```

With sanitizers:

```sh
cmake --preset simulator-asan
cmake --build --preset simulator-asan
```

## 3. Build for MCU

Example:

```sh
cmake --preset board-debug-eth-ksz8041 -DBOARD_NAME=TEST
cmake --build --preset board-debug-eth-ksz8041
```

The build output is copied to:

- `out/build/latest.elf`

## 4. Debug from VSCode

Launch configurations available in `.vscode/launch.json`:

- `MCU | OpenOCD | Build + Debug (RTT)`
- `MCU | OpenOCD | Debug (No Build, RTT)`
- `MCU | OpenOCD | Attach (External Server, RTT)`
- `MCU | ST-LINK | Build + Debug`
- `MCU | ST-LINK | Debug (No Build)`
- `MCU | ST-LINK | Attach (External GDB Server)`
- `SIM | ST-LIB Tests | Debug`

Useful tasks in `.vscode/tasks.json`:

- `MCU | OpenOCD | Start Server`
- `MCU | OpenOCD | RTT Console`
- `MCU | ST-LINK | Start GDB Server`
