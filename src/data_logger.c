#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "data_logger.h"
#include "acquisition.h"

typedef struct SampleRecord {
    uint32_t index;
    uint16_t sample;
} __attribute__((packed)) SampleRecord;

int init_logger(DataLogger *logger, const char *filename) {
    logger->fp = fopen(filename, "wb");
    if (!logger->fp) return -1;

    logger->sample_counter = 0;
    logger->buffer = malloc(BUFFER_SAMPLES * sizeof(SampleRecord)); // Max expected buffer
    if (!logger->buffer) {
        fclose(logger->fp);
        return -1;
    }

    return 0;
}

void log_samples(DataLogger *logger, const uint8_t *rx_buf, size_t len) {
    if (!logger || !logger->buffer || !rx_buf) return;

    size_t num_samples = len / 2;
    SampleRecord *records = logger->buffer;

    for (size_t i = 0; i < num_samples; ++i) {
        uint16_t sample = ((uint16_t) rx_buf[i * 2] << 8) | rx_buf[i * 2 + 1];
        records[i].index = logger->sample_counter++;
        records[i].sample = sample;
    }

    fwrite(records, sizeof(SampleRecord), num_samples, logger->fp);
}

void close_logger(DataLogger *logger) {
    if (logger->fp) fclose(logger->fp);
    if (logger->buffer) free(logger->buffer);
    logger->buffer = NULL;
}
