#include "bias_api.h"
#include <stdio.h>
#include <unistd.h>  // for usleep


// This entry point is for testing the bias API directly.
int main(void) {
    if (bias_api_start_io() != 0) {
        fprintf(stderr, "Failed to start IO\n");
        return 1;
    }

    // Turn off HV
    bias_api_hv_off();
    usleep(500000);

    // Set 500 mV
    bias_api_set_voltage_mV(50);
    usleep(500000);

    // Set polarity negative
    bias_api_set_polarity(true);
    usleep(500000);

    // Print status and voltage
    bool en = false, neg = false;
    int32_t mv = 0;
    if (bias_api_get_status(&en, &neg, 50000) == 0) {
        printf("Status: enabled=%d, polarity=%s\n", en, neg ? "NEG" : "POS");
    }
    usleep(500000);
    if (bias_api_get_bias_mV(&mv, 50000) == 0) {
        printf("Voltage: %.3f V\n", mv / 1000.0);
    }
    usleep(2000000);

    // Turn on
    bias_api_hv_on();
    usleep(500000);

    // Print again
    if (bias_api_get_status(&en, &neg, 50000) == 0) {
        printf("Status: enabled=%d, polarity=%s\n", en, neg ? "NEG" : "POS");
    }
    usleep(500000);
    if (bias_api_get_bias_mV(&mv, 50000) == 0) {
        printf("Voltage: %.3f V\n", mv / 1000.0);
    }
    usleep(500000);

    // Turn off
    bias_api_hv_off();
    usleep(500000);

    bias_api_stop_io();
    return 0;
}
