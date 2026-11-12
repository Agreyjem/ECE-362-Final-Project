#ifndef SPINNING_H
#define SPINNING_H

#include "pico/stdlib.h"
#include <hardware/gpio.h>
#include <stdio.h>
#include "queue.h"
#include <time.h>
#include "parts.h"

void getUserchoices(int bladeChoiceLead,  int discChoiceLead, int tipChoiceLead, int bladeChoiceFollow, int discChoiceFollow, int tipChoiceFollow);
float calculateIntermediateScore(Part leadPart, Part followPart, float angleLead, float angleFollow, float speedLead, float speedFollow);
int determineWinner(float angleLead, float speedLead, float angleFollow, float speedFollow, int bladeChoiceLead, int discChoiceLead, int tipChoiceLead, int bladeChoiceFollow, int discChoiceFollow, int tipChoiceFollow);

#endif