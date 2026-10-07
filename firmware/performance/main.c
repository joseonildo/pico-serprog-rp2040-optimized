/**
 * Copyright (C) 2021, Mate Kukri <km@mkukri.xyz>
 * Based on "pico-serprog" by Thomas Roth <code@stacksmashing.net>
 * 
 * Licensed under GPLv3
 *
 * Also based on stm32-vserprog:
 *  https://github.com/dword1511/stm32-vserprog
 * 
 */

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "tusb.h"
#include "serprog.h"

#define CDC_ITF     0           // USB CDC interface no

#define SPI_IF      spi0        // Which PL022 to use
#define SPI_BAUD    12000000    // Default baudrate (12 MHz)
#define SPI_CS      5
#define SPI_MISO    4
#define SPI_MOSI    3
#define SPI_SCK     2

static void enable_spi(uint baud)
{
    // Setup chip select GPIO
    gpio_init(SPI_CS);
    gpio_put(SPI_CS, 1);
    gpio_set_dir(SPI_CS, GPIO_OUT);

    // Setup PL022
    spi_init(SPI_IF, baud);
    gpio_set_function(SPI_MISO, GPIO_FUNC_SPI);
    gpio_set_function(SPI_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(SPI_SCK,  GPIO_FUNC_SPI);
}

static void disable_spi()
{
    // Set all pins to SIO inputs
    gpio_init(SPI_CS);
    gpio_init(SPI_MISO);
    gpio_init(SPI_MOSI);
    gpio_init(SPI_SCK);

    // Disable all pulls
    gpio_set_pulls(SPI_CS, 0, 0);
    gpio_set_pulls(SPI_MISO, 0, 0);
    gpio_set_pulls(SPI_MOSI, 0, 0);
    gpio_set_pulls(SPI_SCK, 0, 0);

    // Disable SPI peripheral
    spi_deinit(SPI_IF);
}

static inline void cs_select(uint cs_pin)
{
    asm volatile("nop \n nop \n nop"); // FIXME
    gpio_put(cs_pin, 0);
    asm volatile("nop \n nop \n nop"); // FIXME
}

static inline void cs_deselect(uint cs_pin)
{
    asm volatile("nop \n nop \n nop"); // FIXME
    gpio_put(cs_pin, 1);
    asm volatile("nop \n nop \n nop"); // FIXME
}

static void wait_for_read(void)
{
    do
        tud_task();
    while (!tud_cdc_n_available(CDC_ITF));
}

static inline void readbytes_blocking(void *b, uint32_t len)
{
    while (len) {
        wait_for_read();
        uint32_t r = tud_cdc_n_read(CDC_ITF, b, len);
        b += r;
        len -= r;
    }
}

static inline uint8_t readbyte_blocking(void)
{
    wait_for_read();
    uint8_t b;
    tud_cdc_n_read(CDC_ITF, &b, 1);
    return b;
}

static void wait_for_write(void)
{
    do {
        tud_task();
    } while (!tud_cdc_n_write_available(CDC_ITF));
}

static inline void sendbytes_blocking(const void *b, uint32_t len)
{
    while (len) {
        wait_for_write();
        uint32_t w = tud_cdc_n_write(CDC_ITF, b, len);
        b += w;
        len -= w;
    }
}

static inline void sendbyte_blocking(uint8_t b)
{
    wait_for_write();
    tud_cdc_n_write(CDC_ITF, &b, 1);
}


static int spi_dma_tx = -1;
static int spi_dma_rx = -1;

static uint8_t spi_dma_dummy = 0x00;

static void spi_dma_init(void)
{
    spi_dma_tx = dma_claim_unused_channel(true);
    spi_dma_rx = dma_claim_unused_channel(true);
}

static void spi_read_dma_start(uint8_t *dst, uint32_t len)
{
    dma_channel_config tx_cfg =
        dma_channel_get_default_config(spi_dma_tx);

    channel_config_set_transfer_data_size(&tx_cfg, DMA_SIZE_8);
    channel_config_set_read_increment(&tx_cfg, false);
    channel_config_set_write_increment(&tx_cfg, false);
    channel_config_set_dreq(&tx_cfg, spi_get_dreq(SPI_IF, true));

    dma_channel_config rx_cfg =
        dma_channel_get_default_config(spi_dma_rx);

    channel_config_set_transfer_data_size(&rx_cfg, DMA_SIZE_8);
    channel_config_set_read_increment(&rx_cfg, false);
    channel_config_set_write_increment(&rx_cfg, true);
    channel_config_set_dreq(&rx_cfg, spi_get_dreq(SPI_IF, false));

    dma_channel_configure(
        spi_dma_rx,
        &rx_cfg,
        dst,
        &spi_get_hw(SPI_IF)->dr,
        len,
        false
    );

    dma_channel_configure(
        spi_dma_tx,
        &tx_cfg,
        &spi_get_hw(SPI_IF)->dr,
        &spi_dma_dummy,
        len,
        false
    );

    dma_start_channel_mask(
        (1u << spi_dma_rx) |
        (1u << spi_dma_tx)
    );
}

static void spi_read_dma_wait(void)
{
    dma_channel_wait_for_finish_blocking(spi_dma_rx);
    dma_channel_wait_for_finish_blocking(spi_dma_tx);

    while (spi_is_busy(SPI_IF))
        tight_loop_contents();
}


static void command_loop(void)
{
    uint baud = spi_get_baudrate(SPI_IF);

    for (;;) {
        switch (readbyte_blocking()) {
        case S_CMD_NOP:
            sendbyte_blocking(S_ACK);
            break;
        case S_CMD_Q_IFACE:
            sendbyte_blocking(S_ACK);
            sendbyte_blocking(0x01);
            sendbyte_blocking(0x00);
            break;
        case S_CMD_Q_CMDMAP:
            {
                static const uint32_t cmdmap[8] = {
                    (1 << S_CMD_NOP)       |
                      (1 << S_CMD_Q_IFACE)   |
                      (1 << S_CMD_Q_CMDMAP)  |
                      (1 << S_CMD_Q_PGMNAME) |
                      (1 << S_CMD_Q_SERBUF)  |
                      (1 << S_CMD_Q_BUSTYPE) |
                      (1 << S_CMD_SYNCNOP)   |
                      (1 << S_CMD_O_SPIOP)   |
                      (1 << S_CMD_S_BUSTYPE) |
                      (1 << S_CMD_S_SPI_FREQ)|
                      (1 << S_CMD_S_PIN_STATE)
                };

                sendbyte_blocking(S_ACK);
                sendbytes_blocking((uint8_t *) cmdmap, sizeof cmdmap);
                break;
            }
        case S_CMD_Q_PGMNAME:
            {
                static const char progname[16] = "pico-serprog";

                sendbyte_blocking(S_ACK);
                sendbytes_blocking(progname, sizeof progname);
                break;
            }
        case S_CMD_Q_SERBUF:
            sendbyte_blocking(S_ACK);
            sendbyte_blocking(0xFF);
            sendbyte_blocking(0xFF);
            break;
        case S_CMD_Q_BUSTYPE:
            sendbyte_blocking(S_ACK);
            sendbyte_blocking((1 << 3)); // BUS_SPI
            break;
        case S_CMD_SYNCNOP:
            sendbyte_blocking(S_NAK);
            sendbyte_blocking(S_ACK);
            break;
        case S_CMD_S_BUSTYPE:
            // If SPI is among the requsted bus types we succeed, fail otherwise
            if((uint8_t) readbyte_blocking() & (1 << 3))
                sendbyte_blocking(S_ACK);
            else
                sendbyte_blocking(S_NAK);
            break;
        case S_CMD_O_SPIOP:
            {
                static uint8_t buf[2][4096];

                uint32_t wlen = 0;
                readbytes_blocking(&wlen, 3);
                uint32_t rlen = 0;
                readbytes_blocking(&rlen, 3);

                cs_select(SPI_CS);

                while (wlen) {
                    uint32_t cur = MIN(wlen, sizeof buf[0]);
                    readbytes_blocking(buf[0], cur);
                    spi_write_blocking(SPI_IF, buf[0], cur);
                    wlen -= cur;
                }

                sendbyte_blocking(S_ACK);

                if (rlen) {
                    uint current = 0;
                    uint32_t current_len = MIN(rlen, sizeof buf[0]);

                    /*
                     * Prime o pipeline:
                     * comeca a primeira leitura SPI por DMA.
                     */
                    spi_read_dma_start(buf[current], current_len);
                    rlen -= current_len;

                    for (;;) {
                        /*
                         * O buffer atual precisa estar completo antes
                         * de ser entregue ao TinyUSB.
                         */
                        spi_read_dma_wait();

                        if (rlen) {
                            /*
                             * Enquanto o USB consome o buffer atual,
                             * o DMA preenche o outro buffer.
                             */
                            uint next = current ^ 1u;
                            uint32_t next_len =
                                MIN(rlen, sizeof buf[0]);

                            spi_read_dma_start(buf[next], next_len);
                            rlen -= next_len;

                            sendbytes_blocking(
                                buf[current],
                                current_len
                            );

                            current = next;
                            current_len = next_len;
                        } else {
                            /*
                             * Ultimo buffer: nao ha nova leitura
                             * para iniciar.
                             */
                            sendbytes_blocking(
                                buf[current],
                                current_len
                            );
                            break;
                        }
                    }
                }

                cs_deselect(SPI_CS);
            }
            break;
        case S_CMD_S_SPI_FREQ:
            {
                uint32_t want_baud;
                readbytes_blocking(&want_baud, 4);
                if (want_baud) {
                    // Set frequence
                    baud = spi_set_baudrate(SPI_IF, want_baud);
                    // Send back actual value
                    sendbyte_blocking(S_ACK);
                    sendbytes_blocking(&baud, 4);
                } else {
                    // 0 Hz is reserved
                    sendbyte_blocking(S_NAK);
                }
                break;
            }
        case S_CMD_S_PIN_STATE:
            if (readbyte_blocking())
                enable_spi(baud);
            else
                disable_spi();
            sendbyte_blocking(S_ACK);
            break;
        default:
            sendbyte_blocking(S_NAK);
            break;
        }

        tud_cdc_n_write_flush(CDC_ITF);
    }
}

int main()
{
    // Setup USB
    tusb_init();
    // Setup PL022 SPI
    // Clock consolidado: SYS/PERI = 120 MHz.
    // Permite ao divisor do SPI gerar 30.000 MHz reais.
    if (!set_sys_clock_khz(120000, true)) {
        while (1) {
            tight_loop_contents();
        }
    }

    clock_configure_undivided(
        clk_peri,
        0,
        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
        120000000
    );

    enable_spi(SPI_BAUD);

    // Setup DMA usado nas leituras SPI
    spi_dma_init();

    command_loop();
}
