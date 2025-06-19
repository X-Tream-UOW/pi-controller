#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <string.h>
#include <errno.h>
#include <gpiod.h>

#define SPI_DEV "/dev/spidev0.0"
#define SPI_SPEED 2000000  // 2 MHz
#define BITS_PER_WORD 8
#define CHUNK_SIZE 4096

#define GPIO_CHIP_NAME "gpiochip4"  // For Raspberry Pi 5: GPIO 0–31 = gpiochip4
#define READY_GPIO 5                // BCM GPIO 5 → line 5 in gpiochip4

#define BUFFER_SAMPLES 65536
#define BUFFER_SIZE (BUFFER_SAMPLES * 2)  // 16-bit samples

struct gpiod_chip *chip;
struct gpiod_line *ready_line;

int init_ready_gpio() {
    chip = gpiod_chip_open_by_name(GPIO_CHIP_NAME);
    if (!chip) {
        perror("Failed to open GPIO chip");
        return -1;
    }

    ready_line = gpiod_chip_get_line(chip, READY_GPIO);
    if (!ready_line) {
        perror("Failed to get GPIO line");
        gpiod_chip_close(chip);
        return -1;
    }

    if (gpiod_line_request_input(ready_line, "spi_ready") < 0) {
        perror("Failed to request GPIO line as input");
        gpiod_chip_close(chip);
        return -1;
    }

    return 0;
}

void wait_for_ready_high() {
    while (gpiod_line_get_value(ready_line) == 0) {
        usleep(10);  // 10 microseconds
    }
}

void cleanup_gpio() {
    if (ready_line) gpiod_line_release(ready_line);
    if (chip) gpiod_chip_close(chip);
}

int main() {
    if (init_ready_gpio() != 0) {
        return 1;
    }

    int fd = open(SPI_DEV, O_RDWR);
    if (fd < 0) {
        perror("Failed to open SPI device");
        cleanup_gpio();
        return 1;
    }

    uint8_t mode = SPI_MODE_0;
    uint8_t bits = BITS_PER_WORD;
    uint32_t speed = SPI_SPEED;

    if (ioctl(fd, SPI_IOC_WR_MODE, &mode) == -1 ||
        ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits) == -1 ||
        ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) == -1) {
        perror("SPI configuration failed");
        close(fd);
        cleanup_gpio();
        return 1;
        }

    uint8_t tx_buf[BUFFER_SIZE] = {0};  // dummy transmit buffer
    uint8_t rx_buf[BUFFER_SIZE];

    while (1) {
        printf("Waiting for READY (GPIO %d)...\n", READY_GPIO);
        wait_for_ready_high();

        printf("READY detected. Starting SPI transfer of %d bytes...\n", BUFFER_SIZE);

        // Transfer in chunks
        for (int offset = 0; offset < BUFFER_SIZE; offset += CHUNK_SIZE) {
            int chunk_len = (BUFFER_SIZE - offset > CHUNK_SIZE) ? CHUNK_SIZE : (BUFFER_SIZE - offset);

            struct spi_ioc_transfer tr_chunk = {
                .tx_buf = (unsigned long)(tx_buf + offset),
                .rx_buf = (unsigned long)(rx_buf + offset),
                .len = chunk_len,
                .delay_usecs = 0,
                .speed_hz = speed,
                .bits_per_word = bits,
            };

            if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr_chunk) < 1) {
                perror("SPI chunk transfer failed");
                break;
            }
        }

        printf("Transfer complete. First 10 samples:\n");
        for (int i = 0; i < 10; i++) {
            uint16_t sample = ((uint16_t)rx_buf[2*i] << 8) | rx_buf[2*i + 1];
            printf("Sample[%d] = 0x%04X\n", i, sample);
        }

        printf("Waiting for next buffer...\n");
    }

    close(fd);
    cleanup_gpio();
    return 0;
}

