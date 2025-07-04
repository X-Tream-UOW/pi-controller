#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <string.h>
#include <errno.h>
#include <gpiod.h>
#include <time.h>

#define SPI_DEV "/dev/spidev0.0"
#define SPI_SPEED 20000000
#define BITS_PER_WORD 16
#define CHUNK_SIZE 4096

#define GPIO_CHIP_NAME "gpiochip4"
#define READY_GPIO 5
#define ACK_GPIO   6
#define ACQ_GPIO  13

#define BUFFER_SAMPLES 65536
#define BUFFER_SIZE (BUFFER_SAMPLES * 2)

struct gpiod_chip *chip;
struct gpiod_line *ready_line;
struct gpiod_line *ack_line;
struct gpiod_line *acq_line;

int init_gpio() {
    chip = gpiod_chip_open_by_name(GPIO_CHIP_NAME);
    if (!chip) return -1;

    ready_line = gpiod_chip_get_line(chip, READY_GPIO);
    ack_line   = gpiod_chip_get_line(chip, ACK_GPIO);
    acq_line   = gpiod_chip_get_line(chip, ACQ_GPIO);

    if (!ready_line || !ack_line || !acq_line) {
        gpiod_chip_close(chip);
        return -1;
    }

    if (gpiod_line_request_input(ready_line, "ready_in") < 0 ||
        gpiod_line_request_output(ack_line, "ack_out", 0) < 0 ||
        gpiod_line_request_output(acq_line, "acq_out", 0) < 0) {
        gpiod_chip_close(chip);
        return -1;
    }

    return 0;
}

void cleanup_gpio() {
    if (acq_line) gpiod_line_release(acq_line);
    if (ack_line) gpiod_line_release(ack_line);
    if (ready_line) gpiod_line_release(ready_line);
    if (chip) gpiod_chip_close(chip);
}

void wait_for_ready() {
    while (gpiod_line_get_value(ready_line) == 0) {
        usleep(10);
    }
}

void send_ack_pulse() {
    gpiod_line_set_value(ack_line, 1);
    usleep(10);
    gpiod_line_set_value(ack_line, 0);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <duration_seconds>\n", argv[0]);
        return 1;
    }

    int duration = atoi(argv[1]);
    if (duration <= 0) {
        fprintf(stderr, "Invalid duration: %s\n", argv[1]);
        return 1;
    }

    if (init_gpio() != 0) {
        fprintf(stderr, "GPIO initialization failed\n");
        return 1;
    }

    int fd = open(SPI_DEV, O_RDWR);
    if (fd < 0) {
        perror("open(SPI_DEV)");
        cleanup_gpio();
        return 1;
    }

    uint8_t mode = SPI_MODE_1;
    uint8_t bits = BITS_PER_WORD;
    uint32_t speed = SPI_SPEED;

    if (ioctl(fd, SPI_IOC_WR_MODE, &mode) == -1 ||
        ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits) == -1 ||
        ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) == -1) {
        perror("SPI ioctl");
        close(fd);
        cleanup_gpio();
        return 1;
    }

    uint8_t tx_buf[BUFFER_SIZE] = {0};
    uint8_t rx_buf[BUFFER_SIZE];

    gpiod_line_set_value(acq_line, 1);

    struct timespec start, now;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);

    while (1) {
        clock_gettime(CLOCK_MONOTONIC_RAW, &now);
        double elapsed_ms = (now.tv_sec - start.tv_sec) * 1000.0 +
                            (now.tv_nsec - start.tv_nsec) / 1.0e6;

        if (elapsed_ms >= duration * 1000.0)
            break;

        for (int offset = 0; offset < BUFFER_SIZE; offset += CHUNK_SIZE) {
            struct spi_ioc_transfer tr = {
                .tx_buf = (unsigned long)(tx_buf + offset),
                .rx_buf = (unsigned long)(rx_buf + offset),
                .len = CHUNK_SIZE,
                .speed_hz = speed,
                .bits_per_word = bits,
            };

            if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) < 1) {
                perror("SPI transfer failed");
                break;
            }

        }


        send_ack_pulse();
    }

    gpiod_line_set_value(acq_line, 0);
    close(fd);
    cleanup_gpio();
    return 0;
}
