#include "item_catalog.h"
#include "constants.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void load_default_names(char item_names[MAX_ITEMS][MAX_NAME_LEN])
{
    static const char *default_names[MAX_ITEMS] = {
        "Пусто",
        "Дерево",
        "Камень",
        "Семена_пшеницы",
        "Морковь",
        "Яблоко",
        "Волокно",
        "Железная_руда",
        "Рыба",
        "Молоко"
    };
    int item_id;

    for (item_id = 0; item_id < MAX_ITEMS; ++item_id) {
        snprintf(item_names[item_id], MAX_NAME_LEN, "%s", default_names[item_id]);
    }
}

static bool parse_item_line(char *line, int *item_id_out, char *name_out)
{
    char *cursor = line;
    char *end = NULL;
    char *name_end;
    long parsed_id;

    line[strcspn(line, "\r\n")] = '\0';
    while (isspace((unsigned char)*cursor)) {
        ++cursor;
    }

    if (*cursor == '\0' || *cursor == '#') {
        return false;
    }

    errno = 0;
    parsed_id = strtol(cursor, &end, INPUT_BASE);
    if (cursor == end || errno == ERANGE || parsed_id < ITEM_ID_MIN || parsed_id >= MAX_ITEMS) {
        return false;
    }
    if (!isspace((unsigned char)*end)) {
        return false;
    }

    while (isspace((unsigned char)*end)) {
        ++end;
    }

    name_end = end + strlen(end);
    while (name_end > end && isspace((unsigned char)name_end[-1])) {
        --name_end;
    }
    *name_end = '\0';

    if (*end == '\0' || strlen(end) >= MAX_NAME_LEN) {
        return false;
    }

    strcpy(name_out, end);
    *item_id_out = (int)parsed_id;
    return true;
}

void item_catalog_load(char item_names[MAX_ITEMS][MAX_NAME_LEN])
{
    FILE *file;
    char line[MAX_FILE_LINE_LEN];
    int line_number = 0;
    bool loaded[MAX_ITEMS] = { false };
    int item_id;

    load_default_names(item_names);
    file = fopen(ITEMS_FILE_NAME, "r");
    if (file == NULL) {
        fprintf(stderr,
                "Предупреждение: не удалось открыть %s. Используются стандартные названия.\n",
                ITEMS_FILE_NAME);
        return;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        char parsed_name[MAX_NAME_LEN];
        int parsed_id;
        int character;

        ++line_number;

        if (strchr(line, '\n') == NULL && !feof(file)) {
            do {
                character = fgetc(file);
            } while (character != '\n' && character != EOF);
            fprintf(stderr, "Предупреждение: строка %d в %s слишком длинная и пропущена.\n",
                    line_number, ITEMS_FILE_NAME);
            continue;
        }

        if (parse_item_line(line, &parsed_id, parsed_name)) {
            if (loaded[parsed_id]) {
                fprintf(stderr, "Предупреждение: повторный ID %d в строке %d; использовано последнее название.\n",
                        parsed_id, line_number);
            }
            snprintf(item_names[parsed_id], MAX_NAME_LEN, "%s", parsed_name);
            loaded[parsed_id] = true;
        } else if (line[0] != '\0' && line[0] != '\n' && line[0] != '\r' && line[0] != '#') {
            fprintf(stderr, "Предупреждение: некорректная строка %d в %s.\n",
                    line_number, ITEMS_FILE_NAME);
        }
    }

    if (ferror(file)) {
        fprintf(stderr, "Предупреждение: ошибка чтения файла %s.\n", ITEMS_FILE_NAME);
    }

    fclose(file);

    for (item_id = 0; item_id < MAX_ITEMS; ++item_id) {
        if (!loaded[item_id]) {
            fprintf(stderr, "Предупреждение: для ID %d оставлено стандартное название.\n",
                    item_id);
        }
    }
}

const char *item_catalog_get_name(char item_names[MAX_ITEMS][MAX_NAME_LEN], int item_id)
{
    if (item_id < ITEM_ID_MIN || item_id >= MAX_ITEMS) {
        return "Неизвестный предмет";
    }
    return item_names[item_id];
}

bool item_catalog_find_id(char item_names[MAX_ITEMS][MAX_NAME_LEN],
                          const char *search_name, int *item_id_out)
{
    int item_id;

    for (item_id = ITEM_ID_MIN; item_id < MAX_ITEMS; ++item_id) {
        if (strcmp(item_names[item_id], search_name) == 0) {
            *item_id_out = item_id;
            return true;
        }
    }

    return false;
}
