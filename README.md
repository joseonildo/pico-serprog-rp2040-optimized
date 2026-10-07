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


## Tested SPI NOR chips — comparison with Neo Programmer

Seven physical SPI NOR devices from multiple manufacturers were used in the benchmark bank. Reads from the RP2040 programmer were validated against known SHA-256 reference images.

| SPI NOR | Size | Neo Programmer | RP2040 optimized | Speed-up vs Neo | Verification |
|---|---:|---:|---:|---:|---|
| Spansion S25FL032A/P | 4 MiB | 39.034 s | 9.087 s @ 30 MHz | **4.30x** | SHA-256 exact |
| Macronix MX25L3233F | 4 MiB | 38.849 s | 9.071 s @ 30 MHz | **4.28x** | SHA-256 exact |
| Macronix MX25L6406E | 8 MiB | 91.975 s | 14.104 s @ 30 MHz | **6.52x** | SHA-256 exact |
| EON EN25Q64 | 8 MiB | 90.173 s | 14.109 s @ 20 MHz | **6.39x** | SHA-256 exact |
| Winbond W25Q64FV | 8 MiB | 88.069 s | 14.116 s @ 20 MHz | **6.24x** | SHA-256 exact |
| Winbond W25Q128FV | 16 MiB | 180.771 s | 27.195 s @ 30 MHz | **6.65x** | SHA-256 exact |
| Macronix MX25L12835F | 16 MiB | 145.152 s | 30.216 s @ 30 MHz | **4.80x** | SHA-256 exact |

In this test bank, the optimized RP2040 programmer completed full-chip reads approximately **4.28x to 6.65x faster than Neo Programmer**, while producing the expected SHA-256 images.

The EN25Q64 and W25Q64FV entries also demonstrate why **20 MHz is the stable default**: both retained excellent end-to-end throughput at the lower SPI clock, while providing greater electrical margin. More detailed results, including CH341A + flashrom measurements, are available in [BENCHMARKS.md](BENCHMARKS.md).

## Why this project exists

The work started by evaluating existing RP2040 serprog implementations and then measuring each bottleneck instead of assuming that SPI clock alone determined performance. The optimization path moved from PIO/hardware-SPI comparisons through larger USB buffers, exact RP2040 clocking, DMA and pipelining.

Profiling of the current pipeline showed that USB/TinyUSB service, rather than SPI transfer time, became the dominant remaining bottleneck. Future experiments therefore focus on USB scheduling/service behavior before pursuing higher SPI clocks.

## Firmware source and build

The audited stable source tree is available in **[firmware/stable](firmware/stable/)**. It preserves the upstream licensing files alongside the modified implementation.

Build requirements:

- Raspberry Pi Pico SDK
- CMake
- ARM GNU toolchain

A typical build from the repository root is:

```bash
cd firmware/stable
mkdir -p build
cd build
cmake .. -DPICO_SDK_PATH="$HOME/pico-sdk"
cmake --build . -j"$(nproc)"
```

The resulting `pico_serprog.uf2` can be copied to a Raspberry Pi Pico while it is mounted in **BOOTSEL** mode. After reboot, the firmware enumerates as a TinyUSB CDC device and can be used by flashrom through the serprog programmer interface.

Example host invocation:

```bash
sudo flashrom -p serprog:dev=/dev/ttyACM0:spispeed=20M
```

The stable prebuilt UF2 is distributed separately through the repository's **GitHub Releases**, allowing the source tree to remain free of build artifacts. Verify the published SHA-256 before flashing.

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
