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
float calculateIntermediateScore( Part leadPart, Part followPart, float angleLead, float speedLead, float angleFollow, float speedFollow) {
    srand(time(NULL));
    int K = (rand() % 9) + 1;

    leadPart.atk = (.5+.7*angleLead)*leadPart.atk;
    leadPart.def =  (1.25-.5*angleLead)*leadPart.def;
    leadPart.sta = (1.15-.3*angleLead)*leadPart.sta;
    leadPart.bur = (1.1-.2*angleLead)*leadPart.bur;
    
    leadPart.atk = (.65+.45*speedLead)*leadPart.atk;
    leadPart.def = (.7+.4*speedLead)*leadPart.def;
    leadPart.sta = (.8+.3*speedLead)*leadPart.sta;
    leadPart.bur = (.9+.15*speedLead)*leadPart.bur;
    leadPart.ss = (.7+.4*speedLead)*leadPart.ss;

    followPart.atk = (.5+.7*angleFollow)*followPart.atk;
    followPart.def =  (1.25-.5*angleFollow)*followPart.def;
    followPart.sta = (1.15-.3*angleFollow)*followPart.sta;
    followPart.bur = (1.1-.2*angleFollow)*followPart.bur;

    followPart.atk = (.65+.45*speedFollow)*followPart.atk;
    followPart.def = (.7+.4*speedFollow)*followPart.def;
    followPart.sta = (.8+.3*speedFollow)*followPart.sta;
    followPart.bur = (.9+.15*speedFollow)*followPart.bur;
    followPart.ss = (.7+.4*speedFollow)*followPart.ss;

    //formulas from read me 
    float leadScore = 25*leadPart.atk + .2*leadPart.def + .3*leadPart.sta + .15*leadPart.bur;
    float followScore = 25*followPart.atk + .2*followPart.def + .3*followPart.sta + .15*followPart.bur;
    if(speedLead > speedFollow) {
        float leadScore = leadScore +.15*leadPart.ss;
    } else if (speedFollow > speedLead) {
        float followScore = followScore +.15*followPart.ss;
    }

    float percentWin = 1/ (1 + exp(-K * (leadScore - followScore)));
    return percentWin;
}

int determineWinner(float angleLead, float speedLead, float angleFollow, float speedFollow, BeybladeConfig leadConfig, BeybladeConfig followConfig) {
    getUserchoices(leadConfig.blade, leadConfig.disc, leadConfig.tip, followConfig.blade, followConfig.disc, followConfig.tip);

    //calcualte winner of each part battle 
    float bladeWin = calculateIntermediateScore(bladeLead, bladeFollow, angleLead, speedLead, angleFollow, speedFollow);
    float discWin  = calculateIntermediateScore(discLead, discFollow,  angleLead, speedLead, angleFollow, speedFollow);
    float tipWin   = calculateIntermediateScore(tipCLead, tipFollow, angleLead, speedLead, angleFollow, speedFollow);
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


