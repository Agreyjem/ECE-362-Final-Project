#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "queue.h"
#include "parts.h"
#include "header.h"

BeybladeConfig configbeyblade();

int main() {
    
    stdio_init_all();
    configbeyblade();

	for (;;) {
        // printf("Hello, world!\n");
        // sleep_ms(1000);
		// battle loop goes here
	}
}
