#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "data_logger.h"

typedef struct {
    uint32_t index;
    uint16_t sample;
} __attribute__((packed)) SampleRecord;

int init_logger(DataLogger *logger, const char *filename) {
    logger->fp = fopen(filename, "wb");
    if (!logger->fp) return -1;
    logger->sample_counter = 0;
    return 0;
}

void log_samples(DataLogger *logger, const uint8_t *rx_buf, size_t len) {
    size_t num_samples = len / 2;
    SampleRecord *records = malloc(num_samples * sizeof(SampleRecord));
    if (!records) return;

    for (size_t i = 0; i < num_samples; ++i) {
        uint16_t sample = ((uint16_t)rx_buf[i * 2] << 8) | rx_buf[i * 2 + 1];
        records[i].index = logger->sample_counter++;
        records[i].sample = sample;
    }

    fwrite(records, sizeof(SampleRecord), num_samples, logger->fp);
    free(records);
}

void close_logger(DataLogger *logger) {
    if (logger->fp) fclose(logger->fp);
}