#ifndef UART_CUSTOM_H
#define UART_CUSTOM_H

#include <stdint.h>
#include <stdbool.h>
#include "header.h"

#define START_BYTE    0xAA
#define MAX_PAYLOAD   16          // must be >= CONFIG_WIRE_SIZE

typedef enum { START, CHOOSE, BATTLE, END } mode_t_;

// If BeybladeConfig is already defined in your own header, delete this and include yours.
// If you add fields, update CONFIG_WIRE_SIZE, pack_config() and unpack_config().
typedef struct {
    int blade;
    int disc;
    int tip;
} BeybladeConfig;

#define CONFIG_WIRE_SIZE 12       // 3 ints x 4 bytes

enum {
    CMD_HELLO       = 1,   // M->S  no payload
    CMD_GET_CONFIG  = 2,   // M->S  no payload: "send me your config if you're done choosing"
    CMD_CONFIG_RESP = 3,   // S->M  payload: slave's (player 2) config
    CMD_CONFIG      = 4,   // M->S  payload: master's (player 1) config
    CMD_START       = 5,   // M->S  no payload
    CMD_WINNER      = 6,   // M->S  payload: winner id (0 = P1/master, 1 = P2/slave)
    CMD_ACK         = 7    // payload: [0] = command being acknowledged
};

typedef struct {
    uint8_t cmd;
    uint8_t len;
    uint8_t data[MAX_PAYLOAD];
} frame_t;

static inline uint8_t checksum(uint8_t cmd, uint8_t len, const uint8_t *p) {
    uint8_t c = cmd ^ len;
    for (uint8_t i = 0; i < len; i++) c ^= p[i];
    return c;
}

// ---- UART setup and raw byte access ----
void init_uart(void);
void init_uart_irq(void);
bool uart_rx_pop(uint8_t *c);

// ---- Framing ----
void send_frame(uint8_t cmd, const uint8_t *payload, uint8_t len);
void send_ack(uint8_t acked_cmd);
bool poll_frame(frame_t *out);
bool send_reliable(uint8_t cmd, const uint8_t *payload, uint8_t len);

// ---- Config serialization ----
void pack_config(const BeybladeConfig *c, uint8_t out[CONFIG_WIRE_SIZE]);
bool unpack_config(const frame_t *f, BeybladeConfig *c);      // false if length is wrong

// Master: ask the slave for its config. Returns true once it answers.
bool request_slave_config(BeybladeConfig *out);

#endif // UART_CUSTOM_H