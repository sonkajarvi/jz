#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jz/lexer.h>
#include <jz/token.h>

static bool is_whitespace(const int c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static int peek_n(struct lexer *lx, const size_t n)
{
    if (lx->index + n >= lx->size)
        return -1;

    return lx->buffer[lx->index + n];
}

static int peek(struct lexer *lx)
{
    return peek_n(lx, 0);
}

static int read(struct lexer *lx)
{
    int c;
    if ((c = peek(lx)) != -1)
        lx->index++;

    return c;
}

int utf8_to_codepoint(const char *bytes, int *size)
{
    int retval;

    if (!bytes)
        return -1;

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

static void skip_whitespace(struct lexer *lx)
{
    while (is_whitespace(peek(lx)))
        read(lx);
}

void lexer_init(struct lexer *lx, const char *bytes, const size_t size)
{
    if (!lx || !bytes)
        return;

    lx->buffer = bytes;
    lx->index = 0;
    lx->size = size;

    token_set(&lx->current, TOKEN_UNKNOWN);
}

struct token *lexer_next(struct lexer *lx)
{
    if (!lx)
        return NULL;

    skip_whitespace(lx);

    switch (read(lx)) {
    case -1:
        token_set(&lx->current, TOKEN_EOF);
        break;

#define C(ch, token)                    \
    case ch:                            \
        token_set(&lx->current, token); \
        break

    C('{', TOKEN_BRACE_LEFT);
    C('}', TOKEN_BRACE_RIGHT);
    C(':', TOKEN_COLON);
    C(',', TOKEN_COMMA);
    C('(', TOKEN_PAREN_LEFT);
    C(')', TOKEN_PAREN_RIGHT);
    C(';', TOKEN_SEMICOLON);
    C('[', TOKEN_SQUARE_LEFT);
    C(']', TOKEN_SQUARE_RIGHT);
    C('~', TOKEN_TILDE);

#undef C

    // & && &&= &=
    case '&':
        if (peek(lx) != '&' && peek(lx) != '=') {
            token_set(&lx->current, TOKEN_AMPERSAND);
            break;
        }

        if (peek(lx) == '=') {
            read(lx);
            token_set(&lx->current, TOKEN_AMPERSAND_EQUALS);
            break;
        }

        read(lx);
        if (peek(lx) == '=') {
            read(lx);
            token_set(&lx->current, TOKEN_AMPERSAND_AMPERSAND_EQUALS);
            break;
        }

        token_set(&lx->current, TOKEN_AMPERSAND_AMPERSAND);
        break;

    // * ** **= *=
    case '*':
        if (peek(lx) != '*' && peek(lx) != '=') {
            token_set(&lx->current, TOKEN_ASTERISK);
            break;
        }

        if (peek(lx) == '=') {
            read(lx);
            token_set(&lx->current, TOKEN_ASTERISK_EQUALS);
            break;
        }

        read(lx);
        if (peek(lx) == '=') {
            read(lx);
            token_set(&lx->current, TOKEN_ASTERISK_ASTERISK_EQUALS);
            break;
        }

        token_set(&lx->current, TOKEN_ASTERISK_ASTERISK);
        break;

    // ^ ^=
    case '^':
        if (peek(lx) == '=') {
            read(lx);
            token_set(&lx->current, TOKEN_CARET_EQUALS);
            break;
        }

        token_set(&lx->current, TOKEN_CARET);
        break;

    // . ...
    case '.':
        if (peek(lx) == '.' && peek_n(lx, 1) == '.') {
            read(lx);
            read(lx);
            token_set(&lx->current, TOKEN_DOT_DOT_DOT);
            break;
        }

        token_set(&lx->current, TOKEN_DOT);
        break;

    // = => == ===
    case '=':
        if (peek(lx) != '>' && peek(lx) != '=') {
            token_set(&lx->current, TOKEN_EQUALS);
            break;
        }

        if (read(lx) == '>') {
            token_set(&lx->current, TOKEN_ARROW);
            break;
        }

        if (peek(lx) == '=') {
            read(lx);
            token_set(&lx->current, TOKEN_EQUALS_EQUALS_EQUALS);
            break;
        }

        token_set(&lx->current, TOKEN_EQUALS_EQUALS);
        break;

    // ! != !==
    case '!':
        if (peek(lx) != '=') {
            token_set(&lx->current, TOKEN_EXCLAMATION);
            break;
        }

        read(lx);
        if (peek(lx) != '=') {
            token_set(&lx->current, TOKEN_EXCLAMATION_EQUALS);
            break;
        }

        read(lx);
        token_set(&lx->current, TOKEN_EXCLAMATION_EQUALS_EQUALS);
        break;

    // > >= >> >>= >>> >>>=
    case '>':
        if (peek(lx) != '=' && peek(lx) != '>') {
            token_set(&lx->current, TOKEN_GREATER);
            break;
        }

        if (read(lx) == '=') {
            token_set(&lx->current, TOKEN_GREATER_EQUALS);
            break;
        }

        if (peek(lx) != '=' && peek(lx) != '>') {
            token_set(&lx->current, TOKEN_GREATER_GREATER);
            break;
        }

        if (read(lx) == '=')  {
            token_set(&lx->current, TOKEN_GREATER_GREATER_EQUALS);
            break;
        }

        if (peek(lx) == '=')  {
            read(lx);
            token_set(&lx->current, TOKEN_GREATER_GREATER_GREATER_EQUALS);
            break;
        }

        token_set(&lx->current, TOKEN_GREATER_GREATER_GREATER);
        break;

    // < <= << <<=
    case '<':
        if (peek(lx) != '=' && peek(lx) != '<') {
            token_set(&lx->current, TOKEN_LESS);
            break;
        }

        if (read(lx) == '=') {
            token_set(&lx->current, TOKEN_LESS_EQUALS);
            break;
        }

        if (peek(lx) == '=') {
            read(lx);
            token_set(&lx->current, TOKEN_LESS_LESS_EQUALS);
            break;
        }

        token_set(&lx->current, TOKEN_LESS_LESS);
        break;

    // - -= --
    case '-':
        if (peek(lx) == '=') {
            read(lx);
            token_set(&lx->current, TOKEN_MINUS_EQUALS);
            break;
        }
        else if (peek(lx) == '-') {
            read(lx);
            token_set(&lx->current, TOKEN_MINUS_MINUS);
            break;
        }

        token_set(&lx->current, TOKEN_MINUS);
        break;

    // % %=
    case '%':
        if (peek(lx) == '=') {
            read(lx);
            token_set(&lx->current, TOKEN_PERCENT_EQUALS);
            break;
        }

        token_set(&lx->current, TOKEN_PERCENT);
        break;

    // + += ++
    case '+':
        if (peek(lx) == '=') {
            read(lx);
            token_set(&lx->current, TOKEN_PLUS_EQUALS);
            break;
        }
        else if (peek(lx) == '+') {
            read(lx);
            token_set(&lx->current, TOKEN_PLUS_PLUS);
            break;
        }

        token_set(&lx->current, TOKEN_PLUS);
        break;

    // ? ?. ?? ??=
    case '?':
        if (peek(lx) != '.' && peek(lx) != '?') {
            token_set(&lx->current, TOKEN_QUESTION);
            break;
        }

        if (read(lx) == '.') {
            token_set(&lx->current, TOKEN_QUESTION_DOT);
            break;
        }

        if (peek(lx) == '=') {
            read(lx);
            token_set(&lx->current, TOKEN_QUESTION_QUESTION_EQUALS);
            break;
        }

        token_set(&lx->current, TOKEN_QUESTION_QUESTION);
        break;

    // / /=
    case '/':
        if (peek(lx) == '=') {
            read(lx);
            token_set(&lx->current, TOKEN_SLASH_EQUALS);
            break;
        }

        token_set(&lx->current, TOKEN_SLASH);
        break;

    // | |= || ||=
    case '|':
        if (peek(lx) != '=' && peek(lx) != '|') {
            token_set(&lx->current, TOKEN_VERTICAL);
            break;
        }

        if (read(lx) == '=') {
            token_set(&lx->current, TOKEN_VERTICAL_EQUALS);
            break;
        }

        if (peek(lx) == '=') {
            read(lx);
            token_set(&lx->current, TOKEN_VERTICAL_VERTICAL_EQUALS);
            break;
        }

        token_set(&lx->current, TOKEN_VERTICAL_VERTICAL);
        break;
    }

    return &lx->current;
}
