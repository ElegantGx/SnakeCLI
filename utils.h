//
// Created by gx on 2026/8/11.
//
// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 ElegantGx

#ifndef SNAKECLI_UTILS_H
#define SNAKECLI_UTILS_H

#include <ncurses.h>

#include "game.h"

int check_command_mode(int argc, char **argv);

void print_logo_menu (WINDOW *logo_win);

void print_center_window(WINDOW *win, const char *text);

void print_about_menu (WINDOW *about_win);

void create_options_win(WINDOW *options_wins[], const char *options_label[], int options_count, Position size);

void draw_selected_option (WINDOW *options_wins[], const char *options_label[], int options_count, int selected_option);

int select_option (WINDOW *win, WINDOW *options_wins[], const char *options_label[], int options_count, int selected_option);

void render_snake_play(WINDOW *main_win, const SnakeCLI *snake_cli);

int get_input_play(WINDOW *win);

int map_the_key_play(int input);

#endif //SNAKECLI_UTILS_H