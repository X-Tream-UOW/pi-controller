#include "spi_handler.h"
#include "gpio_handler.h"
#include "acquisition.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

volatile sig_atomic_t stop_requested = 0;

void handle_signal(int signum) {
    stop_requested = 1;
}

int main(int argc, char *argv[]) {
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <num_buffers>\n", argv[0]);
        return 1;
    }

    int num_buffers = atoi(argv[1]);
    if (num_buffers <= 0) {
        fprintf(stderr, "Invalid number of buffers: %s\n", argv[1]);
        return 1;
    }

    if (init_gpio() != 0) {
        fprintf(stderr, "GPIO initialization failed\n");
        return 1;
    }

    int fd = init_spi();
    if (fd < 0) {
        cleanup_gpio();
        return 1;
    }

    double mean = acquire_buffers(fd, num_buffers, &stop_requested);
    if (!stop_requested)
        printf("Mean sample value: %.2f\n", mean);

    close(fd);
    cleanup_gpio();

    if (stop_requested) {
        fprintf(stderr, "\nInterrupted – cleanup done.\n");
        return 130;
    }

    return 0;
}
