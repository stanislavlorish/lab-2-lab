#ifndef FARMER_GAME_TIME_H
#define FARMER_GAME_TIME_H

#include <stdbool.h>

void game_time_print(int current_day, int current_hour);
bool game_time_advance(int *current_day, int *current_hour);

#endif
