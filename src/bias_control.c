#include "bias_control.h"
#include <gpiod.h>
#include <time.h>

static struct gpiod_chip *g_chip;
static struct gpiod_line *g_tx, *g_rx;
static unsigned g_bit_us = 1000; // default; set by bias_open

static inline void tx_hi(void) { gpiod_line_set_value(g_tx, 1); }
static inline void tx_lo(void) { gpiod_line_set_value(g_tx, 0); }
static inline int  rx_rd(void) { return gpiod_line_get_value(g_rx); } // kept for future RX

static inline uint64_t now_us(void) {
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000ull + (uint64_t)ts.tv_nsec / 1000ull;
}

static inline void sleep_until_us(uint64_t t_us) {
    for (;;) {
        uint64_t n = now_us();
        if (n + 10 >= t_us) { while (now_us() < t_us) {} return; }
        uint64_t rem = t_us - n - 10;
        struct timespec ts = { .tv_sec = rem/1000000ull,
                               .tv_nsec = (long)((rem%1000000ull)*1000ull) };
        clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);
    }
}

int bias_open(unsigned bit_us) {
    g_bit_us = bit_us ? bit_us : 1000;

    g_chip = gpiod_chip_open_by_name("gpiochip0");
    if (!g_chip) return -1;

    g_tx = gpiod_chip_get_line(g_chip, PI_TX);
    g_rx = gpiod_chip_get_line(g_chip, PI_RX);
    if (!g_tx || !g_rx) return -2;

    if (gpiod_line_request_output(g_tx, "bias_tx", 1) < 0) return -3; // idle high
    if (gpiod_line_request_input(g_rx,  "bias_rx")    < 0) return -4; // idle high via pull-up

    tx_hi();
    return 0;
}

void bias_close(void) {
    if (g_rx)  { gpiod_line_release(g_rx);  g_rx  = NULL; }
    if (g_tx)  { gpiod_line_release(g_tx);  g_tx  = NULL; }
    if (g_chip){ gpiod_chip_close(g_chip);  g_chip = NULL; }
}

void bias_send_frame(uint8_t cmd3, int16_t data16) {
    const unsigned START_US    = g_bit_us;      // 1 bit
    const unsigned IFG_US      = g_bit_us;      // inter-frame gap (high)
    uint64_t t = now_us();

    // START (low, 1 bit)
    tx_lo();                 t += START_US; sleep_until_us(t);

    // CMD (3 bits, LSB-first)
    for (int i = 0; i < 3; ++i) {
        ((cmd3 >> i) & 1) ? tx_hi() : tx_lo();
        t += g_bit_us; sleep_until_us(t);
    }

    // DATA (16 bits, LSB-first)
    uint16_t d = (uint16_t)data16;
    for (int i = 0; i < 16; ++i) {
        ((d >> i) & 1) ? tx_hi() : tx_lo();
        t += g_bit_us; sleep_until_us(t);
    }

    // STOP/IDLE (line high, at least 1 bit as guard)
    tx_hi();
    t += IFG_US;  sleep_until_us(t);
}
