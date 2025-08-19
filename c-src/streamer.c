/* This file implements the streaming of the downsampled file. It keeps track of the last access to the file to
 avoid re-sending the same data. */

#include "streamer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data_logger.h"
extern DataLogger current_logger;

static uint32_t last_index = 0;
static char current_filename[256] = {0};

int get_streamed_samples(StreamedSamples *out) {
    const char *ds_filename = current_logger.ds_path;
    if (!ds_filename || !out) return -1;

    // If the file changed, reset tracking
    if (strcmp(ds_filename, current_filename) != 0) {
        strncpy(current_filename, ds_filename, sizeof(current_filename) - 1);
        current_filename[sizeof(current_filename) - 1] = '\0';
        last_index = 0;
    }

    FILE *fp = fopen(ds_filename, "rb");
    if (!fp) return -1;

    fseek(fp, 0, SEEK_END);
    size_t total_records = ftell(fp) / sizeof(SampleRecord);
    rewind(fp);

    if (last_index >= total_records) {
        fclose(fp);
        out->buffer = NULL;
        out->count = 0;
        return 0;
    }

    size_t new_count = total_records - last_index;
    SampleRecord *buffer = malloc(new_count * sizeof(SampleRecord));
    if (!buffer) {
        fclose(fp);
        return -1;
    }

    fseek(fp, last_index * sizeof(SampleRecord), SEEK_SET);
    fread(buffer, sizeof(SampleRecord), new_count, fp);
    fclose(fp);

    out->buffer = buffer;
    out->count = new_count;
    last_index = total_records;
    printf("Returning %zu samples from offset %u\n", new_count, last_index);
    fflush(stdout);
    return 0;
}

void free_streamed_samples(StreamedSamples *r) {
    if (r && r->buffer) {
        free(r->buffer);
        r->buffer = NULL;
        r->count = 0;
    }
}

void reset_stream_state(void) {
    last_index = 0;
    current_filename[0] = '\0';
}
