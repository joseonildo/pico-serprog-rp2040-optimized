# pico-serprog RP2040 - Performance firmware

This directory contains the optional Performance firmware variant of
pico-serprog-rp2040-optimized.

The existing Stable firmware remains the recommended choice when maximum
USB host compatibility is required.

## What is different?

The Performance build retains the validated hardware SPI and DMA
implementation, but disables the Pico SDK workaround for RP2040 USB
Errata 15.

Relevant build configuration:

    PICO_RP2040_USB_DEVICE_UFRAME_FIX=0
    PICO_RP2040_USB_FAST_IRQ=1

SPI configuration remains conservative:

- RP2040 SYS/PERI clock: 120 MHz
- firmware startup SPI clock: 12 MHz
- recommended flashrom SPI clock: 20 MHz
- hardware SPI0
- DMA enabled
- two 4096-byte pipeline buffers
- TinyUSB CDC RX/TX/EP buffers: 4096 bytes

The performance gain does not come from SPI overclocking.

## RP2040 USB Errata 15 warning

The Pico SDK normally enables a workaround for RP2040 USB Errata 15.

The workaround avoids making USB Bulk IN buffers available during a
critical period near the end of a USB Full-Speed frame. The SDK source
notes that this reduces available Bulk IN bandwidth by approximately 20%.

The Performance firmware deliberately disables this workaround.

Testing on the development host showed a substantial and repeatable
increase in flash read throughput. Seven physical SPI NOR devices were
validated with exact size and SHA256 verification.

However, disabling the workaround may reduce compatibility with some USB
hosts. Raspberry Pi 4 and Raspberry Pi 400 hosts are specifically relevant
to the workaround described by the Pico SDK.

If the Performance firmware produces USB communication problems on your
host, use the Stable firmware instead.

## Recommended flashrom configuration

The firmware starts at 12 MHz.

For the validated Performance operating point, request 20 MHz:

    flashrom -p 'serprog:dev=/dev/ttyACM0,spispeed=20M' \
      -c 'W25Q64BV/W25Q64CV/W25Q64FV' -r backup.bin

Note the comma before spispeed.

## W25Q64FV result

Stable:

    14.102 s
    580.9 KiB/s

Performance, average of three runs:

    11.520 s
    711.1 KiB/s

This represents approximately 22.4% higher throughput.

## USB profiling

Low-overhead profiling was used to compare USB behavior.

Stable / EXP04:

    wait_calls          : 4260
    tud_task_calls      : 5174160
    avail_zero          : 5169900
    avail_nonzero       : 4260
    avail_min           : 4090
    avail_max           : 4096
    avail_nonzero_avg   : 4095

Performance / EXP06:

    wait_calls          : 4260
    tud_task_calls      : 4006977
    avail_zero          : 4002717
    avail_nonzero       : 4260
    avail_min           : 4090
    avail_max           : 4096
    avail_nonzero_avg   : 4095

The number of useful TX-space events remained exactly 4260, while
tud_task() polling and zero-availability samples decreased by
approximately 22.6%.

This strongly supports the conclusion that the performance improvement
comes from removing additional USB Bulk IN scheduling delay rather than
from increasing SPI speed.

See BENCHMARKS-PERFORMANCE.md in the repository root for the complete
seven-chip validation.
