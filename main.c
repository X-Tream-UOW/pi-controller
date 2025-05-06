#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <string.h>

#define DEVICE "/dev/spidev0.0"
#define SPEED_HZ 1000000
#define WORDS 8  // nb of 16-bit words to read each loop (BUFFER_SIZE on pico)

int main() {
    int spi_fd = open(DEVICE, O_RDWR);
    if (spi_fd < 0) {
        perror("Failed to open SPI device");
        return 1;
    }

    uint8_t mode = SPI_MODE_0;
    uint8_t bits_per_word = 8;

    ioctl(spi_fd, SPI_IOC_WR_MODE, &mode);
    ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits_per_word);
    ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &(uint32_t){SPEED_HZ});

    uint8_t tx[WORDS * 2] = {0};
    uint8_t rx[WORDS * 2];

    while (1) {
        struct spi_ioc_transfer transfer = {
            .tx_buf = (unsigned long)tx,
            .rx_buf = (unsigned long)rx,
            .len = sizeof(tx),
            .delay_usecs = 0,
            .speed_hz = SPEED_HZ,
            .bits_per_word = bits_per_word,
        };

        int ret = ioctl(spi_fd, SPI_IOC_MESSAGE(1), &transfer);
        if (ret < 0) {
            perror("SPI transfer failed");
            break;
        }

        printf("Received: ");
        for (int i = 0; i < WORDS; i++) {
            uint16_t word = (rx[2*i] << 8) | rx[2*i+1];  // Big endian
            printf("0x%04X ", word);
        }
        printf("\n");
        usleep(1 * 1000 * 1000);
    }

    close(spi_fd);
    return 0;
}
