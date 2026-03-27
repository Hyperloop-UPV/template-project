Recommend the correct CMake preset for: $ARGUMENTS

Read `CMakePresets.json` to confirm available presets, then recommend based on this decision logic:

**No hardware / testing / CI:**
- Default → `simulator`
- With memory safety checks → `simulator-asan` (AddressSanitizer + UBSan)

**STM32H7 Nucleo development board** (not a custom HyperloopUPV PCB):
- No Ethernet, debugging → `nucleo-debug`
- No Ethernet, production deploy → `nucleo-release`
- No Ethernet, profiling (optimized + debug symbols) → `nucleo-relwithdebinfo`
- With Ethernet, debugging → `nucleo-debug-eth`
- With Ethernet, production → `nucleo-release-eth`

**Custom HyperloopUPV PCB:**
- No Ethernet, debugging → `board-debug`
- No Ethernet, production → `board-release`
- Ethernet + KSZ8041 PHY chip, debugging → `board-debug-eth-ksz8041`
- Ethernet + KSZ8041 PHY chip, production → `board-release-eth-ksz8041`
- Ethernet + LAN8700 PHY chip, debugging → `board-debug-eth-lan8700`
- Ethernet + LAN8700 PHY chip, production → `board-release-eth-lan8700`

**Key differences between nucleo and board:**
- `nucleo` presets set `TARGET_NUCLEO=ON` — adjusts LED pin (PB0 on Nucleo vs PF13 on custom board) and some other pin remappings
- `board` presets target the custom HyperloopUPV PCB pinout exactly

**PHY chip identification:**
- KSZ8041 → Microchip/Micrel, used in older HyperloopUPV boards
- LAN8700 → Microchip, used in newer HyperloopUPV boards
- Check the board schematic or ask a hardware engineer if unsure

Provide the exact preset name and the full `./hyper build` command to use it.
