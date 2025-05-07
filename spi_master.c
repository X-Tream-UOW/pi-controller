#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <stdbool.h>

#define SPI_DEV "/dev/spidev0.0"
#define SPEED 10000
#define MODE SPI_MODE_0

int main() {
    int fd = open(SPI_DEV, O_RDWR);
    if (fd < 0) {
        perror("Failed to open SPI device");
        return 1;
    }

    uint8_t mode = MODE;
    uint32_t speed = SPEED;

    if (ioctl(fd, SPI_IOC_WR_MODE, &mode) == -1) {
        perror("Can't set SPI mode");
        return 1;
    }

    if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) == -1) {
        perror("Can't set max speed");
        return 1;
    }

    uint8_t tx = 0x42;
    uint8_t rx = 0x00;

    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)&tx,
        .rx_buf = (unsigned long)&rx,
        .len = 1,
        .delay_usecs = 0,
        .speed_hz = SPEED,
        .bits_per_word = 8,
    };

    while (true) {
        if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) < 1) {
            perror("SPI transfer failed");
            return 1;
        }

        printf("Sent: 0x%02X | Received: 0x%02X\n", tx, rx);
        sleep(1);
    }

    close(fd);
    return 0;
}
