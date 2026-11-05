#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "queue.h"

void keypad_init_pins();
void keypad_init_timer();
extern KeyEvents kev;

int main() {
    keypad_init_pins();
    keypad_init_timer();

    uint16_t keyevent = key_pop();
    if (keyevent & 0x100) {
        char key = keyevent & 0xFF;
        
    }
    for (;;) {

    }
}