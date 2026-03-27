Generate a new ST-LIB example for: $ARGUMENTS

Read `Core/Src/Examples/ExampleADC.cpp` first to understand the exact pattern, then create `Core/Src/Examples/Example$ARGUMENTS.cpp`.

Follow these rules precisely:

1. Wrap the entire file in `#ifdef EXAMPLE_[NAME]` / `#endif // EXAMPLE_[NAME]`
2. Includes: `main.h`, `ST-LIB.hpp`. Use `using namespace ST_LIB`.
3. Use an anonymous `namespace { }` for all constants, configs, and helper functions
4. Declare hardware configs as `constexpr` at namespace scope — never inside `main()`
5. Output variables (ADC values, sensor readings) must be `constinit float` (or matching type), also at namespace scope
6. Create at least two test variants:
   - `#ifdef TEST_0` — minimal/single-input variant
   - `#ifdef TEST_1` — extended/multi-input variant
   - Each variant has its own `int main(void)`
7. Board type defined inside each variant: `using Example[Name]Board = ST_LIB::Board<cfg1, cfg2, ...>`
8. Init sequence: `Example[Name]Board::init()` → `Example[Name]Board::instance_of<cfg>()` to get runtime instance
9. Add a `start_terminal()` helper if UART output is needed (UART3, 115200 8N1 via ST-LINK VCP)
10. Add a `print_banner()` helper that prints the test name, wiring hints, and column headers

After creating the file, verify it compiles:
```
./hyper build [name_lowercase] --test 0 --preset simulator
./hyper build [name_lowercase] --test 1 --preset simulator
```
