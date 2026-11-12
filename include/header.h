#include "pico/stdlib.h"
#define N_BODIES   2      // Number of bodies in the simulation
#define G          12.0f  // Gravitational constant
#define DT         0.01f // Simulation time step
#define SOFTENING  5.0f   // Prevents extreme forces at close range

    // Colors as per the 16-bit RGB565 specification.
#define BLACK      0x0000
#define RED        0xF800
#define LIME       0x07E0   // brighter green
#define BLUE       0x001F

#define PIN_SDI    15
#define PIN_CS     13
#define PIN_SCK    14
#define PIN_DC     17
#define PIN_nRESET 16
#define THRESH 500

#define COST_CAP 100
// one string per menu
#define MENU_START "BEYBLADE BUILDER - press any button to start.\n"

#define MENU_BLADE "Pick a blade:   \n 1. Wizard Rod\
                                    \n 2. Shark Scale\
                                    \n 3. Phoenix Wing\
                                    \n 4. Cobalt Dragon\
                                    \n 5. Hover Wyvern\
                                    \n # to confirm.\n"

#define MENU_DISC "Pick a disc/ratchet: \n 1. Heavy\
                                        \n 2. Knuckle(A)\
                                        \n 3. Spread\
                                        \n 4. 3-60\
                                        \n 5. 9-60\
                                        \n # to confirm.\n"

#define MENU_TIP "Pick a tip:   \n 1. Hexa\
                                \n 2. Low Rush\
                                \n 3. Ball\
                                \n 4. Elevate\
                                \n 5. Rush\
                                \n # to confirm.\n"

#define MENU_CONFIRM "Confirm: # to confirm, 1=change blade, 2=change disc, " \
"3=change tip.\n"

typedef struct {
    float x, y, vx, vy, mass, radius, restitution;
    uint16_t outcolor, incolor;
} Body;

typedef struct {
	int blade;
	int disc;
	int tip;
} BeybladeConfig;