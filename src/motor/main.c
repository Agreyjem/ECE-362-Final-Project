#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/irq.h"
#include "hardware/structs/timer.h"

static volatile uint16_t adc_highest = 0;
static volatile bool data_done = true;

// ADC interrupt: process one sample
void adc_isr() {
    if (!adc_fifo_is_empty()) {
        uint16_t result = adc_fifo_get();

        if (result > adc_highest) {
            adc_highest = result;
        }
    }
}

// Timer interrupt: end measurement
void adc_max_isr() {
    timer0_hw->intr = 1u << 2;

    // Stop further ADC conversions
    adc_run(false);

    // Disable ADC FIFO interrupt
    adc_irq_set_enabled(false);

    // Process any remaining FIFO samples
    while (!adc_fifo_is_empty()) {
        uint16_t result = adc_fifo_get();

        if (result > adc_highest) {
            adc_highest = result;
        }
    }

    data_done = true;

    timer0_hw->inte &= ~(1u << 2);
}

// Start an ADC measurement (non-blocking)
void adc_get_max_start(uint32_t duration_ms) {
    adc_run(false);
    adc_irq_set_enabled(false);

    adc_fifo_drain();

    adc_highest = 0;
    data_done = false;

    // Start ADC interrupt and conversions
    adc_irq_set_enabled(true);
    adc_run(true);

    // Start measurement timer
    timer0_hw->inte |= 1u << 2;
    timer0_hw->alarm[2] =
        (uint32_t)timer0_hw->timerawl + duration_ms * 1000;
}

// Initialize timer interrupt
void init_adc_max_timer() {
    irq_set_exclusive_handler(TIMER0_IRQ_2, adc_max_isr);
    irq_set_enabled(TIMER0_IRQ_2, true);
}

// Initialize ADC and FIFO interrupt
void init_adc_freerun() {
    adc_init();
    adc_gpio_init(45);
    adc_select_input(5);

    adc_fifo_setup(
        true,   // Enable FIFO
        false,  // Disable DMA
        1,      // Interrupt threshold: 1 sample
        false,  // No error bit
        false   // No byte shift
    );

    irq_set_exclusive_handler(ADC_IRQ_FIFO, adc_isr);
    irq_set_enabled(ADC_IRQ_FIFO, true);

    adc_irq_set_enabled(false);
}

//////////////////////////////////////////////////////////////////////////////

int main() {
    stdio_init_all();

    init_adc_freerun();
    init_adc_max_timer();

    while (1) {
        printf("Get ready to spin!\n");
        sleep_ms(1000);

        // Start measurement without blocking
        adc_get_max_start(100);

        // Do other work here (MPU6050, etc.)
        while (!data_done) {
            // Other processing
        }

        printf("Highest ADC value: %u\n", adc_highest);

        sleep_ms(100);
    }
}