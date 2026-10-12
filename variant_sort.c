#include "variant_sort.h"
#include "constants.h"
#include "inventory.h"

#include <stdio.h>

void variant_sort_run(void)
{
    int *items = inventory_items();
    int read_index;
    int write_index = SLOT_FIRST;

    puts("Инвентарь до сортировки:");
    inventory_print();

    for (read_index = SLOT_FIRST; read_index < INVENTORY_SIZE; ++read_index) {
        if (items[read_index] != ITEM_EMPTY) {
            items[write_index] = items[read_index];
            ++write_index;
        }
    }

    while (write_index < INVENTORY_SIZE) {
        items[write_index] = ITEM_EMPTY;
        ++write_index;
    }

    puts("Инвентарь после сортировки:");
    inventory_print();
}
