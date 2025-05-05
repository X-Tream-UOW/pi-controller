#include <gpiod.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#define CHIP "/dev/gpiochip0" // gpiodetect to chose the right one, on rpi5 it's the 0

#define DB_COUNT 16

unsigned int db_pins[DB_COUNT] = {
    5, 6, 7, 8, 9, 10, 11, 12,
    13, 14, 15, 16, 17, 18, 19, 20
};

const unsigned int CONVST_PIN = 22;
const unsigned int CS_PIN     = 23;
const unsigned int RD_PIN     = 24;
const unsigned int BUSY_PIN   = 21;

void error_exit(const char *msg) {
    perror(msg);
    exit(1);
}

int main() {
    struct gpiod_chip *chip = gpiod_chip_open(CHIP);
    if (!chip) error_exit("gpiod_chip_open");

    struct gpiod_line *convst = gpiod_chip_get_line(chip, CONVST_PIN);
    struct gpiod_line *cs     = gpiod_chip_get_line(chip, CS_PIN);
    struct gpiod_line *rd     = gpiod_chip_get_line(chip, RD_PIN);
    struct gpiod_line *busy   = gpiod_chip_get_line(chip, BUSY_PIN);

    if (gpiod_line_request_output(convst, "reader", 1) ||
        gpiod_line_request_output(cs, "reader", 1) ||
        gpiod_line_request_output(rd, "reader", 1) ||
        gpiod_line_request_input(busy, "reader")) {
        error_exit("request control lines");
    }

    struct gpiod_line_bulk db_lines;
    gpiod_chip_get_lines(chip, db_pins, DB_COUNT, &db_lines);
    if (gpiod_line_request_bulk_input(&db_lines, "reader"))
        error_exit("request DB lines");

    printf("Starting acquisition...\n");

    while (1) {
        gpiod_line_set_value(cs, 0);
        gpiod_line_set_value(convst, 0);
        usleep(1);
        gpiod_line_set_value(convst, 1);

        while (gpiod_line_get_value(busy) == 1);   // Wait for BUSY to go LOW

        gpiod_line_set_value(rd, 0);
        usleep(1);
        int values[DB_COUNT];
        gpiod_line_get_value_bulk(&db_lines, values);
        gpiod_line_set_value(rd, 1);
        gpiod_line_set_value(cs, 1);

        uint16_t sample = 0;
        for (int i = 0; i < DB_COUNT; ++i) {   // Assembling 16-bit word
            sample |= (values[i] << i);
        }

        printf("Sample: %u (0x%04X)\n", sample, sample);
        usleep(1 * 1000 * 1000);
    }

    gpiod_chip_close(chip);
    return 0;
}