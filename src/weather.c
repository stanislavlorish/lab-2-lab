#include "weather.h"
#include "constants.h"

#include <stdlib.h>

static int random_between(int min_value, int max_value)
{
    return min_value + rand() % (max_value - min_value + RANDOM_INCLUSIVE_OFFSET);
}

void weather_randomize(Weather *weather)
{
    if (weather == NULL) {
        return;
    }

    weather->type = (WeatherType)random_between(WEATHER_CLEAR,
                                                WEATHER_TYPE_COUNT - RANDOM_INCLUSIVE_OFFSET);
    switch (weather->type) {
        case WEATHER_CLEAR:
            weather->temperature_c = random_between(WEATHER_CLEAR_TEMP_MIN,
                                                    WEATHER_CLEAR_TEMP_MAX);
            weather->wind_strength = random_between(WEATHER_CLEAR_WIND_MIN,
                                                    WEATHER_CLEAR_WIND_MAX);
            break;
        case WEATHER_RAIN:
            weather->temperature_c = random_between(WEATHER_RAIN_TEMP_MIN,
                                                    WEATHER_RAIN_TEMP_MAX);
            weather->wind_strength = random_between(WEATHER_RAIN_WIND_MIN,
                                                    WEATHER_RAIN_WIND_MAX);
            break;
        case WEATHER_FOG:
            weather->temperature_c = random_between(WEATHER_FOG_TEMP_MIN,
                                                    WEATHER_FOG_TEMP_MAX);
            weather->wind_strength = random_between(WEATHER_FOG_WIND_MIN,
                                                    WEATHER_FOG_WIND_MAX);
            break;
        case WEATHER_HURRICANE:
            weather->temperature_c = random_between(WEATHER_HURRICANE_TEMP_MIN,
                                                    WEATHER_HURRICANE_TEMP_MAX);
            weather->wind_strength = random_between(WEATHER_HURRICANE_WIND_MIN,
                                                    WEATHER_HURRICANE_WIND_MAX);
            break;
        case WEATHER_TYPE_COUNT:
        default:
            weather->type = WEATHER_CLEAR;
            weather->temperature_c = random_between(WEATHER_CLEAR_TEMP_MIN,
                                                    WEATHER_CLEAR_TEMP_MAX);
            weather->wind_strength = random_between(WEATHER_CLEAR_WIND_MIN,
                                                    WEATHER_CLEAR_WIND_MAX);
            break;
    }
}

const char *weather_name(WeatherType type)
{
    switch (type) {
        case WEATHER_CLEAR:
            return "Ясно";
        case WEATHER_RAIN:
            return "Ливень";
        case WEATHER_FOG:
            return "Туман";
        case WEATHER_HURRICANE:
            return "Ураган";
        case WEATHER_TYPE_COUNT:
        default:
            return "Неизвестно";
    }
}

int weather_stamina_multiplier(const Weather *weather)
{
    return weather != NULL && weather->type == WEATHER_RAIN
        ? RAIN_STAMINA_MULTIPLIER
        : BASE_STEP_STAMINA_COST;
}
