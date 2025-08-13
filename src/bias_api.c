#include "bias_api.h"
#include "bias_control.h"
#include <stdint.h>
#include <stdbool.h>

static int g_started = 0;
static unsigned g_bit_us = 1000;


int bias_api_start_io(unsigned bit_us) {
    g_bit_us = bit_us ? bit_us : 1000;
    int r = bias_open(g_bit_us);
    if (r == 0) g_started = 1;
    return r; /* propagate low-level error codes (-1..-4) */
}

void bias_api_stop_io(void) {
    if (!g_started) return;
    bias_close();
    g_started = 0;
}


int bias_api_set_voltage_mV(int32_t mV) {
    if (!g_started) return BIAS_API_ERR_NOT_STARTED;
    if (mV > 32767) mV = 32767; /* wire is int16 */
    if (mV < -32768) mV = -32768;
    bias_send_frame(CMD_SET_VOLTAGE, (int16_t) mV);
    return BIAS_API_OK;
}

int bias_api_set_polarity(bool negative) {
    if (!g_started) return BIAS_API_ERR_NOT_STARTED;
    int16_t data = negative ? 1 : 0; /* bit0: 1=NEG */
    bias_send_frame(CMD_SET_POLARITY, data);
    return BIAS_API_OK;
}

int bias_api_hv_on(void) {
    if (!g_started) return BIAS_API_ERR_NOT_STARTED;
    bias_send_frame(CMD_ON, 0);
    return BIAS_API_OK;
}

int bias_api_hv_off(void) {
    if (!g_started) return BIAS_API_ERR_NOT_STARTED;
    bias_send_frame(CMD_OFF, 0);
    return BIAS_API_OK;
}

int bias_api_get_status(bool *enabled, bool *is_negative, unsigned timeout_us) {
    if (!g_started) return BIAS_API_ERR_NOT_STARTED;

    uint8_t rc;
    int16_t rd;
    int r = bias_send_expect_reply(CMD_GET_STATUS, 0, &rc, &rd, timeout_us);
    if (r < 0) return BIAS_API_ERR_TIMEOUT; /* low-level uses -1 for timeout */
    if (rc != CMD_GET_STATUS) return BIAS_API_ERR_BAD_ECHO;

    if (enabled) *enabled = (rd & 0x01) != 0;
    if (is_negative) *is_negative = (rd & 0x02) != 0;
    return BIAS_API_OK;
}

int bias_api_get_bias_mV(int32_t *mV, unsigned timeout_us) {
    if (!g_started) return BIAS_API_ERR_NOT_STARTED;

    uint8_t rc;
    int16_t rd; /* wire: decivolts */
    int r = bias_send_expect_reply(CMD_GET_BIAS, 0, &rc, &rd, timeout_us);
    if (r < 0) return BIAS_API_ERR_TIMEOUT;
    if (rc != CMD_GET_BIAS) return BIAS_API_ERR_BAD_ECHO;

    if (mV) *mV = (int32_t) rd * 100; /* dV -> mV */
    return BIAS_API_OK;
}
