#ifndef ACQUISITION_H
#define ACQUISITION_H

double acquire_buffers(int fd, int num_buffers, volatile sig_atomic_t *stop_flag);

#endif
