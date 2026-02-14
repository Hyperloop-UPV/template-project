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
