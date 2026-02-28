# ExampleEthernet

## Purpose

`ExampleEthernet` is the minimal Ethernet bring-up loop.

It validates:

- board Ethernet pinset selection
- PHY selection (`LAN8742`, `LAN8700`, or `KSZ8041`)
- MAC + IPv4 initialization
- continuous `eth_instance.update()` polling in the main loop
- basic board liveness through the status LED

## Build

Nucleo + on-board LAN8742:

```sh
./tools/build-example.sh --example ethernet --preset nucleo-debug-eth --test 0
```

Custom board with KSZ8041:

```sh
./tools/build-example.sh --example ethernet --preset board-debug-eth-ksz8041 --test 0
```

Equivalent macro selection:

- `EXAMPLE_ETHERNET`
- `TEST_0`

## Network configuration

This example uses a fixed board IP:

- Board IP: `192.168.1.7`
- Netmask: `255.255.0.0`
- MAC: `00:80:e1:00:01:07`

## Runtime behavior

- The LED on `PB0` is turned on and stays on.
- If Ethernet support is enabled (`STLIB_ETH`), the example continuously calls `eth_instance.update()`.
- There is no socket traffic in this example.

## How to validate

1. Connect the board Ethernet port to the host.
2. Put the host NIC on the same subnet.
3. Flash and reset the board.
4. Ping the board:

```sh
ping 192.168.1.7
```

## Expected result

- The board responds to ping.
- The LED remains on.
- There should be no hard fault on boot.

## What it does not validate

- TCP socket creation
- UDP socket creation
- packet parsing
- reconnection logic

Use `ExampleTCPIP` or `ExamplePackets` for that.

## What a failure usually means

- No ping and no link LEDs: PHY, cable, host NIC config, or wrong preset.
- LED on but no ping: MAC/PHY init issue, wrong subnet, or host routing problem.
- Build fails: wrong Ethernet preset for the board/PHY combination.
