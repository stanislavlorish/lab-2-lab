#ifndef FARMER_GAME_TIME_H
#define FARMER_GAME_TIME_H

#include <stdbool.h>
#include <stdio.h>

void game_time_print(FILE *stream, int current_day, int current_hour);
bool game_time_add(int *current_day, int *current_hour, int hours_to_add);

#endif

