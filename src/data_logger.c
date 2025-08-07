#include "data_logger.h"
#include "acquisition.h"  // For BUFFER_SAMPLES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DOWNSAMPLE_FACTOR 200000

int init_logger(DataLogger *logger, const char *base_name) {
    char bin_filename[MAX_FILENAME_LEN];

    snprintf(bin_filename, MAX_FILENAME_LEN, "%s.bin", base_name);
    snprintf(logger->ds_path, MAX_FILENAME_LEN, "%s_ds.bin", base_name);

    logger->fp = fopen(bin_filename, "wb");
    if (!logger->fp) return -1;

    logger->ds_fp = fopen(logger->ds_path, "wb");
    if (!logger->ds_fp) {
        fclose(logger->fp);
        return -1;
    }

    logger->sample_counter = 0;
    logger->buffer = malloc(BUFFER_SAMPLES * sizeof(SampleRecord));
    if (!logger->buffer) {
        fclose(logger->fp);
        fclose(logger->ds_fp);
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

    for (size_t i = 0; i < num_samples; i += DOWNSAMPLE_FACTOR) {
        fwrite(&records[i], sizeof(SampleRecord), 1, logger->ds_fp);
    }
    fflush(logger->ds_fp);
    fflush(logger->fp);
}

void close_logger(DataLogger *logger) {
    if (logger->fp) fclose(logger->fp);
    if (logger->ds_fp) fclose(logger->ds_fp);
    if (logger->buffer) free(logger->buffer);
    logger->buffer = NULL;
}
