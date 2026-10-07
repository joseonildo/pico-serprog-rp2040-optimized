# pico-serprog RP2040 optimized

High-performance SPI NOR flash programmer for the Raspberry Pi Pico / RP2040, derived from the pico-serprog family and focused on **measured throughput, repeatable verification and practical signal-integrity margins**.

## Current stable baseline

The current stable configuration is:

- RP2040 system/peripheral clock: **120 MHz**
- hardware SPI (spi0)
- default SPI clock: **20 MHz exact**
- DMA-assisted transfers
- double pipeline buffer: **2 x 4096 bytes**
- TinyUSB CDC RX/TX/endpoint buffers: **4096 bytes**
- host: flashrom using the serprog protocol

Reference result with a Winbond W25Q64FV (8 MiB):

**14.116 s — 580.3 KiB/s — SHA-256 verified**

A 30 MHz SPI clock remains useful for experiments, but 20 MHz was selected as the stable default because it produced essentially identical end-to-end throughput while providing more electrical margin. An EON EN25Q64 exposed the practical difference: it was nondeterministic at 30 MHz in the test setup and verified correctly at 20 MHz after the clip/contact path was corrected.

## Why this project exists

The work started by evaluating existing RP2040 serprog implementations and then measuring each bottleneck instead of assuming that SPI clock alone determined performance. The optimization path moved from PIO/hardware-SPI comparisons through larger USB buffers, exact RP2040 clocking, DMA and pipelining.

Profiling of the current pipeline showed that USB/TinyUSB service, rather than SPI transfer time, became the dominant remaining bottleneck. Future experiments therefore focus on USB scheduling/service behavior before pursuing higher SPI clocks.

## Documentation

- [Benchmark results](BENCHMARKS.md)
- [Optimization history](CHANGELOG.md)
- [Hardware and pinout](HARDWARE.md)
- [Upstream projects, credits and licensing](CREDITS.md)

## Safety and reproducibility

Benchmark images are verified by SHA-256, but firmware dumps from tested devices are **not** published in this repository. Results describe the specific hardware, wiring, chips and software environment used during testing and should not be interpreted as guaranteed limits for every RP2040 or SPI NOR device.

## Project status

The stable firmware is being preserved while experimental changes are developed in separate trees. Experimental results are promoted to the stable baseline only after repeatable verification.

## License and attribution

The firmware lineage is GPLv3. This repository preserves upstream copyright and licensing requirements. See [CREDITS.md](CREDITS.md) before redistributing code or media.

This project is an optimization and benchmarking effort built on prior open-source work; it does not claim authorship of the original pico-serprog implementation.
