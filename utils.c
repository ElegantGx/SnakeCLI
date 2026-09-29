//
// Created by gx on 2026/8/11.
//
// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 ElegantGx

#include "utils.h"

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

//处理参数模式
int check_command_mode(const int argc, const char **argv) {
    if (argc >= 2) {
        if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-V") == 0) {
            printf("snakecli 1.1.0-rc.1\n\n");

            printf("Copyright (c) 2026 ElegantGx\n\n");
            printf("License: GPLv3+ (GNU GPL version 3 or later)\n");
            printf("         <https://gnu.org/licenses/gpl.html>\n\n");

            printf("This is free software: you are free to change\n");
            printf("and redistribute it. There is NO WARRANTY,\n");
            printf("to the extent permitted by law.\n");
            return 0;
        }

        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-H") == 0) {
            printf("Usage: snakecli [OPTION]\n");
            printf("Start the Snake game in your terminal.\n\n");
            printf("Options:\n");
            printf("  -V, --version    Print version information and exit\n");
            printf("  -H, --help       Display this help message and exit\n");
            printf("  -C, --check      Check terminal width\n");
            return 0;
        }

        if (strcmp(argv[1], "--check") == 0 || strcmp(argv[1], "-C") == 0) {
            printf("Check if your terminal width is a multiple of 6\n");
            printf("Press Ctrl C to quit\n\n");

            sigset_t sig_resize;                    //声明信号合集
            sigemptyset(&sig_resize);               //清零信号合集
            sigaddset(&sig_resize, SIGWINCH);   //加入SIGWINCH信号
            sigprocmask(SIG_BLOCK, &sig_resize, nullptr);   //加入信号屏蔽字

            struct winsize ter_size;

            while (true) {
                if (!isatty(STDOUT_FILENO) || ioctl(STDOUT_FILENO, TIOCGWINSZ, &ter_size) == -1) {
                    fprintf(stderr, "snakecli: not a terminal\n");
                    return 1;
                }

                printf("Terminal size: %dx%d", ter_size.ws_col, ter_size.ws_row);

                if (ter_size.ws_col % 6 == 0) {
                    const int col_before = ter_size.ws_col;
                    usleep(100000);
                    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ter_size);
                    if (ter_size.ws_col == col_before) {   // 防抖
                        printf("\tGood size!\n");
                        return 0;
                    }
                    printf("\n");
                    continue;
                }

                printf("\ttry %dx%d or %dx%d", ter_size.ws_col + (6 - ter_size.ws_col % 6), ter_size.ws_row, ter_size.ws_col - ter_size.ws_col % 6, ter_size.ws_row);
                printf("\n");

                int sig;
                sigwait(&sig_resize, &sig);
            }
        }

        fprintf(stderr, "snakecli: invalid option -- '%s'\n", argv[1]);
        fprintf(stderr, "Try 'snakecli --help' for more information.\n");
        return 1;

    }
    return -1;
}

//绘制开始菜单的蛇LOGO
void print_logo_menu(WINDOW *logo_win) {
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

//传入窗口、单行文本，将文本居中打印至该窗口
void print_center_window(WINDOW *win, const char *text) {
    int row, col;
    getmaxyx(win, row, col);
    col = (col - (int)strlen(text)) / 2;
    row = row / 2;
    if (col < 1) col = 1;
    mvwprintw(win, row, col, "%s", text);
}

//绘制开始菜单的ABOUT选项
void print_about_menu(WINDOW *about_win) {
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

//绘制新选项
void create_options_win(WINDOW *options_wins[], const char *options_label[], const int options_count, const Position size) {
    int option_cols = size.col / options_count;
    for (int i = 0; i < options_count ; i++) {
        const int begin_col = i * option_cols;
        if (i == options_count - 1) option_cols = size.col - i * option_cols;
        options_wins[i] = newwin(3, option_cols, size.row - 3, begin_col);
        box(options_wins[i], 0, 0);
        print_center_window(options_wins[i], options_label[i]);
        wrefresh(options_wins[i]);
    }
}

//传入选项窗口数组、选项标签数组、选项数、选中状态，高亮选中选项
void draw_selected_option(WINDOW *options_wins[], const char *options_label[], const int options_count, const int selected_option) {
    for (int i = 0; i < options_count; i++) {
        if (i == selected_option) {
            wattron(options_wins[i], A_REVERSE);
        }
        print_center_window(options_wins[i], options_label[i]);
        wattroff(options_wins[i], A_REVERSE);
        wrefresh(options_wins[i]);
    }
}

//选择选项，返回选择选项对应值
int select_option (WINDOW *win, WINDOW *options_wins[], const char *options_label[], const int options_count, int selected_option) {
    draw_selected_option(options_wins, options_label, options_count, selected_option);

    while (true) {
        const int tmp_input = wgetch(win);
        switch (tmp_input) {
            case KEY_RIGHT:
                selected_option++;
                break;
            case KEY_LEFT:
                selected_option--;
                break;
            default: break;
        }
        selected_option = (selected_option + options_count) % options_count;   // 循环切换
        draw_selected_option(options_wins, options_label, options_count, selected_option);
        if (tmp_input == KEY_ENTER || tmp_input == '\n') return selected_option;
    }
}

//渲染蛇
void render_snake_play(WINDOW *main_win, const SnakeCLI *snake_cli) {
    mvwaddch(main_win, snake_cli->head.row, snake_cli->head.col, SNAKE_CELL);
    mvwaddch(
        main_win, snake_cli->path[(snake_cli->step - snake_cli->len + CAP) % CAP].row,
        snake_cli->path[(snake_cli->step - snake_cli->len + CAP) % CAP].col, DEFAULT_CELL
    );
}

//读取输入
int get_input_play(WINDOW *win) {
    const int input = wgetch(win);

    static const int available_keys[] = {KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, 27};
    for (size_t i = 0; i < sizeof(available_keys)/sizeof(available_keys[0]); ++i) {
        if (input == available_keys[i]) return input;
    }

    return ERR;
}

//将输入转换为PlayDirection所需
int map_the_key_play(const int input) {
    switch (input) {
        case KEY_UP:
            return 0;
        case KEY_DOWN:
            return 1;
        case KEY_LEFT:
            return 2;
        case KEY_RIGHT:
            return 3;
        case 27:
            return -1;
        default: return -2;
    }
}
