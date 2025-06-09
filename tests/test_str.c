#include <string.h>

#include <str.h>

#include "test.h"

TEST(string_from_utf8)
{
    struct string str, str2;

    S(&str, "$€𐐷");

    string_init(&str2);
    string_push_codepoint(&str2, 0x24);
    string_push_codepoint(&str2, 0x20ac);
    string_push_codepoint(&str, 0x10437);

    ASSERT_EQ(memcmp(str.data, str2.data, 4), 0);

    string_free(&str);
    string_free(&str2);
}

TEST(string_push_codepoint)
{
    struct string str;
    string_init(&str);

    string_push_codepoint(&str, 0x24);
    ASSERT_EQ(str.data[0], 0x24);

    string_push_codepoint(&str, 0x20ac);
    ASSERT_EQ(str.data[1], 0x20ac);

    string_push_codepoint(&str, 0x10437);
    ASSERT_EQ(str.data[2], 0xd801);
    ASSERT_EQ(str.data[3], 0xdc37);

    string_push_codepoint(&str, 0x24b62);
    ASSERT_EQ(str.data[4], 0xd852);
    ASSERT_EQ(str.data[5], 0xdf62);

    string_free(&str);
}
