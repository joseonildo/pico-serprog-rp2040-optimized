# Credits, upstream projects and provenance

This project exists because of several open-source projects and prior implementations. The purpose of this document is to make provenance explicit and to avoid presenting upstream work as original work from this repository.

## Firmware lineage

### Libreboot pico-serprog

The hardware-SPI optimization branch used for the current stable firmware was developed from the pico-serprog source distributed by the Libreboot project.

- Project: Libreboot pico-serprog
- Upstream project: https://codeberg.org/libreboot/pico-serprog
- Baseline commit used during this work: 4e1c534
- License: GNU GPL v3

The local optimization history documented in this repository starts from that baseline and includes USB buffer experiments, exact RP2040 clock configuration, DMA and double-buffer pipelining.

### opensensor/pico-serprog

The opensensor fork was the first pico-serprog tree used during the project and was important for bringing up the Raspberry Pi Pico as a working serprog programmer and establishing the initial flashrom workflow.

- Project: opensensor/pico-serprog
- Source: https://github.com/opensensor/pico-serprog
- Commit tested in the original setup: 3e05d27

It is treated here as an upstream/reference implementation, not as code authored by this project.

### stacksmashing/pico-serprog

Thomas Roth (stacksmashing) published the RP2040 pico-serprog implementation that formed an important part of the pico-serprog ecosystem. Its PIO-based SPI implementation was independently benchmarked during this project and provided an important comparison against the hardware-SPI lineage.

- Project: stacksmashing/pico-serprog
- Source: https://github.com/stacksmashing/pico-serprog
- Commit tested during this work: 7f4c4b7

The stacksmashing project itself documents that its SPI implementation builds on Raspberry Pi Pico SDK examples and prior serprog work.

### stm32-vserprog

The pico-serprog lineage credits stm32-vserprog as prior serprog implementation work.

- Project: stm32-vserprog
- Source: https://github.com/dword1511/stm32-vserprog
- License: GNU GPL v3

## Core dependencies and tools

### Raspberry Pi Pico SDK

The Raspberry Pi Pico SDK provides the RP2040 platform APIs, hardware interfaces, build integration and examples used by RP2040 projects.

- Source: https://github.com/raspberrypi/pico-sdk
- License: BSD 3-Clause

### TinyUSB

TinyUSB provides the USB device stack used by pico-serprog. USB CDC buffering and task servicing became central to the performance investigation in this project.

- Source: https://github.com/hathach/tinyusb
- License: MIT

### flashrom

flashrom is the host-side flash programming utility used throughout the benchmarks. Communication with the Pico uses flashrom's serprog programmer support.

- Project: https://flashrom.org/
- Source: https://github.com/flashrom/flashrom
- License: GNU GPL v2 or later

## What this repository adds

The work documented here consists primarily of measurement-driven optimization and validation of the RP2040 pico-serprog path, including:

- comparison of PIO and hardware-SPI implementations;
- systematic SPI-frequency testing;
- analysis of RP2040 SPI clock-divider behavior;
- operation at 120 MHz to obtain exact 12/15/20/30 MHz SPI clocks;
- TinyUSB CDC RX/TX/endpoint buffer experiments;
- identification of a 4096-byte USB endpoint configuration as a high-performance configuration in this setup;
- DMA-assisted SPI transfers;
- double-buffer pipelining (2 x 4096 bytes);
- profiling that identified USB/TinyUSB service as the dominant remaining bottleneck;
- stability testing across multiple SPI NOR families;
- SHA-256 verification of benchmark reads;
- comparison against CH341A/flashrom and Neo Programmer.

These modifications and measurements do not erase or replace upstream authorship. Files derived from upstream code retain the applicable upstream license and notices.

## Media and documentation licensing

Do not assume that diagrams, pinout images or other media from an upstream project use the same license as its source code. When upstream media is reused, its individual license and attribution must be preserved. Where practical, this repository will prefer original diagrams/documentation or links to upstream material rather than copying media without a verified license.


## AI-assisted development

This optimization and benchmarking effort was developed with assistance from **ChatGPT by OpenAI**.

ChatGPT was used as an AI-assisted engineering tool throughout the project, including:

- technical discussion and troubleshooting;
- analysis of benchmark and profiling results;
- design and review of controlled optimization experiments;
- generation and review of shell scripts used during testing;
- organization and comparison of measurements;
- documentation drafting and project-history reconstruction.

Hardware work, firmware flashing, physical measurements, benchmark execution and validation were performed by the project owner. AI assistance does not replace or alter the authorship, copyright or licensing of any upstream project listed above.

- ChatGPT: https://chatgpt.com/
- OpenAI: https://openai.com/

## Thanks

Thanks to the Libreboot contributors, Thomas Roth / stacksmashing, opensensor contributors, stm32-vserprog contributors, Raspberry Pi Pico SDK contributors, TinyUSB contributors, flashrom developers, and the broader open-source firmware and hardware community whose work made this project possible.

If an attribution is incomplete or inaccurate, please open an issue so it can be corrected.
