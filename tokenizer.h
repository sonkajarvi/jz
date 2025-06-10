#ifndef COMMON_H_
#define COMMON_H_

#include <stddef.h>
#include <stdint.h>

#include "token.h"

struct context;
struct reader
{
    // Get codepoint without advancing
    // __attribute__((__warn_unused_result__))
    uint32_t (*peek)(struct context *);
    // Get codepoint and advance
    uint32_t (*read)(struct context *);
};

struct context
{
    struct reader reader;
    const uint8_t *bytes;
    size_t size;
    size_t index;
};

int next_token(struct context *ctx, struct token *tok);
void print_token(struct token *tok);

#endif // COMMON_H_
