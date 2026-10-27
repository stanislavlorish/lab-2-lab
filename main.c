#include "constants.h"
#include "diary.h"
#include "game_time.h"
#include "input.h"
#include "inventory.h"
#include "item_catalog.h"
#include "resource_report.h"
#include "variant_sort.h"

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static void print_menu(const char *farmer_name)
{
    printf("\n========== Ферма фермера %s ==========\n", farmer_name);
    puts("[0] Выход");
    puts("[1] Посмотреть на часы");
    puts("[2] Промотать время (поработать)");
    puts("[3] Посмотреть инвентарь");
    puts("[4] Положить предмет в слот");
    puts("[5] Выбросить предмет");
    puts("[6] Сортировка рюкзака (задание ЛР2)");
    puts("[7] Поиск предмета в рюкзаке по названию");
    puts("[8] Записать состояние в дневник фермера");
    puts("[9] Подсчёт ресурсов из input.txt");
}

static bool handle_work(int *current_day, int *current_hour)
{
    int hours_to_work;

    if (!read_integer("Сколько часов потратить на работу? ",
                      MIN_WORK_HOURS, INT_MAX, &hours_to_work)) {
        return false;
    }

    if (!game_time_add(current_day, current_hour, hours_to_work)) {
        puts("Не удалось изменить время: номер дня превысит допустимое значение.");
        return true;
    }

    printf("Отработано часов: %d.\n", hours_to_work);
    game_time_print(stdout, *current_day, *current_hour);
    return true;
}

static bool handle_place_item(char item_names[MAX_ITEMS][MAX_NAME_LEN])
{
    int slot;
    int item_id;
    char prompt[INPUT_BUFFER_SIZE];

    snprintf(prompt, sizeof(prompt), "Введите индекс слота (%d-%d): ",
             SLOT_FIRST, SLOT_LAST);
    if (!read_integer(prompt, SLOT_FIRST, SLOT_LAST, &slot)) {
        return false;
    }

    snprintf(prompt, sizeof(prompt), "Введите ID предмета (%d-%d): ",
             ITEM_ID_MIN, ITEM_ID_MAX);
    if (!read_integer(prompt, ITEM_ID_MIN, ITEM_ID_MAX, &item_id)) {
        return false;
    }

    if (!inventory_set_item(slot, item_id)) {
        puts("Не удалось изменить слот.");
        return true;
    }

    printf("В слот %d помещён предмет: %s.\n",
           slot, item_catalog_get_name(item_names, item_id));
    return true;
}

static bool handle_drop_item(void)
{
    int slot;
    char prompt[INPUT_BUFFER_SIZE];

    snprintf(prompt, sizeof(prompt), "Введите индекс слота (%d-%d): ",
             SLOT_FIRST, SLOT_LAST);
    if (!read_integer(prompt, SLOT_FIRST, SLOT_LAST, &slot)) {
        return false;
    }

    if (!inventory_clear_slot(slot)) {
        puts("Не удалось очистить слот.");
        return true;
    }

    printf("Слот %d очищен.\n", slot);
    return true;
}

static void handle_sort_inventory(char item_names[MAX_ITEMS][MAX_NAME_LEN])
{
    puts("Инвентарь до сортировки:");
    inventory_write(stdout, item_names);

    inventory_compact();

    puts("Инвентарь после сортировки:");
    inventory_write(stdout, item_names);
}

static bool handle_search_item(char item_names[MAX_ITEMS][MAX_NAME_LEN])
{
    char search_name[MAX_NAME_LEN];
    int item_id;
    int slots[INVENTORY_SIZE];
    int found_count;
    int index;

    if (!read_line("Введите точное название предмета из каталога: ",
                   search_name, sizeof(search_name))) {
        return false;
    }

    if (!item_catalog_find_id(item_names, search_name, &item_id)) {
        printf("Предмет «%s» отсутствует в каталоге.\n", search_name);
        return true;
    }

    found_count = inventory_find_slots(item_id, slots);
    if (found_count == 0) {
        printf("Предмет «%s» найден в каталоге (ID %d), но его нет в инвентаре.\n",
               search_name, item_id);
        return true;
    }

    printf("Предмет «%s» (ID %d) находится в слотах: ", search_name, item_id);
    for (index = 0; index < found_count; ++index) {
        printf("%d", slots[index]);
        if (index + 1 < found_count) {
            printf(", ");
        }
    }
    putchar('\n');
    return true;
}

int main(void)
{
    int current_day = STARTING_DAY;
    int current_hour = STARTING_HOUR;
    int menu_choice;
    char farmer_name[MAX_FARMER_NAME_LEN];
    char item_names[MAX_ITEMS][MAX_NAME_LEN];

    inventory_initialize();
    item_catalog_load(item_names);

    for (;;) {
        if (!read_line("Введите имя фермера (до 32 символов): ",
                       farmer_name, sizeof(farmer_name))) {
            puts("Ввод завершён. Программа остановлена.");
            return EXIT_SUCCESS;
        }
        if (utf8_character_count(farmer_name) <= MAX_FARMER_NAME_CHARS) {
            break;
        }
        printf("Имя должно содержать не более %d символов.\n",
               MAX_FARMER_NAME_CHARS);
    }

    while (true) {
        print_menu(farmer_name);
        if (!read_integer("Выберите пункт меню: ", MENU_MIN, MENU_MAX, &menu_choice)) {
            puts("\nВвод завершён. Программа остановлена.");
            return EXIT_SUCCESS;
        }

        switch (menu_choice) {
            case MENU_EXIT:
                puts("Выход из игры. До встречи!");
                return EXIT_SUCCESS;

            case MENU_SHOW_TIME:
                game_time_print(stdout, current_day, current_hour);
                break;

            case MENU_WORK:
                if (!handle_work(&current_day, &current_hour)) {
                    puts("\nВвод завершён. Программа остановлена.");
                    return EXIT_SUCCESS;
                }
                break;

            case MENU_SHOW_INVENTORY:
                inventory_write(stdout, item_names);
                break;

            case MENU_PLACE_ITEM:
                if (!handle_place_item(item_names)) {
                    puts("\nВвод завершён. Программа остановлена.");
                    return EXIT_SUCCESS;
                }
                break;

            case MENU_DROP_ITEM:
                if (!handle_drop_item()) {
                    puts("\nВвод завершён. Программа остановлена.");
                    return EXIT_SUCCESS;
                }
                break;

            case MENU_SORT_INVENTORY:
                handle_sort_inventory(item_names);
                break;

            case MENU_SEARCH_ITEM:
                if (!handle_search_item(item_names)) {
                    puts("\nВвод завершён. Программа остановлена.");
                    return EXIT_SUCCESS;
                }
                break;

            case MENU_WRITE_DIARY:
                diary_append(farmer_name, current_day, current_hour, item_names);
                break;

            case MENU_RESOURCE_REPORT:
                resource_report_run();
                break;

            default:
                puts("Неизвестный пункт меню.");
                break;
        }
    }
}
