#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <wiringPi.h>

#define SPI_DEV "/dev/spidev0.0"
#define SPEED 2000000  // 2 MHz
#define MODE SPI_MODE_0
#define BITS_PER_WORD 8

#define READY_PIN 5  // BCM GPIO 5
#define BUFFER_SAMPLES 65536
#define BUFFER_SIZE_BYTES (BUFFER_SAMPLES * 2)

int wait_for_ready() {
    while (digitalRead(READY_PIN) == 0) {
        // Wait for READY to go HIGH
        usleep(10);  // Avoid busy-looping too hard
    }
    return 0;
}

int main() {
    // Init wiringPi for GPIO access
    if (wiringPiSetupGpio() == -1) {
        perror("Failed to initialize wiringPi");
        return 1;
    }

    pinMode(READY_PIN, INPUT);

    // Open SPI device
    int fd = open(SPI_DEV, O_RDWR);
    if (fd < 0) {
        perror("Failed to open SPI device");
        return 1;
    }

    // Configure SPI mode
    uint8_t mode = MODE;
    uint8_t bits = BITS_PER_WORD;
    uint32_t speed = SPEED;

    if (ioctl(fd, SPI_IOC_WR_MODE, &mode) == -1 ||
        ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits) == -1 ||
        ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) == -1) {
        perror("SPI config failed");
        close(fd);
        return 1;
    }

    // Buffers
    uint8_t tx_buf[BUFFER_SIZE_BYTES] = {0};
    uint8_t rx_buf[BUFFER_SIZE_BYTES];

    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx_buf,
        .rx_buf = (unsigned long)rx_buf,
        .len = BUFFER_SIZE_BYTES,
        .delay_usecs = 0,
        .speed_hz = speed,
        .bits_per_word = bits,
    };

    printf("Waiting for READY signal on GPIO %d...\n", READY_PIN);

    while (true) {
        wait_for_ready();
        printf("READY detected. Starting SPI read...\n");

        if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) < 1) {
            perror("SPI transfer failed");
            break;
        }

        // Process received data
        for (int i = 0; i < BUFFER_SAMPLES; i++) {
            uint16_t sample = ((uint16_t)rx_buf[2 * i] << 8) | rx_buf[2 * i + 1];
//            printf("Sample[%d] = 0x%04X\n", i, sample);

//            if (i == 10) break;
        }

        printf("Transfer complete. Waiting for next READY...\n");
    }

    close(fd);
    return 0;
}
