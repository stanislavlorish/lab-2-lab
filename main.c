#include "constants.h"
#include "game_time.h"
#include "input.h"
#include "inventory.h"
#include "variant_sort.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static void print_menu(void)
{
    puts("\n Меню ");
    puts("[0] Выход");
    puts("[1] Посмотреть на часы");
    puts("[2] Промотать время (поработать)");
    puts("[3] Посмотреть инвентарь");
    puts("[4] Положить предмет в слот");
    puts("[5] Выбросить предмет");
    puts("[6] Сортировка");
}

int main(void)
{
    int current_day = STARTING_DAY;
    int current_hour = STARTING_HOUR;
    int menu_choice;

    inventory_initialize();

    while (true) {
        print_menu();
        if (!read_integer("Выберите пункт меню: ", MENU_EXIT, MENU_VARIANT_TASK,
                          &menu_choice)) {
            puts("\nВвод завершён.");
            return EXIT_SUCCESS;
        }

        switch (menu_choice) {
            case MENU_EXIT:
                puts("Выход из игры. До встречи!");
                return EXIT_SUCCESS;

            case MENU_SHOW_TIME:
                game_time_print(current_day, current_hour);
                break;

            case MENU_WORK:
                if (!game_time_advance(&current_day, &current_hour)) {
                    puts("\nВвод завершён.");
                    return EXIT_SUCCESS;
                }
                break;

            case MENU_SHOW_INVENTORY:
                inventory_print();
                break;

            case MENU_PLACE_ITEM:
                if (!inventory_place_item()) {
                    puts("\nВвод завершён. ");
                    return EXIT_SUCCESS;
                }
                break;

            case MENU_DROP_ITEM:
                if (!inventory_drop_item()) {
                    puts("\nВвод завершён.");
                    return EXIT_SUCCESS;
                }
                break;

            case MENU_VARIANT_TASK:
                variant_sort_run();
                break;

            default:
                puts("Неизвестный пункт меню.");
                break;
        }
    }
}
