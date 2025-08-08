#ifndef STREAMER_H
#define STREAMER_H

#include "data_logger.h"

typedef struct {
    SampleRecord *buffer;
    size_t count;
} StreamedSamples;

int get_streamed_samples(StreamedSamples *out);

void free_streamed_samples(StreamedSamples *r);

void reset_stream_state(void);

#endif
