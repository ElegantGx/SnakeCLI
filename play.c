//
// Created by gx on 2026/8/12.
//

#include <ncurses.h>
#include <stdlib.h>
#include <time.h>

#include "game.h"
#include "utils.h"

#define fps_60 60

//游玩与暂停
typedef enum {PLAY_PLAYING, PLAY_PAUSED, PLAY_FINISHED} PlayState;

//方向键
typedef enum {UP, DOWN, LEFT, RIGHT, ESC = -1, NONE = -2} PlayDirection;

//暂停选单
typedef enum {CONTINUE, RETURN} PlayMenuState;

//方向向量
static const Position DELTA[] = {
    [UP] = {.row = -1, .col = 0},
    [DOWN] = {.row = 1, .col = 0},
    [LEFT] = {.row = 0, .col = -1},
    [RIGHT] = {.row = 0, .col = 1},
};

static void check_finish_play(const SnakeCLI *snake_cli, Position game_size, PlayState *play_state);

static void generate_apple_play(SnakeCLI *snake_cli, Position game_size);

//游戏主进程
GameState play (WINDOW *main_win, WINDOW *sentence_win, const Position ter_size) {
    GameState state = PLAY;

    PlayState play_state = PLAY_PLAYING;

    PlayDirection input = RIGHT;

    PlayMenuState play_menu_state = CONTINUE;

    srandom((unsigned int)time(nullptr));

    //状态绘制
    wclear(sentence_win);
    print_center_window(sentence_win, "Playing...Use ESC to pause game");
    wrefresh(sentence_win);

    //选项绘制
    WINDOW *play_option_win[2];
    const char *play_option_label[] = {
        [CONTINUE] = "Continue",
        [RETURN] = "Return to Menu",
    };
    create_options_win(play_option_win, play_option_label, 2, ter_size);

    //刷新显示，启用非阻塞轮询
    wclear(main_win);
    nodelay(main_win, TRUE);    //启用非阻塞轮询
    keypad(main_win, TRUE);

    //定义核心变量
    SnakeCLI snake_cli = {
        .step = 0,
        .len = 1,
    };

    //获取初始坐标
    int game_max_row, game_max_col;
    getmaxyx(main_win, game_max_row, game_max_col);
    Position game_max = {.row = game_max_row, .col = game_max_col};
    snake_cli.head.row = game_max_row / 2;
    snake_cli.head.col = (game_max_col - 1) / 2;

    //获取苹果初始坐标
    generate_apple_play(&snake_cli, game_max);

    //注入初始坐标
    snake_cli.path[snake_cli.step % CAP] = snake_cli.head;

    //初始化绘制
    wclear(main_win);
    box(main_win, 0, 0);
    print_center_window(main_win, "@");
    mvwprintw(main_win, snake_cli.apple.row, snake_cli.apple.col, "@");

    wrefresh(main_win);

    while (true) {
        napms(fps_60);

        //获取输入
        const int tmp_input = map_the_key_play(get_input_play(main_win));

        //判断是否需要暂停
        if (tmp_input == ESC) play_state = PLAY_PAUSED;

        if ((input ^ tmp_input) != 1 && tmp_input >= 0) input = tmp_input;

        //处理游戏状态
        switch (play_state) {
            case PLAY_PLAYING: {
                snake_cli.step++;
                snake_cli.head.row += DELTA[input].row;
                snake_cli.head.col += DELTA[input].col;
                break;
            }
            case PLAY_PAUSED: {
                wclear(sentence_win);
                print_center_window(sentence_win, "Paused");
                play_menu_state = select_option(sentence_win,play_option_win, play_option_label, 2, 0);
                switch (play_menu_state) {
                    case CONTINUE: {
                        play_state = PLAY_PLAYING;
                        print_center_window(sentence_win, "Playing...Use ESC to pause game");
                        break;
                    }
                    case RETURN: {
                        state = MENU;
                        goto cleanup;
                    }
                }
                break;
            }
            case PLAY_FINISHED: {
                wclear(sentence_win);
                print_center_window(sentence_win, "Let's Try Again!");

                wclear(main_win);
                box(main_win, 0, 0);
                char msg[32]="";
                if (snake_cli.len != 999) {
                    snprintf(msg, sizeof(msg), "Game Over. Score: %d", snake_cli.len - 1);
                } else {
                    snprintf(msg, sizeof(msg), "You Win! You are God!");
                }
                print_center_window(main_win, msg);

                typedef enum {RESTART, RETURN_FINISH, EXIT_FINISH} PlayFinishMenu;
                PlayFinishMenu play_finish_menu = RESTART;

                WINDOW *play_finish_win[3];
                const char *play_finish_win_label[3] = {
                    [RESTART] = "Restart",
                    [RETURN_FINISH] = "Return to Menu",
                    [EXIT_FINISH] = "Exit",
                };
                create_options_win(play_finish_win, play_finish_win_label, 3, ter_size);

                play_finish_menu = select_option(sentence_win, play_finish_win, play_finish_win_label, 3, 0);

                switch (play_finish_menu) {
                    case RESTART: {
                        state = PLAY;
                        goto cleanup;
                    }
                    case RETURN_FINISH: {
                        state = MENU;
                        goto cleanup;
                    }
                    case EXIT_FINISH:
                        state = EXIT;
                        goto cleanup;
                }
            }
            default:
                state = ERR;
                goto cleanup;
        }

        //检查苹果
        if (snake_cli.head.row == snake_cli.apple.row && snake_cli.head.col == snake_cli.apple.col) {
            snake_cli.len++;
            generate_apple_play(&snake_cli, game_max);
        }

        //检查是否死亡或胜利
        check_finish_play(&snake_cli, game_max, &play_state);

        //更新数组
        snake_cli.path[snake_cli.step % CAP] = snake_cli.head;

        //渲染画面
        render_snake_play(main_win, &snake_cli);
        mvwprintw(main_win, snake_cli.apple.row, snake_cli.apple.col, "@");
    }

    cleanup:
        wclear(play_option_win[CONTINUE]);
        wclear(play_option_win[RETURN]);
        delwin(play_option_win[CONTINUE]);
        delwin(play_option_win[RETURN]);
        return state;
}

void check_finish_play(const SnakeCLI *snake_cli, const Position game_size, PlayState *play_state) {
    if (snake_cli->head.row < 1 || snake_cli->head.col < 1 ||
        snake_cli->head.row > game_size.row - 2 || snake_cli->head.col > game_size.col - 2)
    {
        *play_state = PLAY_FINISHED;
        return;
    }

    for (int i = 0; i < snake_cli->len; i++) {
        if (snake_cli->head.row == snake_cli->path[(snake_cli->step - i - 1) % CAP].row &&
            snake_cli->head.col == snake_cli->path[(snake_cli->step - i - 1) % CAP].col)
        {
            *play_state = PLAY_FINISHED;
            return;
        }
    }

    if (snake_cli->len == 999) {
        *play_state = PLAY_FINISHED;
        return;
    }
}

//生成苹果坐标
void generate_apple_play(SnakeCLI *snake_cli, const Position game_size) {
    int conflict;
    do {
        // 生成苹果
        snake_cli->apple.row = (int)random() % (game_size.row - 2) + 1;
        snake_cli->apple.col = (int)random() % (game_size.col - 2) + 1;

        conflict = 0;
        for (int j = 0; j <= snake_cli->len; j++) {
            if (snake_cli->apple.row == snake_cli->path[(snake_cli->step - j + CAP) % CAP].row &&
                snake_cli->apple.col == snake_cli->path[(snake_cli->step - j + CAP) % CAP].col) {
                conflict = 1;
                break;
                }
        }
    } while (conflict);
}