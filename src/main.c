#include "acquisition_api.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <duration_ms>\n", argv[0]);
        return 1;
    }
    int duration_ms = atoi(argv[1]);

    set_duration_ms(duration_ms);
    start_acquisition();
    return 0;
}
