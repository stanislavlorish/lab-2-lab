#include "resource_report.h"
#include "constants.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool line_is_blank(const char *line)
{
    while (isspace((unsigned char)*line)) {
        ++line;
    }
    return *line == '\0';
}

static bool parse_resource_line(char *line, char *name_out, long long *quantity_out)
{
    char *start = line;
    char *end;
    char *quantity_text;
    char *quantity_end = NULL;
    char *name_end;
    long long quantity;

    line[strcspn(line, "\r\n")] = '\0';
    while (isspace((unsigned char)*start)) {
        ++start;
    }

    if (*start == '\0') {
        return false;
    }

    end = start + strlen(start);
    while (end > start && isspace((unsigned char)end[-1])) {
        --end;
    }
    *end = '\0';

    quantity_text = end;
    while (quantity_text > start &&
           !isspace((unsigned char)quantity_text[-1])) {
        --quantity_text;
    }
    if (quantity_text == start) {
        return false;
    }

    name_end = quantity_text;
    while (name_end > start && isspace((unsigned char)name_end[-1])) {
        --name_end;
    }
    if (name_end == start) {
        return false;
    }
    *name_end = '\0';

    errno = 0;
    quantity = strtoll(quantity_text, &quantity_end, INPUT_BASE);
    if (quantity_end == quantity_text || errno == ERANGE ||
        quantity < MIN_RESOURCE_QUANTITY) {
        return false;
    }

    while (isspace((unsigned char)*quantity_end)) {
        ++quantity_end;
    }
    if (*quantity_end != '\0' || strlen(start) >= MAX_NAME_LEN) {
        return false;
    }

    strcpy(name_out, start);
    *quantity_out = quantity;
    return true;
}

static int find_resource(char resource_names[MAX_RESOURCE_TYPES][MAX_NAME_LEN],
                         int resource_count, const char *name)
{
    int index;

    for (index = 0; index < resource_count; ++index) {
        if (strcmp(resource_names[index], name) == 0) {
            return index;
        }
    }

    return -1;
}

static void write_resource_report(FILE *stream,
                                  char resource_names[MAX_RESOURCE_TYPES][MAX_NAME_LEN],
                                  const long long resource_totals[MAX_RESOURCE_TYPES],
                                  int resource_count, int valid_lines, int skipped_lines)
{
    int index;

    fprintf(stream, "Сводная ведомость поставок:\n");
    if (resource_count == 0) {
        fprintf(stream, "Корректных записей не найдено.\n");
    }

    for (index = 0; index < resource_count; ++index) {
        fprintf(stream, "%s: %lld шт.\n",
                resource_names[index], resource_totals[index]);
    }

    fprintf(stream, "Успешно обработано записей: %d\n", valid_lines);
    fprintf(stream, "Пропущено некорректных строк: %d\n", skipped_lines);
}

static void write_input_error(const char *message)
{
    FILE *output_file;

    fprintf(stderr, "%s\n", message);
    output_file = fopen(RESOURCE_OUTPUT_FILE_NAME, "w");
    if (output_file != NULL) {
        fprintf(output_file, "%s\n", message);
        fclose(output_file);
    }
}

void resource_report_run(void)
{
    FILE *input_file;
    FILE *output_file;
    char resource_names[MAX_RESOURCE_TYPES][MAX_NAME_LEN] = { { 0 } };
    long long resource_totals[MAX_RESOURCE_TYPES] = { 0 };
    char line[MAX_FILE_LINE_LEN];
    int resource_count = 0;
    int valid_lines = 0;
    int skipped_lines = 0;
    int line_number = 0;

    input_file = fopen(RESOURCE_INPUT_FILE_NAME, "r");
    if (input_file == NULL) {
        write_input_error("Ошибка: не удалось открыть input.txt.");
        return;
    }

    while (fgets(line, sizeof(line), input_file) != NULL) {
        char resource_name[MAX_NAME_LEN];
        long long quantity;
        int resource_index;
        int character;

        ++line_number;

        if (strchr(line, '\n') == NULL && !feof(input_file)) {
            do {
                character = fgetc(input_file);
            } while (character != '\n' && character != EOF);
            ++skipped_lines;
            fprintf(stderr, "Строка %d слишком длинная и пропущена.\n", line_number);
            continue;
        }

        if (line_is_blank(line)) {
            continue;
        }

        if (!parse_resource_line(line, resource_name, &quantity)) {
            ++skipped_lines;
            fprintf(stderr, "Строка %d имеет неверный формат и пропущена.\n", line_number);
            continue;
        }

        resource_index = find_resource(resource_names, resource_count, resource_name);
        if (resource_index < 0) {
            if (resource_count >= MAX_RESOURCE_TYPES) {
                ++skipped_lines;
                fprintf(stderr, "Достигнут предел числа разных ресурсов; строка %d пропущена.\n",
                        line_number);
                continue;
            }
            resource_index = resource_count;
            snprintf(resource_names[resource_index], MAX_NAME_LEN, "%s", resource_name);
            resource_totals[resource_index] = 0;
            ++resource_count;
        }

        if (resource_totals[resource_index] > LLONG_MAX - quantity) {
            ++skipped_lines;
            fprintf(stderr, "Переполнение суммы в строке %d; строка пропущена.\n", line_number);
            continue;
        }

        resource_totals[resource_index] += quantity;
        ++valid_lines;
    }

    if (ferror(input_file)) {
        fprintf(stderr, "Ошибка чтения файла %s.\n", RESOURCE_INPUT_FILE_NAME);
        ++skipped_lines;
    }
    fclose(input_file);

    write_resource_report(stdout, resource_names, resource_totals,
                          resource_count, valid_lines, skipped_lines);

    output_file = fopen(RESOURCE_OUTPUT_FILE_NAME, "w");
    if (output_file == NULL) {
        fprintf(stderr, "Ошибка: не удалось создать %s.\n", RESOURCE_OUTPUT_FILE_NAME);
        return;
    }

    write_resource_report(output_file, resource_names, resource_totals,
                          resource_count, valid_lines, skipped_lines);
    if (fclose(output_file) != 0) {
        fprintf(stderr, "Ошибка при закрытии файла %s.\n", RESOURCE_OUTPUT_FILE_NAME);
        return;
    }

    printf("Отчёт также сохранён в %s.\n", RESOURCE_OUTPUT_FILE_NAME);
}
