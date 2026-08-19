//
// Created by gx on 2026/8/14.
//
// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 ElegantGx

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include "game.h"
#include "utils.h"

int main(const int argc, char *argv[]) {
    //处理参数模式
    const int cli = check_command_mode(argc, argv);
    if (cli >= 0) return cli;

    //忽略Ctrl C与Ctrl \，防止终端错乱
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);

    //进入看门狗循环
    int crash_count = 0;
    while (true) {
        const pid_t pid = fork();

        //错误处理
        if (pid == -1) {
            perror("Fork failed");
            return 1;
        }

        if (pid == 0) {
            return game();
        }

        //获取子进程退出码
        int status;
        waitpid(pid, &status, 0);

        //判断是否正常退出
        if (WIFEXITED(status)) {
            const int exit_code = WEXITSTATUS(status);
            if (exit_code == 0 || exit_code == 2 || exit_code == 3) {
                system("stty sane");
                break;
            }
            if (exit_code == 4) {
                crash_count++;
            }
        } else if (WIFSIGNALED(status)) {
            crash_count++;
        }
        
        if (crash_count > 5) {
            system("stty sane");
            printf("Unknown Error\n");
            return 1;
        }

        //重置终端
        system("stty sane");
    }

    return 0;
}
