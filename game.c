#include <ncurses.h>
#include <setjmp.h>
#include <signal.h>

#include "game.h"

static sigjmp_buf ter_resize;

static void handler(int sig) {
    siglongjmp(ter_resize, 1);
}

int game() {
    //注册信号
    signal(SIGWINCH, handler);

    //处理大小变化
    if (sigsetjmp(ter_resize, 1)) {
        goto cleanup_by_resize;
    }

    //定义默认状态
    GameState state = MENU;

    initscr();      //接管终端
    cbreak();       //按键立刻生效
    noecho();       //按键不回显
    curs_set(0);    //隐藏光标
    set_escdelay(20); //处理ESC延迟
    clear();        //清空默认状态

    //获取终端大小
    int ter_row, ter_col;
    getmaxyx(stdscr, ter_row, ter_col);
    if (ter_row < 20 || ter_col < 35) {
        endwin();
        printf("Your terminal is too small.\n");
        return 0;
    }
    const Position ter_size = {.row = ter_row, .col = ter_col};

    //注册窗口名
    putp("\033]0;Snake CLI\007");
    fflush(stdout);

    //创建主窗口
    WINDOW *main_win = newwin(ter_row - 6, ter_col, 0, 0);

    //创建句窗口
    WINDOW *sentence_win = newwin(3, ter_col, ter_row - 6, 0);
    keypad(sentence_win, true);

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
                return 1;
        }
    }

    cleanup_by_exit:
        delwin(main_win);
        delwin(sentence_win);
        endwin();
        return 0;

    cleanup_by_resize:
        endwin();
        return 1;
}