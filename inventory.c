#include "inventory.h"
#include "constants.h"
#include "input.h"

#include <stdio.h>

static int inventory[INVENTORY_SIZE] = { ITEM_EMPTY };

static const char *item_name(int item_id)
{
    switch (item_id) {
        case ITEM_EMPTY:
            return "Пусто";
        case ITEM_TREE:
            return "Дерево";
        case ITEM_STONE:
            return "Камень";
        case ITEM_SEEDS:
            return "Семена";
        case ITEM_CARROT:
            return "Морковь";
        case ITEM_APPLE:
            return "Яблоко";
        case ITEM_FIBER:
            return "Волокно";
        case ITEM_IRON_ORE:
            return "Железная руда";
        case ITEM_FISH:
            return "Рыба";
        case ITEM_MILK:
            return "Молоко";
        default:
            return "Неизвестный предмет";
    }
}

void inventory_initialize(void)
{
    inventory[START_SLOT_TREE_FIRST] = ITEM_TREE;
    inventory[START_SLOT_STONE] = ITEM_STONE;
    inventory[START_SLOT_SEEDS] = ITEM_SEEDS;
    inventory[START_SLOT_CARROT] = ITEM_CARROT;
    inventory[START_SLOT_TREE_SECOND] = ITEM_TREE;
    inventory[START_SLOT_FIBER] = ITEM_FIBER;
}

void inventory_print(void)
{
    int slot;

    for (slot = SLOT_FIRST; slot < INVENTORY_SIZE; ++slot) {
        printf("Слот %d: [%d]", slot, inventory[slot]);
        if (inventory[slot] != ITEM_EMPTY) {
            printf(" (%s)", item_name(inventory[slot]));
        }
        putchar('\n');
    }
}

bool inventory_place_item(void)
{
    int slot;
    int item_id;

    if (!read_integer("Введите индекс слота (0-9): ", SLOT_FIRST, SLOT_LAST, &slot)) {
        return false;
    }
    if (!read_integer("Введите ID предмета (0-9): ", ITEM_ID_MIN, ITEM_ID_MAX, &item_id)) {
        return false;
    }

    inventory[slot] = item_id;
    printf("В слоте %d теперь: [%d] (%s).\n",
           slot, item_id, item_name(item_id));
    return true;
}

bool inventory_drop_item(void)
{
    int slot;

    if (!read_integer("Введите индекс слота (0-9): ", SLOT_FIRST, SLOT_LAST, &slot)) {
        return false;
    }

    inventory[slot] = ITEM_EMPTY;
    printf("Слот %d очищен.\n", slot);
    return true;
}

int *inventory_items(void)
{
    return inventory;
}
