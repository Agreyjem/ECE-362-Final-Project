#include "pico/stdlib.h"
#include <hardware/gpio.h>
#include <stdio.h>
#include "queue.h"
#include <time.h>
#include "parts.h"

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
float calculateIntermediateScore(Part leadPart, Part followPart, float angleLead, float angleFollow, float speedLead, float speedFollow) {
    srand(time(NULL));
    int K = (rand() % 9) + 1;

    //need to implement the speed and angle normalizations and scoring here 
    
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

int determineWinner(float angleLead, float speedLead, float angleFollow, float speedFollow, int bladeChoiceLead, int discChoiceLead, int tipChoiceLead, int bladeChoiceFollow, int discChoiceFollow, int tipChoiceFollow) {
    getUserchoices(bladeChoiceLead, discChoiceLead, tipChoiceLead, bladeChoiceFollow, discChoiceFollow, tipChoiceFollow);

    //calcualte winner of each part battle 
    float bladeWin = calculateIntermediateScore(bladeLead, bladeFollow, angleLead, angleFollow, speedLead, speedFollow);
    float discWin  = calculateIntermediateScore(discLead, discFollow, angleLead, angleFollow, speedLead, speedFollow);
    float tipWin   = calculateIntermediateScore(tipCLead, tipFollow, angleLead, angleFollow, speedLead, speedFollow);
    //combine the results and dvide by 3 to get overall winner
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


