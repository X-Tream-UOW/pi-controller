#include "bias_control.h"
#include <gpiod.h>
#include <time.h>
#include <stdint.h>

static struct gpiod_chip *g_chip;
static struct gpiod_line *g_tx, *g_rx;
static unsigned g_bit_us = 1000; // default; set by bias_open

static inline void tx_hi(void) { gpiod_line_set_value(g_tx, 1); }
static inline void tx_lo(void) { gpiod_line_set_value(g_tx, 0); }
static inline int  rx_rd(void) { return gpiod_line_get_value(g_rx); } // future RX use

static inline uint64_t now_us(void) {
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000ull + (uint64_t)ts.tv_nsec / 1000ull;
}

static inline void busy_wait_until_us(uint64_t t_us) {
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

    // TX: output, idle high
    if (gpiod_line_request_output(g_tx, "bias_tx", 1) < 0) return -3;

    // RX: input with pull-up enabled (libgpiod v2+ flag)
    if (gpiod_line_request_input_flags(
            g_rx, "bias_rx", GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_UP) < 0) return -4;

    tx_hi();
    return 0;
}

void bias_close(void) {
    if (g_rx)  { gpiod_line_release(g_rx);  g_rx  = NULL; }
    if (g_tx)  { gpiod_line_release(g_tx);  g_tx  = NULL; }
    if (g_chip){ gpiod_chip_close(g_chip);  g_chip = NULL; }
}

void bias_send_frame(uint8_t cmd3, int16_t data16) {
    const unsigned START_US    = g_bit_us;      // 1 bit start
    const unsigned IFG_US      = g_bit_us;      // inter-frame guard (idle high)
    uint64_t t = now_us();

    // START (low, 1 bit)
    tx_lo();                 t += START_US; busy_wait_until_us(t);

    // CMD (3 bits, LSB-first)
    for (int i = 0; i < 3; ++i) {
        ((cmd3 >> i) & 1) ? tx_hi() : tx_lo();
        t += g_bit_us; busy_wait_until_us(t);
    }

    // DATA (16 bits, LSB-first)
    uint16_t d = (uint16_t)data16;
    for (int i = 0; i < 16; ++i) {
        ((d >> i) & 1) ? tx_hi() : tx_lo();
        t += g_bit_us; busy_wait_until_us(t);
    }

    // STOP/IDLE (line high, at least 1 bit)
    tx_hi();
    t += IFG_US;  busy_wait_until_us(t);
}

/* ---- RX support for replies (CMD_GET_STATUS, CMD_GET_BIAS) ---------------- */

static int wait_start_low(unsigned timeout_us) {
    const uint64_t deadline = now_us() + timeout_us;
    // Ensure we see an actual transition: wait for high (idle), then low
    while (now_us() < deadline) {
        if (rx_rd() > 0) break;
    }
    while (now_us() < deadline) {
        if (rx_rd() == 0) return 0; // start bit detected
    }
    return -1; // timeout
}

// Receive one 19-bit frame (3-bit cmd LSB-first + 16-bit data LSB-first).
// Returns 0 on success; -1 on timeout or framing error.
int bias_recv_frame(uint8_t *cmd3, int16_t *data16, unsigned timeout_us) {
    if (wait_start_low(timeout_us) < 0) return -1;

    const uint64_t t0 = now_us();
    // Sample midpoint of the first DATA bit:
    // start bit lasts 1 bit; first data bit center is at 1.5 * bit_us after start edge
    uint64_t t_sample = t0 + (uint64_t)g_bit_us * 3 / 2;

    // Read 3 command bits, LSB-first
    uint8_t c = 0;
    for (int i = 0; i < 3; ++i) {
        busy_wait_until_us(t_sample);
        int v = rx_rd() & 1;
        c |= (uint8_t)(v << i);
        t_sample += g_bit_us;
    }

    // Read 16 data bits, LSB-first
    uint16_t d = 0;
    for (int i = 0; i < 16; ++i) {
        busy_wait_until_us(t_sample);
        int v = rx_rd() & 1;
        d |= (uint16_t)(v << i);
        t_sample += g_bit_us;
    }

    // Optional: small guard to let stop bit pass
    busy_wait_until_us(t_sample);

    if (cmd3)   *cmd3 = (uint8_t)(c & 0x7);
    if (data16) *data16 = (int16_t)d;
    return 0;
}

// Convenience: send a request and wait for its reply (blocking)
int bias_send_expect_reply(uint8_t tx_cmd3, int16_t tx_data16,
                           uint8_t *rx_cmd3, int16_t *rx_data16,
                           unsigned timeout_us)
{
    bias_send_frame(tx_cmd3, tx_data16);
    return bias_recv_frame(rx_cmd3, rx_data16, timeout_us);
}


int bias_get_status(bool *enabled, bool *is_negative, unsigned timeout_us) {
    uint8_t rc;
    int16_t rd; // wire format: bit0=en, bit1=pol (1=negative)
    int r = bias_send_expect_reply(CMD_GET_STATUS, 0, &rc, &rd, timeout_us);
    if (r < 0) return r;          // timeout / RX error
    if (rc != CMD_GET_STATUS) return -2; // unexpected command echo

    if (enabled)     *enabled     = (rd & 0x01) != 0;
    if (is_negative) *is_negative = (rd & 0x02) != 0;
    return 0;
}

int bias_get_bias(int32_t *hv_mV, unsigned timeout_us) {
    uint8_t rc;
    int16_t rd; // received in decivolts
    int r = bias_send_expect_reply(CMD_GET_BIAS, 0, &rc, &rd, timeout_us);
    if (r < 0) return r;
    if (rc != CMD_GET_BIAS) return -2;
    if (hv_mV) *hv_mV = rd * 100; // convert decivolts → millivolts
    return 0;
}
