# Snake CLI（终端贪吃蛇）

这是我自己编写的终端贪吃蛇游戏。C23 + ncurses，POSIX 平台。

推荐使用GNU/Linux或MacOS构建运行。

## 游戏截图

![游戏开始界面 ](./Pictures/game_start.png)

![死亡](./Pictures/game_over.png)

## 构建与运行

依赖：ncurses、CMake ≥ 4.3、支持 C23 的编译器。终端至少 20×35。

```sh
cmake -B build
cmake --build build
./build/SnakeCLI
```

## 架构说明

### 看门狗

本项目引入了看门狗来解决终端缩放问题。SIGWINCH信号通过 `siglongjmp` 使子进程非 0 退出，游戏被看门狗整体重启，变相达到自适应效果，免去了繁琐的轮询判断。

注意：siglongjmp在本项目中仅用于让子进程endwin交还终端后exit，请勿随意更改

### 状态机

- `game.c`：顶层 `MENU / PLAY / EXIT`，`menu()`、`play()` 返回下一状态。
- `play.c`：内部 `PLAY_PLAYING / PLAY_PAUSED / PLAY_FINISHED / PLAY_QUIT`，每状态一函数，返回下一状态；`PLAY_QUIT` 为终态，去向经 `GameState *` 出参传递。

### 窗口布局与所有权

```
+----------------------+
|      main_win        |  游戏区（ter_row - 6 行）
+----------------------+
|     sentence_win     |  提示语（3 行）
+----------------------+
| score_win / 选项菜单  |  底部 3 行：游戏中记分栏，暂停/结算时被选项窗口替代
+----------------------+
```

- 局级资源（score_win，存活整局）：`play()` 创建销毁，状态函数仅有使用权。
- 会话级资源（暂停/结算选项窗口）：状态函数进入时创建、退出时销毁，自产自销。
- 输入模式：各状态函数入口自设 `nodelay`（MENU 阻塞、PLAY 非阻塞轮询），不依赖上游残留；函数内临时切换的（暂停/结算菜单需阻塞输入），退出前恢复。

### 核心数据结构：环形缓冲区

`SnakeCLI.path[CAP]` 记录轨迹：

- `path[0]` 注入初始位置，`step` 从 0 计；每 tick `step++`，写 `path[step % CAP]`
- 蛇身 = 最近 `len` 次写入：`path[(step - i) % CAP]`，`i ∈ [0, len)`
- 不变量 `CAP`为蛇最大长度，胜利长度`CAP - 1`防止越界
- 渲染增量式：每帧只画新头、擦旧尾，蛇身靠屏幕残影绘制

### 苹果生成：枚举法

将地图所有空格集合为新数组，通过`random() % n` 抽取达到随机生成目的。无空格则返回 `{ERR, ERR}` 变相告知胜利。

## 文件结构

```
main.c    看门狗进程（fork / waitpid / 终端修复）
game.c    ncurses 初始化、SIGWINCH 处理、顶层状态机
menu.c    主菜单（Play / About / Exit）
play.c    游戏本体（内部状态机、移动、苹果、渲染）
utils.c   通用绘制与输入辅助
game.h    共享数据结构（Position / SnakeCLI / GameState）
```

---
本项目使用GPL v3协议开源