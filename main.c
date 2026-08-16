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

    //进入看门狗循环
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
            if (exit_code == 0) {
                break;
            }
        }

        //重置终端
        system("stty sane");
    }

    return 0;
}
