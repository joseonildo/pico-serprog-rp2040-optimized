# Performance Firmware Benchmarks

The Performance firmware disables the RP2040 USB Errata 15 workaround
while retaining the validated 20 MHz SPI operating point.

All Performance results below are full-chip reads with exact size and
SHA256 verification.

| Flash chip | Size | Previous Pico result | Performance @ 20 MHz | Throughput | Validation |
|---|---:|---:|---:|---:|---|
| Spansion S25FL032P | 4 MiB | 9.087 s | 7.099 s | 577.0 KiB/s | SHA256 exact |
| Macronix MX25L3233F | 4 MiB | 9.071 s | 7.416 s | 552.3 KiB/s | SHA256 exact |
| Macronix MX25L6406E | 8 MiB | 14.104 s | 10.985 s | 745.7 KiB/s | SHA256 exact |
| EON EN25Q64 | 8 MiB | 14.109 s @ 20 MHz | 11.235 s | 729.1 KiB/s | SHA256 exact |
| Winbond W25Q64FV | 8 MiB | 14.104 s | 11.537 s | 710.1 KiB/s | SHA256 exact |
| Winbond W25Q128FV | 16 MiB | 27.195 s | 21.674 s | 755.9 KiB/s | SHA256 exact |
| Macronix MX25L12835F | 16 MiB | 30.216 s | 26.518 s | 617.8 KiB/s | SHA256 exact |

## W25Q64FV repeatability

Three consecutive Performance reads at 20 MHz:

| Run | Time | Throughput |
|---|---:|---:|
| 1 | 11.493 s | 712.8 KiB/s |
| 2 | 11.565 s | 708.3 KiB/s |
| 3 | 11.503 s | 712.2 KiB/s |
| Average | 11.520 s | 711.1 KiB/s |

All three reads produced the expected SHA256:

    285c35339745f185928520f06f972c3a3fdfd2ed3d6e2efff75041a945b6b54b

## USB profiling

| Counter | Stable / EXP04 | Performance / EXP06 |
|---|---:|---:|
| wait_calls | 4,260 | 4,260 |
| tud_task_calls | 5,174,160 | 4,006,977 |
| avail_zero | 5,169,900 | 4,002,717 |
| avail_nonzero | 4,260 | 4,260 |
| avail_min | 4,090 | 4,090 |
| avail_max | 4,096 | 4,096 |
| avail_nonzero_avg | 4,095 | 4,095 |

The Performance configuration reduced tud_task() polling and
zero-availability samples by approximately 22.6%.

The number of useful TX-space events and available FIFO space per useful
event remained essentially unchanged.

## Compatibility

The Performance firmware deliberately disables the Pico SDK workaround
for RP2040 USB Errata 15.

This improves USB Bulk IN throughput but may reduce compatibility with
some USB hosts.

Use the Stable firmware when maximum USB host compatibility is preferred.
