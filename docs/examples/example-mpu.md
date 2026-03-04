# ExampleMPU

## Purpose

`ExampleMPU` is the MPU and static buffer allocation validation suite.

It validates:

- buffer reservation in different memory domains
- cached vs non-cached placement
- alignment and packing constraints
- type safety of `MPUDomain::Buffer<T>`
- selected runtime fault scenarios for invalid access
- compatibility with legacy `MPUManager` allocation

## Build

Baseline build:

```sh
./tools/build-example.sh --example mpu --preset nucleo-debug --test 0
```

Pick any other selector with `--test <id>`.

Examples:

```sh
./tools/build-example.sh --example mpu --preset nucleo-debug --test 11
./tools/build-example.sh --example mpu --preset nucleo-debug --test 12
```

Equivalent macro selection:

- `EXAMPLE_MPU`
- one of `TEST_0` to `TEST_15`

## Test matrix

- `TEST_0`: no buffers requested; baseline boot path.
- `TEST_1`: one non-cached buffer in default domain (D2).
- `TEST_2`: one non-cached buffer in D1.
- `TEST_3`: one non-cached buffer in D3.
- `TEST_4`: intentionally oversized allocation; should fail.
- `TEST_5`: intentionally wrong `as<>` type usage; should fail.
- `TEST_6`: mixes cached and non-cached buffers in the same program.
- `TEST_7`: alignment stress with differently sized element types.
- `TEST_8`: non-POD type request (`std::vector<int>`); should fail.
- `TEST_9`: too many buffers requested; should fail.
- `TEST_10`: POD struct buffer.
- `TEST_11`: mixed sizes, alignments, memory types, and legacy placement macros.
- `TEST_12`: invalid memory dereference at `0x80000000`; runtime fault expected.
- `TEST_13`: `construct<>()` path for in-place object construction.
- `TEST_14`: legacy `MPUManager::allocate_non_cached_memory()` compatibility.
- `TEST_15`: null pointer dereference; runtime fault expected.

## Expected behavior by class of test

Build-and-run tests:

- `0`, `1`, `2`, `3`, `6`, `7`, `10`, `11`, `13`, `14`
- These should build, boot, and stay alive in the idle loop.

Intentional build-time rejection tests:

- `4`, `5`, `8`, `9`
- These are expected to fail to compile or fail during static/resource validation.

Intentional runtime fault tests:

- `12`, `15`
- These should build, then hard fault at runtime.

## How to validate

For successful runtime tests:

- Confirm the board boots and stays running.
- Confirm there is no hard fault.

For expected-failure tests:

- Treat build failure as the pass condition.
- The point is to prove the API rejects invalid usage.

For runtime fault tests:

- Flash the build.
- Let it fault.
- Run:

```sh
python3 hard_faullt_analysis.py
```

## What a failure usually means

- A “should fail” test builds cleanly: a safety check is missing.
- A “should run” test faults: placement or MPU configuration is wrong.
- A runtime fault test does not fault: the invalid access path did not execute as expected.
