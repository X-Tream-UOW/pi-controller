#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include "spi_handler.h"

#define SPI_DEV "/dev/spidev0.0"
#define SPI_SPEED 20000000
#define BITS_PER_WORD 16

int init_spi(void) {
    int fd = open(SPI_DEV, O_RDWR);
    if (fd < 0) {
        perror("open(SPI_DEV)");
        return -1;
    }

    uint8_t mode = SPI_MODE_1;
    uint8_t bits = BITS_PER_WORD;
    uint32_t speed = SPI_SPEED;
    uint8_t lsb = 0;

    if (ioctl(fd, SPI_IOC_WR_MODE, &mode) == -1 ||
        ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits) == -1 ||
        ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) == -1) {
        perror("SPI ioctl");
        close(fd);
        return -1;
        }

    ioctl(fd, SPI_IOC_WR_LSB_FIRST, &lsb);
    ioctl(fd, SPI_IOC_RD_LSB_FIRST, &lsb);

    return fd;
}
