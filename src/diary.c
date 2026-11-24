#include "diary.h"
#include "constants.h"
#include "inventory.h"
#include "game_time.h"

#include <stdio.h>

static void write_diary_entry(FILE *stream, const char *farmer_name,
                              int current_day, int current_hour,
                              char item_names[MAX_ITEMS][MAX_NAME_LEN])
{
    fprintf(stream, "===== Дневник фермера =====\n");
    fprintf(stream, "Фермер: %s\n", farmer_name);
    game_time_print(stream, current_day, current_hour);
    fprintf(stream, "Инвентарь:\n");
    inventory_write(stream, item_names);
    fprintf(stream, "===========================\n\n");
}

void diary_append(const char *farmer_name, int current_day, int current_hour,
                  char item_names[MAX_ITEMS][MAX_NAME_LEN])
{
    FILE *diary_file;

    write_diary_entry(stdout, farmer_name, current_day, current_hour, item_names);

    diary_file = fopen(DIARY_FILE_NAME, "a");
    if (diary_file == NULL) {
        fprintf(stderr, "Ошибка: не удалось открыть %s для дозаписи.\n", DIARY_FILE_NAME);
        return;
    }

    write_diary_entry(diary_file, farmer_name, current_day, current_hour, item_names);
    if (fclose(diary_file) != 0) {
        fprintf(stderr, "Ошибка при закрытии файла %s.\n", DIARY_FILE_NAME);
        return;
    }

    printf("Сводка добавлена в %s.\n", DIARY_FILE_NAME);
}
