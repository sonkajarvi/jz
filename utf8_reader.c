#include <stddef.h>
#include <stdint.h>

#include "utf8_reader.h"

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

uint32_t utf8_reader_peek(struct context *c)
{
    return utf8_to_codepoint(&c->bytes[c->index], NULL);
}

uint32_t utf8_reader_read(struct context *c)
{
    uint32_t cp;
    int size;

    if ((cp = utf8_to_codepoint(&c->bytes[c->index], &size)) != UINT32_MAX)
        c->index += size;
    return cp;
}
