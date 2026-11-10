#include "adventure.h"
#include "constants.h"
#include "input.h"

#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int random_between(int min_value, int max_value)
{
    return min_value + rand() % (max_value - min_value + RANDOM_INCLUSIVE_OFFSET);
}

static char **allocate_map(int height, int width)
{
    char **map = calloc((size_t)height, sizeof(*map));
    int row;

    if (map == NULL) {
        return NULL;
    }

    for (row = 0; row < height; ++row) {
        map[row] = malloc((size_t)width * sizeof(*map[row]));
        if (map[row] == NULL) {
            while (row > 0) {
                --row;
                free(map[row]);
            }
            free(map);
            return NULL;
        }
    }

    return map;
}

static void free_map(char ***map_pointer, int height)
{
    int row;

    if (map_pointer == NULL || *map_pointer == NULL) {
        return;
    }

    for (row = 0; row < height; ++row) {
        free((*map_pointer)[row]);
    }
    free(*map_pointer);
    *map_pointer = NULL;
}

static void place_down_stair(char **map, int height, int width,
                             int player_x, int player_y)
{
    int down_y = height - MAP_BOUNDARY_THICKNESS - DIRECTION_STEP;
    int down_x = random_between(MAP_BOUNDARY_THICKNESS,
                                width - MAP_BOUNDARY_THICKNESS - DIRECTION_STEP);
    int row;
    int column;

    for (row = player_y + DIRECTION_STEP; row <= down_y; ++row) {
        map[row][player_x] = TILE_FLOOR;
    }

    if (player_x <= down_x) {
        for (column = player_x; column < down_x; ++column) {
            map[down_y][column] = TILE_FLOOR;
        }
    } else {
        for (column = player_x; column > down_x; --column) {
            map[down_y][column] = TILE_FLOOR;
        }
    }

    map[down_y][down_x] = TILE_DOWN_STAIR;
}

static bool generate_dungeon_floor(char ***map_out, int *height_out, int *width_out,
                                   int *player_x_out, int *player_y_out)
{
    static const char obstacle_tiles[OBSTACLE_TYPE_COUNT] = {
        TILE_OBSTACLE_FIRST,
        TILE_OBSTACLE_SECOND
    };
    char **map;
    int row;
    int column;
    int obstacle_percent;
    int up_x;
    int up_y;
    int ice_x;
    int ice_y;

    *map_out = NULL;
    *height_out = random_between(MAP_MIN_SIZE, MAP_MAX_SIZE);
    *width_out = random_between(MAP_MIN_SIZE, MAP_MAX_SIZE);
    map = allocate_map(*height_out, *width_out);
    if (map == NULL) {
        return false;
    }

    obstacle_percent = random_between(MAP_OBSTACLE_PERCENT_MIN,
                                      MAP_OBSTACLE_PERCENT_MAX);

    for (row = 0; row < *height_out; ++row) {
        for (column = 0; column < *width_out; ++column) {
            bool is_boundary = row < MAP_BOUNDARY_THICKNESS ||
                row >= *height_out - MAP_BOUNDARY_THICKNESS ||
                column < MAP_BOUNDARY_THICKNESS ||
                column >= *width_out - MAP_BOUNDARY_THICKNESS;

            if (is_boundary) {
                map[row][column] = TILE_WALL;
            } else if (random_between(DIRECTION_STEP, PERCENT_CHANCE_MAX) <= obstacle_percent) {
                int obstacle_type = random_between(DIRECTION_NONE,
                                                   OBSTACLE_TYPE_COUNT - DIRECTION_STEP);
                map[row][column] = obstacle_tiles[obstacle_type];
            } else if (random_between(DIRECTION_STEP, PERCENT_CHANCE_MAX) <= ICE_CHANCE_PERCENT) {
                map[row][column] = TILE_ICE;
            } else {
                map[row][column] = TILE_FLOOR;
            }
        }
    }

    *player_x_out = *width_out / MAP_CENTER_DIVISOR;
    *player_y_out = *height_out / MAP_CENTER_DIVISOR;
    up_x = *player_x_out - DIRECTION_STEP;
    up_y = *player_y_out;
    ice_x = *player_x_out + DIRECTION_STEP;
    ice_y = *player_y_out;

    map[*player_y_out][*player_x_out] = TILE_FLOOR;
    map[up_y][up_x] = TILE_UP_STAIR;
    map[ice_y][ice_x] = TILE_ICE;

    place_down_stair(map, *height_out, *width_out,
                     *player_x_out, *player_y_out);

    *map_out = map;
    return true;
}

static void print_map_border(int width)
{
    int column;

    printf("   +");
    for (column = 0; column < width; ++column) {
        printf("---");
    }
    puts("+");
}

static void print_map(char **map, int height, int width,
                      int player_x, int player_y, int depth)
{
    int row;
    int column;

    printf("\n========== ПОДЗЕМЕЛЬЕ | ЭТАЖ %d ==========\n", depth);
    printf("Размер карты: ширина %d x высота %d | Позиция фермера: столбец %d, строка %d\n",
           width, height, player_x + DIRECTION_STEP, player_y + DIRECTION_STEP);
    puts("Управление: W — вверх, A — влево, S — вниз, D — вправо; Q — выход на U.");

    printf("    ");
    for (column = 0; column < width; ++column) {
        printf("%2d ", column + DIRECTION_STEP);
    }
    putchar('\n');
    print_map_border(width);

    for (row = 0; row < height; ++row) {
        printf("%2d |", row + DIRECTION_STEP);
        for (column = 0; column < width; ++column) {
            char tile = row == player_y && column == player_x
                ? TILE_HERO
                : map[row][column];
            printf(" %c ", tile);
        }
        puts("|");
    }
    print_map_border(width);

    puts("Легенда: @ фермер | # стена | . пол | T дерево-препятствие | R камень-препятствие | ~ лёд | U вверх | D вниз.");
    if (map[player_y][player_x] == TILE_UP_STAIR) {
        puts("Фермер стоит на лестнице U: нажмите Q, чтобы выйти из подземелья.");
    } else if (map[player_y][player_x] == TILE_ICE) {
        puts("Фермер стоит на льду: при следующем движении он продолжит скользить.");
    }
}

static bool direction_delta(char direction, int *delta_x, int *delta_y)
{
    *delta_x = DIRECTION_NONE;
    *delta_y = DIRECTION_NONE;

    switch (direction) {
        case KEY_UP:
            *delta_y = -DIRECTION_STEP;
            return true;
        case KEY_LEFT:
            *delta_x = -DIRECTION_STEP;
            return true;
        case KEY_DOWN:
            *delta_y = DIRECTION_STEP;
            return true;
        case KEY_RIGHT:
            *delta_x = DIRECTION_STEP;
            return true;
        default:
            return false;
    }
}

static bool is_inside_map(int x, int y, int width, int height)
{
    return x >= MAP_INDEX_MIN && x < width &&
           y >= MAP_INDEX_MIN && y < height;
}

static bool is_walkable(char tile)
{
    return tile == TILE_FLOOR || tile == TILE_ICE ||
           tile == TILE_UP_STAIR || tile == TILE_DOWN_STAIR;
}

static void slide_on_ice(char **map, int height, int width,
                         int *player_x, int *player_y,
                         int delta_x, int delta_y)
{
    if (map[*player_y][*player_x] != TILE_ICE) {
        return;
    }

    puts("Фермер ступил на лёд и скользит в том же направлении!");

    while (map[*player_y][*player_x] == TILE_ICE) {
        int next_x = *player_x + delta_x;
        int next_y = *player_y + delta_y;

        if (!is_inside_map(next_x, next_y, width, height) ||
            !is_walkable(map[next_y][next_x])) {
            puts("Скольжение остановилось у стены или препятствия.");
            return;
        }

        *player_x = next_x;
        *player_y = next_y;

        if (map[*player_y][*player_x] == TILE_FLOOR) {
            puts("Фермер выехал на обычный пол.");
            return;
        }
        if (map[*player_y][*player_x] == TILE_UP_STAIR ||
            map[*player_y][*player_x] == TILE_DOWN_STAIR) {
            puts("Скольжение остановилось у лестницы.");
            return;
        }
    }
}

void adventure_run(void)
{
    char **map = NULL;
    int height;
    int width;
    int player_x;
    int player_y;
    int depth = DIRECTION_STEP;

    if (!generate_dungeon_floor(&map, &height, &width, &player_x, &player_y)) {
        fprintf(stderr, "Ошибка: не удалось выделить память для карты.\n");
        return;
    }

    puts("Приключение началось. W/A/S/D — движение (D — вправо); Q — выход только на U.");
    print_map(map, height, width, player_x, player_y, depth);

    while (true) {
        char command[INPUT_BUFFER_SIZE];
        char direction;
        int delta_x;
        int delta_y;
        int new_x;
        int new_y;

        if (!read_line("Ход (W/A/S/D; Q — выход на U): ", command, sizeof(command))) {
            puts("Ввод завершён. Выход из приключения.");
            break;
        }

        if (strlen(command) != DIRECTION_STEP) {
            puts("Введите одну клавишу: W, A, S, D или Q.");
            print_map(map, height, width, player_x, player_y, depth);
            continue;
        }

        direction = (char)toupper((unsigned char)command[0]);
        if (direction == KEY_QUIT) {
            if (map[player_y][player_x] == TILE_UP_STAIR) {
                puts("Вы вышли из подземелья домой.");
                break;
            }
            puts("Домой можно выйти только с лестницы вверх U.");
            print_map(map, height, width, player_x, player_y, depth);
            continue;
        }

        if (!direction_delta(direction, &delta_x, &delta_y)) {
            puts("Неизвестное направление. Используйте W, A, S, D или Q.");
            print_map(map, height, width, player_x, player_y, depth);
            continue;
        }

        new_x = player_x + delta_x;
        new_y = player_y + delta_y;

        if (!is_inside_map(new_x, new_y, width, height)) {
            puts("За пределами карты пройти нельзя.");
        } else if (!is_walkable(map[new_y][new_x])) {
            puts("Путь преграждает стена или препятствие.");
        } else {
            player_x = new_x;
            player_y = new_y;
            slide_on_ice(map, height, width, &player_x, &player_y, delta_x, delta_y);

            if (map[player_y][player_x] == TILE_DOWN_STAIR) {
                if (depth == INT_MAX) {
                    puts("Достигнута максимальная глубина.");
                } else {
                    free_map(&map, height);
                    ++depth;

                    if (!generate_dungeon_floor(&map, &height, &width,
                                                &player_x, &player_y)) {
                        fprintf(stderr, "Ошибка: не удалось выделить память для нового этажа.\n");
                        break;
                    }

                    printf("Вы спустились на новый этаж. Глубина: %d.\n", depth);
                }
            } else if (map[player_y][player_x] == TILE_UP_STAIR) {
                puts("Вы на лестнице вверх U. Нажмите Q, чтобы выйти домой.");
            }
        }

        print_map(map, height, width, player_x, player_y, depth);
    }

    free_map(&map, height);
    puts("Память карты освобождена.");
}
