#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

#include "spi_handler.h"
#include "gpio_handler.h"
#include "acquisition.h"

#define BUFFER_DURATION_MS 65.536 // 65536 samples at 1 MSPS
#define MAX_FILENAME_LEN 128

__attribute__((weak)) extern int running_as_executable;

static int configured_num_buffers = 0;
static volatile sig_atomic_t stop_requested = 0;
static char custom_filename[MAX_FILENAME_LEN] = {0};
static char resolved_output_path[MAX_FILENAME_LEN] = {0};

void set_duration_ms(int duration_ms) {
    configured_num_buffers = (int)ceil(duration_ms / BUFFER_DURATION_MS);
    if (configured_num_buffers < 1)
        configured_num_buffers = 1;
}

void set_custom_filename(const char* user_input) {
    if (!user_input || strlen(user_input) == 0 || strlen(user_input) >= MAX_FILENAME_LEN - 4) {
        fprintf(stderr, "Invalid custom filename — using default.\n");
        fflush(stderr);
        custom_filename[0] = '\0';
        return;
    }

    // Copy and ensure it ends with .bin
    strncpy(custom_filename, user_input, MAX_FILENAME_LEN - 1);
    custom_filename[MAX_FILENAME_LEN - 1] = '\0';

    // Append ".bin" if not present
    const char *dot = strrchr(custom_filename, '.');
    if (!dot || strcmp(dot, ".bin") != 0) {
        strncat(custom_filename, ".bin", MAX_FILENAME_LEN - strlen(custom_filename) - 1);
    }
}

static void generate_default_filename(char *buf, size_t buflen) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (!t) {
        snprintf(buf, buflen, "fallback_output.bin");
        return;
    }

    int duration_ms = (int)(configured_num_buffers * BUFFER_DURATION_MS);
    snprintf(buf, buflen,
             "%02d_%02d_%04d_%02d_%02d_%02d_ACQ%dms.bin",
             t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
             t->tm_hour, t->tm_min, t->tm_sec, duration_ms);
}

static const char* get_output_path(void) {
    if (custom_filename[0] != '\0') {
        return custom_filename;
    } else {
        generate_default_filename(resolved_output_path, sizeof(resolved_output_path));
        return resolved_output_path;
    }
}

void handle_signal(int signum) {
    stop_requested = 1;
}

void start_acquisition(void) {

  	if (&running_as_executable && running_as_executable) {
    	signal(SIGINT, handle_signal);
    	signal(SIGTERM, handle_signal);
    }

    if (configured_num_buffers <= 0) {
        fprintf(stderr, "Acquisition duration not configured. Call set_duration_ms() first.\n");
        fflush(stderr);
        return;
    }

    const char *output_path = get_output_path();

    if (init_gpio() != 0) {
        fprintf(stderr, "GPIO initialization failed\n");
        fflush(stderr);
        return;
    }

    int fd = init_spi();
    if (fd < 0) {
        cleanup_gpio();
        return;
    }

    printf("→ Saving acquisition to: %s\n", output_path);
	fflush(stdout);

    double mean = acquire_buffers(fd, configured_num_buffers, &stop_requested, output_path);
    if (!stop_requested)
        printf("Mean sample value: %.2f\n", mean);

    fflush(stdout);

    close(fd);
    cleanup_gpio();

    if (stop_requested)
        fprintf(stderr, "\nInterrupted – cleanup done.\n");

    fflush(stderr);
    printf("Acquisition complete, returning.\n");
    return;
}

void stop_acquisition(void) { // TODO : investigate why we can't start again after this (RD line toggles)
    stop_requested = 1;
}
