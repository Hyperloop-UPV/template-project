Run the hard fault analysis tool and interpret the output in plain language.

1. Run the tool:
   ```bash
   ./hyper hardfault-analysis
   ```
   Common failures:
   - `"Stop debugging"` → you must disconnect the active debug session in your IDE first
   - `STM32_Programmer_CLI not found` → install STM32CubeCLT and ensure it's in PATH
   - ELF not found → the board must be built first; `out/build/latest.elf` is copied automatically on each board build

2. Parse and explain the output:

   **Fault type** (from CFSR register):
   - MMFSR bits → Memory Management Fault (MPU violation, invalid memory access)
   - BFSR bits → Bus Fault (invalid bus transaction, imprecise/precise)
   - UFSR bits → Usage Fault (undefined instruction, unaligned access, divide-by-zero)
   - HFSR → Hard Fault escalation (forced from lower priority fault)

   **Fault address**:
   - If MMARVALID=1 → MMFAR is valid, shows exact faulting address
   - If BFARVALID=1 → BFAR is valid, shows exact bus fault address
   - If IMPRECISERR=1 → address is unreliable (out-of-order execution); to narrow down: rebuild with `-fno-inline` or add `__DSB()` barriers around suspect code

   **PC at fault**: the instruction address that triggered the fault — cross-reference with source using `addr2line -e out/build/latest.elf -f [PC_VALUE]`

   **Call stack**: the sequence of functions leading to the crash — look up each address with addr2line

3. Suggest the most likely root cause based on the fault type and addresses. Common causes in embedded firmware:
   - NULL pointer dereference → MMFSR DACCVIOL
   - Stack overflow → MMFSR MSTKERR or DACCVIOL near stack bottom
   - Unaligned access of packed struct → UFSR UNALIGNED
   - Return to corrupted LR → BFSR INVSTATE or INVPC
   - DMA writing to wrong memory region → MMFSR DACCVIOL
