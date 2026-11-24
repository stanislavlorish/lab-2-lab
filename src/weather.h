#ifndef FARMER_WEATHER_H
#define FARMER_WEATHER_H

typedef enum WeatherType {
    WEATHER_CLEAR = 0,
    WEATHER_RAIN,
    WEATHER_FOG,
    WEATHER_HURRICANE,
    WEATHER_TYPE_COUNT
} WeatherType;

typedef struct Weather {
    WeatherType type;
    int temperature_c;
    int wind_strength;
} Weather;

void weather_randomize(Weather *weather);
const char *weather_name(WeatherType type);
int weather_stamina_multiplier(const Weather *weather);

#endif
