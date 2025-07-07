#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    FILE *fp;
    uint64_t sample_counter;
} DataLogger;

int init_logger(DataLogger *logger, const char *filename);
void log_samples(DataLogger *logger, const uint8_t *rx_buf, size_t len);
void close_logger(DataLogger *logger);

#endif
