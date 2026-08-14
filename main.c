//
// Created by gx on 2026/8/14.
//

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#include "game.h"

int main() {

    //进入看门狗循环
    while (true) {
        pid_t pid = fork();

        //错误处理
        if (pid == -1) {
            perror("Fork failed.Maybe out of memory?");
            return 1;
        }

        if (pid == 0) {
            return game_main();
            break;
        }

        //获取子进程退出码
        int status;
        waitpid(pid, &status, 0);

        //判断是否正常退出
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            if (exit_code == 0) {
                break;
            }
        }
    }
    return 0;
}