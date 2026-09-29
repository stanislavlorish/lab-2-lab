#ifndef FARMER_INPUT_H
#define FARMER_INPUT_H

#include <stdbool.h>

bool read_integer(const char *prompt, int min_value, int max_value, int *value_out);

#endif
