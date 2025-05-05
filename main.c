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

    struct gpiod_line_bulk ctrl_lines;
    struct gpiod_line *ctrl_array[3];
    ctrl_array[0] = gpiod_chip_get_line(chip, CS_PIN);
    ctrl_array[1] = gpiod_chip_get_line(chip, CONVST_PIN);
    ctrl_array[2] = gpiod_chip_get_line(chip, RD_PIN);

    gpiod_line_bulk_init(&ctrl_lines);
    gpiod_line_bulk_add_array(&ctrl_lines, ctrl_array, 3);

    if (gpiod_line_request_bulk_output(&ctrl_lines, "reader", (int[]){1, 1, 1}))
        error_exit("request control lines");

    struct gpiod_line *busy = gpiod_chip_get_line(chip, BUSY_PIN);
    if (gpiod_line_request_falling_edge_events(busy, "reader"))
        error_exit("request BUSY events");

    struct gpiod_line_bulk db_lines;
    gpiod_chip_get_lines(chip, db_pins, DB_COUNT, &db_lines);
    if (gpiod_line_request_bulk_input(&db_lines, "reader"))
        error_exit("request DB lines");

    printf("Starting optimized acquisition with event-based BUSY...\n");

    struct timespec start_time, now;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    int sample_count = 0;

    while (1) {
        gpiod_line_set_value(ctrl_array[0], 0); // CS = 0
        gpiod_line_set_value(ctrl_array[1], 0); // CONVST = 0
        gpiod_line_set_value(ctrl_array[1], 1); // CONVST = 1

        struct timespec timeout = {1, 0};
        if (gpiod_line_event_wait(busy, &timeout) <= 0)
            continue;

        struct gpiod_line_event ev;
        gpiod_line_event_read(busy, &ev);

        gpiod_line_set_value(ctrl_array[2], 0); // RD = 0
        int values[DB_COUNT];
        gpiod_line_get_value_bulk(&db_lines, values);
        gpiod_line_set_value(ctrl_array[2], 1); // RD = 1
        gpiod_line_set_value(ctrl_array[0], 1); // CS = 1

        uint16_t sample = 0;
        for (int i = 0; i < DB_COUNT; ++i)
            sample |= (values[i] << i);

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
