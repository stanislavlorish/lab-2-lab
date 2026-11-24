#ifndef FARMER_GAME_H
#define FARMER_GAME_H

#include "map.h"
#include "player.h"
#include "weather.h"

#include <stdbool.h>

typedef struct GameState {
    Player *player;
    Map *map;
    Weather weather;
    int day;
    int hour;
    char item_names[MAX_ITEMS][MAX_NAME_LEN];
} GameState;

typedef enum MenuOption {
    MENU_EXIT = 0,
    MENU_CREATE_PLAYER = 1,
    MENU_CHARACTER_CARD = 2,
    MENU_TIME = 3,
    MENU_SHOW_INVENTORY = 4,
    MENU_PLACE_ITEM = 5,
    MENU_DROP_ITEM = 6,
    MENU_SORT_INVENTORY = 7,
    MENU_SEARCH_ITEM = 8,
    MENU_WRITE_DIARY = 9,
    MENU_ADVENTURE = 10,
    MENU_RESOURCE_REPORT = 11
} MenuOption;

typedef enum TimeMenuOption {
    TIME_MENU_BACK = 0,
    TIME_MENU_SHOW_CLOCK = 1,
    TIME_MENU_WAIT = 2,
    TIME_MENU_SLEEP = 3
} TimeMenuOption;

void game_initialize(GameState *game);
void game_destroy(GameState *game);
bool game_has_player(const GameState *game);
void game_create_player(GameState *game);
void game_show_character(const GameState *game);
void game_show_inventory(const GameState *game);
void game_place_item(GameState *game);
void game_drop_item(GameState *game);
void game_sort_inventory(GameState *game);
void game_search_item(GameState *game);
void game_write_diary(GameState *game);
void game_time_menu(GameState *game);
void game_start_adventure(GameState *game);
void game_show_resource_report(void);

#endif
