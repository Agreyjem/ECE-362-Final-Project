#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/irq.h"
#include "hardware/structs/adc.h"
#include "hardware/structs/timer.h"
#include "hardware/timer.h"
#include "header.h"

// If ADC/DMA SDK functions and macros aren't being highlighted by VScode 
// and you're not sure why, make sure to read the tip in step 2!


int main()
{
    // for(;;) {
    //     printf("HII");
    // }
    init_dcollect();
    // int result = 0;
    int highest = 0;
    float angle_x = 0.0f;
    float angle_y = 0.0f;
    float angle_z = 0.0f;
    // sleep_ms(2000);
    // printf(" GET READY (wait 2 seconds before starting)");
    // sleep_ms(2000);
    for(;;) {
        run_dcollect(&highest, &angle_x, &angle_y, &angle_z);
        // reset_dcollector osmething
        data_done = 0;
        highest = 0;
        angle_x = 0;
        angle_y = 0;
        angle_z = 0;
        printf("\n");
    }
}