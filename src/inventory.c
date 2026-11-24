#include "inventory.h"
#include "constants.h"
#include "item_catalog.h"

static int inventory[INVENTORY_SIZE] = { ITEM_EMPTY };

void inventory_initialize(void)
{
    inventory[START_SLOT_TREE_FIRST] = ITEM_TREE;
    inventory[START_SLOT_STONE] = ITEM_STONE;
    inventory[START_SLOT_SEEDS] = ITEM_SEEDS;
    inventory[START_SLOT_CARROT] = ITEM_CARROT;
    inventory[START_SLOT_TREE_SECOND] = ITEM_TREE;
    inventory[START_SLOT_FIBER] = ITEM_FIBER;
}

bool inventory_set_item(int slot, int item_id)
{
    if (slot < SLOT_FIRST || slot > SLOT_LAST ||
        item_id < ITEM_ID_MIN || item_id > ITEM_ID_MAX) {
        return false;
    }

    inventory[slot] = item_id;
    return true;
}

bool inventory_clear_slot(int slot)
{
    if (slot < SLOT_FIRST || slot > SLOT_LAST) {
        return false;
    }

    inventory[slot] = ITEM_EMPTY;
    return true;
}

int inventory_get_slot(int slot)
{
    if (slot < SLOT_FIRST || slot > SLOT_LAST) {
        return ITEM_EMPTY;
    }

    return inventory[slot];
}

int *inventory_items(void)
{
    return inventory;
}

int inventory_find_slots(int item_id, int slots_out[INVENTORY_SIZE])
{
    int slot;
    int found_count = 0;

    for (slot = SLOT_FIRST; slot <= SLOT_LAST; ++slot) {
        if (inventory[slot] == item_id) {
            slots_out[found_count] = slot;
            ++found_count;
        }
    }

    return found_count;
}

void inventory_write(FILE *stream, char item_names[MAX_ITEMS][MAX_NAME_LEN])
{
    int slot;

    for (slot = SLOT_FIRST; slot <= SLOT_LAST; ++slot) {
        int item_id = inventory[slot];
        fprintf(stream, "Слот %d: [%d] - %s\n",
                slot, item_id, item_catalog_get_name(item_names, item_id));
    }
}
