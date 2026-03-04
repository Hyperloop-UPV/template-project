# Examples

This directory contains one focused document per example under `Core/Src/Examples/`.

Use this folder as the quick reference for:

- what each example is for
- how to build it
- what hardware it needs
- how to validate that it is working
- what a failure usually means

## Common workflow

List available examples:

```sh
./tools/build-example.sh --list
```

Build one example on a Nucleo:

```sh
./tools/build-example.sh --example adc --preset nucleo-debug --test 0
```

Build one Ethernet example on a Nucleo:

```sh
./tools/build-example.sh --example tcpip --preset nucleo-debug-eth --extra-cxx-flags "-DTCPIP_TEST_HOST_IP=192.168.1.9"
```

For Ethernet examples, configure the host-side board-link interface with a static IPv4 on the same subnet as the board (default examples expect `192.168.1.9` on the host and `192.168.1.7` on the board).

Flash the latest build:

```sh
STM32_Programmer_CLI -c port=SWD mode=UR -w out/build/latest.elf -v -rst
```

## Documents

- [ExampleADC](./example-adc.md)
- [ExampleEthernet](./example-ethernet.md)
- [ExampleEXTI](./example-exti.md)
- [ExampleHardFault](./example-hardfault.md)
- [ExampleMPU](./example-mpu.md)
- [ExamplePackets](./example-packets.md)
- [ExampleTCPIP](./example-tcpip.md)
- [LinearSensorCharacterization](./example-linear-sensor-characterization.md)

## JSON_ADE / Packet Schema Compatibility

Not all examples depend on the same generated packet schema.

That matters because the generated `OrderPackets` and `DataPackets` APIs are produced from the active contents of:

- `Core/Inc/Code_generation/JSON_ADE`
- `boards.json`
- the selected `BOARD_NAME`

In practice, that means some examples are source-compatible only with a specific `JSON_ADE` branch or schema set.

### Schema-independent examples

These do not depend on generated `OrderPackets` / `DataPackets` symbols:

- `ExampleADC`
- `ExampleEthernet`
- `ExampleEXTI`
- `ExampleHardFault`
- `ExampleMPU`
- `ExampleTCPIP`

You can build them as long as the normal board/preset requirements are satisfied.

### Examples tied to the current `TEST` schema

- `ExamplePackets`

`ExamplePackets` is designed around the current `TEST` packet schema active in this branch.

It expects the generated API associated with the `TEST` board definition and the packet/order set used by `tools/example_packets_check.py`.

The required `JSON_ADE` branch for this setup is:

- `fw-test-packets`

### Examples tied to a different legacy characterization schema

- `LinearSensorCharacterization`

`LinearSensorCharacterization` does not match the current `TEST` schema.

It expects a different `JSON_ADE` branch or board definition that generates the legacy characterization symbols (`characterize`, `characterization`, `Value`, etc.).

The required `JSON_ADE` branch for this setup is:

- `linear-sensor-char`

### Important consequence

`ExamplePackets` and `LinearSensorCharacterization` are currently not plug-and-play compatible with the same `JSON_ADE` checkout.

If you switch the `JSON_ADE` branch/schema to satisfy one of them, you can break the other.

Before building a packet-dependent example, check its document first and confirm:

- which `JSON_ADE` schema it expects
- which `BOARD_NAME` it must be generated with
- whether the required generated symbols exist

## Notes

- `tools/build-example.sh` now handles named tests such as `usage_fault` as `TEST_USAGE_FAULT`.
- If an example does not define any `TEST_*` selector, the script no longer injects `TEST_0` by default.
- `out/build/latest.elf` always points to the most recent MCU build, so flash immediately after building the example you want.
- `Core/Inc/Communications/Packets/DataPackets.hpp` and `Core/Inc/Communications/Packets/OrderPackets.hpp` are generated packet headers. They are not source-of-truth files and are intentionally gitignored.
