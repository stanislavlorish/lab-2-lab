#ifndef FARMER_DIARY_H
#define FARMER_DIARY_H

#include "constants.h"

void diary_append(const char *farmer_name, int current_day, int current_hour,
                  char item_names[MAX_ITEMS][MAX_NAME_LEN]);

#endif
