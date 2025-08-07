#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t index;
    uint16_t sample;
} __attribute__((packed)) SampleRecord;

typedef struct {
    SampleRecord* data;
    size_t count;
} DownsampleResult;

int downsample_file(const char* filename, size_t max_points, DownsampleResult* out);
void free_downsampled(DownsampleResult* r);
