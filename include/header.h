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

typedef struct {
    float x, y, vx, vy, mass, radius, restitution;
    uint16_t outcolor, incolor;
} Body;

typedef struct {
	int blade;
	int disc;
	int tip;
} BeybladeConfig;