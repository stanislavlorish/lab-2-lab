#ifndef FARMER_INVENTORY_H
#define FARMER_INVENTORY_H

#include <stdbool.h>

void inventory_initialize(void);
void inventory_print(void);
bool inventory_place_item(void);
bool inventory_drop_item(void);
int *inventory_items(void);

#endif
