#include "game.h"
#include "constants.h"
#include "diary.h"
#include "item_catalog.h"
#include "resource_report.h"
#include "utils.h"

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum FloorTransitionResult {
    FLOOR_TRANSITION_READY,
    FLOOR_TRANSITION_FAILED,
    FLOOR_TRANSITION_EVACUATE
} FloorTransitionResult;

static bool require_player(const GameState *game, const char *action)
{
    if (game == NULL || game->player == NULL) {
        printf("Сначала создайте персонажа в пункте [1], чтобы выполнить действие «%s».\n",
               action);
        return false;
    }
    return true;
}

static void announce_weather(const GameState *game, const char *reason)
{
    printf("Погода изменилась (%s): %s, температура %d C, ветер %d%%.\n",
           reason, weather_name(game->weather.type), game->weather.temperature_c,
           game->weather.wind_strength);
}

void game_initialize(GameState *game)
{
    if (game == NULL) {
        return;
    }
    memset(game, 0, sizeof(*game));
    game->day = STARTING_DAY;
    game->hour = STARTING_HOUR;
    item_catalog_load(game->item_names);
    weather_randomize(&game->weather);
}

void game_destroy(GameState *game)
{
    if (game == NULL) {
        return;
    }
    map_destroy(&game->map);
    player_destroy(&game->player);
}

bool game_has_player(const GameState *game)
{
    return game != NULL && game->player != NULL;
}

void game_create_player(GameState *game)
{
    char name[MAX_FARMER_NAME_LEN];

    if (game == NULL) {
        return;
    }
    if (game->player != NULL) {
        bool confirm;
        if (!read_yes_no("Пересоздать героя? Старые данные будут удалены (Y/N): ",
                         &confirm)) {
            return;
        }
        if (!confirm) {
            puts("Пересоздание отменено.");
            return;
        }
        player_destroy(&game->player);
    }

    for (;;) {
        char prompt[INPUT_BUFFER_SIZE];
        snprintf(prompt, sizeof(prompt), "Введите имя фермера (до %d символов): ",
                 MAX_FARMER_NAME_CHARS);
        if (!read_line(prompt, name, sizeof(name))) {
            puts("Создание персонажа отменено: ввод завершён.");
            return;
        }
        if (utf8_character_count(name) <= MAX_FARMER_NAME_CHARS) {
            break;
        }
        printf("Имя должно содержать не более %d символов.\n",
               MAX_FARMER_NAME_CHARS);
    }

    game->player = player_create(name);
    if (game->player == NULL) {
        fprintf(stderr, "Ошибка: не удалось выделить память для персонажа.\n");
        return;
    }
    printf("Персонаж «%s» создан. HP: %d, выносливость: %d, золото: %d.\n",
           game->player->name, game->player->hp, game->player->stamina,
           game->player->gold);
}

void game_show_character(const GameState *game)
{
    const Player *player;

    if (!require_player(game, "просмотр карточки персонажа")) {
        return;
    }
    player = game->player;
    puts("\n========== КАРТОЧКА ПЕРСОНАЖА ==========");
    printf("Имя: %s\n", player->name);
    printf("Здоровье: %d/%d\n", player->hp, player->max_hp);
    printf("Выносливость: %d/%d\n", player->stamina, player->max_stamina);
    printf("Золото: %d\n", player->gold);
    printf("Последние координаты на карте: x=%d, y=%d\n", player->x, player->y);
    print_game_time(stdout, game->day, game->hour);
    print_real_datetime(stdout);
    puts("Инвентарь:");
    player_print_inventory(stdout, player, (char (*)[MAX_NAME_LEN])game->item_names);
    puts("=========================================");
}

void game_show_inventory(const GameState *game)
{
    if (!require_player(game, "просмотр инвентаря")) {
        return;
    }
    player_print_inventory(stdout, game->player,
                           (char (*)[MAX_NAME_LEN])game->item_names);
}

void game_place_item(GameState *game)
{
    int slot;
    int item_id;
    char prompt[INPUT_BUFFER_SIZE];

    if (!require_player(game, "изменение инвентаря")) {
        return;
    }
    snprintf(prompt, sizeof(prompt), "Индекс слота (%d-%d): ", SLOT_FIRST, SLOT_LAST);
    if (!read_integer(prompt, SLOT_FIRST, SLOT_LAST, &slot)) {
        return;
    }
    snprintf(prompt, sizeof(prompt), "ID предмета (%d-%d): ", ITEM_ID_MIN, ITEM_ID_MAX);
    if (!read_integer(prompt, ITEM_ID_MIN, ITEM_ID_MAX, &item_id)) {
        return;
    }
    if (!player_set_item(game->player, slot, item_id)) {
        puts("Не удалось изменить слот.");
        return;
    }
    printf("В слот %d помещено: %s.\n", slot,
           item_catalog_get_name(game->item_names, item_id));
}

void game_drop_item(GameState *game)
{
    int slot;
    char prompt[INPUT_BUFFER_SIZE];

    if (!require_player(game, "изменение инвентаря")) {
        return;
    }
    snprintf(prompt, sizeof(prompt), "Индекс слота (%d-%d): ", SLOT_FIRST, SLOT_LAST);
    if (!read_integer(prompt, SLOT_FIRST, SLOT_LAST, &slot)) {
        return;
    }
    if (player_get_slot(game->player, slot) == ITEM_EMPTY) {
        printf("Слот %d уже пуст.\n", slot);
        return;
    }
    player_clear_slot(game->player, slot);
    printf("Предмет из слота %d выброшен.\n", slot);
}

void game_sort_inventory(GameState *game)
{
    if (!require_player(game, "сортировка инвентаря")) {
        return;
    }
    puts("Инвентарь до сортировки:");
    player_print_inventory(stdout, game->player,
                           (char (*)[MAX_NAME_LEN])game->item_names);
    player_compact_inventory(game->player);
    puts("Инвентарь после сортировки:");
    player_print_inventory(stdout, game->player,
                           (char (*)[MAX_NAME_LEN])game->item_names);
}

void game_search_item(GameState *game)
{
    char name[MAX_NAME_LEN];
    int item_id;
    int slots[INVENTORY_SIZE];
    int count;
    int index;

    if (!require_player(game, "поиск предмета")) {
        return;
    }
    if (!read_line("Введите точное название предмета: ", name, sizeof(name))) {
        return;
    }
    if (!item_catalog_find_id(game->item_names, name, &item_id)) {
        printf("Предмет «%s» отсутствует в каталоге.\n", name);
        return;
    }
    count = player_find_slots(game->player, item_id, slots);
    if (count == 0) {
        printf("Предмет «%s» есть в каталоге, но отсутствует в инвентаре.\n", name);
        return;
    }
    printf("Предмет «%s» (ID %d) находится в слотах: ", name, item_id);
    for (index = 0; index < count; ++index) {
        printf("%d%s", slots[index], index + STEP_UNIT < count ? ", " : "");
    }
    putchar('\n');
}

void game_write_diary(GameState *game)
{
    if (!require_player(game, "запись в дневник")) {
        return;
    }
    diary_append(game->player, game->day, game->hour, game->item_names);
}

static void show_time_menu(void)
{
    puts("\n========== ВРЕМЯ И ОТДЫХ ==========");
    printf("[%d] Назад\n", TIME_MENU_BACK);
    printf("[%d] Посмотреть на часы\n", TIME_MENU_SHOW_CLOCK);
    printf("[%d] Ждать\n", TIME_MENU_WAIT);
    printf("[%d] Лечь спать\n", TIME_MENU_SLEEP);
}

static void wait_for_hours(GameState *game)
{
    int maximum = hours_until_night(game->hour);
    int hours;
    bool day_changed = false;

    if (maximum <= 0) {
        puts("Уже 02:00. Ждать дальше нельзя — фермеру нужно лечь спать.");
        return;
    }
    printf("До ближайших 02:00 можно ждать не более %d ч.\n", maximum);
    if (!read_integer("Сколько часов ждать? ", 1, maximum, &hours)) {
        return;
    }
    if (!advance_game_time(&game->day, &game->hour, hours, &day_changed)) {
        puts("Не удалось изменить время: номер дня достиг предела.");
        return;
    }
    if (day_changed) {
        weather_randomize(&game->weather);
        announce_weather(game, "начался новый день");
    }
    print_game_time(stdout, game->day, game->hour);
    if (game->hour == NIGHT_HOUR) {
        puts("Наступило 02:00. Чтобы продолжить, выберите отдых.");
    }
}

static void sleep_until_morning(GameState *game)
{
    if (game->day == INT_MAX) {
        puts("Нельзя начать новый день: достигнут предел счётчика дней.");
        return;
    }
    ++game->day;
    game->hour = MORNING_HOUR;
    if (game->player != NULL) {
        game->player->hp = game->player->max_hp;
        game->player->stamina = game->player->max_stamina;
        puts("Здоровье и выносливость полностью восстановлены.");
    }
    weather_randomize(&game->weather);
    announce_weather(game, "наступило утро");
    print_game_time(stdout, game->day, game->hour);
}

void game_time_menu(GameState *game)
{
    int choice;

    if (game == NULL) {
        return;
    }
    for (;;) {
        show_time_menu();
        if (!read_integer("Выберите действие: ", TIME_MENU_BACK, TIME_MENU_SLEEP, &choice)) {
            return;
        }
        switch (choice) {
            case TIME_MENU_BACK:
                return;
            case TIME_MENU_SHOW_CLOCK:
                print_game_time(stdout, game->day, game->hour);
                break;
            case TIME_MENU_WAIT:
                wait_for_hours(game);
                break;
            case TIME_MENU_SLEEP:
                sleep_until_morning(game);
                break;
            default:
                puts("Неизвестный пункт подменю.");
                break;
        }
    }
}

void game_show_resource_report(void)
{
    resource_report_run();
}

static bool char_to_direction(char command, Direction *direction_out)
{
    if (direction_out == NULL) {
        return false;
    }
    switch (command) {
        case KEY_UP:
            *direction_out = DIR_UP;
            return true;
        case KEY_LEFT:
            *direction_out = DIR_LEFT;
            return true;
        case KEY_DOWN:
            *direction_out = DIR_DOWN;
            return true;
        case KEY_RIGHT:
            *direction_out = DIR_RIGHT;
            return true;
        default:
            *direction_out = DIR_NONE;
            return false;
    }
}

static void emergency_return(GameState *game, const char *reason)
{
    int previous_gold = 0;

    if (game == NULL) {
        return;
    }
    puts(reason);
    map_destroy(&game->map);
    if (game->player != NULL) {
        previous_gold = game->player->gold;
        game->player->gold /= 2;
        game->player->x = 0;
        game->player->y = 0;
        printf("Спасательная служба доставила фермера домой: золото %d -> %d.\n",
               previous_gold, game->player->gold);
    }
}

static FloorTransitionResult transition_to_next_floor(GameState *game)
{
    int next_floor;
    int spawn_x;
    int spawn_y;
    bool day_changed = false;
    bool stamina_remains;
    Map *next_map;
    long long reward;

    if (game->map->floor == INT_MAX) {
        puts("Достигнута максимальная глубина; дальше спуститься нельзя.");
        return FLOOR_TRANSITION_FAILED;
    }
    next_floor = game->map->floor + STEP_UNIT;
    next_map = map_create(next_floor, &spawn_x, &spawn_y);
    if (next_map == NULL) {
        fprintf(stderr, "Ошибка: не удалось выделить память для следующего этажа.\n");
        return FLOOR_TRANSITION_FAILED;
    }

    if (!advance_game_time(&game->day, &game->hour, STEP_UNIT,
                           &day_changed)) {
        map_destroy(&next_map);
        puts("Переход невозможен: не удалось увеличить игровое время.");
        return FLOOR_TRANSITION_FAILED;
    }
    stamina_remains = player_spend_stamina(game->player,
                                           FLOOR_TRANSITION_STAMINA_COST);
    if (day_changed) {
        weather_randomize(&game->weather);
        announce_weather(game, "начался новый день");
    }
    if (!stamina_remains || game->hour == NIGHT_HOUR) {
        map_destroy(&next_map);
        return FLOOR_TRANSITION_EVACUATE;
    }

    map_destroy(&game->map);
    game->map = next_map;
    game->player->x = spawn_x;
    game->player->y = spawn_y;
    if (!day_changed) {
        weather_randomize(&game->weather);
        announce_weather(game, "новый этаж");
    }
    reward = (long long)BASE_FLOOR_REWARD * next_floor;
    player_add_gold(game->player, reward);
    printf("Вы спустились на этаж %d. Потрачено выносливости: %d; награда: %lld золота.\n",
           next_floor, FLOOR_TRANSITION_STAMINA_COST, reward);
    return FLOOR_TRANSITION_READY;
}

void game_start_adventure(GameState *game)
{
    int spawn_x;
    int spawn_y;
    bool returned_normally = false;
    bool evacuated = false;
    MapMoveResult move_result;

    if (!require_player(game, "поход в подземелье")) {
        return;
    }
    if (game->player->stamina <= 0) {
        puts("Нельзя отправиться в поход: выносливость закончилась. Отдохните.");
        return;
    }
    if (game->hour == NIGHT_HOUR) {
        puts("Нельзя отправиться в поход в 02:00. Сначала лягте спать.");
        return;
    }
    if (game->map != NULL) {
        map_destroy(&game->map);
    }

    game->map = map_create(STEP_UNIT, &spawn_x, &spawn_y);
    if (game->map == NULL) {
        fprintf(stderr, "Ошибка: не удалось создать карту подземелья.\n");
        return;
    }
    game->player->x = spawn_x;
    game->player->y = spawn_y;
    weather_randomize(&game->weather);
    announce_weather(game, "началась экспедиция");
    puts("Экспедиция началась. На карте U — выход, D — спуск на следующий этаж.");

    while (game->map != NULL) {
        char command_text[INPUT_BUFFER_SIZE];
        char command;
        Direction direction;

        map_render(game->map, game->player, &game->weather, game->day, game->hour);
        if (!read_line("Ход (W/A/S/D; Q — выйти на U): ",
                       command_text, sizeof(command_text))) {
            puts("Ввод завершён; экспедиция прервана без штрафа.");
            break;
        }
        if (strlen(command_text) != STRING_TERMINATOR_SIZE) {
            puts("Введите одну клавишу: W, A, S, D или Q.");
            continue;
        }
        command = (char)toupper((unsigned char)command_text[0]);
        if (command == KEY_QUIT) {
            if (map_player_on_up_stair(game->map, game->player)) {
                puts("Фермер вернулся домой. Заработанное золото сохранено.");
                returned_normally = true;
                break;
            }
            puts("Выйти можно только стоя на лестнице вверх U.");
            continue;
        }
        if (!char_to_direction(command, &direction)) {
            puts("Неизвестная команда. Используйте W, A, S, D или Q.");
            continue;
        }

        move_result = map_move(game->map, game->player, &game->weather, direction);
        if (move_result == MAP_MOVE_EXHAUSTED) {
            emergency_return(game,
                "Фермер потерял силы и потерял сознание! Начинается экстренная эвакуация.");
            evacuated = true;
            break;
        }
        if (move_result == MAP_MOVE_AT_DOWN_STAIR) {
            FloorTransitionResult transition = transition_to_next_floor(game);
            if (transition == FLOOR_TRANSITION_EVACUATE) {
                emergency_return(game,
                    game->hour == NIGHT_HOUR
                        ? "Наступило 02:00 ночи. Фермер теряет сознание; начинается эвакуация."
                        : "После перехода выносливость закончилась; начинается экстренная эвакуация.");
                evacuated = true;
                break;
            }
        }
    }

    if (!evacuated) {
        map_destroy(&game->map);
        if (!returned_normally) {
            puts("Экспедиция завершена; фермер вернулся домой.");
        }
    }
}
