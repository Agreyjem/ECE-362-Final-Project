#include "pico/stdlib.h"
#include <hardware/gpio.h>
#include <stdio.h>
#include "queue.h"
#include <time.h>
#include "parts.h"
#include "header.h"

// define user choices
Part bladeLead;
Part discLead;
Part tipCLead;


Part bladeFollow;
Part discFollow;
Part tipFollow;
typedef struct {
    float x, y, vx, vy, mass, radius, restitution;
    uint16_t outcolor, incolor;
} Body;

// get the user choices from each keypad entry
void getUserchoices(int bladeChoiceLead,  int discChoiceLead, int tipChoiceLead, int bladeChoiceFollow, int discChoiceFollow, int tipChoiceFollow) {
    //from leader
    bladeLead = blades[bladeChoiceLead];
    discLead = discs[discChoiceLead];
    tipCLead = tips[tipChoiceLead];

    //from follower 
    bladeFollow = blades[bladeChoiceFollow];
    discFollow = discs[discChoiceFollow];
    tipFollow = tips[tipChoiceFollow];
}

//calculate scores 
float calculateIntermediateScore(Part leadPart, Part followPart, float vxLead, float vxFollow, float vyLead, float vyFollow) {
    srand(time(NULL));
    int K = (rand() % 9) + 1;

    float leadVelocity = sqrt(pow(vxLead,2)+pow(vyLead, 2));
    float followVelocity = sqrt(pow(vxFollow,2)+pow(vyFollow,2));

    //formulas from read me 
    float leadScore = 25*leadPart.atk + .2*leadPart.def + .3*leadPart.sta + .15*leadPart.bur;
    float followScore = 25*followPart.atk + .2*followPart.def + .3*followPart.sta + .15*followPart.bur;
    if(leadVelocity > followVelocity) {
        float leadScore = leadScore +.15*leadPart.ss;
    } else if (followVelocity > leadVelocity) {
        float followScore = followScore +.15*followPart.ss;
    }

    float percentWin = 1/ (1 + exp(-K * (leadScore - followScore)));
    return percentWin;
}

int determineWinner(Body leadBody, Body followBody, BeybladeConfig leadConfig, BeybladeConfig followConfig) {
    getUserchoices(leadConfig.blade, leadConfig.disc, leadConfig.tip, followConfig.blade, followConfig.disc, followConfig.tip);

    //calcualte winner of each part battle 
    float bladeWin = calculateIntermediateScore(bladeLead, bladeFollow, leadBody.vx, followBody.vx, leadBody.vy, followBody.vy);
    float discWin  = calculateIntermediateScore(discLead, discFollow,  leadBody.vx, followBody.vx, leadBody.vy, followBody.vy);
    float tipWin   = calculateIntermediateScore(tipCLead, tipFollow, leadBody.vx, followBody.vx, leadBody.vy, followBody.vy);
    float totalWin = (bladeWin + discWin + tipWin) / 3;

    //need to add a check for an immediate ko 
    if (totalWin > 0.5) {
        return 1; // Leader wins
    } else if (totalWin < 0.5) {
        return 2; // Follower wins
    } else {
        return 0; // Tie
    }
}


