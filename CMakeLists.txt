cmake_minimum_required(VERSION 3.20)

project(jz)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(jz
    src/lexer.c
    # parser.c
    src/token.c

    src/string.c
)

target_include_directories(jz PUBLIC
    include
)

target_compile_options(jz PRIVATE
    -g
    -O3
    -Wall
    -Wextra
    -Werror
    -Wstrict-prototypes
)

add_subdirectory(tests)
