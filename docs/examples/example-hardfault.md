# ExampleHardFault

## Purpose

`ExampleHardFault` is a controlled fault generator.

It exists to validate:

- hard fault capture in flash
- CFSR/MMFAR/BFAR decoding
- call stack extraction
- the offline analyzer in `hard_faullt_analysis.py`

## Available tests

- `memory_fault`: invalid memory access through an oversized buffer index
- `bus_fault`: write to `0xdead0000`
- `usage_fault`: explicit trap (`__builtin_trap()`)

## Build

Usage fault example:

```sh
./tools/build-example.sh --example hardfault --preset nucleo-debug --test usage_fault
```

Bus fault example:

```sh
./tools/build-example.sh --example hardfault --preset nucleo-debug --test bus_fault
```

Memory fault example:

```sh
./tools/build-example.sh --example hardfault --preset nucleo-debug --test memory_fault
```

Equivalent macro selection:

- `EXAMPLE_HARDFAULT`
- one of `TEST_USAGE_FAULT`, `TEST_BUS_FAULT`, `TEST_MEMORY_FAULT`

## Runtime behavior

Each test deliberately faults almost immediately after boot.

This is expected.

The goal is not for the application to keep running. The goal is to verify that the hard-fault logging path records useful diagnostic data.

## How to validate

1. Build and flash one fault mode.
2. Let the board execute and fault.
3. Run the analyzer:

```sh
python3 hard_faullt_analysis.py
```

If the script says you must stop debugging first, disconnect the live debugger and run it again.

## Expected result

- The analyzer should report the corresponding fault category.
- It should print decoded CFSR bits.
- It should usually show a useful PC/call trace.

## What a failure usually means

- Analyzer says there was no hard fault: the example did not actually execute, or the log area was not updated.
- Analyzer cannot read flash: ST-LINK/SWD is not available or the board is still under active debug control.
- Fault type does not match the injected test: fault logging/decoding is wrong, or the test hit a different exception first.
