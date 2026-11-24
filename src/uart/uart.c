#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "uartcustom.h"
#include "string.h"


#define RXBUF_SIZE 64
void uart_rx_handler(void);
static volatile uint8_t rxbuf[RXBUF_SIZE];
static volatile uint8_t rx_head = 0;
static volatile uint8_t rx_tail = 0;

void init_uart(void) {
    gpio_set_function(0, UART_FUNCSEL_NUM(uart0, 0));
    gpio_set_function(1, UART_FUNCSEL_NUM(uart0, 1));
    uart_init(uart0, 115200);
    uart_set_format(uart0, 8, 1, UART_PARITY_NONE);
    uart_set_translate_crlf(uart0, false);
}

void init_uart_irq(void) {
    uart_set_fifo_enabled(uart0, true);
    irq_set_exclusive_handler(UART0_IRQ, uart_rx_handler);
    irq_set_enabled(UART0_IRQ, true);
    uart_set_irq_enables(uart0, true, false);
}

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

bool uart_rx_pop(uint8_t *c) {
    if (rx_tail == rx_head) return false;
    *c = rxbuf[rx_tail];
    rx_tail = (rx_tail + 1) % RXBUF_SIZE;
    return true;
}

// ================================== Framing ====================================
void send_frame(uint8_t cmd, const uint8_t *payload, uint8_t len) {
    uart_putc_raw(uart0, START_BYTE);
    uart_putc_raw(uart0, cmd);
    uart_putc_raw(uart0, len);
    for (uint8_t i = 0; i < len; i++) uart_putc_raw(uart0, payload[i]);
    uart_putc_raw(uart0, checksum(cmd, len, payload));
}

void send_ack(uint8_t acked_cmd) {
    send_frame(CMD_ACK, &acked_cmd, 1);
}

// Parser state lives at file scope so it persists between calls.
typedef enum { P_WAIT_START, P_CMD, P_LEN, P_DATA, P_CHK } parse_state_t;
static parse_state_t pstate = P_WAIT_START;
static frame_t cur;
static uint8_t didx;
static uint32_t last_byte_ms;

bool poll_frame(frame_t *out) {
    uint8_t b;
    while (uart_rx_pop(&b)) {
        last_byte_ms = to_ms_since_boot(get_absolute_time());
        switch (pstate) {
        case P_WAIT_START:
            if (b == START_BYTE) pstate = P_CMD;
            break;
        case P_CMD:
            cur.cmd = b;
            pstate = P_LEN;
            break;
        case P_LEN:
            if (b > MAX_PAYLOAD) { pstate = P_WAIT_START; break; }
            cur.len = b;
            didx = 0;
            pstate = (b == 0) ? P_CHK : P_DATA;
            break;
        case P_DATA:
            cur.data[didx++] = b;
            if (didx >= cur.len) pstate = P_CHK;
            break;
        case P_CHK:
            pstate = P_WAIT_START;
            if (b == checksum(cur.cmd, cur.len, cur.data)) {
                *out = cur;
                return true;
            }
            break;
        }
    }
    // Discard a half-received frame if the sender went quiet
    if (pstate != P_WAIT_START &&
        to_ms_since_boot(get_absolute_time()) - last_byte_ms > 20) {
        pstate = P_WAIT_START;
    }
    return false;
}

bool send_reliable(uint8_t cmd, const uint8_t *payload, uint8_t len) {
    frame_t f;
    for (int attempt = 0; attempt < 5; attempt++) {
        send_frame(cmd, payload, len);
        absolute_time_t deadline = make_timeout_time_ms(100);
        while (!time_reached(deadline)) {
            if (poll_frame(&f) && f.cmd == CMD_ACK && f.len >= 1 && f.data[0] == cmd)
                return true;
        }
    }
    return false;
}

// ============================== Config serialization ===========================
// Each int goes over as 4 bytes, little-endian, so struct padding never matters.
static void put_i32(uint8_t *o, int v) {
    uint32_t u = (uint32_t)v;
    o[0] = u & 0xFF;
    o[1] = (u >> 8) & 0xFF;
    o[2] = (u >> 16) & 0xFF;
    o[3] = (u >> 24) & 0xFF;
}

static int get_i32(const uint8_t *i) {
    return (int)((uint32_t)i[0]
               | (uint32_t)i[1] << 8
               | (uint32_t)i[2] << 16
               | (uint32_t)i[3] << 24);
}

void pack_config(const BeybladeConfig *c, uint8_t out[CONFIG_WIRE_SIZE]) {
    put_i32(&out[0], c->blade);
    put_i32(&out[4], c->disc);
    put_i32(&out[8], c->tip);
}

bool unpack_config(const frame_t *f, BeybladeConfig *c) {
    if (f->len != CONFIG_WIRE_SIZE) return false;
    c->blade = get_i32(&f->data[0]);
    c->disc  = get_i32(&f->data[4]);
    c->tip   = get_i32(&f->data[8]);
    return true;
}

// Master: one poll attempt. Call repeatedly until it returns true.
bool request_slave_config(BeybladeConfig *out) {
    frame_t f;
    send_frame(CMD_GET_CONFIG, NULL, 0);
    absolute_time_t deadline = make_timeout_time_ms(100);
    while (!time_reached(deadline)) {
        if (poll_frame(&f) && f.cmd == CMD_CONFIG_RESP && unpack_config(&f, out))
            return true;
    }
    return false;
}
bool request_slave_launch(float *angle, float *speed) {
    send_frame(CMD_GET_LAUNCH, NULL, 0);
    absolute_time_t deadline = make_timeout_time_ms(100);
    frame_t f;
    while (!time_reached(deadline)) {
        if (poll_frame(&f) && f.cmd == CMD_LAUNCH_RESP && f.len == 8) {
            memcpy(angle, f.data,     4);
            memcpy(speed, f.data + 4, 4);
            return true;
        }
    }
    return false;
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
//                 // send the key over UART
//         }
//         last_key = key;

//         uint8_t rx;
//         while (uart_rx_pop(&rx)) {
//             // recieved character is now in rx do smth with it
//         }

//         char win = determineWinner();
//         uart_putc_raw(uart0, win); // send the winner over UART
//         sleep_ms(10);  
//     }
// }