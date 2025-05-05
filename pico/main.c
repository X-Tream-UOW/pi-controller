#include <stdio.h>
#include "pico/stdlib.h"

#define CONVST_PIN 2
#define CS_PIN     3
#define RD_PIN     4
#define BUSY_PIN   5
#define DB_MIN     6
#define DB_MAX     21
#define LED_PIN    25

uint16_t current_data = 0;

void init_data_bus() {
    for (int i = DB_MIN; i <= DB_MAX; ++i) {
        gpio_init(i);
        gpio_set_dir(i, GPIO_OUT);
        gpio_put(i, 0);
    }
}

void write_output(uint16_t value) {
    for (int i = DB_MIN; i <= DB_MAX; ++i) {
        gpio_put(i, (value >> (i - DB_MIN)) & 1);
    }
}

void wait_for_convst_falling_edge() {
    while (gpio_get(CONVST_PIN));       // wait for CONVST to be high
    while (!gpio_get(CONVST_PIN));      // wait for falling edge (low)
}

void start_conversion() {
    gpio_put(BUSY_PIN, 1);
    sleep_us(1); 
    gpio_put(BUSY_PIN, 0);
}

void print_debug(uint16_t value) {
    printf("Sim data: %u (0x%04X), CS=%d, RD=%d, BUSY=%d\n",
           value,
           value,
           gpio_get(CS_PIN),
           gpio_get(RD_PIN),
           gpio_get(BUSY_PIN));
    fflush(stdout);
}

int main() {
    stdio_init_all();
    sleep_ms(2000); // Let USB console connect

    printf("ADS8405 simulator starting...\n");
    fflush(stdout);

    gpio_init(CONVST_PIN); gpio_set_dir(CONVST_PIN, GPIO_IN);
    gpio_init(CS_PIN);     gpio_set_dir(CS_PIN, GPIO_IN);
    gpio_init(RD_PIN);     gpio_set_dir(RD_PIN, GPIO_IN);
    gpio_init(BUSY_PIN);   gpio_set_dir(BUSY_PIN, GPIO_OUT);
    gpio_put(BUSY_PIN, 0);

    gpio_init(LED_PIN); gpio_set_dir(LED_PIN, GPIO_OUT); gpio_put(LED_PIN, 0);

    init_data_bus();

    while (true) {
        wait_for_convst_falling_edge();

        if (gpio_get(CS_PIN) == 0) {
            start_conversion();
            current_data++;
            write_output(current_data);
            // print_debug(current_data);
        }

        // if (gpio_get(CS_PIN) == 0 && gpio_get(RD_PIN) == 0) {
        //     gpio_put(LED_PIN, 1);
        //     sleep_ms(10);
        //     gpio_put(LED_PIN, 0);
        // }
    }

    return 0;
}
