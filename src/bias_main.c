#include "bias_control.h"
#include <stdio.h>
#include <unistd.h>

int main(void) {
    const unsigned BIT_US = 1000;

    if (bias_open(BIT_US) != 0) {
        fprintf(stderr, "bias_open() failed\n");
        return 1;
    }

    bias_send_frame(CMD_SET_VOLTAGE, (uint16_t)500);

    sleep(1);

    bias_send_frame(CMD_SET_POLARITY, 0);

    sleep(1);

    bias_send_frame(CMD_ON, 0);

    sleep(3);

    bias_send_frame(CMD_OFF, 0);

    bias_close();
    return 0;
}
