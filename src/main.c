#include "game.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static void print_main_menu(const GameState *game)
{
    printf("\n========== ВЕСЁЛЫЙ ФЕРМЕР ==========\n");
    if (game_has_player(game)) {
        printf("Герой: %s | HP: %d/%d | Выносливость: %d/%d | Золото: %d\n",
               game->player->name, game->player->hp, game->player->max_hp,
               game->player->stamina, game->player->max_stamina, game->player->gold);
    } else {
        puts("Персонаж ещё не создан.");
    }
    printf("[%d] Выход\n", MENU_EXIT);
    printf("[%d] Создать / пересоздать персонажа\n", MENU_CREATE_PLAYER);
    printf("[%d] Карточка персонажа\n", MENU_CHARACTER_CARD);
    printf("[%d] Время и отдых\n", MENU_TIME);
    printf("[%d] Посмотреть инвентарь\n", MENU_SHOW_INVENTORY);
    printf("[%d] Положить предмет в слот\n", MENU_PLACE_ITEM);
    printf("[%d] Выбросить предмет из слота\n", MENU_DROP_ITEM);
    printf("[%d] Сортировка рюкзака\n", MENU_SORT_INVENTORY);
    printf("[%d] Поиск предмета по названию\n", MENU_SEARCH_ITEM);
    printf("[%d] Записать состояние в дневник\n", MENU_WRITE_DIARY);
    printf("[%d] Отправиться в приключение\n", MENU_ADVENTURE);
    printf("[%d] Подсчёт ресурсов\n", MENU_RESOURCE_REPORT);
}

static void dispatch_menu_option(GameState *game, MenuOption option)
{
    switch (option) {
        case MENU_CREATE_PLAYER:
            game_create_player(game);
            break;
        case MENU_CHARACTER_CARD:
            game_show_character(game);
            break;
        case MENU_TIME:
            game_time_menu(game);
            break;
        case MENU_SHOW_INVENTORY:
            game_show_inventory(game);
            break;
        case MENU_PLACE_ITEM:
            game_place_item(game);
            break;
        case MENU_DROP_ITEM:
            game_drop_item(game);
            break;
        case MENU_SORT_INVENTORY:
            game_sort_inventory(game);
            break;
        case MENU_SEARCH_ITEM:
            game_search_item(game);
            break;
        case MENU_WRITE_DIARY:
            game_write_diary(game);
            break;
        case MENU_ADVENTURE:
            game_start_adventure(game);
            break;
        case MENU_RESOURCE_REPORT:
            game_show_resource_report();
            break;
        case MENU_EXIT:
            break;
        default:
            puts("Неизвестный пункт меню.");
            break;
    }
}

int main(void)
{
    GameState game;
    int choice;

    srand((unsigned int)time(NULL));
    game_initialize(&game);
    puts("Программа запущена. Создайте персонажа через пункт [1].");

    for (;;) {
        print_main_menu(&game);
        if (!read_integer("Выберите пункт меню: ", MENU_EXIT,
                          MENU_RESOURCE_REPORT, &choice)) {
            puts("\nВвод завершён. Программа закрывается.");
            break;
        }
        if (choice == MENU_EXIT) {
            puts("Выход из игры. До встречи!");
            break;
        }
        dispatch_menu_option(&game, (MenuOption)choice);
    }

    game_destroy(&game);
    return EXIT_SUCCESS;
}
