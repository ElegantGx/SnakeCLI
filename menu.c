//
// Created by gx on 2026/8/12.
//
// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 ElegantGx

#include <ncurses.h>
#include "game.h"
#include "utils.h"

//选项状态
typedef enum {MENU_PLAY, MENU_ABOUT, MENU_EXIT} MenuState;

//绘制开始菜单的蛇LOGO
static void print_logo_menu(WINDOW *logo_win);

//绘制开始菜单的ABOUT选项
static void print_about_menu(WINDOW *about_win);

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
    const char *menu_options_label[3] = {
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

static void print_logo_menu(WINDOW *logo_win) {
    static const char *logo[] = {
        " ____              _",
        "/ ___| _ __   __ _| | _____",
        "\\___ \\| '_ \\ / _` | |/ / _ \\",
        " ___) | | | | (_| |   <  __/",
        "|____/|_| |_|\\__,_|_|\\_\\___|",
        "",
        "        ____ _     ___",
        "       / ___| |   |_ _|",
        "      | |   | |    | |",
        "      | |___| |___ | |",
        "       \\____|_____|___|"
    };
    constexpr int logo_rows = 11;
    constexpr int logo_cols  = 28;

    int rows, cols;
    getmaxyx(logo_win, rows, cols);
    const int start_y = (rows - logo_rows) / 2;
    const int start_x = (cols - logo_cols) / 2;

    for (int i = 0; i < logo_rows; i++)
        mvwprintw(logo_win, start_y + i, start_x, "%s", logo[i]);
    wrefresh(logo_win);
}

static void print_about_menu(WINDOW *about_win) {
    wclear(about_win);
    box(about_win, 0, 0);
    mvwprintw(about_win, 1, 2, "About Snake:");
    mvwprintw(about_win, 3, 2,"This game is made by Gx.");
    mvwprintw(about_win, 5, 2,"How to play the game?");
    mvwprintw(about_win, 7, 2,"Press UP DOWN LEFT RIGHT to move the snake.");
    mvwprintw(about_win, 8, 2,"Press ENTER to confirm.");
    mvwprintw(about_win, 9, 2,"Press ESC to paused game.");
    mvwprintw(about_win, 11, 2, "Don't resize Terminal when you are playing.");
    mvwprintw(about_win, 12, 2, "The game wil reset.");
    mvwprintw(about_win, 14, 2,"Now press ESC to close About.");
    wrefresh(about_win);
}