#ifndef TEST_H_
#define TEST_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

struct test_value
{
    const char *expr;
    union {
        int64_t i;
        double d;
        const char *s;
    } value;
};

struct test_result
{
    uint8_t status;

    const char *file;
    int line;

    uint8_t type;
    struct test_value lhs;
    struct test_value rhs;
};

struct test_case
{
    const char *name;
    void (*callback)(struct test_result *);
};

struct test_context
{
    struct test_case *tests;
    size_t capacity;
    size_t size;
};

#define TEST_PASSED_ 0
#define TEST_FAILED_ 1
#define TEST_CASE_RESULT_PARAM_ result_

#define TEST_CASE_NAME_(name) TEST_CASE_##name##_
#define TEST_WRAPPER_NAME_(name) TEST_WRAPPER_##name##_

#define TEST(name)                                    \
    void TEST_CASE_NAME_(name)(struct test_result *); \
    __attribute__((constructor))                      \
    void TEST_WRAPPER_NAME_(name)(void) {             \
        test_add_(#name, TEST_CASE_NAME_(name)); }    \
    void TEST_CASE_NAME_(name)(struct test_result *TEST_CASE_RESULT_PARAM_)

#define ASSERT_(a, b, cmp, type, field) do {            \
    type a_ = (type)(a);                                \
    type b_ = (type)(b);                                \
    if (!(a_ cmp b_)) {                                 \
        TEST_CASE_RESULT_PARAM_->status = TEST_FAILED_; \
        TEST_CASE_RESULT_PARAM_->file = __FILE__;       \
        TEST_CASE_RESULT_PARAM_->line = __LINE__;       \
        TEST_CASE_RESULT_PARAM_->lhs.expr = #a;         \
        TEST_CASE_RESULT_PARAM_->lhs.value.field = a_;  \
        TEST_CASE_RESULT_PARAM_->rhs.expr = #b;         \
        TEST_CASE_RESULT_PARAM_->rhs.value.field = b_;  \
        return; }} while (0)

#define ASSERT_INT_(a, b, cmp) ASSERT_(a, b, cmp, uint64_t, i)

#define ASSERT_EQ(a, b) ASSERT_INT_(a, b, ==)
#define ASSERT_NE(a, b) ASSERT_INT_(a, b, !=)
#define ASSERT_GT(a, b) ASSERT_INT_(a, b, > )
#define ASSERT_GE(a, b) ASSERT_INT_(a, b, >=)
#define ASSERT_LT(a, b) ASSERT_INT_(a, b, < )
#define ASSERT_LE(a, b) ASSERT_INT_(a, b, <=)

void test_add_(const char *name, void (*callback)(struct test_result *));

#endif // TEST_H_
