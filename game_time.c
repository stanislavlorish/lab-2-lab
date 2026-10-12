#include "game_time.h"
#include "constants.h"
#include "input.h"

#include <limits.h>
#include <stdio.h>

void game_time_print(int current_day, int current_hour)
{
    printf("Текущее время: День %d, %02d:%02d\n",
           current_day, current_hour, MINUTES_AT_HOUR);
}

bool game_time_advance(int *current_day, int *current_hour)
{
    int hours_to_work;
    long long total_hours;
    long long days_to_add;
    long long resulting_day;

    if (!read_integer("Сколько часов потратить на работу? ",
                      MIN_WORK_HOURS, INT_MAX, &hours_to_work)) {
        return false;
    }

    total_hours = (long long)*current_hour + hours_to_work;
    days_to_add = total_hours / HOURS_PER_DAY;
    resulting_day = (long long)*current_day + days_to_add;

    if (resulting_day > INT_MAX) {
        printf("Нельзя промотать столько времени: номер дня выйдет за пределы типа int.\n");
        return true;
    }

    *current_day = (int)resulting_day;
    *current_hour = (int)(total_hours % HOURS_PER_DAY);

    printf("Отработано часов: %d.\n", hours_to_work);
    game_time_print(*current_day, *current_hour);
    return true;
}
