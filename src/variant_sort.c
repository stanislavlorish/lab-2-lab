#include "variant_sort.h"
#include "constants.h"
#include "inventory.h"

void inventory_compact(void)
{
    int *items = inventory_items();
    int read_index;
    int write_index = SLOT_FIRST;

    for (read_index = SLOT_FIRST; read_index <= SLOT_LAST; ++read_index) {
        if (items[read_index] != ITEM_EMPTY) {
            items[write_index] = items[read_index];
            ++write_index;
        }
    }

    while (write_index <= SLOT_LAST) {
        items[write_index] = ITEM_EMPTY;
        ++write_index;
    }
}
