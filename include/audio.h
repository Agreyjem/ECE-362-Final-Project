
#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>

void sound_init(void);                          // call once at startup

void play_start(void);                          // 3, 2, 1, GO!
void play_winner(void);
void play_loser(void);
void play_collide(void);                        // random recorded hit

bool sound_is_playing(void);
void sound_wait(void);                          // block until the sound ends
void sound_set_volume(int melody, int collide); // 0-100 each

#endif