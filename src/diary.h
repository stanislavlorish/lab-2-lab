#ifndef FARMER_DIARY_H
#define FARMER_DIARY_H

#include "player.h"

void diary_append(const Player *player, int day, int hour,
                  char item_names[MAX_ITEMS][MAX_NAME_LEN]);

#endif
