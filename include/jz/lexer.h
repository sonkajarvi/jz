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

// utf8_to_codepoint()
//
// Converts an UTF-8 encoded character to its corresponding Unicode codepoint.
// Returns -1 for errors, otherwise the codepoint is returned. The size of the
// character in bytes may be written in the optional @size argument. The value
// of @size is undefined if the function returns -1
//
// Validates and checks for:
//  - Prefix and continuation bits
//  - Overlong encodings
//  - Code point within range for Unicode but
//    outside of the range reserved for surrogates
//
// @bytes: UTF-8 encoded character, must be nonnull
// @size: Output to store the size of the character in bytes,
//        may be null
int utf8_to_codepoint(const char *bytes, int *count);

#endif // LEXER_H
