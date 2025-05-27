#ifndef TEST_H
#define TEST_H

#include <stdio.h>

#ifdef TEST_EXIT_ON_FAILURE
# include <stdlib.h>
#endif

#define GREEN   "\033[1;92m"
#define RED     "\033[1;91m"
#define BLACK   "\033[90m"
#define RESET   "\033[0m"

#define TEST_SUCCESS 0
#define TEST_FAILURE 1

#define TEST_NAME(x) __TEST_##x

#define PASS_FORMAT "[ " GREEN "pass" RESET " ] %s\n"
#define FAIL_FORMAT "[ " RED "fail" RESET " ] %s " BLACK "(assert '%s', in %s:%d)" RESET "\n"

#define test_declare(x) ({            \
    struct result TEST_NAME(x)(void); \
    test_run(&(struct test){ TEST_NAME(x), #x }); })

#define test_case(x) struct result TEST_NAME(x)(void)

#define test_assert(expr) \
    if (!(expr))          \
        return (struct result){ TEST_FAILURE, __LINE__, #expr, __FILE_NAME__ }

#define test_success() \
    return (struct result){ TEST_SUCCESS, 0, 0, 0 }

struct result
{
    int status, line;
    const char *expr, *file;
};

struct test
{
    struct result (*function)(void);
    const char *name;
};

static inline void test_run(struct test *test)
{
    struct result result = test->function();

    if (result.status == TEST_SUCCESS) {
        printf(PASS_FORMAT, test->name);
    } else {
        printf(FAIL_FORMAT, test->name, result.expr, result.file, result.line);
#ifdef TEST_EXIT_ON_FAILURE
        exit(1);
#endif
    }
}

#endif // TEST_H
