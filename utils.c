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
        if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "-V") == 0) {
            printf("snakecli 1.1.1\n\n");

            printf("Copyright (c) 2026 ElegantGx\n\n");
            printf("License: GPLv3+ (GNU GPL version 3 or later)\n");
            printf("         <https://gnu.org/licenses/gpl.html>\n\n");

            printf("This is free software: you are free to change\n");
            printf("and redistribute it. There is NO WARRANTY,\n");
            printf("to the extent permitted by law.\n");
            return 0;
        }

        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "-H") == 0) {
            printf("Usage: snakecli [OPTION]\n");
            printf("Start the Snake game in your terminal.\n\n");
            printf("Options:\n");
            printf("  -v, -V, --version    Print version information and exit\n");
            printf("  -h, -H, --help       Display this help message and exit\n");
            printf("  -c, -C, --check      Check terminal width\n");
            return 0;
        }

        if (strcmp(argv[1], "--check") == 0 || strcmp(argv[1], "-c") == 0 || strcmp(argv[1], "-C") == 0) {
            printf("Check if your terminal size is perfect\n");
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

                int col = ter_size.ws_col;
                int row = ter_size.ws_row;
                printf("Terminal size: %dx%d", col, row);

                if (col % 6 == 0 && row > 23) {
                    const int col_before = col;
                    const int row_before = row;

                    // 防抖
                    usleep(100000);
                    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ter_size);
                    col = ter_size.ws_col;
                    row = ter_size.ws_row;
                    if (col == col_before && row == row_before) {
                        printf("\tGood size!\n");
                        return 0;
                    }
                    printf("\n");
                    continue;
                }

                int recommend_row = 24;
                if (row > 24) recommend_row = row;
                printf("\ttry %dx%d or %dx%d", col + (6 - col % 6), recommend_row, col - col % 6, recommend_row);
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

//传入窗口、单行文本，将文本居中打印至该窗口
void print_center_window(WINDOW *win, const char *text) {
    int row, col;
    getmaxyx(win, row, col);
    col = (col - (int)strlen(text)) / 2;
    row = row / 2;
    if (col < 1) col = 1;
    mvwprintw(win, row, col, "%s", text);
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
