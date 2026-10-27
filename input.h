#ifndef FARMER_INPUT_H
#define FARMER_INPUT_H

#include <stdbool.h>
#include <stddef.h>

bool read_integer(const char *prompt, int min_value, int max_value, int *value_out);
bool read_line(const char *prompt, char *buffer, size_t buffer_size);
size_t utf8_character_count(const char *text);

#endif
