#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


typedef struct {
    FILE *fp;
    FILE *ds_fp;
    uint32_t sample_counter;
    struct SampleRecord *buffer;
} DataLogger;

typedef struct SampleRecord {
    uint32_t index;
    uint16_t sample;
} __attribute__((packed)) SampleRecord;

int init_logger(DataLogger *logger, const char *filename);

void log_samples(DataLogger *logger, const uint8_t *rx_buf, size_t len);

void close_logger(DataLogger *logger);

#endif
