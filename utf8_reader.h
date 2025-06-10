#ifndef UTF8_READER_H_
#define UTF8_READER_H_

#include <stdint.h>

#include "tokenizer.h"

#define UTF8_READER_INIT {        \
    .reader = {                   \
        .peek = utf8_reader_peek, \
        .read = utf8_reader_read  \
    }                             \
}

struct utf8_reader
{
    struct reader reader;
};

uint32_t utf8_reader_peek(struct context *c);
uint32_t utf8_reader_read(struct context *c);

#endif // UTF8_READER_H_
