// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 ElegantGx

#include <ncurses.h>
#include <setjmp.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>

#include "game.h"
#include "utils.h"

static sigjmp_buf ter_resize;
static sigjmp_buf sigint_env;
static sigjmp_buf sigquit_env;

static int check_terminal(Position ter_size);

static void handler(const int sig) {
    if (sig == SIGINT) {
        siglongjmp(sigint_env, 1);
    }
    if (sig == SIGQUIT) {
        siglongjmp(sigquit_env, 1);
    }
    siglongjmp(ter_resize, 1);
}

int game() {
    //注册信号
    signal(SIGWINCH, handler);
    signal(SIGINT, handler);
    signal(SIGQUIT, handler);

    //处理大小变化
    if (sigsetjmp(ter_resize, 1)) {
        goto cleanup_by_resize;
    }
    //处理Ctrl C
    if (sigsetjmp(sigint_env, 1)) {
        goto cleanup_by_sigint;
    }
    //处理退出
    if (sigsetjmp(sigquit_env, 1)) {
        goto cleanup_by_sigquit;
    }

    //定义默认状态
    GameState state = MENU;

    initscr();      //接管终端
    cbreak();       //按键立刻生效
    noecho();       //按键不回显
    curs_set(0);    //隐藏光标
    set_escdelay(20); //处理ESC延迟

    //获取终端大小
    Position ter_size;
    getmaxyx(stdscr, ter_size.row, ter_size.col);

    //检查终端能力
    if (!check_terminal(ter_size)) {
        endwin();
        return 0;
    }

    start_color();    //开启颜色
    use_default_colors(); //允许用 -1 表示终端默认色
    clear();        //清空默认状态

    init_pair(CP_SNAKE, COLOR_BLACK, COLOR_GREEN);
    init_pair(CP_APPLE, COLOR_BLACK, COLOR_RED);

    //注册窗口名
    putp("\033]0;Snake CLI\007");
    fflush(stdout);

    //创建主窗口
    WINDOW *main_win = newwin(ter_size.row - 6, ter_size.col, 0, 0);

    //创建句窗口
    WINDOW *sentence_win = newwin(3, ter_size.col, ter_size.row - 6, 0);
    keypad(sentence_win, true);

    srandom((unsigned)time(nullptr));

    while (true) {
        switch (state) {
            case MENU: {
                state = menu(main_win, sentence_win, ter_size);
                break;
            }
            case PLAY: {
                state = play(main_win, sentence_win, ter_size);
                break;
            }
            case EXIT: goto cleanup_by_exit;
            default:
                endwin();
                return 5;
        }
    }

    cleanup_by_exit:
        delwin(main_win);
        delwin(sentence_win);
        endwin();
        return 0;

    cleanup_by_resize:
        endwin();
        return 2;

    cleanup_by_sigint:
        endwin();
        return 3;

    cleanup_by_sigquit:
        endwin();
        return 4;
}

static int check_terminal(const Position ter_size) {
    if (!has_colors()) {
        print_center_window(stdscr, "No color support. q quits.");
        while (true) {
            const int input = getch();
            if (input == 'q' || input == 'Q') return 0;
        }
    }

    if (ter_size.row < 23 || ter_size.col < 35) {
        char msg[48]="";
        snprintf(msg, sizeof msg, "Need 35x23+, now %dx%d. q quits", ter_size.col, ter_size.row);
        print_center_window(stdscr, msg);
        while (true) {
            const int input = getch();
            if (input == 'q' || input == 'Q') return 0;
        }
    }

    if (ter_size.col % 2 != 0) {
        char msg[48]="";
        snprintf(msg, sizeof msg,"Odd width %d, try %d/%d. q quits",
            ter_size.col, ter_size.col - 1, ter_size.col + 1);
        print_center_window(stdscr, msg);
        while (true) {
            const int input = getch();
            if (input == 'q' || input == 'Q') return 0;
        }
    }

    return 1;
}