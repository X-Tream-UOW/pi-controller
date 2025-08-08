#ifndef ACQUISITION_H
#define ACQUISITION_H

#include <stdint.h>
#include <signal.h>

#define BUFFER_SAMPLES 65536

double acquire_buffers(int fd, int num_buffers, volatile sig_atomic_t *stop_flag, const char *output_path);

#endif
