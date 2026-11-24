#ifndef FARMER_PLAYER_H
#define FARMER_PLAYER_H

#include "constants.h"

#include <stdbool.h>
#include <stdio.h>

typedef struct Player {
    char name[MAX_FARMER_NAME_LEN];
    int hp;
    int max_hp;
    int stamina;
    int max_stamina;
    int gold;
    int x;
    int y;
    int inventory[MAX_ITEMS];
} Player;

Player *player_create(const char *name);
void player_destroy(Player **player_pointer);
bool player_spend_stamina(Player *player, int amount);
void player_add_gold(Player *player, long long amount);

bool player_set_item(Player *player, int slot, int item_id);
bool player_clear_slot(Player *player, int slot);
int player_get_slot(const Player *player, int slot);
void player_compact_inventory(Player *player);
int player_find_slots(const Player *player, int item_id, int slots_out[INVENTORY_SIZE]);
void player_print_inventory(FILE *stream, const Player *player,
                            char item_names[MAX_ITEMS][MAX_NAME_LEN]);

#endif
