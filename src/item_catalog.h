#ifndef FARMER_ITEM_CATALOG_H
#define FARMER_ITEM_CATALOG_H

#include <stdbool.h>

#include "constants.h"

void item_catalog_load(char item_names[MAX_ITEMS][MAX_NAME_LEN]);
const char *item_catalog_get_name(char item_names[MAX_ITEMS][MAX_NAME_LEN], int item_id);
bool item_catalog_find_id(char item_names[MAX_ITEMS][MAX_NAME_LEN],
                          const char *search_name, int *item_id_out);

#endif
