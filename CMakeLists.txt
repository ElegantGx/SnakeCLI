cmake_minimum_required(VERSION 4.3)
project(SnakeCLI C)

set(CMAKE_C_STANDARD 23)

find_package(Curses REQUIRED)

add_executable(SnakeCLI game_main.c
        utils.c
        utils.h
        menu.c
        game.h
        play.c
        main.c
)

target_link_libraries(SnakeCLI ${CURSES_LIBRARIES})