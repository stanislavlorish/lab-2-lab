#include "diary.h"
#include "constants.h"
#include "item_catalog.h"
#include "utils.h"

#include <stdio.h>

static void write_diary_entry(FILE *stream, const Player *player, int day, int hour,
                              char item_names[MAX_ITEMS][MAX_NAME_LEN])
{
    fprintf(stream, "===== Дневник фермера =====\n");
    fprintf(stream, "Фермер: %s\n", player->name);
    print_game_time(stream, day, hour);
    fprintf(stream, "Здоровье: %d/%d | Выносливость: %d/%d | Золото: %d\n",
            player->hp, player->max_hp, player->stamina, player->max_stamina,
            player->gold);
    fprintf(stream, "Инвентарь:\n");
    player_print_inventory(stream, player, item_names);
    fprintf(stream, "===========================\n\n");
}

void diary_append(const Player *player, int day, int hour,
                  char item_names[MAX_ITEMS][MAX_NAME_LEN])
{
    FILE *diary_file;

    if (player == NULL) {
        return;
    }
    write_diary_entry(stdout, player, day, hour, item_names);

    diary_file = fopen(DIARY_FILE_NAME, "a");
    if (diary_file == NULL) {
        fprintf(stderr, "Ошибка: не удалось открыть %s для дозаписи.\n", DIARY_FILE_NAME);
        return;
    }
    write_diary_entry(diary_file, player, day, hour, item_names);
    if (fclose(diary_file) != 0) {
        fprintf(stderr, "Ошибка при закрытии файла %s.\n", DIARY_FILE_NAME);
        return;
    }
    printf("Запись добавлена в %s.\n", DIARY_FILE_NAME);
}
