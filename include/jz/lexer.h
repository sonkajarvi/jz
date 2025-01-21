#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>

#include <jz/token.h>

struct lexer
{
    const char *buffer;
    size_t index;
    size_t size;

    struct token current;
};

void lexer_init(struct lexer *lx, const char *bytes, const size_t size);

struct token *lexer_next(struct lexer *lx);

int utf8_to_codepoint(const char *bytes, int *count);

#endif // LEXER_H
