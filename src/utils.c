#include "utils.h"
#include "constants.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

bool discard_line_tail(FILE *stream)
{
    int character;

    if (stream == NULL) {
        return false;
    }
    character = fgetc(stream);
    if (character == EOF || character == '\n') {
        return false;
    }
    if (character == '\r') {
        int next_character = fgetc(stream);
        if (next_character == '\n' || next_character == EOF) {
            return false;
        }
    }
    do {
        character = fgetc(stream);
    } while (character != '\n' && character != EOF);
    return true;
}

bool read_integer(const char *prompt, int min_value, int max_value, int *value_out)
{
    char input[INPUT_BUFFER_SIZE];

    if (prompt == NULL || value_out == NULL || min_value > max_value) {
        return false;
    }

    while (true) {
        char *end = NULL;
        long parsed_value;

        printf("%s", prompt);
        fflush(stdout);
        if (fgets(input, sizeof(input), stdin) == NULL) {
            return false;
        }

        if (strchr(input, '\n') == NULL && discard_line_tail(stdin)) {
            puts("Ввод слишком длинный. Попробуйте ещё раз.");
            continue;
        }

        errno = 0;
        parsed_value = strtol(input, &end, INPUT_BASE);
        if (end == input) {
            printf("Введите целое число от %d до %d.\n", min_value, max_value);
            continue;
        }

        while (isspace((unsigned char)*end)) {
            ++end;
        }
        if (errno == ERANGE || *end != '\0' ||
            parsed_value < min_value || parsed_value > max_value) {
            printf("Введите целое число от %d до %d.\n", min_value, max_value);
            continue;
        }

        *value_out = (int)parsed_value;
        return true;
    }
}

bool read_line(const char *prompt, char *buffer, size_t buffer_size)
{
    if (prompt == NULL || buffer == NULL || buffer_size < 2 ||
        buffer_size > (size_t)INT_MAX) {
        return false;
    }

    while (true) {
        char *start;
        char *end;

        printf("%s", prompt);
        fflush(stdout);
        if (fgets(buffer, (int)buffer_size, stdin) == NULL) {
            return false;
        }

        if (strchr(buffer, '\n') == NULL && discard_line_tail(stdin)) {
            printf("Строка слишком длинная (максимум %zu байт).\n",
                   buffer_size - STRING_TERMINATOR_SIZE);
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
            memmove(buffer, start, strlen(start) + STRING_TERMINATOR_SIZE);
        }
        if (buffer[0] == '\0') {
            puts("Пустая строка недопустима. Повторите ввод.");
            continue;
        }
        return true;
    }
}

bool read_yes_no(const char *prompt, bool *answer_out)
{
    char answer[INPUT_BUFFER_SIZE];

    if (answer_out == NULL) {
        return false;
    }

    for (;;) {
        if (!read_line(prompt, answer, sizeof(answer))) {
            return false;
        }
        if (strlen(answer) == STRING_TERMINATOR_SIZE) {
            int letter = toupper((unsigned char)answer[0]);
            if (letter == KEY_YES) {
                *answer_out = true;
                return true;
            }
            if (letter == KEY_NO) {
                *answer_out = false;
                return true;
            }
        }
        puts("Введите Y (да) или N (нет).");
    }
}

size_t utf8_character_count(const char *text)
{
    const unsigned char *current = (const unsigned char *)text;
    size_t count = 0;

    if (text == NULL) {
        return 0;
    }
    while (*current != '\0') {
        if ((*current & 0xC0U) != 0x80U) {
            ++count;
        }
        ++current;
    }
    return count;
}

void print_game_time(FILE *stream, int day, int hour)
{
    if (stream == NULL) {
        return;
    }
    fprintf(stream, "Игровое время: день %d, %02d:00\n", day, hour);
}

void print_real_datetime(FILE *stream)
{
    time_t now;
    struct tm *local_time;
    char formatted[64];

    if (stream == NULL) {
        return;
    }
    now = time(NULL);
    local_time = localtime(&now);
    if (local_time == NULL ||
        strftime(formatted, sizeof(formatted), "%d.%m.%Y %H:%M:%S", local_time) == 0U) {
        fputs("Системная дата и время недоступны.\n", stream);
        return;
    }
    fprintf(stream, "Системная дата и время: %s\n", formatted);
}

bool advance_game_time(int *day, int *hour, int hours_to_add, bool *day_changed_out)
{
    long long total_hours;
    long long days_to_add;
    long long resulting_day;

    if (day == NULL || hour == NULL || hours_to_add < 0 ||
        *day < STARTING_DAY || *hour < 0 || *hour >= HOURS_PER_DAY) {
        return false;
    }

    total_hours = (long long)*hour + hours_to_add;
    days_to_add = total_hours / HOURS_PER_DAY;
    resulting_day = (long long)*day + days_to_add;
    if (resulting_day > INT_MAX) {
        return false;
    }

    *day = (int)resulting_day;
    *hour = (int)(total_hours % HOURS_PER_DAY);
    if (day_changed_out != NULL) {
        *day_changed_out = days_to_add > 0;
    }
    return true;
}

int hours_until_night(int hour)
{
    if (hour < 0 || hour >= HOURS_PER_DAY || hour == NIGHT_HOUR) {
        return 0;
    }
    if (hour < NIGHT_HOUR) {
        return NIGHT_HOUR - hour;
    }
    return HOURS_PER_DAY - hour + NIGHT_HOUR;
}
