#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>

#define SPI_DEVICE "/dev/spidev0.0"
#define SPI_SPEED 1000000
#define BITS_PER_WORD 8

int main() {
    int spi_fd = open(SPI_DEVICE, O_RDWR);
    if (spi_fd < 0) {
        perror("Failed to open SPI device");
        return 1;
    }

    uint8_t mode = SPI_MODE_0;
    uint8_t bits = BITS_PER_WORD;
    uint32_t speed = SPI_SPEED;

    // SPI configuration
    ioctl(spi_fd, SPI_IOC_WR_MODE, &mode);
    ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);

    while (1) {
        uint8_t tx = 0x00;
        uint8_t rx = 0;

        struct spi_ioc_transfer tr = {
            .tx_buf = (unsigned long) &tx,
            .rx_buf = (unsigned long) &rx,
            .len = 1,
            .speed_hz = speed,
            .delay_usecs = 0,
            .bits_per_word = bits,
        };

        if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr) < 1) {
            perror("SPI transfer failed");
            break;
        }

        printf("Received: %u\n", rx);
        sleep(1);
    }

    close(spi_fd);
    return 0;
}
