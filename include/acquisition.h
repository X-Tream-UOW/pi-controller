#ifndef ACQUISITION_H
#define ACQUISITION_H

#include <stdint.h>
#include <signal.h>

double acquire_buffers(int fd, int num_buffers, volatile sig_atomic_t *stop_flag);

#endif
