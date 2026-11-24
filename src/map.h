#ifndef FARMER_MAP_H
#define FARMER_MAP_H

#include "player.h"
#include "weather.h"

typedef enum Direction {
    DIR_NONE = 0,
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT,
    DIR_QUIT
} Direction;

typedef enum TileType {
    TILE_WALL = '#',
    TILE_FLOOR = '.',
    TILE_TREE = 'T',
    TILE_ROCK = 'R',
    TILE_HERO = '@',
    TILE_ICE = '~',
    TILE_UP_STAIR = 'U',
    TILE_DOWN_STAIR = 'D'
} TileType;

typedef struct Map {
    char **cells;
    int width;
    int height;
    int floor;
} Map;

typedef enum MapMoveResult {
    MAP_MOVE_INVALID = 0,
    MAP_MOVE_BLOCKED,
    MAP_MOVE_SUCCESS,
    MAP_MOVE_AT_UP_STAIR,
    MAP_MOVE_AT_DOWN_STAIR,
    MAP_MOVE_EXHAUSTED
} MapMoveResult;

Map *map_create(int floor, int *spawn_x_out, int *spawn_y_out);
void map_destroy(Map **map_pointer);
MapMoveResult map_move(Map *map, Player *player, const Weather *weather, Direction direction);
bool map_player_on_up_stair(const Map *map, const Player *player);
void map_render(const Map *map, const Player *player, const Weather *weather,
                int day, int hour);

#endif
