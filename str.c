#include <assert.h>
#include <string.h>

#include "str.h"
#include "vec.h"

static uint32_t utf8_to_codepoint(const uint8_t *bytes, int *size)
{
    int retval;

#define A(i, m, x) ((bytes[i] & m) == x)
#define B(i, m, x) ((bytes[i] & m) << x)

    // 0xxx xxxx
    if (A(0, 0x80, 0x0)) {
        if (size)
            *size = 1;
        return bytes[0];
    }

    // 110xx xxx 10xx xxxx
    if (A(0, 0xe0, 0xc0) && A(1, 0xc0, 0x80)) {
        retval = B(0, 0x1f, 6) | B(1, 0x3f, 0);
        if (retval < 0x80)
            return -1;
        if (size)
            *size = 2;
        return retval;
    }

    // 1110 xxxx 10xx xxxx 10xx xxxx
    if (A(0, 0xf0, 0xe0) && A(1, 0xc0, 0x80) && A(2, 0xc0, 0x80)) {
        retval = B(0, 0xf, 12) | B(1, 0x3f, 6) | B(2, 0x3f, 0);
        if (retval < 0x800 || (retval >= 0xd800 && retval <= 0xdfff))
            return -1;
        if (size)
            *size = 3;
        return retval;
    }

    // 1111 0xxx 10xx xxxx 10xx xxxx 10xx xxxx
    if (A(0, 0xf8, 0xf0) && A(1, 0xc0, 0x80) && A(2, 0xc0, 0x80)  && A(3, 0xc0, 0x80)) {
        retval = B(0, 0x7, 18) | B(1, 0x3f, 12) | B(2, 0x3f, 6) | B(3, 0x3f, 0);
        if (retval < 0x10000 || retval > 0x10ffff)
            return -1;
        if (size)
            *size = 4;
        return retval;
    }

#undef A
#undef B

    return -1;
}

size_t string_len(struct string *str)
{
    assert(str);

    return vec_len(str->data);
}

bool string_eq(struct string *str, struct string *other)
{
    assert(str && other);

    if (string_len(str) != string_len(other))
        return false;

    return memcmp(str->data, other->data, string_len(str) * 2) == 0;
}

void string_init(struct string *str)
{
    assert(str);

    str->data = NULL;
}

void string_free(struct string *str)
{
    assert(str);

    vec_free(str->data);
    str->data = NULL;
}

void string_from_utf8(struct string *str, uint8_t *utf8, size_t len)
{
    uint32_t cp;
    int size;

    assert(str && utf8);
    str->data = NULL;

    for (size_t i = 0; i < len; i += size) {
        cp = utf8_to_codepoint(&utf8[i], &size);
        assert(cp != (uint32_t)-1);
        string_push_codepoint(str, cp);
    }
}

void string_push_codepoint(struct string *str, uint32_t cp)
{
    assert(str);
    assert(cp <= 0x10ffff);
    assert(cp <= 0xd7ff || cp >= 0xe000);

    if (cp <= 0xffff) {
        vec_push(str->data, cp);
    } else {
        cp -= 0x10000;
        vec_push(str->data, (cp >> 10) + 0xd800);
        vec_push(str->data, (cp & 0x3ff) + 0xdc00);
    }
}
