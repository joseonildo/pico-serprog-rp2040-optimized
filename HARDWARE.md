# Hardware and test setup

## Programmer

- Raspberry Pi Pico / RP2040
- USB connection to a WSL2 host
- flashrom using serprog over TinyUSB CDC
- SPI NOR powered from the Pico during the off-board benchmark campaign

## SPI pinout

| Signal | RP2040 GPIO | Pico physical pin | SPI NOR pin |
|---|---:|---:|---:|
| SCK | GP2 | 4 | 6 |
| MOSI | GP3 | 5 | 5 |
| MISO | GP4 | 6 | 2 |
| CS# | GP5 | 7 | 1 |
| GND | — | 8 | 4 |
| 3V3 | — | 36 | 8 |

WP# (flash pin 3) and HOLD# (flash pin 7) are held high at 3.3 V in the off-board setup.

## Stable clock configuration

- RP2040 SYS: 120 MHz
- RP2040 PERI: 120 MHz
- default SPI: 20 MHz exact

120 MHz was selected because it provides clean integer SPI ratios, including:

- 120 / 10 = 12 MHz
- 120 / 8 = 15 MHz
- 120 / 6 = 20 MHz
- 120 / 4 = 30 MHz

This avoids the unintuitive divider quantization observed with the original 125 MHz system clock.

## Signal integrity

The benchmark setup used approximately 20 cm wiring/clip connections. Most tested chips operated successfully at exact 30 MHz, and testing reached approximately 31 MHz before JEDEC identification became unreliable around 32 MHz.

This should not be interpreted as an RP2040 silicon limit. Wiring length, ground return, SOIC clip quality, chip characteristics and in-circuit loading can all lower the practical limit.

The stable project default is therefore 20 MHz.

## In-circuit warning

In-circuit programming can electrically back-power or contend with the target board. Verify target power isolation, rail voltage, WP#/HOLD# state and other devices connected to the SPI bus before attaching the programmer.

The off-board benchmark results do not guarantee identical behavior in-circuit.
