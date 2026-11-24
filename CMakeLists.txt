cmake_minimum_required(VERSION 3.16)
project(farmer_lab5 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(farmer
    src/main.c
    src/game.c
    src/player.c
    src/map.c
    src/weather.c
    src/utils.c
    src/item_catalog.c
    src/diary.c
    src/resource_report.c
)

target_include_directories(farmer PRIVATE src)

add_custom_command(TARGET farmer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/data"
            "$<TARGET_FILE_DIR:farmer>/data"
)

if(MSVC)
    target_compile_options(farmer PRIVATE /W4)
    target_compile_definitions(farmer PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    target_compile_options(farmer PRIVATE -Wall -Wextra -Wpedantic)
endif()
