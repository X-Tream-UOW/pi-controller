#include "downsampler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int downsample_file(const char* filename, size_t max_points, DownsampleResult* out) {
    if (!out || !filename || max_points == 0) return -1;

    out->data = NULL;
    out->count = 0;

    FILE* fp = fopen(filename, "rb");
    if (!fp) return -1;

    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    size_t total_records = filesize / sizeof(SampleRecord);
    fseek(fp, 0, SEEK_SET);

    if (total_records == 0 || max_points == 0) {
        fclose(fp);
        return -1;
    }

    // Read entire file into memory
    SampleRecord* all_records = malloc(total_records * sizeof(SampleRecord));
    if (!all_records) {
        fclose(fp);
        return -1;
    }

    if (fread(all_records, sizeof(SampleRecord), total_records, fp) != total_records) {
        free(all_records);
        fclose(fp);
        return -1;
    }

    fclose(fp);

    // Allocate output buffer
    size_t count = (total_records < max_points) ? total_records : max_points;
    out->data = malloc(count * sizeof(SampleRecord));
    if (!out->data) {
        free(all_records);
        return -1;
    }

    // Select evenly spaced samples
    double step = (double)total_records / count;
    for (size_t i = 0; i < count; ++i) {
        size_t idx = (size_t)(i * step);
        if (idx >= total_records) idx = total_records - 1;
        out->data[i] = all_records[idx];
    }

    out->count = count;
    free(all_records);
    return 0;
}

void free_downsampled(DownsampleResult* r) {
    if (r && r->data) {
        free(r->data);
        r->data = NULL;
        r->count = 0;
    }
}
