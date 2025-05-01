#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>

#define SPI_DEVICE "/dev/spidev0.0"

int main() {
    int fd = open(SPI_DEVICE, O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    uint8_t mode = SPI_MODE_0;
    uint8_t bits = 8;
    uint32_t speed = 500000;

    ioctl(fd, SPI_IOC_WR_MODE, &mode);
    ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);

    uint8_t tx = 0x00;
    uint8_t rx;

    for (int i = 0; i < 10; ++i) {
        struct spi_ioc_transfer tr = {
            .tx_buf = (unsigned long)&tx,
            .rx_buf = (unsigned long)&rx,
            .len = 1,
            .speed_hz = speed,
            .bits_per_word = bits,
        };

        ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
        printf("Received: %d\n", rx);
        usleep(500000);  // wait 0.5 sec
    }

    close(fd);
    return 0;
}
