#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#define BLADE_COUNT 5
#define DISC_COUNT 5
#define TIP_COUNT 5


typedef struct {
    const char* name;
    uint8_t atk;
    uint8_t def;
    uint8_t sta;
    uint8_t bur;
    uint8_t ss;
    uint8_t cost;
} Part;

typedef enum {B_WIZARD_ROD, B_SHARK_SCALE, B_PHOENIX_WING, B_COBALT_DRAGON, B_HOVER_WYVERN} Blades;
typedef enum {D_HEAVY_A, D_KNUCKLE_A, D_SPREAD_A, D_3_60_R, D_9_60_R} Discs;
typedef enum {T_HEXA, T_LOW_RUSH, T_BALL, T_ELEVATE, T_RUSH} Tips;

extern const Part blades[BLADE_COUNT];
extern const Part discs[DISC_COUNT];
extern const Part tips[TIP_COUNT];
