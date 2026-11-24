#include "map.h"
#include "constants.h"

#include <stdio.h>
#include <stdlib.h>

static int random_between(int min_value, int max_value)
{
    return min_value + rand() % (max_value - min_value + RANDOM_INCLUSIVE_OFFSET);
}

static bool map_contains(const Map *map, int x, int y)
{
    return map != NULL && x >= MAP_INDEX_MIN && x < map->width &&
           y >= MAP_INDEX_MIN && y < map->height;
}

static bool tile_is_walkable(char tile)
{
    return tile == TILE_FLOOR || tile == TILE_ICE ||
           tile == TILE_UP_STAIR || tile == TILE_DOWN_STAIR;
}

static void free_cells(char ***cells_pointer, int height)
{
    int row;

    if (cells_pointer == NULL || *cells_pointer == NULL) {
        return;
    }
    for (row = MAP_INDEX_MIN; row < height; ++row) {
        free((*cells_pointer)[row]);
    }
    free(*cells_pointer);
    *cells_pointer = NULL;
}

static bool allocate_cells(Map *map)
{
    int row;

    map->cells = calloc((size_t)map->height, sizeof(*map->cells));
    if (map->cells == NULL) {
        return false;
    }
    for (row = MAP_INDEX_MIN; row < map->height; ++row) {
        map->cells[row] = malloc((size_t)map->width * sizeof(*map->cells[row]));
        if (map->cells[row] == NULL) {
            return false;
        }
    }
    return true;
}

static void carve_down_path(Map *map, int start_x, int start_y)
{
    int down_y = map->height - MAP_BOUNDARY_THICKNESS - STEP_UNIT;
    int down_x = random_between(MAP_BOUNDARY_THICKNESS,
                                map->width - MAP_BOUNDARY_THICKNESS - STEP_UNIT);
    int row;
    int column;

    for (row = start_y + STEP_UNIT; row <= down_y; ++row) {
        map->cells[row][start_x] = TILE_FLOOR;
    }
    if (start_x <= down_x) {
        for (column = start_x; column < down_x; ++column) {
            map->cells[down_y][column] = TILE_FLOOR;
        }
    } else {
        for (column = start_x; column > down_x; --column) {
            map->cells[down_y][column] = TILE_FLOOR;
        }
    }
    map->cells[down_y][down_x] = TILE_DOWN_STAIR;
}

static bool generate_cells(Map *map, int *spawn_x_out, int *spawn_y_out)
{
    const char obstacle_tiles[OBSTACLE_TYPE_COUNT] = {
        TILE_TREE,
        TILE_ROCK
    };
    int row;
    int column;
    int obstacle_percent;
    int start_x;
    int start_y;

    obstacle_percent = random_between(MAP_OBSTACLE_PERCENT_MIN,
                                      MAP_OBSTACLE_PERCENT_MAX);
    for (row = MAP_INDEX_MIN; row < map->height; ++row) {
        for (column = MAP_INDEX_MIN; column < map->width; ++column) {
            bool boundary = row < MAP_BOUNDARY_THICKNESS ||
                row >= map->height - MAP_BOUNDARY_THICKNESS ||
                column < MAP_BOUNDARY_THICKNESS ||
                column >= map->width - MAP_BOUNDARY_THICKNESS;

            if (boundary) {
                map->cells[row][column] = TILE_WALL;
            } else if (random_between(RANDOM_INCLUSIVE_OFFSET, RANDOM_CHANCE_MAX) <=
                       obstacle_percent) {
                int obstacle_index = random_between(MAP_INDEX_MIN,
                                                    OBSTACLE_TYPE_COUNT - RANDOM_INCLUSIVE_OFFSET);
                map->cells[row][column] = obstacle_tiles[obstacle_index];
            } else if (random_between(RANDOM_INCLUSIVE_OFFSET, RANDOM_CHANCE_MAX) <=
                       ICE_CHANCE_PERCENT) {
                map->cells[row][column] = TILE_ICE;
            } else {
                map->cells[row][column] = TILE_FLOOR;
            }
        }
    }

    start_x = map->width / MAP_CENTER_DIVISOR;
    start_y = map->height / MAP_CENTER_DIVISOR;
    *spawn_x_out = start_x;
    *spawn_y_out = start_y;

    map->cells[start_y][start_x] = TILE_FLOOR;
    map->cells[start_y][start_x - STEP_UNIT] = TILE_UP_STAIR;
    map->cells[start_y][start_x + STEP_UNIT] = TILE_ICE;
    carve_down_path(map, start_x, start_y);
    return true;
}

Map *map_create(int floor, int *spawn_x_out, int *spawn_y_out)
{
    Map *map;

    if (floor < STEP_UNIT || spawn_x_out == NULL || spawn_y_out == NULL) {
        return NULL;
    }
    map = calloc((size_t)STEP_UNIT, sizeof(*map));
    if (map == NULL) {
        return NULL;
    }

    map->floor = floor;
    map->width = random_between(MAP_MIN_SIZE, MAP_MAX_SIZE);
    map->height = random_between(MAP_MIN_SIZE, MAP_MAX_SIZE);
    if (!allocate_cells(map) || !generate_cells(map, spawn_x_out, spawn_y_out)) {
        map_destroy(&map);
        return NULL;
    }
    return map;
}

void map_destroy(Map **map_pointer)
{
    void *address;

    if (map_pointer == NULL || *map_pointer == NULL) {
        return;
    }

    free_cells(&(*map_pointer)->cells, (*map_pointer)->height);
    address = *map_pointer;
    printf("Память структуры по адресу %p освобождена.\n", address);
    free(*map_pointer);
    *map_pointer = NULL;
}

static bool direction_delta(Direction direction, int *delta_x, int *delta_y)
{
    *delta_x = MAP_INDEX_MIN;
    *delta_y = MAP_INDEX_MIN;
    switch (direction) {
        case DIR_UP:
            *delta_y = -STEP_UNIT;
            return true;
        case DIR_DOWN:
            *delta_y = STEP_UNIT;
            return true;
        case DIR_LEFT:
            *delta_x = -STEP_UNIT;
            return true;
        case DIR_RIGHT:
            *delta_x = STEP_UNIT;
            return true;
        case DIR_NONE:
        case DIR_QUIT:
        default:
            return false;
    }
}

static bool move_one_tile(Player *player, const Weather *weather,
                          int delta_x, int delta_y)
{
    int stamina_cost = weather_stamina_multiplier(weather);

    player->x += delta_x;
    player->y += delta_y;
    if (!player_spend_stamina(player, stamina_cost)) {
        puts("Фермер израсходовал всю выносливость.");
        return false;
    }
    return true;
}

static MapMoveResult move_in_direction(Map *map, Player *player,
                                       const Weather *weather, Direction direction)
{
    int delta_x;
    int delta_y;
    int next_x;
    int next_y;

    if (!direction_delta(direction, &delta_x, &delta_y)) {
        return MAP_MOVE_INVALID;
    }
    next_x = player->x + delta_x;
    next_y = player->y + delta_y;
    if (!map_contains(map, next_x, next_y) ||
        !tile_is_walkable(map->cells[next_y][next_x])) {
        return MAP_MOVE_BLOCKED;
    }

    if (!move_one_tile(player, weather, delta_x, delta_y)) {
        return MAP_MOVE_EXHAUSTED;
    }
    if (map->cells[player->y][player->x] == TILE_ICE) {
        puts("Фермер ступил на лёд и скользит в том же направлении.");
    }

    while (map->cells[player->y][player->x] == TILE_ICE) {
        next_x = player->x + delta_x;
        next_y = player->y + delta_y;
        if (!map_contains(map, next_x, next_y) ||
            !tile_is_walkable(map->cells[next_y][next_x])) {
            puts("Скольжение остановилось у стены или препятствия.");
            break;
        }
        if (!move_one_tile(player, weather, delta_x, delta_y)) {
            return MAP_MOVE_EXHAUSTED;
        }
        if (map->cells[player->y][player->x] == TILE_FLOOR) {
            puts("Фермер выехал на обычный пол.");
            break;
        }
        if (map->cells[player->y][player->x] == TILE_UP_STAIR ||
            map->cells[player->y][player->x] == TILE_DOWN_STAIR) {
            puts("Скольжение остановилось у лестницы.");
            break;
        }
    }
    return MAP_MOVE_SUCCESS;
}

static MapMoveResult result_for_position(const Map *map, const Player *player)
{
    char tile = map->cells[player->y][player->x];
    if (tile == TILE_DOWN_STAIR) {
        return MAP_MOVE_AT_DOWN_STAIR;
    }
    if (tile == TILE_UP_STAIR) {
        return MAP_MOVE_AT_UP_STAIR;
    }
    return MAP_MOVE_SUCCESS;
}

static Direction random_direction(void)
{
    return (Direction)random_between(DIR_UP, DIR_RIGHT);
}

MapMoveResult map_move(Map *map, Player *player, const Weather *weather,
                       Direction direction)
{
    MapMoveResult result;
    bool moved;

    if (map == NULL || map->cells == NULL || player == NULL || weather == NULL ||
        !map_contains(map, player->x, player->y)) {
        return MAP_MOVE_INVALID;
    }
    result = move_in_direction(map, player, weather, direction);
    if (result == MAP_MOVE_EXHAUSTED || result == MAP_MOVE_INVALID) {
        return result;
    }
    moved = result == MAP_MOVE_SUCCESS;
    if (moved) {
        result = result_for_position(map, player);
        if (result == MAP_MOVE_AT_UP_STAIR || result == MAP_MOVE_AT_DOWN_STAIR) {
            return result;
        }
    }

    if (weather->type == WEATHER_HURRICANE &&
        random_between(RANDOM_INCLUSIVE_OFFSET, RANDOM_CHANCE_MAX) <=
        HURRICANE_PUSH_CHANCE_PERCENT) {
        Direction gust_direction = random_direction();
        MapMoveResult gust_result;

        puts("Порыв ураганного ветра пытается сдвинуть фермера!");
        gust_result = move_in_direction(map, player, weather, gust_direction);
        if (gust_result == MAP_MOVE_EXHAUSTED) {
            return gust_result;
        }
        if (gust_result == MAP_MOVE_SUCCESS) {
            moved = true;
            result = result_for_position(map, player);
            if (result == MAP_MOVE_AT_UP_STAIR || result == MAP_MOVE_AT_DOWN_STAIR) {
                return result;
            }
        } else if (!moved) {
            puts("Ветер не смог сдвинуть фермера с места.");
        }
    }

    if (!moved) {
        puts("Путь преграждает стена или препятствие.");
        return MAP_MOVE_BLOCKED;
    }
    return MAP_MOVE_SUCCESS;
}

bool map_player_on_up_stair(const Map *map, const Player *player)
{
    return map != NULL && player != NULL &&
           map_contains(map, player->x, player->y) &&
           map->cells[player->y][player->x] == TILE_UP_STAIR;
}

static void print_map_border(int width)
{
    int column;
    printf("   +");
    for (column = MAP_INDEX_MIN; column < width; ++column) {
        printf("---");
    }
    puts("+");
}

void map_render(const Map *map, const Player *player, const Weather *weather,
                int day, int hour)
{
    int row;
    int column;

    if (map == NULL || map->cells == NULL || player == NULL || weather == NULL ||
        !map_contains(map, player->x, player->y)) {
        return;
    }

    printf("\n========== ПОДЗЕМЕЛЬЕ | ЭТАЖ %d | %s ==========\n",
           map->floor, weather_name(weather->type));
    printf("HP: %d/%d | Выносливость: %d/%d | Золото: %d | Этаж: %d | Время: %02d:00\n",
           player->hp, player->max_hp, player->stamina, player->max_stamina,
           player->gold, map->floor, hour);
    printf("Игровой день: %d | Температура: %d C | Ветер: %d%%\n",
           day, weather->temperature_c, weather->wind_strength);
    printf("Размер карты: ширина %d x высота %d | Позиция: столбец %d, строка %d\n",
           map->width, map->height, player->x + STEP_UNIT, player->y + STEP_UNIT);
    puts("Управление: W — вверх, A — влево, S — вниз, D — вправо; Q — выход только на U.");

    printf("    ");
    for (column = MAP_INDEX_MIN; column < map->width; ++column) {
        printf("%2d ", column + STEP_UNIT);
    }
    putchar('\n');
    print_map_border(map->width);
    for (row = MAP_INDEX_MIN; row < map->height; ++row) {
        printf("%2d |", row + STEP_UNIT);
        for (column = MAP_INDEX_MIN; column < map->width; ++column) {
            char tile = row == player->y && column == player->x
                ? TILE_HERO
                : map->cells[row][column];
            printf(" %c ", tile);
        }
        puts("|");
    }
    print_map_border(map->width);
    puts("Легенда: @ фермер | # стена | . пол | T дерево-препятствие | R камень-препятствие | ~ лёд | U лестница вверх | D лестница вниз.");
    if (map_player_on_up_stair(map, player)) {
        puts("Вы на лестнице U: Q — вернуться домой.");
    } else if (map->cells[player->y][player->x] == TILE_DOWN_STAIR) {
        puts("Вы на лестнице D: перейдите на неё, чтобы спуститься на следующий этаж.");
    }
    if (map->cells[player->y][player->x] == TILE_ICE) {
        puts("Фермер стоит на льду; следующее движение также будет скользящим.");
    }
}
