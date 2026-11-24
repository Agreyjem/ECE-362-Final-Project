#include <stdio.h>
#include <string.h>
#include <stdint.h>fcd
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "lcd.h"
#include "hardware/spi.h"


double mpu_getreading(void);

int main () {
    stdio_init_all();
    double a = 0;
    for (;;) {
        a = mpu_getreading();
        printf("a=%lf\n", a);
        // printf("HI WORLD");
    }
}