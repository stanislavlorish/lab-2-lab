#include "game_time.h"
#include "constants.h"

#include <limits.h>

void game_time_print(FILE *stream, int current_day, int current_hour)
{
    fprintf(stream, "Текущее время: День %d, %02d:%02d\n",
            current_day, current_hour, MINUTES_AT_HOUR);
}

bool game_time_add(int *current_day, int *current_hour, int hours_to_add)
{
    long long total_hours;
    long long days_to_add;
    long long resulting_day;

    if (hours_to_add < MIN_WORK_HOURS) {
        return false;
    }

    total_hours = (long long)*current_hour + hours_to_add;
    days_to_add = total_hours / HOURS_PER_DAY;
    resulting_day = (long long)*current_day + days_to_add;

    if (resulting_day > INT_MAX) {
        return false;
    }

    *current_day = (int)resulting_day;
    *current_hour = (int)(total_hours % HOURS_PER_DAY);
    return true;
}
