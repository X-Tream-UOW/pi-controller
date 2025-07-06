#include "gpio_handler.h"
#include <gpiod.h>
#include <stdio.h>
#include <unistd.h>

#define GPIO_CHIP_NAME "gpiochip4"
#define READY_GPIO 5
#define ACK_GPIO   6
#define ACQ_GPIO   13

static struct gpiod_chip *chip;
static struct gpiod_line *ready_line;
static struct gpiod_line *ack_line;
static struct gpiod_line *acq_line;

int init_gpio(void) {
    chip = gpiod_chip_open_by_name(GPIO_CHIP_NAME);
    if (!chip) return -1;

    ready_line = gpiod_chip_get_line(chip, READY_GPIO);
    ack_line   = gpiod_chip_get_line(chip, ACK_GPIO);
    acq_line   = gpiod_chip_get_line(chip, ACQ_GPIO);

    if (!ready_line || !ack_line || !acq_line) return -1;

    if (gpiod_line_request_input(ready_line, "ready_in") < 0 ||
        gpiod_line_request_output(ack_line, "ack_out", 0) < 0 ||
        gpiod_line_request_output(acq_line, "acq_out", 0) < 0) {
        return -1;
        }

    return 0;
}

void cleanup_gpio(void) {
    if (acq_line) gpiod_line_release(acq_line);
    if (ack_line) gpiod_line_release(ack_line);
    if (ready_line) gpiod_line_release(ready_line);
    if (chip) gpiod_chip_close(chip);
}

void wait_for_ready(void) {
    while (gpiod_line_get_value(ready_line) == 0) usleep(10);
}

void send_ack_pulse(void) {
    gpiod_line_set_value(ack_line, 1);
    usleep(10);
    gpiod_line_set_value(ack_line, 0);
}

void set_acq(int value) {
    gpiod_line_set_value(acq_line, value);
}
