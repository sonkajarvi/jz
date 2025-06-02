cmake_minimum_required(VERSION 3.20)

project(jz LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(jz
    tokenizer.c
)

find_package(ICU REQUIRED COMPONENTS uc)
target_link_libraries(jz PRIVATE
    ICU::uc
)

target_compile_options(jz PRIVATE
    -g
    -O3
    -Wall
    -Wextra
    -Werror
    -Wstrict-prototypes
    -Wno-trigraphs
)

add_subdirectory(tests)
