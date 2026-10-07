# Optimization history

This document records the major technical milestones of the project. It is a reconstruction of the measured development path rather than the original Git commit history of upstream projects.

## 1. Working RP2040 serprog

The project began with the opensensor pico-serprog tree and established a reliable flashrom-to-RP2040 serprog workflow under WSL2.

A 4 MiB SPI NOR was successfully detected and read, proving the basic hardware, USB CDC and flashrom path.

## 2. PIO implementation benchmark

The stacksmashing PIO implementation was built and benchmarked.

On the W25Q64-class 8 MiB test case, reads were approximately 57 seconds. Increasing PIO SPI clock improved the result very little, and sufficiently high clocks caused detection failures.

This suggested that raw SPI clock was not the only bottleneck.

## 3. Hardware-SPI lineage

A clean Libreboot pico-serprog baseline (commit 4e1c534) was selected for controlled optimization.

Initial 8 MiB hardware-SPI reads were around 48 seconds.

## 4. Host transaction analysis

flashrom traffic was inspected. For the 8 MiB test, the host was already issuing large reads (typically 64 KiB), ruling out a simple host-side tiny-transaction explanation for the low throughput.

## 5. TinyUSB buffer experiments

CDC and endpoint buffering were increased systematically.

The decisive change was increasing the TinyUSB CDC endpoint buffer to 4096 bytes. End-to-end 8 MiB read time fell from roughly 45 seconds to roughly 14 seconds.

Larger configurations were not automatically better: an 8192-byte endpoint experiment lost synchronization, while increasing only the TX buffer to 8192 produced no meaningful gain.

## 6. Exact RP2040 clocks

At the default 125 MHz peripheral clock, requested SPI frequencies are quantized by the RP2040 SPI divider. For example, a nominal 30 MHz request could result in approximately 20.833 MHz.

SYS/PERI were changed to 120 MHz, providing exact and convenient SPI frequencies such as 12, 15, 20 and 30 MHz.

## 7. SPI ceiling investigation

Exact 25 MHz and 30 MHz operation was validated on suitable chips. Approximately 31 MHz remained functional in the physical test setup, while around 32 MHz JEDEC identification became unreliable.

The practical ceiling appeared dominated by the approximately 20 cm wiring/clip setup rather than by a useful throughput limit.

## 8. DMA

DMA was added to the hardware-SPI path. DMA alone did not substantially improve end-to-end read time because USB transmission remained dominant.

## 9. Double-buffer pipeline

A two-buffer 2 x 4096-byte pipeline was implemented so SPI acquisition and USB transmission could overlap.

The pipeline reduced exposed SPI waiting dramatically, but total read time remained near 14 seconds.

Profiling showed why: USB send/service dominated active SPIOP time.

## 10. USB bottleneck identified

Deep profiling measured millions of write-wait iterations and substantial time inside TinyUSB task servicing. At this point, further SPI clock increases could not materially improve total throughput.

The next optimization target became TinyUSB/USB service behavior.

## 11. Stability bank

Seven salvaged SPI NOR devices across Spansion, Macronix, EON and Winbond families were benchmarked against CH341A/flashrom and Neo Programmer reference times.

Six devices verified successfully at 30 MHz. The EN25Q64 showed nondeterministic results at 30 MHz in the original clip setup.

## 12. Stable 20 MHz baseline

The W25Q64FV was tested directly at exact 30 and 20 MHz:

- 30 MHz: 14.104 s / 580.8 KiB/s
- 20 MHz: 14.116 s / 580.3 KiB/s

The performance difference was approximately 0.08%.

The EN25Q64 subsequently verified correctly at 20 MHz after correcting the clip contact. Because 20 MHz preserved effectively all throughput while increasing electrical margin, it became the stable default.

## Current stable baseline

- RP2040 SYS/PERI: 120 MHz
- hardware SPI: 20 MHz default
- DMA
- double pipeline: 2 x 4096
- TinyUSB CDC RX/TX/EP: 4096
- reference W25Q64FV result: 14.116 s / 580.3 KiB/s / SHA-256 exact

Future changes are developed as experiments and promoted only after repeatable verification.
