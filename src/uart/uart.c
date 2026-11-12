#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"


#define RXBUF_SIZE 16

static volatile uint8_t rxbuf[RXBUF_SIZE];
static volatile uint8_t rx_head = 0;   
static volatile uint8_t rx_tail = 0;  

void uart_rx_handler(void);
// Initializes the UART with the specified baud rate and configures the GPIO pins for UART functionality.
void init_uart(void) {
    gpio_set_function(0, UART_FUNCSEL_NUM(uart0, 0));
    gpio_set_function(1, UART_FUNCSEL_NUM(uart0, 1));
    uart_init(uart0, 115200);
    uart_set_format(uart0, 8, 1, UART_PARITY_NONE);
    uart_set_translate_crlf(uart0, false);
}
// Initializes the UART and sets up the interrupt handler for receiving data.
void init_uart_irq(void) {
    uart_set_fifo_enabled(uart0, true);
    irq_set_exclusive_handler(UART0_IRQ, uart_rx_handler);
    irq_set_enabled(UART0_IRQ, true);
    uart_set_irq_enables(uart0, true, false);
}
// Interrupt handler for UART RX
void uart_rx_handler(void) {
    while (uart_is_readable(uart0)) {
        uint8_t c = uart_getc(uart0);
        uint8_t next = (rx_head + 1) % RXBUF_SIZE;
        if (next != rx_tail) {         
            rxbuf[rx_head] = c;
            rx_head = next;
        }
    }
}

//Determines if there is a character in the buffer and pops it if there is one. Returns true if a character was popped, false otherwise.
bool uart_rx_pop(uint8_t *c) {
    if (rx_tail == rx_head) return false;
    *c = rxbuf[rx_tail];
    rx_tail = (rx_tail + 1) % RXBUF_SIZE;
    return true;
}


// int main(void) {
//     stdio_init_all();
//     init_uart();
//     init_uart_irq();
    
//     char last_key = 0;

//     while (true) {
    
//         char key = keypad_get_key();
//         if (key != 0 && key != last_key) {
//             uart_putc_raw(uart0, key);
    //             send the key over UART
//         }
//         last_key = key;

//         uint8_t rx;
//         while (uart_rx_pop(&rx)) {
            // recieved character is now in rx do smth with it
//         }

//         char win = determineWinner();
//         uart_putc_raw(uart0, win); // send the winner over UART
//         sleep_ms(10);  
//     }
// }