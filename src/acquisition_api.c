#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>

#include "spi_handler.h"
#include "gpio_handler.h"
#include "acquisition.h"

#define OUTPUT_PATH "output_data.bin"
#define BUFFER_DURATION_MS 65.536 // For 65536 samples at 1 MSPS

static int configured_num_buffers = 0;
static volatile sig_atomic_t stop_requested = 0;

void set_duration_ms(int duration_ms) {
    configured_num_buffers = (int)ceil(duration_ms / BUFFER_DURATION_MS);
    if (configured_num_buffers < 1) {
        configured_num_buffers = 1;
    }
}

void handle_signal(int signum) {
    stop_requested = 1;
}

void start_acquisition(void) {
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    if (configured_num_buffers <= 0) {
        fprintf(stderr, "Acquisition duration not configured. Call set_duration_ms() first.\n");
        return;
    }

    if (init_gpio() != 0) {
        fprintf(stderr, "GPIO initialization failed\n");
        return;
    }

    int fd = init_spi();
    if (fd < 0) {
        cleanup_gpio();
        return;
    }

    double mean = acquire_buffers(fd, configured_num_buffers, &stop_requested, OUTPUT_PATH);
    if (!stop_requested)
        printf("Mean sample value: %.2f\n", mean);

    close(fd);
    cleanup_gpio();

    if (stop_requested)
        fprintf(stderr, "\nInterrupted – cleanup done.\n");
}
