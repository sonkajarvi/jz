cmake_minimum_required(VERSION 3.20)

project(tests LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(tests
    main.c

    lexer/test_lexer.c
)

if (EXIT_ON_FAILURE)
    target_compile_definitions(tests PRIVATE
        TEST_EXIT_ON_FAILURE
    )
endif()

target_include_directories(tests PRIVATE
    .
)

target_link_libraries(tests PRIVATE
    jz
)

target_compile_options(tests PRIVATE
    -g
    -O3
    -Wall
    -Wextra
    -Werror
    -Wstrict-prototypes
    -Wno-trigraphs
)
