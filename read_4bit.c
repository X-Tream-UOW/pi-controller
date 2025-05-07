#include <gpiod.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define GPIOCHIP "/dev/gpiochip0" // Adjust if needed with `gpiodetect`
#define NUM_BITS 4

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <0-15>\n", argv[0]);
        return 1;
    }

    int value = atoi(argv[1]);
    if (value < 0 || value > 15) {
        fprintf(stderr, "Error: value must be between 0 and 15\n");
        return 1;
    }

    unsigned int pins[NUM_BITS] = {2, 3, 4, 5};
    struct gpiod_chip *chip = gpiod_chip_open(GPIOCHIP);
    if (!chip) {
        perror("gpiod_chip_open");
        return 1;
    }

    struct gpiod_line_bulk lines;
    if (gpiod_chip_get_lines(chip, pins, NUM_BITS, &lines) < 0) {
        perror("gpiod_chip_get_lines");
        return 1;
    }

    if (gpiod_line_request_bulk_output(&lines, "4bit-output", NULL) < 0) {
        perror("gpiod_line_request_bulk_output");
        return 1;
    }

    int values[NUM_BITS];
    for (int i = 0; i < NUM_BITS; i++) {
        values[i] = (value >> i) & 1;
    }

    if (gpiod_line_set_value_bulk(&lines, values) < 0) {
        perror("gpiod_line_set_value_bulk");
        return 1;
    }

    printf("Outputting value 0x%X on GPIOs 2-5 (LSB -> MSB)\n", value);
    printf("Press Ctrl+C to exit.\n");

    while (1) {
        pause();
    }

    gpiod_chip_close(chip);
    return 0;
}
