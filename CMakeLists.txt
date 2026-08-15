cmake_minimum_required(VERSION 4.3)
project(SnakeCLI C)

set(CMAKE_C_STANDARD 23)

set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED)

add_executable(SnakeCLI game.c
        utils.c
        utils.h
        menu.c
        game.h
        play.c
        main.c
)

target_include_directories(SnakeCLI PRIVATE ${CURSES_INCLUDE_DIRS})
target_link_libraries(SnakeCLI ${CURSES_LIBRARIES})