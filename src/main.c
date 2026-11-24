#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "lcd.h"
#include <stdio.h>
#include <string.h>
#include <math.h>   
#include "uartcustom.h" // uartcustom includes header 
#include "spinning.h"
#include "audio.h"

/****************************************** */
#define PIN_SDI    15
#define PIN_CS     13
#define PIN_SCK    14
#define PIN_DC     17
#define PIN_nRESET 16
#define THRESH 500
#define PIN_ROLE   10

// Uncomment the following #define when 
// you are ready to run Step 3.

// WARNING: The process will take a VERY 
// long time as it compiles and uploads 
// all the image frames into the uploaded 
// binary!  Expect to wait 5 minutes.
// #define ANIMATION

/****************************************** */
#ifdef ANIMATION
#include "images.h"
#endif
/****************************************** */

BeybladeConfig configbeyblade(void);

// Own state enum so we don't depend on what spinning.h defines.
typedef enum { GS_WAIT, GS_START, GS_CHOOSE, GS_BATTLE, GS_END } game_state_t;

static game_state_t state;
static bool is_leader;

static BeybladeConfig p1_config;   // leader's player
static BeybladeConfig p2_config;   // follower's player
static bool chosen  = false;       // this board finished configbeyblade()
static bool have_p2 = false;       // leader only: follower config received
static bool have_p1 = false;       // follower only: leader config received
static uint8_t winner = 0;         // 0 = tie, 1 = player 1, 2 = player 2
static bool winner_sent = false;   // leader only
static int  frame_count = 0;

static Body bodies[N_BODIES];

// ============================== Role detection ==============================
// Runs once at boot. Same binary on both boards; a jumper decides the role.
static void init_role(void) {
    gpio_init(PIN_ROLE);
    gpio_set_dir(PIN_ROLE, GPIO_IN);
    gpio_pull_up(PIN_ROLE);
    sleep_ms(5);                       // let the pull-up settle
    is_leader = gpio_get(PIN_ROLE);    // high = leader, jumpered low = follower
}
void init_spi_lcd() {
    gpio_set_function(PIN_CS, GPIO_FUNC_SIO);
    gpio_set_function(PIN_DC, GPIO_FUNC_SIO);
    gpio_set_function(PIN_nRESET, GPIO_FUNC_SIO);

    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_set_dir(PIN_DC, GPIO_OUT);
    gpio_set_dir(PIN_nRESET, GPIO_OUT);

    gpio_put(PIN_CS, 1); // CS high
    gpio_put(PIN_DC, 0); // DC low
    gpio_put(PIN_nRESET, 1); // nRESET high

    // initialize SPI1 with 48 MHz clock
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SDI, GPIO_FUNC_SPI);
    spi_init(spi1, 100 * 1000 * 1000);
    spi_set_format(spi1, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
}

Picture* load_image(const char* image_data);
void free_image(Picture* pic);


// Make things easier to keep track of for each "body".
int handle_collision(Body *a, Body *b) {
    float dx = b->x - a->x;
    float dy = b->y - a->y;

    float distance = sqrtf(dx * dx + dy * dy);
    float min_distance = a->radius + b->radius;

    if (distance >= min_distance || distance == 0.0f)
        return 0;

    // Collision normal
    float nx = dx / distance;
    float ny = dy / distance;

    // Relative velocity
    float rvx = b->vx - a->vx;
    float rvy = b->vy - a->vy;

    // Relative velocity along collision normal
    float vel_normal = rvx * nx + rvy * ny;

    // Ignore if bodies are already separating
    if (vel_normal > 0)
        return 0;

    // Coefficient of restitution
    float e = fminf(a->restitution, b->restitution);

    // Calculate impulse
    float impulse = -(1.0f + e) * vel_normal /
                    (1.0f/a->mass + 1.0f/b->mass);

    // Update velocities
    a->vx -= impulse * nx / a->mass;
    a->vy -= impulse * ny / a->mass;

    b->vx += impulse * nx / b->mass;
    b->vy += impulse * ny / b->mass;

    // Correct overlapping positions
    float overlap = min_distance - distance;
    float total_mass = a->mass + b->mass;

    a->x -= nx * overlap * (b->mass / total_mass);
    a->y -= ny * overlap * (b->mass / total_mass);

    b->x += nx * overlap * (a->mass / total_mass);
    b->y += ny * overlap * (a->mass / total_mass);

    return 1;

}
static void reset_bodies(void) {
    bodies[0] = (Body){ .x=120.0f, .y=100.0f, .vx= 1.2f, .vy= 0.5f, .mass=20.0f,
                        .radius=10, .restitution=1, .outcolor=RED,  .incolor=BLUE };
    bodies[1] = (Body){ .x=180.0f, .y=250.0f, .vx=-0.8f, .vy=-1.0f, .mass=25.0f,
                        .radius=10, .restitution=1, .outcolor=LIME, .incolor=RED };
}

static void erase_body(const Body *b) {
    LCD_Circle(b->x, b->y, b->radius, 0, BLACK);
    LCD_Circle(b->x, b->y, 5, 0, BLACK);
}

// ============================== Screens / transitions ==============================
static void drain_rx(void) {
    uint8_t b;
    while (uart_rx_pop(&b)) {}
}

static void show_result(uint8_t w) {
    LCD_Clear(BLACK);
    LCD_DrawString(30, 150, CYAN, BLACK, "THANK YOU FOR PLAYING", 16, 1);
    if (w == 1)      LCD_DrawString(90, 170, CYAN, BLACK, "Player 1 Won!", 16, 1);
    else if (w == 2) LCD_DrawString(90, 170, CYAN, BLACK, "Player 2 Won!", 16, 1);
    else             LCD_DrawString(90, 170, CYAN, BLACK, "Tie!",     16, 1);
}

static void enter_wait(void) {            // follower idle screen
    state = GS_WAIT;
    LCD_Clear(BLACK);
    LCD_DrawString(30, 150, CYAN, BLACK, "Waiting for opponent", 16, 1);
}

static void enter_start(void) {           // leader title screen + HELLO retries
    state = GS_START;
    sound_init();
    play_start();
    sound_wait();
    LCD_Clear(BLACK);
    LCD_DrawString(90, 150, CYAN, BLACK, "BEYBLADE", 16, 1);
}

static void enter_choose(void) {
    state = GS_CHOOSE;
    memset(&p1_config, 0, sizeof(p1_config));
    memset(&p2_config, 0, sizeof(p2_config));
    chosen = have_p1 = have_p2 = false;
    LCD_Clear(BLACK);
}

static void enter_battle(void) {
    state = GS_BATTLE;
    reset_bodies();
    frame_count = 0;
    LCD_Clear(BLACK);
}

static void enter_end_leader(void) {
    state = GS_END;
    // TODO: fill in the real launch angle/speed for each player.
    float p1_angle = 0, p1_speed = 0, p2_angle = 0, p2_speed = 0;
    winner = (uint8_t)determineWinner(p1_angle, p1_speed, p2_angle, p2_speed,
                                      p1_config, p2_config);
    winner_sent = false;
    show_result(winner);
}

// ============================== Follower message handler ==============================
static void handle_incoming_as_follower(void) {
    frame_t f;
    if (!poll_frame(&f)) return;

    switch (f.cmd) {
    case CMD_HELLO:
        send_ack(CMD_HELLO);
        if (state != GS_CHOOSE) enter_choose();   // idempotent: retries don't wipe a choice
        break;

    case CMD_GET_CONFIG:
        if (state == GS_CHOOSE && chosen) {
            uint8_t buf[CONFIG_WIRE_SIZE];
            pack_config(&p2_config, buf);
            send_frame(CMD_CONFIG_RESP, buf, sizeof(buf));
        }
        break;

    case CMD_CONFIG:
        if (unpack_config(&f, &p1_config)) {
            have_p1 = true;
            send_ack(CMD_CONFIG);
        }
        break;

    case CMD_START:
        if (have_p1 && chosen) {
            send_ack(CMD_START);
            if (state != GS_BATTLE) enter_battle();   // ACK duplicates, don't restart
        }
        break;

    case CMD_WINNER:
        send_ack(CMD_WINNER);
        if (f.len == 1 && state != GS_END) {
            winner = f.data[0];
            state = GS_END;
            show_result(winner);
        }
        break;
    }
}

// ============================== Main ==============================
#ifndef ANIMATION
int main(void) {
    init_uart();
    init_uart_irq();
    init_spi_lcd();
    LCD_Setup();
    LCD_Clear(BLACK);

    init_role();
    if (is_leader) enter_start();
    else           enter_wait();

    for (;;) {
        // Only the follower polls frames here. The leader uses
        // send_reliable()/request_slave_config(), which read the replies themselves.
        if (!is_leader) handle_incoming_as_follower();

        // NOTE: use `break` only to leave the switch. Never break the for(;;).
        switch (state) {

        case GS_WAIT:                         // follower: HELLO handler moves us on
            break;

        case GS_START:                        // leader
            if (send_reliable(CMD_HELLO, NULL, 0)) enter_choose();
            else sleep_ms(200);               // follower not up yet
            break;

        case GS_CHOOSE:
            if (is_leader) {
                if (!chosen) {
                    p1_config = configbeyblade();
                    chosen = true;
                    LCD_Clear(BLACK);
                    LCD_DrawString(30, 150, CYAN, BLACK, "Waiting for opponent", 16, 1);
                }
                if (!have_p2) {
                    if (!request_slave_config(&p2_config)) { sleep_ms(150); break; }
                    have_p2 = true;
                }
                uint8_t buf[CONFIG_WIRE_SIZE];
                pack_config(&p1_config, buf);
                if (send_reliable(CMD_CONFIG, buf, sizeof(buf)) &&
                    send_reliable(CMD_START, NULL, 0)) {
                    enter_battle();
                } else {
                    sleep_ms(200);            // retry next pass
                }
            } else {
                if (!chosen) {
                    p2_config = configbeyblade();
                    chosen = true;
                    drain_rx();               // drop polls that arrived while we were busy
                    LCD_Clear(BLACK);
                    LCD_DrawString(30, 150, CYAN, BLACK, "Waiting for opponent", 16, 1);
                }
            }
            break;

        case GS_BATTLE:
            for (int i = 0; i < N_BODIES; i++)
                for (int j = i + 1; j < N_BODIES; j++)
                    if (handle_collision(&bodies[i], &bodies[j])) LCD_Clear(BLACK);

            for (int i = 0; i < N_BODIES; i++) {
                Body *b = &bodies[i];
                if (b->x - b->radius < 0) {
                    erase_body(b);
                    b->x = b->radius;
                    b->vx = fabsf(b->vx) * b->restitution;
                }
                if (b->x + b->radius >= 240) {
                    erase_body(b);
                    b->x = 240 - b->radius;
                    b->vx = -fabsf(b->vx) * b->restitution;
                }
                if (b->y - b->radius < 0) {
                    erase_body(b);
                    b->y = b->radius;
                    b->vy = fabsf(b->vy) * b->restitution;
                }
                if (b->y + b->radius >= 320) {
                    erase_body(b);
                    b->y = 320 - b->radius;
                    b->vy = -fabsf(b->vy) * b->restitution;   // was vx: bug
                }
            }

            for (int i = 0; i < N_BODIES; i++) {
                Body *b = &bodies[i];
                int old_x = (int)b->x;
                int old_y = (int)b->y;
                b->x += b->vx * DT * 350;
                b->y += b->vy * DT * 350;

                LCD_Circle(old_x, old_y, b->radius, 0, BLACK);
                LCD_Circle(old_x, old_y, 5, 0, BLACK);
                LCD_Circle((uint16_t)b->x, (uint16_t)b->y, b->radius, 0, b->outcolor);
                LCD_Circle((uint16_t)b->x, (uint16_t)b->y, 5, 0, b->incolor);
            }
            // Leader decides when the match ends. Follower stays in the battle
            // until CMD_WINNER arrives (handled above).
            if (is_leader && ++frame_count > THRESH) enter_end_leader();
            break;

        case GS_END:
            if (is_leader) {
                winner = determineWinner(.1,.5,.5,.1,p1_config,p2_config);
                if (!winner_sent) {
                    if (send_reliable(CMD_WINNER, &winner, 1)) winner_sent = true;
                    else { sleep_ms(200); break; }
                }
                sleep_ms(3000);
                enter_start();                // HELLO pulls the follower back to CHOOSE
            }
            break;
        }
    }
}

#else  // ANIMATION
int main(void) {
    init_uart();
    init_uart_irq();
    init_spi_lcd();
    LCD_Setup();
    LCD_Clear(BLACK);

    int frame_index = 0;
    for (;;) {
        Picture* frame_pic = load_image(mystery_frames[frame_index]);
        if (frame_pic) {
            LCD_DrawPicture(0, 0, frame_pic);
            free_image(frame_pic);
        }
        if (++frame_index >= mystery_frame_count) frame_index = 0;
        sleep_ms(1);
    }
}
#endif