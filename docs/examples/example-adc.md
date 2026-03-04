# ExampleADC

## Purpose

`ExampleADC` is the minimal ADC smoke test.

It validates:

- ADC peripheral initialization
- pin configuration on `PA0`
- repeated sampling through the `ADCDomain` API
- end-to-end visibility of the converted value in firmware memory

## Build

```sh
./tools/build-example.sh --example adc --preset nucleo-debug --test 0
```

Equivalent macro selection:

- `EXAMPLE_ADC`
- `TEST_0`

## Hardware setup

- Use a Nucleo or board build that exposes `PA0`.
- Wire `PA0` first to `3.3V`, then to `GND`.
- Open a debugger watch on `adc_value`.

## Runtime behavior

The example:

- creates one ADC instance on `PA0`
- samples every 100 ms
- stores the latest raw reading in the global `adc_value`

There is no UART, TCP, UDP, or automated host-side checker for this example.

## Expected result

- With `PA0` tied to `3.3V`, `adc_value` should move close to the top of the ADC range.
- With `PA0` tied to `GND`, `adc_value` should move close to zero.
- The variable should update continuously in the debugger.

## What a failure usually means

- Value does not move: wrong pin, missing board init, bad wiring, or ADC clock/config issue.
- Value is noisy or saturated: floating input, analog ground issue, or wrong source impedance.
- Build fails only for this example: board pin mapping does not support `PA0` as configured.
