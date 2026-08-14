//
// Created by gx on 2026/8/12.
//

#include <ncurses.h>
#include "game.h"
#include "utils.h"

//选项状态
typedef enum {MENU_PLAY, MENU_ABOUT, MENU_EXIT} MenuState;

//处理MENU状态
GameState menu (WINDOW *main_win, WINDOW *sentence_win, const Position ter_size) {
    GameState state = MENU;

    //LOGO菜单绘制
    wclear(main_win);
    box(main_win, 0, 0);
    keypad(main_win, TRUE);
    keypad(sentence_win, TRUE);
    nodelay(main_win, FALSE);
    print_logo_menu(main_win);

    //提示绘制
    wclear(sentence_win);
    print_center_window(sentence_win, "Please read About first");
    wrefresh(sentence_win);

    //定义选项数组
    WINDOW *menu_options_win[3];
    const char *menu_options_label[] = {
        [MENU_PLAY] = "Play",
        [MENU_ABOUT] = "About",
        [MENU_EXIT] = "Exit",
    };

    //创建选项
    create_options_win(menu_options_win, menu_options_label, 3, ter_size);

    wrefresh(main_win);
    wrefresh(sentence_win);

    //初始化选项
    MenuState menu_selected_state = MENU_PLAY;

    //选择选项
    menu_selected_state = select_option(sentence_win, menu_options_win, menu_options_label, 3, 0);

    switch (menu_selected_state) {
        case MENU_PLAY: {
            state = PLAY;
            goto cleanup;
        }
        case MENU_ABOUT: {
            print_about_menu(main_win);
            wrefresh(main_win);
            while (wgetch(main_win) != 27) {}
            goto cleanup;
        }
        case MENU_EXIT: {
            state = EXIT;
        }
    }
    cleanup:
        wclear(menu_options_win[MENU_PLAY]);
        wclear(menu_options_win[MENU_ABOUT]);
        wclear(menu_options_win[MENU_EXIT]);
        wrefresh(menu_options_win[MENU_PLAY]);
        wrefresh(menu_options_win[MENU_ABOUT]);
        wrefresh(menu_options_win[MENU_EXIT]);
        delwin(menu_options_win[MENU_PLAY]);
        delwin(menu_options_win[MENU_ABOUT]);
        delwin(menu_options_win[MENU_EXIT]);
        return state;
}