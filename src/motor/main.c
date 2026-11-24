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

// If ADC/DMA SDK functions and macros aren't being highlighted by VScode 
// and you're not sure why, make sure to read the tip in step 2!

absolute_time_t previous_time;
volatile int data_done = 0;

mpu6050_init();

void init_adc_freerun() {
    // fill in
    adc_init();
    adc_gpio_init(45);
    adc_select_input(5);
    adc_run(true); // enable with bit 3 of adc_hw->cs register, 96 cycle, 500kS/s
}
//////////////////////////////////////////////////////////////////////////////

void reset_isr(void) {
    timer0_hw->intr = 1u << 2;

    data_done = 1;

    timer0_hw->alarm[2] = timer0_hw->timerawl + 5000000;
}

void init_timer(void) {
    irq_set_exclusive_handler(TIMER0_IRQ_2, reset_isr);
    irq_set_enabled(TIMER0_IRQ_2, true);

    timer0_hw->inte |= 1u << 2;
    timer0_hw->alarm[2] = timer0_hw->timerawl + 5000000;
    data_done = 0;
}

void run_dcollect(int* spinspeed, float* angle_x, float* angle_y, float* angle_z) {
    int result = 0;
    float anglex = 0.0f;
    float angley = 0.0f;
    float anglez = 0.0f;

    while (!data_done) {
        result = adc_hw->result;

        if (result > *spinspeed) {
            *spinspeed = result;
            *angle_x = anglex;
            *angle_y = angley;
            *angle_z = anglez;
        }

        update_orientation(&anglex, &angley, &anglez, previous_time);
        previous_time = get_absolute_time();

        printf("ADC Result: %d | Highest Result: %d x %f y %f z %f\r",
               result, *spinspeed, *angle_x, *angle_y, *angle_z);

        fflush(stdout);
        sleep_ms(15);
    }
}

void init_dcollect() {
    stdio_init_all();
    init_adc_freerun();
    mpu6050_init();
    init_timer();
    data_done = 0;
    // we need to print something before this happens
    sleep_ms(2000);
}

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