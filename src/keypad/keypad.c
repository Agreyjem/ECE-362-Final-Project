#include "pico/stdlib.h"
#include <hardware/gpio.h>
#include <stdio.h>
#include "queue.h"

// Global column variable
int col = -1;

// Global key state
static bool state[17]; // Are keys pressed/released

// Keymap for the keypad
const char keymap[16] = "DCBA#9630852*741";

// Defined here to avoid circular dependency issues with autotest
// You can see the struct definition in queue.h
// KeyEvents kev = { 
//     .head = 0, 
//     .tail = 0 
// };

void keypad_drive_column();
void keypad_isr();

/********************************************************* */
// Implement the functions below.

// replace timer_hw with timer0_hw

void keypad_init_pins() {
    gpio_init_mask(0b11111111<<2);
    // gpio_set_dir_in_masked(0b1111<<2);
    sio_hw->gpio_oe_clr = 0b1111<<2;
    // gpio_set_dir_out_masked(0b1111<<6);
    sio_hw->gpio_oe_set = 0b1111<<6;
    sio_hw->gpio_clr = 0b1111<<6; // set out to 0, do we need this ?
}

// TIMER0 and TIMER1 1 tick per µs
void keypad_init_timer() {
    timer0_hw->inte = (1u<<0) | (1u<<1);
    // timer0_hw->inte = 1u<<1;
    irq_set_exclusive_handler(TIMER0_IRQ_0, &keypad_drive_column);
    irq_set_exclusive_handler(TIMER0_IRQ_1, &keypad_isr);
    // irq_set_enabled
    nvic_hw->icpr[0] = (1<<TIMER0_IRQ_0) | (1<<TIMER0_IRQ_1);
    nvic_hw->iser[0] = (1<<TIMER0_IRQ_0) | (1<<TIMER0_IRQ_1);
    timer0_hw->alarm[0] = (uint32_t) timer0_hw->timerawl + 100000;
    timer0_hw->alarm[1] = (uint32_t) timer0_hw->timerawl + 101000;
    // do we need intr here?
}

void keypad_drive_column() {
    timer0_hw->intr = 1u<<0; // acknowledge
    col = (uint8_t)(col + 1) & (0b11);
    sio_hw->gpio_set = (0b1<<(6 + (col)));
    sio_hw->gpio_clr = (0b1<<(6 + ((1 + col) & 0b11)));
    sio_hw->gpio_clr = (0b1<<(6 + ((2 + col) & 0b11)));
    sio_hw->gpio_clr = (0b1<<(6 + ((3 + col) & 0b11)));
    timer0_hw->alarm[0] = (uint32_t) timer0_hw->timerawl + 25000;
}

uint8_t keypad_read_rows() {
    return sio_hw->gpio_in & (0b1111<<2); // 4321
}

void keypad_isr() {
    timer0_hw->intr = 1u<<1; // acknowledge
    uint8_t rows = keypad_read_rows();
    for (int i = 0; i < 4; i++) {
        uint16_t idx = col*4+i;
        char keychar = keymap[idx];
        if (rows>>(i+2) & 1) {
            if (!state[idx]) {
                state[idx] = 1;
                key_push((1<<8)|(uint16_t)keychar);
            }
        }
        else {
            if (state[idx]) {
                state[idx] = 0;
                key_push((0<<8)|(uint16_t)keychar);
            } 
        }
    }
    timer0_hw->alarm[1] = (uint32_t) timer0_hw->timerawl + 25000;
}