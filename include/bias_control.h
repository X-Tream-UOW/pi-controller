#pragma once
#include <stdint.h>
#include <stdbool.h>

#define PI_TX 15
#define PI_RX 14

#define CMD_SET_VOLTAGE  0x01  // DATA=int16 mV
#define CMD_SET_POLARITY 0x02  // DATA bit0: 0=POS, 1=NEG
#define CMD_ON           0x03  // DATA ignored
#define CMD_OFF          0x04  // DATA ignored
#define CMD_GET_STATUS   0x05  // reply DATA: flags (bit0=en, bit1=pol)
#define CMD_GET_BIAS     0x06  // reply DATA: hv in deci-volts (int16)

// Open GPIO and set the bit period (microseconds per bit). Returns 0 on success.
int bias_open();

// Close GPIO.
void bias_close(void);

// Send one frame: start (1*bit), 3-bit CMD (LSB-first), 16-bit DATA (LSB-first), then idle-high.
// Blocking, absolute-timed.
void bias_send_frame(uint8_t cmd3, int16_t data16);

int bias_send_expect_reply(uint8_t tx_cmd3, int16_t tx_data16,
                           uint8_t *rx_cmd3, int16_t *rx_data16,
                           unsigned timeout_us);
