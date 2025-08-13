#pragma once
#include <stdbool.h>
#include <stdint.h>

/* Public API return codes (negative = error) */
enum {
    BIAS_API_OK = 0,
    BIAS_API_ERR_NOT_STARTED = -10,
    BIAS_API_ERR_TIMEOUT     = -11,
    BIAS_API_ERR_BAD_ECHO    = -12
};

/* Lifecycle */
int  bias_api_start_io(unsigned bit_us);   /* calls bias_open(bit_us) */
void bias_api_stop_io(void);               /* calls bias_close() */

/* 6 high-level endpoints */
int  bias_api_set_voltage_mV(int32_t mV);            /* sends CMD_SET_VOLTAGE */
int  bias_api_set_polarity(bool negative);           /* true=NEG, false=POS */
int  bias_api_hv_on(void);                           /* sends CMD_ON */
int  bias_api_hv_off(void);                          /* sends CMD_OFF */
int  bias_api_get_status(bool *enabled, bool *is_negative, unsigned timeout_us);
int  bias_api_get_bias_mV(int32_t *mV, unsigned timeout_us);
