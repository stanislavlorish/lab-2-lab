#ifndef FARMER_INVENTORY_H
#define FARMER_INVENTORY_H

#include <stdbool.h>
#include <stdio.h>

#include "constants.h"

void inventory_initialize(void);
bool inventory_set_item(int slot, int item_id);
bool inventory_clear_slot(int slot);
int inventory_get_slot(int slot);
int *inventory_items(void);
int inventory_find_slots(int item_id, int slots_out[INVENTORY_SIZE]);
void inventory_write(FILE *stream, char item_names[MAX_ITEMS][MAX_NAME_LEN]);

#endif
