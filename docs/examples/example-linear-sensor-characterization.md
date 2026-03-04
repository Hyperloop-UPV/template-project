# LinearSensorCharacterization

## Purpose

`LinearSensorCharacterization` is an application example that performs online linear calibration of one ADC reading against a reference value received through generated packets.

It is intended to validate:

- ADC acquisition on `PA0`
- Ethernet runtime loop
- generated `OrderPackets` reception
- generated `DataPackets` transmission
- incremental linear regression (`slope` and `offset` estimation)

## JSON_ADE dependency

This example depends on a different packet schema than `ExamplePackets`.

It is not compatible with the current `TEST` packet schema active in this branch.

The required `JSON_ADE` branch for this example is:

- `linear-sensor-char`

It expects a legacy characterization-oriented `JSON_ADE` branch or board definition that generates the specific packet/order API used by this source.

That means:

- simply enabling `EXAMPLE_LINEAR_SENSOR_CHARACTERIZATION` is not enough
- the active `Core/Inc/Code_generation/JSON_ADE` contents must match this example
- the selected `BOARD_NAME` must generate the legacy characterization symbols

At the moment, `ExamplePackets` and `LinearSensorCharacterization` are effectively tied to different `JSON_ADE` worlds.

Switching the `JSON_ADE` branch/schema to make this example build can break `ExamplePackets`, and vice versa.

## Current status in this branch

This example is currently not directly buildable with the active `BOARD_NAME=TEST` packet schema.

Reason:

- the source expects generated symbols such as `OrderPackets::characterize_init`
- the current `TEST` JSON schema does not generate those symbols
- building this example with `BOARD_NAME=TEST` fails at compile time

The failure is expected until a matching board schema is restored or the example is migrated to the new `TEST` packet definitions.

## What it needs to build

This example requires a board JSON definition that generates at least:

- `OrderPackets::characterize_init`
- `OrderPackets::characterize_flag`
- `DataPackets::characterization_init`
- `DataPackets::Value_init`
- `DataPackets::packets_socket`
- `DataPackets::characterization_packet`

In practice, this means you need the legacy characterization schema checked out in `JSON_ADE`, then a full reconfigure/rebuild so the generated packet headers are replaced.

Typical sequence once that schema is restored:

```sh
cmake --preset nucleo-debug-eth -DBUILD_EXAMPLES=ON -DCMAKE_CXX_FLAGS='-DEXAMPLE_LINEAR_SENSOR_CHARACTERIZATION'
cmake --build --preset nucleo-debug-eth
```

## Intended runtime behavior

When backed by a matching packet schema, the example should:

- read the raw ADC value from `PA0`
- receive a reference `real_value` through a generated order
- update `slope` and `offset` with incremental OLS
- compute `value = slope * sensor_value + offset`
- publish the updated characterization packet over Ethernet

## Recommended next step

Pick one of these approaches before using this example again:

- restore the original board JSON that defined `characterize`, `characterization`, and `Value`
- or migrate `LinearSensorCharacterization.cpp` to the new `TEST` packet names

## Validation once restored

After the packet schema matches again, validate it by:

- sending known reference values
- checking that `slope` and `offset` converge correctly
- verifying the emitted data packets track the fitted line
