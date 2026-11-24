#include "player.h"
#include "item_catalog.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

Player *player_create(const char *name)
{
    Player *player;

    if (name == NULL || name[0] == '\0' ||
        strlen(name) >= MAX_FARMER_NAME_LEN) {
        return NULL;
    }

    player = calloc((size_t)STEP_UNIT, sizeof(*player));
    if (player == NULL) {
        return NULL;
    }

    snprintf(player->name, sizeof(player->name), "%s", name);
    player->hp = STARTING_HP;
    player->max_hp = STARTING_HP;
    player->stamina = STARTING_STAMINA;
    player->max_stamina = STARTING_STAMINA;
    player->gold = STARTING_GOLD;
    player->inventory[START_SLOT_TREE_FIRST] = ITEM_TREE;
    player->inventory[START_SLOT_STONE] = ITEM_STONE;
    player->inventory[START_SLOT_SEEDS] = ITEM_SEEDS;
    player->inventory[START_SLOT_CARROT] = ITEM_CARROT;
    player->inventory[START_SLOT_TREE_SECOND] = ITEM_TREE;
    player->inventory[START_SLOT_FIBER] = ITEM_FIBER;
    return player;
}

void player_destroy(Player **player_pointer)
{
    void *address;

    if (player_pointer == NULL || *player_pointer == NULL) {
        return;
    }

    address = *player_pointer;
    printf("Память структуры по адресу %p освобождена.\n", address);
    free(*player_pointer);
    *player_pointer = NULL;
}

bool player_spend_stamina(Player *player, int amount)
{
    if (player == NULL || amount < 0) {
        return false;
    }
    if (player->stamina <= amount) {
        player->stamina = 0;
        return false;
    }

    player->stamina -= amount;
    return true;
}

void player_add_gold(Player *player, long long amount)
{
    long long total;

    if (player == NULL || amount <= 0) {
        return;
    }

    total = (long long)player->gold + amount;
    player->gold = total > INT_MAX ? INT_MAX : (int)total;
}

bool player_set_item(Player *player, int slot, int item_id)
{
    if (player == NULL || slot < SLOT_FIRST || slot > SLOT_LAST ||
        item_id < ITEM_ID_MIN || item_id > ITEM_ID_MAX) {
        return false;
    }

    player->inventory[slot] = item_id;
    return true;
}

bool player_clear_slot(Player *player, int slot)
{
    if (player == NULL || slot < SLOT_FIRST || slot > SLOT_LAST) {
        return false;
    }

    player->inventory[slot] = ITEM_EMPTY;
    return true;
}

int player_get_slot(const Player *player, int slot)
{
    if (player == NULL || slot < SLOT_FIRST || slot > SLOT_LAST) {
        return ITEM_EMPTY;
    }
    return player->inventory[slot];
}

void player_compact_inventory(Player *player)
{
    int read_index;
    int write_index = SLOT_FIRST;

    if (player == NULL) {
        return;
    }

    for (read_index = SLOT_FIRST; read_index <= SLOT_LAST; ++read_index) {
        if (player->inventory[read_index] != ITEM_EMPTY) {
            player->inventory[write_index] = player->inventory[read_index];
            ++write_index;
        }
    }
    while (write_index <= SLOT_LAST) {
        player->inventory[write_index] = ITEM_EMPTY;
        ++write_index;
    }
}

int player_find_slots(const Player *player, int item_id,
                      int slots_out[INVENTORY_SIZE])
{
    int slot;
    int count = 0;

    if (player == NULL || slots_out == NULL ||
        item_id < ITEM_ID_MIN || item_id > ITEM_ID_MAX) {
        return 0;
    }

    for (slot = SLOT_FIRST; slot <= SLOT_LAST; ++slot) {
        if (player->inventory[slot] == item_id) {
            slots_out[count] = slot;
            ++count;
        }
    }
    return count;
}

void player_print_inventory(FILE *stream, const Player *player,
                            char item_names[MAX_ITEMS][MAX_NAME_LEN])
{
    int slot;

    if (stream == NULL || player == NULL || item_names == NULL) {
        return;
    }

    for (slot = SLOT_FIRST; slot <= SLOT_LAST; ++slot) {
        int item_id = player->inventory[slot];
        fprintf(stream, "Слот %d: [%d] — %s\n", slot, item_id,
                item_catalog_get_name(item_names, item_id));
    }
}
