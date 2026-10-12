#include "input.h"
#include "constants.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
            int character;

            do {
                character = getchar();
            } while (character != '\n' && character != EOF);

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
