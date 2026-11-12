#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "audio.h"


int main(void) {
    stdio_init_all();
    sound_init();
    for (;;) {
        play_start();  
        sound_wait(); 
        for (int i = 0; i < 12; i++) {
            play_collide();
            sleep_ms(150 + rand() % 450);
        }
        play_winner(); sound_wait(); sleep_ms(1000);
        play_loser();  sound_wait(); sleep_ms(2000);
    }
}