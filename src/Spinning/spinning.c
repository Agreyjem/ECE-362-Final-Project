#include "pico/stdlib.h"
#include <hardware/gpio.h>
#include <stdio.h>
#include "queue.h"
#include <time.h>

// define the weights for each of the options
const float bladeWeight[3] = {.8, .3, .6};
const float coreWeight[3] = {.6, .7, .4};
const float discWeight[3] = {.2, .5, .9};
const float tipWeight[3] = {.5, .4, .6};

//define the base score fore each catagory
const int baseScore[4] = {13, 15, 12, 14};

// define user choices
float bladeChoiceLead;
float coreChoiceLead;
float discChoiceLead;
float tipChoiceLead;

float bladeChoiceFollow;
float coreChoiceFollow;
float discChoiceFollow;
float tipChoiceFollow;

// get the user choices from each keypad entry


void getUserchoices(int bladeChoiceLead, int coreChoiceLead, int discChoiceLead, int tipChoiceLead, int bladeChoiceFollow, int coreChoiceFollow, int discChoiceFollow, int tipChoiceFollow) {
    //from leader
    bladeChoiceLead = bladeWeight[bladeChoiceLead];
    coreChoiceLead = coreWeight[coreChoiceLead];
    discChoiceLead = discWeight[discChoiceLead];
    tipChoiceLead = tipWeight[tipChoiceLead];

    //from follower 
    bladeChoiceFollow = bladeWeight[bladeChoiceFollow];
    coreChoiceFollow = coreWeight[coreChoiceFollow];
    discChoiceFollow = discWeight[discChoiceFollow];
    tipChoiceFollow = tipWeight[tipChoiceFollow];
}

int calculateIntermediateScore(float bladeChoice, float coreChoice, float discChoice, float tipChoice) {
    int score = baseScore[0] * bladeChoice + baseScore[1] * coreChoice + baseScore[2] * discChoice + baseScore[3] * tipChoice;
    return score;
}

int determineWinner(){
    srand(time(NULL));
    int leadRand = (rand() % 9) + 1;
    int followRand = (rand() % 9) + 1;
    int leaderScore = leadRand * calculateIntermediateScore(bladeChoiceLead, coreChoiceLead, discChoiceLead, tipChoiceLead);
    int followerScore = followRand * calculateIntermediateScore(bladeChoiceFollow, coreChoiceFollow, discChoiceFollow, tipChoiceFollow);

    if (leaderScore > followerScore) {
        return 1; // Leader wins
    } else if (followerScore > leaderScore) {
        return 2; // Follower wins
    } else {
        return 0; // Tie
    }
}


