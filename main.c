#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <time.h>
#include <locale.h>

#define SPI_DEVICE "/dev/spidev0.0"
#define BUFFER_SIZE 16384

static uint16_t buffer[BUFFER_SIZE];

double timespec_diff_ms(const struct timespec *start, const struct timespec *end) {
    return (end->tv_sec - start->tv_sec) * 1000.0 +
           (end->tv_nsec - start->tv_nsec) / 1000000.0;
}

void configure_spi(int fd) {
    uint8_t mode = SPI_MODE_0;
    uint8_t bits_per_word = 16;
    uint32_t speed = 1 * 1000 * 1000;

    if (ioctl(fd, SPI_IOC_WR_MODE, &mode) < 0) {
        perror("Failed to set SPI mode");
        exit(EXIT_FAILURE);
    }
    if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits_per_word) < 0) {
        perror("Failed to set bits per word");
        exit(EXIT_FAILURE);
    }
    if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        perror("Failed to set SPI speed");
        exit(EXIT_FAILURE);
    }
}


ssize_t read_spi_buffer(int fd, uint16_t *buffer, size_t len) {  /* TODO : improve indian-ness coherence & ensure cs pin management*/
    struct spi_ioc_transfer transfer = {
        .tx_buf = 0,
        .rx_buf = (unsigned long) buffer,
        .len = len,
        .delay_usecs = 0,
        .speed_hz = 0, // Use default
        .bits_per_word = 0 // Use default
    };

    const int ret = ioctl(fd, SPI_IOC_MESSAGE(1), &transfer);
    if (ret < 0) {
        perror("SPI transfer error");
        return -1;
    }

    return ret;
}

int main() {
    setlocale(LC_NUMERIC, "");

    const int fd = open(SPI_DEVICE, O_RDWR);
    if (fd < 0) {
        perror("Failed to open SPI device");
        return 1;
    }

    configure_spi(fd);

    printf("SPI slave ready, waiting...\n");

    struct timespec t_start, t_now;
    clock_gettime(CLOCK_MONOTONIC, &t_start);

    const double display_interval_ms = 2000.0;
    double total_samples = 0;
    double total_time_ms = 0;

    while (1) {
        ssize_t bytes_read = read_spi_buffer(fd, buffer, sizeof(buffer));
        if (bytes_read < 0) {
            break;
        }

        if (bytes_read != sizeof(buffer)) {
            fprintf(stderr, "Warning: partial read (%zd bytes)\n", bytes_read);
            continue;
        }

        const double samples = bytes_read / sizeof(uint16_t);
        total_samples += samples;

        clock_gettime(CLOCK_MONOTONIC, &t_now);
        total_time_ms = timespec_diff_ms(&t_start, &t_now);

        if (total_time_ms >= display_interval_ms) {
            const double seconds = total_time_ms / 1000.0;
            const double avg_frequency = total_samples / seconds;

            printf("Sampling frequency: %'.2f samples/sec\n", avg_frequency);

            clock_gettime(CLOCK_MONOTONIC, &t_start);
            total_samples = 0;
            total_time_ms = 0;
        }
    }

    close(fd);
    return 0;
}
