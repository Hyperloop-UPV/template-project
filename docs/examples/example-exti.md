# ExampleEXTI

## Purpose

`ExampleEXTI` is the minimal external interrupt test.

It validates:

- EXTI configuration on `PC13`
- callback dispatch on both edges
- interaction between EXTI and a GPIO output (`PB0`)

## Build

```sh
./tools/build-example.sh --example exti --preset nucleo-debug --test 0
```

Equivalent macro selection:

- `EXAMPLE_EXTI`
- `TEST_0`

## Hardware setup

- Use the Nucleo user button on `PC13`.
- Observe the LED connected to `PB0`.

## Runtime behavior

- The firmware registers an EXTI callback on `PC13`.
- The trigger mode is `BOTH_EDGES`.
- Every edge calls `toggle_led()`.

That means:

- button press toggles the LED
- button release toggles the LED again

## Expected result

- The LED changes state on every transition of the button signal.
- The main loop stays idle; the observable behavior comes entirely from the interrupt.

## What a failure usually means

- Nothing happens: EXTI line not configured, wrong pin mapping, or callback not firing.
- Only one transition toggles: edge trigger configuration is wrong.
- Repeated chatter: mechanical bounce or noisy input.

## Notes

This example is intentionally simple. It is useful to verify interrupt routing before adding more complex interrupt-driven logic.
