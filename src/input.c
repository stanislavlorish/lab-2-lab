#include "input.h"
#include "constants.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void discard_remaining_line(void)
{
    int character;

    do {
        character = getchar();
    } while (character != '\n' && character != EOF);
}

bool read_integer(const char *prompt, int min_value, int max_value, int *value_out)
{
    char input[INPUT_BUFFER_SIZE];

    while (true) {
        char *end = NULL;
        long parsed_value;

        printf("%s", prompt);
        if (fgets(input, sizeof(input), stdin) == NULL) {
            return false;
        }

        if (strchr(input, '\n') == NULL && !feof(stdin)) {
            discard_remaining_line();
            printf("Ввод слишком длинный. Попробуйте ещё раз.\n");
            continue;
        }

        errno = 0;
        parsed_value = strtol(input, &end, INPUT_BASE);

        if (end == input) {
            printf("Некорректный ввод. Введите целое число от %d до %d.\n",
                   min_value, max_value);
            continue;
        }

        while (isspace((unsigned char)*end)) {
            ++end;
        }

        if (errno == ERANGE || *end != '\0' ||
            parsed_value < min_value || parsed_value > max_value) {
            printf("Некорректный ввод. Введите целое число от %d до %d.\n",
                   min_value, max_value);
            continue;
        }

        *value_out = (int)parsed_value;
        return true;
    }
}

bool read_line(const char *prompt, char *buffer, size_t buffer_size)
{
    if (buffer == NULL || buffer_size < 2) {
        return false;
    }

    while (true) {
        char *start;
        char *end;

        printf("%s", prompt);
        if (fgets(buffer, (int)buffer_size, stdin) == NULL) {
            return false;
        }

        if (strchr(buffer, '\n') == NULL && !feof(stdin)) {
            discard_remaining_line();
            printf("Строка слишком длинная. Допустимый размер — не более %zu байт.\n",
                   buffer_size - 1);
            continue;
        }

        buffer[strcspn(buffer, "\r\n")] = '\0';
        start = buffer;
        while (isspace((unsigned char)*start)) {
            ++start;
        }

        end = start + strlen(start);
        while (end > start && isspace((unsigned char)end[-1])) {
            --end;
        }
        *end = '\0';

        if (start != buffer) {
            memmove(buffer, start, strlen(start) + 1);
        }

        if (buffer[0] == '\0') {
            printf("Пустая строка недопустима. Повторите ввод.\n");
            continue;
        }

        return true;
    }
}

size_t utf8_character_count(const char *text)
{
    const unsigned char *current = (const unsigned char *)text;
    size_t count = 0;

    while (*current != '\0') {
        if ((*current & 0xC0U) != 0x80U) {
            ++count;
        }
        ++current;
    }

    return count;
}
