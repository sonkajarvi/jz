#ifndef COMMON_H_
#define COMMON_H_

#include <stddef.h>
#include <stdint.h>

#include "token.h"

struct context
{
    const uint8_t *bytes;
    size_t size;
    size_t index;
};

int next_token(struct context *ctx, struct token *tok);
void print_token(struct token *tok);

#endif // COMMON_H_
