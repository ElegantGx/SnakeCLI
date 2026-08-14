//
// Created by gx on 2026/8/12.
//

#ifndef SNAKECLI_GAME_H
#define SNAKECLI_GAME_H

#define CAP 1000

#include <ncurses.h>

//核心状态
typedef enum {MENU, PLAY, EXIT} GameState;

//核心变量
typedef struct {
    int row;
    int col;
} Position;

typedef struct {
    Position head;
    Position apple;
    Position path[CAP];
    int step;
    int len;
} SnakeCLI;

int game();

GameState menu (WINDOW *main_win, WINDOW *sentence_win, Position ter_size);

GameState play (WINDOW *main_win, WINDOW *sentence_win, Position ter_size);

#endif //SNAKECLI_GAME_H