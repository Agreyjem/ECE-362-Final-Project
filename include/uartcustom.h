#ifndef UART_CUSTOM_H
#define UART_CUSTOM_H
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"

void uart_rx_handler(void);
void init_uart(void);
void init_uart_irq(void);
bool uart_rx_pop(uint8_t *c);

#endif // UART_CUSTOM_H

