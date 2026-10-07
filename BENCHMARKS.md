# Benchmarks

All reads below were performed on physical SPI NOR chips and validated against a previously established SHA-256 reference image. Times are end-to-end read times observed in the test environment.

The results are specific to this RP2040 board, wiring, clip/contact quality, host and software versions. They are measurements, not guaranteed device limits.

## Stable reference

| Device | Size | SPI | Time | Throughput | Verification |
|---|---:|---:|---:|---:|---|
| Winbond W25Q64FV | 8 MiB | 20 MHz | **14.116 s** | **580.3 KiB/s** | SHA-256 exact |

At 30 MHz the same W25Q64FV measured 14.104 s / 580.8 KiB/s. The difference was only about 0.08%, demonstrating that increasing SPI from 20 to 30 MHz no longer improved end-to-end throughput meaningfully.

## Benchmark bank

| SPI NOR | Size | CH341A + flashrom | Neo Programmer | RP2040 optimized | Result |
|---|---:|---:|---:|---:|---|
| Spansion S25FL032A/P | 4 MiB | 38.809 s | 39.034 s | 9.087 s @ 30 MHz | SHA exact |
| Macronix MX25L3233F | 4 MiB | 39.191 s | 38.849 s | 9.071 s @ 30 MHz | SHA exact |
| Macronix MX25L6406E | 8 MiB | 91.688 s | 91.975 s | 14.104 s @ 30 MHz | SHA exact |
| EON EN25Q64 | 8 MiB | 89.732 s | 90.173 s | 14.109 s @ 20 MHz | SHA exact |
| Winbond W25Q64FV | 8 MiB | 87.707 s | 88.069 s | 14.116 s @ 20 MHz | SHA exact |
| Winbond W25Q128FV | 16 MiB | 176.799 s | 180.771 s | 27.195 s @ 30 MHz | SHA exact |
| Macronix MX25L12835F | 16 MiB | 145.689 s | 145.152 s | 30.216 s @ 30 MHz | SHA exact |

## EN25Q64 stability finding

The EN25Q64 produced non-identical/incorrect reads during repeated 30 MHz testing in the original clip/wiring setup. After correcting the clip contact and reducing SPI to an exact 20 MHz, the read matched the expected SHA-256.

This result is why the project distinguishes **maximum demonstrated speed** from the **stable default**. Because USB is already the dominant throughput limit, 20 MHz retains essentially all end-to-end performance while providing more electrical margin.

## Evolution of the 8 MiB read

Representative milestones:

| Stage | Approx. result |
|---|---:|
| stacksmashing PIO implementation | ~57 s |
| initial hardware-SPI path | ~48 s |
| hardware SPI + USB tuning | ~14 s |
| DMA + double pipeline + USB tuning | ~14.1 s, with SPI wait largely hidden |

Profiling after pipelining showed that USB transmission/service accounted for roughly 96% of active SPIOP pipeline time, making further SPI-frequency increases a poor optimization target at the current stage.

## Comparison caveat

CH341A and Neo Programmer results were collected as practical reference points using the same physical chips. The software stacks and protocols are different, so the comparison represents end-to-end user-observed read time rather than a synthetic bus-level comparison.
