cmake_minimum_required(VERSION 4.3)
project(SnakeCLI C)

set(CMAKE_C_STANDARD 23)

find_package(Curses REQUIRED)
set(CURSES_NEED_NCURSES TRUE) 

add_executable(SnakeCLI game.c
        utils.c
        utils.h
        menu.c
        game.h
        play.c
        main.c
)

target_link_libraries(SnakeCLI ${CURSES_LIBRARIES})

if(APPLE)
    target_compile_definitions(SnakeCLI PRIVATE 
        _DARWIN_C_SOURCE
        _XOPEN_SOURCE_EXTENDED
    )
endif()
