#include "parts.h"

const Part blades[BLADE_COUNT] = {
    [B_WIZARD_ROD] = {
        .name = "Wizard Rod",
        .atk  = 20, .def = 80, .sta = 95, .bur = 70, .ss = 10, .cost = 55,
    },
    [B_SHARK_SCALE] = {
        .name = "Shark Scale",
        .atk  = 90, .def = 30, .sta = 40, .bur = 65, .ss = 5, .cost = 50,
    },
    [B_PHOENIX_WING] = {
        .name = "Phoenix Wing",
        .atk  = 65, .def = 55, .sta = 60, .bur = 60, .ss = 10, .cost = 40,
    },
    [B_COBALT_DRAGON] = {
        .name = "Cobalt Dragon",
        .atk  = 80, .def = 35, .sta = 50, .bur = 55, .ss = 20, .cost = 45,
    },
    [B_HOVER_WYVERN] = {
        .name = "Hover Wyvern",
        .atk  = 55, .def = 50, .sta = 70, .bur = 60, .ss = 10, .cost = 35,
    },
};

const Part discs[DISC_COUNT] = {
    [D_HEAVY_A] = {
        .name = "Heavy (A)",
        .atk  = 20, .def = 85, .sta = 70, .bur = 50, .ss = 0, .cost = 30,
    },
    [D_KNUCKLE_A] = {
        .name = "Knuckle (A)",
        .atk  = 75, .def = 40, .sta = 35, .bur = 55, .ss = 0, .cost = 25,
    },
    [D_SPREAD_A] = {
        .name = "Spread (A)",
        .atk  = 30, .def = 60, .sta = 65, .bur = 45, .ss = 0, .cost = 20,
    },
    [D_3_60_R] = {
        .name = "3-60 Ratchet",
        .atk  = 45, .def = 50, .sta = 55, .bur = 60, .ss = 0, .cost = 25,
    },
    [D_9_60_R] = {
        .name = "9-60 Ratchet",
        .atk  = 30, .def = 60, .sta = 70, .bur = 75, .ss = 0, .cost = 30,
    },
};

const Part tips[TIP_COUNT] = {
    [T_HEXA] = {
        .name = "Hexa",
        .atk  = 15, .def = 85, .sta = 90, .bur = 70, .ss = 5, .cost = 45,
    },
    [T_LOW_RUSH] = {
        .name = "Low Rush",
        .atk  = 90, .def = 25, .sta = 30, .bur = 50, .ss = 0, .cost = 40,
    },
    [T_BALL] = {
        .name = "Ball",
        .atk  = 10, .def = 60, .sta = 95, .bur = 55, .ss = 5, .cost = 30,
    },
    [T_ELEVATE] = {
        .name = "Elevate",
        .atk  = 65, .def = 50, .sta = 65, .bur = 55, .ss = 10, .cost = 35,
    },
    [T_RUSH] = {
        .name = "Rush",
        .atk  = 80, .def = 30, .sta = 35, .bur = 50, .ss = 0, .cost = 30,
    },
};