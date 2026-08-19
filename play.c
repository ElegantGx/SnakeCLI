//
// Created by gx on 2026/8/12.
//
// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 ElegantGx

#include <ncurses.h>
#include <stdlib.h>

#include "game.h"
#include "utils.h"

#define default_render_time 150

//游玩与暂停
typedef enum {PLAY_PLAYING, PLAY_PAUSED, PLAY_FINISHED, PLAY_QUIT} PlayState;

//方向键
typedef enum {UP, DOWN, LEFT, RIGHT, ESC = -1, NONE = -2} PlayDirection;    //枚举顺序与防掉头判定有关

//生成苹果坐标
static Position generate_apple_play(const SnakeCLI *snake_cli, Position game_max_size);

//检查苹果是否在蛇身上
static int is_on_snake(const SnakeCLI *snake_cli, int apple_row, int apple_col);

//游戏状态
static PlayState play_playing(WINDOW *main_win, WINDOW *sentence_win, WINDOW *score_win, SnakeCLI *snake_cli, Position ter_size, Position game_max_size, PlayDirection *play_direction);

//暂停状态
static PlayState play_paused(WINDOW *main_win, WINDOW *sentence_win, Position ter_size, GameState *state);

//结束状态
static PlayState play_finished(WINDOW *main_win, WINDOW *sentence_win, const SnakeCLI *snake_cli, Position ter_size, GameState *state);

//游戏主进程
GameState play(WINDOW *main_win, WINDOW *sentence_win, const Position ter_size) {
    GameState state = PLAY;

    PlayState play_state = PLAY_PLAYING;

    keypad(main_win, TRUE);

    //获取游戏主窗口大小
    Position game_max_size;
    getmaxyx(main_win, game_max_size.row, game_max_size.col);

    //开启非阻塞式输入
    nodelay(main_win, TRUE);

    //初始化
    SnakeCLI snake_cli = {
        .step = 0,
        .len = 1,
    };

    PlayDirection play_direction = RIGHT;

    wclear(main_win);
    box(main_win, 0, 0);
    snake_cli.head.row = game_max_size.row / 2;
    snake_cli.head.col = (game_max_size.col - 1) / 2;
    mvwprintw(main_win, snake_cli.head.row, snake_cli.head.col, "@");

    snake_cli.path[0] = snake_cli.head;

    snake_cli.apple = generate_apple_play(&snake_cli, game_max_size);
    mvwprintw(main_win, snake_cli.apple.row, snake_cli.apple.col, "O");

    wrefresh(main_win);

    WINDOW *score_win = newwin(3, ter_size.col, ter_size.row - 3, 0);
    box(score_win, 0, 0);
    wrefresh(score_win);

    wclear(sentence_win);

    int render_time;
    while (play_state != PLAY_QUIT) {
        render_time = default_render_time - 5 * (snake_cli.len / 5);
        if (render_time < 10) {
            render_time = 10;
        }
        napms(render_time);

        switch (play_state) {
            case PLAY_PLAYING:
                play_state = play_playing(main_win, sentence_win, score_win, &snake_cli, ter_size, game_max_size, &play_direction);
                flushinp();
                break;
            case PLAY_PAUSED:
                wclear(score_win);
                wrefresh(score_win);
                flushinp();
                play_state = play_paused(main_win, sentence_win, ter_size, &state);
                break;
            case PLAY_FINISHED:
                wclear(score_win);
                wrefresh(score_win);
                flushinp();
                play_state = play_finished(main_win, sentence_win, &snake_cli, ter_size, &state);
                break;
            default:
                state = ERR;
                goto cleanup;
        }
    }

    cleanup:
        wclear(main_win);
        wclear(sentence_win);
        wclear(score_win);
        delwin(score_win);
        return state;
}

static Position generate_apple_play(const SnakeCLI *snake_cli, const Position game_max_size) {
    const int game_area = (game_max_size.row - 2) * (game_max_size.col - 2);
    Position *free_cells = malloc(sizeof(Position) * game_area);

    Position apple = {.row = ERR, .col = ERR};

    int i = 0;
    for (int r = 1; r <= game_max_size.row - 2; r++) {
        for (int c = 1; c <= game_max_size.col - 2; c++) {
            if (!is_on_snake(snake_cli, r, c)) {
                free_cells[i] = (Position){.row = r, .col = c};
                i++;
            }
        }
    }

    if (i > 0) apple = free_cells[random() % i];

    //cleanup
    free(free_cells);
    return apple;
}

static int is_on_snake(const SnakeCLI *snake_cli, const int apple_row, const int apple_col) {
    for (int i = 0; i < snake_cli->len; i++) {
        if (apple_row == snake_cli->path[(snake_cli->step - i) % CAP].row &&
            apple_col == snake_cli->path[(snake_cli->step - i) % CAP].col)
        {
            return 1;
        }
    }
    return 0;
}

static PlayState play_playing(WINDOW *main_win, WINDOW *sentence_win, WINDOW *score_win, SnakeCLI *snake_cli, Position ter_size, Position game_max_size, PlayDirection *play_direction) {
    //创建方向向量
    static const Position DELTA[] = {
        [UP] = {.row = -1, .col = 0},
        [DOWN] = {.row = 1, .col = 0},
        [LEFT] = {.row = 0, .col = -1},
        [RIGHT] = {.row = 0, .col = 1},
    };

    const int tmp_input = map_the_key_play(get_input_play(main_win));

    if (tmp_input == ESC) return PLAY_PAUSED;

    if ((tmp_input ^ *play_direction) != 1 && tmp_input >= 0) *play_direction = tmp_input;

    //写入数组
    snake_cli->step++;
    snake_cli->head.row += DELTA[*play_direction].row;
    snake_cli->head.col += DELTA[*play_direction].col;
    snake_cli->path[(snake_cli->step + CAP) % CAP] = snake_cli->head;

    //判定是否吃到苹果
    if (snake_cli->head.row == snake_cli->apple.row && snake_cli->head.col == snake_cli->apple.col) {
        snake_cli->len++;
        snake_cli->apple = generate_apple_play(snake_cli, game_max_size);
    }

    //判定是否死亡
    if (snake_cli->head.row < 1 || snake_cli->head.row > game_max_size.row - 2 ||
        snake_cli->head.col < 1 || snake_cli->head.col > game_max_size.col - 2)
    {
        return PLAY_FINISHED;
    }

    for (int i = 1; i < snake_cli->len; i++) {
        if (snake_cli->head.row == snake_cli->path[(snake_cli->step - i + CAP) % CAP].row &&
            snake_cli->head.col == snake_cli->path[(snake_cli->step - i + CAP) % CAP].col)
        {
            return PLAY_FINISHED;
        }
    }

    //胜利
    if (snake_cli->len == CAP - 1) return PLAY_FINISHED;
    if (snake_cli->apple.row == ERR) return PLAY_FINISHED;

    render_snake_play(main_win, snake_cli);
    mvwprintw(main_win, snake_cli->apple.row, snake_cli->apple.col, "O");
    wrefresh(main_win);

    char msg[32]="";
    sprintf(msg, "Score: %d", snake_cli->len - 1);
    box(score_win, 0, 0);
    print_center_window(score_win, msg);
    wrefresh(score_win);

    print_center_window(sentence_win, "Playing...Press ESC to pause");
    wrefresh(sentence_win);

    return PLAY_PLAYING;
}

static PlayState play_paused(WINDOW *main_win, WINDOW *sentence_win, const Position ter_size, GameState *state) {
    //选项
    typedef enum {CONTINUE, RETURN} PlayMenuState;
    PlayMenuState play_menu_state = CONTINUE;

    PlayState play_state;

    nodelay(main_win, FALSE);

    wclear(sentence_win);
    print_center_window(sentence_win, "Paused");
    wrefresh(sentence_win);

    WINDOW *play_option_win[2];
    const char *play_option_label[] = {
        [CONTINUE] = "Continue",
        [RETURN] = "Return to Menu",
    };
    create_options_win(play_option_win, play_option_label, 2, ter_size);

    play_menu_state = select_option(main_win, play_option_win, play_option_label, 2, 0);

    switch (play_menu_state) {
        case CONTINUE:
            play_state = PLAY_PLAYING;
            goto cleanup;
        case RETURN:
            play_state = PLAY_QUIT;
            *state = MENU;
            goto cleanup;
        default:
            play_state = ERR;
            *state = ERR;
    }

    cleanup:
        wclear(sentence_win);
        wrefresh(sentence_win);
        wclear(play_option_win[CONTINUE]);
        wclear(play_option_win[RETURN]);
        wrefresh(play_option_win[CONTINUE]);
        wrefresh(play_option_win[RETURN]);
        delwin(play_option_win[CONTINUE]);
        delwin(play_option_win[RETURN]);
        nodelay(main_win, TRUE);
        return play_state;
}

static PlayState play_finished(WINDOW *main_win, WINDOW *sentence_win, const SnakeCLI *snake_cli, const Position ter_size, GameState *state) {
    //结束选单
    typedef enum {RESTART, RETURN_FINISH, EXIT_FINISH} PlayFinishMenu;

    PlayState play_state;

    nodelay(main_win, FALSE);

    WINDOW *play_finish_win[3];
    const char *play_finish_win_label[3] = {
        [RESTART] = "Restart",
        [RETURN_FINISH] = "Return to Menu",
        [EXIT_FINISH] = "Exit",
    };
    create_options_win(play_finish_win, play_finish_win_label, 3, ter_size);

    wclear(sentence_win);
    print_center_window(sentence_win, "Let's try again!");
    wrefresh(sentence_win);

    char msg[32]="";

    if (snake_cli->len == CAP - 1 || snake_cli->apple.row == ERR) {
        sprintf(msg, "You Win! Score: %d", snake_cli->len - 1);
        print_center_window(main_win, msg);
    }
    else {
        sprintf(msg, "Game Over...Score: %d", snake_cli->len - 1);
        print_center_window(main_win, msg);
    }

    wrefresh(main_win);

    PlayFinishMenu play_finish_menu = select_option(main_win, play_finish_win, play_finish_win_label, 3, 0);

    switch (play_finish_menu) {
        case RESTART:
            *state = PLAY;
            play_state = PLAY_QUIT;
            goto cleanup;
        case RETURN_FINISH:
            *state = MENU;
            play_state = PLAY_QUIT;
            goto cleanup;
        case EXIT_FINISH:
            *state = EXIT;
            play_state = PLAY_QUIT;
            goto cleanup;
        default:
            play_state = ERR;
            *state = ERR;
    }

    cleanup:
        wclear(main_win);
        wrefresh(main_win);
        wclear(sentence_win);
        wrefresh(sentence_win);
        wclear(play_finish_win[RESTART]);
        wclear(play_finish_win[EXIT_FINISH]);
        wclear(play_finish_win[RETURN_FINISH]);
        wrefresh(play_finish_win[RESTART]);
        wrefresh(play_finish_win[EXIT_FINISH]);
        wrefresh(play_finish_win[RETURN_FINISH]);
        delwin(play_finish_win[RESTART]);
        delwin(play_finish_win[EXIT_FINISH]);
        delwin(play_finish_win[RETURN_FINISH]);
        return play_state;
}