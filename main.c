#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

int main() {
    int fd = open("/dev/spidev0.0", O_RDWR);
    if (fd < 0) {
        printf("Failed to open SPI device\n");
        return 1;
    }

    uint8_t mode = SPI_MODE_0;
    uint32_t speed = 1000000; // 1 MHz
    uint8_t bits = 8;

    ioctl(fd, SPI_IOC_WR_MODE, &mode);
    ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);

    uint8_t tx = 0x00; // Dummy byte
    uint8_t rx;

    while (1) {
        struct spi_ioc_transfer tr = {
            .tx_buf = (unsigned long)&tx,
            .rx_buf = (unsigned long)&rx,
            .len = 1,
            .delay_usecs = 0,
            .speed_hz = speed,
            .bits_per_word = bits,
        };

        ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
        printf("Received: %d\n", rx);
        usleep(100000); // 100ms delay
    }

    close(fd);
    return 0;
}