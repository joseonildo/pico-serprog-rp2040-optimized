# Performance firmware

This release provides an optional high-performance firmware variant for
pico-serprog-rp2040-optimized.

The existing Stable release remains unchanged and remains the recommended
choice for maximum USB host compatibility.

## Performance improvement

The Performance build disables the Pico SDK RP2040 USB Errata 15
workaround, removing a USB Bulk IN scheduling restriction that the SDK
source documents as reducing available Bulk IN bandwidth by approximately
20%.

On the Winbond W25Q64FV:

- Stable: 14.102 s / 580.9 KiB/s
- Performance: 11.520 s / 711.1 KiB/s average
- throughput improvement: approximately 22.4%

Seven physical SPI NOR devices were validated at the recommended 20 MHz
SPI clock. Every full-chip read produced the expected size and SHA256.

Peak measured throughput in the seven-chip validation was approximately
755.9 KiB/s.

## Important compatibility warning

This firmware deliberately disables an RP2040 USB hardware-errata
workaround.

It may therefore be less compatible with some USB hosts. Raspberry Pi 4
and Raspberry Pi 400 systems are specifically relevant to the workaround
described by the Pico SDK.

If you encounter USB communication errors, use the Stable firmware
instead.

## SPI configuration

The performance improvement does not require SPI overclocking.

- firmware startup/default SPI clock: 12 MHz
- recommended flashrom operating clock: 20 MHz

Example:

    flashrom -p 'serprog:dev=/dev/ttyACM0,spispeed=20M' \
      -c 'W25Q64BV/W25Q64CV/W25Q64FV' -r backup.bin

Firmware SHA256:

    aa0ab1317e8a874d633f3fce0ef96ee238cc9a62a3f0439296f8915371707684
