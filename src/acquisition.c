#include <linux/spi/spidev.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <sys/ioctl.h>

#include "acquisition.h"
#include "gpio_handler.h"

#define BUFFER_SAMPLES 65536
#define BUFFER_SIZE (BUFFER_SAMPLES * 2)
#define CHUNK_SIZE 4096

double acquire_buffers(int fd, int num_buffers, volatile sig_atomic_t *stop_flag) {
    uint8_t tx_buf[BUFFER_SIZE] = {0};
    uint8_t rx_buf[BUFFER_SIZE];
    uint64_t total = 0;
    uint64_t total_samples = 0;

    set_acq(1);  // Start acquisition

    for (int i = 0; i < num_buffers && !(*stop_flag); ++i) {
        wait_for_ready();
        if (*stop_flag) break;

        for (int offset = 0; offset < BUFFER_SIZE; offset += CHUNK_SIZE) {
            struct spi_ioc_transfer tr = {
                .tx_buf = (unsigned long)(tx_buf + offset),
                .rx_buf = (unsigned long)(rx_buf + offset),
                .len = CHUNK_SIZE,
                .speed_hz = 20000000,
                .bits_per_word = 16,
            };

            if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) < 1) {
                perror("SPI transfer failed");
                set_acq(0);
                return -1;
            }
        }

        for (int j = 0; j < BUFFER_SIZE; j += 2) {
            uint16_t sample = ((uint16_t)rx_buf[j] << 8) | rx_buf[j + 1];
            total += sample;
        }

        total_samples += BUFFER_SAMPLES;
        send_ack_pulse();
    }

    set_acq(0);  // End acquisition

    return (total_samples > 0) ? (double)total / total_samples : -1;
}
