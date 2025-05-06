#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define SPI_PATH "/dev/spidev0.0"
#define BUF_LEN 16

int main() {
    int fd = open(SPI_PATH, O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    uint8_t tx[BUF_LEN], rx[BUF_LEN];
    for (int i = 0; i < BUF_LEN; ++i) tx[i] = i;

    uint8_t mode = SPI_MODE_0;
    ioctl(fd, SPI_IOC_WR_MODE, &mode);
    uint8_t bits = 8;
    ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    uint32_t speed = 1000000;
    ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);

    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = BUF_LEN,
        .speed_hz = speed,
        .bits_per_word = bits,
    };

    if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) < 1) {
        perror("SPI_IOC_MESSAGE");
        return 1;
    }

    printf("Received: ");
    for (int i = 0; i < BUF_LEN; i++) printf("%02X ", rx[i]);
    printf("\n");

    close(fd);
    return 0;
}
