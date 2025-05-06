#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define SPI_DEV "/dev/spidev0.0"

int main() {
    int fd = open(SPI_DEV, O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    uint8_t mode = SPI_MODE_0;
    ioctl(fd, SPI_IOC_WR_MODE, &mode);

    uint8_t bits = 8;
    ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);

    uint32_t speed = 1 * 1000 * 1000;  // 1 MHz
    ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);

    printf("Talking to Pico...\n");

    for (int i = 0; i < 10; ++i) {
        uint8_t tx = 0x00;  // dummy byte
        uint8_t rx = 0x00;

        struct spi_ioc_transfer tr = {
            .tx_buf = (unsigned long)&tx,
            .rx_buf = (unsigned long)&rx,
            .len = 1,
            .speed_hz = speed,
            .bits_per_word = bits,
        };

        if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) < 1) {
            perror("SPI_IOC_MESSAGE");
            close(fd);
            return 1;
        }

        printf("Received from Pico: %d\n", rx);
        sleep(1);
    }

    close(fd);
    return 0;
}
