#include <gpiod.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

#define CHIP "/dev/gpiochip0"

#define DB_COUNT 1
unsigned int db_pins[DB_COUNT] = { 5 };

const unsigned int CONVST_PIN = 22;
const unsigned int CS_PIN     = 23;
const unsigned int RD_PIN     = 24;
const unsigned int BUSY_PIN   = 21;

void error_exit(const char *msg) {
    perror(msg);
    exit(1);
}

double timespec_diff_sec(const struct timespec *start, const struct timespec *end) {
    return (end->tv_sec - start->tv_sec) + (end->tv_nsec - start->tv_nsec) / 1e9;
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

    struct timespec start_time, now;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    int sample_count = 0;

    while (1) {
        gpiod_line_set_value(cs, 0);
        gpiod_line_set_value(convst, 0);
        gpiod_line_set_value(convst, 1);

        while (gpiod_line_get_value(busy) == 1);

        gpiod_line_set_value(rd, 0);
        int values[DB_COUNT];
        gpiod_line_get_value_bulk(&db_lines, values);
        gpiod_line_set_value(rd, 1);
        gpiod_line_set_value(cs, 1);

        sample_count++;

        clock_gettime(CLOCK_MONOTONIC, &now);
        double elapsed = timespec_diff_sec(&start_time, &now);
        if (elapsed >= 1.0) {
            printf("Sampling rate: %.0f samples/sec\n", sample_count / elapsed);
            sample_count = 0;
            start_time = now;
        }
    }

    gpiod_chip_close(chip);
    return 0;
}
