#ifndef FARMER_UTILS_H
#define FARMER_UTILS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

bool read_integer(const char *prompt, int min_value, int max_value, int *value_out);
bool read_line(const char *prompt, char *buffer, size_t buffer_size);
bool read_yes_no(const char *prompt, bool *answer_out);
bool discard_line_tail(FILE *stream);
size_t utf8_character_count(const char *text);
void print_game_time(FILE *stream, int day, int hour);
void print_real_datetime(FILE *stream);
bool advance_game_time(int *day, int *hour, int hours_to_add, bool *day_changed_out);
int hours_until_night(int hour);

#endif
