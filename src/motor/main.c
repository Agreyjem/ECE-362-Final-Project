#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

// If ADC/DMA SDK functions and macros aren't being highlighted by VScode 
// and you're not sure why, make sure to read the tip in step 2!

void init_adc_freerun() {
    // fill in
    adc_init();
    adc_gpio_init(45);
    adc_select_input(5);
    adc_run(true); // enable with bit 3 of adc_hw->cs register, 96 cycle, 500kS/s
}
//////////////////////////////////////////////////////////////////////////////

int main()
{
    stdio_init_all();

    // for(;;) {
    //     printf("HII");
    // }
    init_adc_freerun();

    int result = 0;
    int highest = 0;
    for(;;) {
        result = adc_hw->result;
        if (result > highest) {
            highest = result;
        }
        printf("ADC Result: %d | Highest Result: %d\r", result, highest);
        fflush(stdout);
        sleep_ms(15);
    }
}