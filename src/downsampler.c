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

    if (total_records == 0) {
        fclose(fp);
        return -1;
    }

    size_t step = total_records / max_points;
    if (step < 1) step = 1;

    size_t max_alloc = max_points * 2;
    out->data = malloc(max_alloc * sizeof(SampleRecord));
    if (!out->data) {
        fclose(fp);
        return -1;
    }

    size_t out_idx = 0;
    size_t processed_windows = 0;
    size_t max_windows = max_points;

    for (size_t i = 0; i < total_records && processed_windows < max_windows; i += step) {
        SampleRecord r, min_rec = {0, 0xFFFF}, max_rec = {0, 0};
        size_t window_end = i + step;
        if (window_end > total_records) window_end = total_records;

        for (size_t j = i; j < window_end; ++j) {
            fseek(fp, j * sizeof(SampleRecord), SEEK_SET);
            if (fread(&r, sizeof(SampleRecord), 1, fp) != 1)
                break;

            if (r.sample < min_rec.sample) min_rec = r;
            if (r.sample > max_rec.sample) max_rec = r;
        }

        if (out_idx + 2 <= max_alloc) {
            out->data[out_idx++] = min_rec;
            out->data[out_idx++] = max_rec;
        } else {
            break;
        }

        processed_windows++;
    }

    out->count = out_idx;
    fclose(fp);
    return 0;
}

void free_downsampled(DownsampleResult* r) {
    if (r && r->data) {
        free(r->data);
        r->data = NULL;
        r->count = 0;
    }
}
