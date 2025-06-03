#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <unicode/uchar.h>

#include "token.h"
#include "tokenizer.h"
#include "vec.h"

static const char *token_strings[] = {
#define F(x) #x,
    TOKEN_LIST(F)
#undef F
};

static uint32_t peek_offset(struct context *ctx, const size_t offset)
{
    assert(ctx && ctx->bytes);

    if (ctx->index + offset >= ctx->size)
        return -1;
    return ctx->bytes[ctx->index + offset];
}

static uint32_t peek(struct context *ctx)
{
    return peek_offset(ctx, 0);
}

static uint32_t read(struct context *ctx)
{
    uint32_t c;
    if ((c = peek(ctx)) == (uint32_t)-1)
        return -1;
    ctx->index++;
    return c;
}

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

__attribute__((unused))
static uint32_t peek_codepoint(struct context *ctx)
{
    uint32_t c;

    // Character is ASCII
    if (peek(ctx) < 0x80)
        return peek(ctx);

    if ((c = utf8_to_codepoint(ctx->bytes, NULL)))
        return -1;
    return c;
}

__attribute__((unused))
static uint32_t read_codepoint(struct context *ctx)
{
    uint32_t c;
    int size;

    // Character is ASCII
    if (peek(ctx) < 0x80)
        return read(ctx);

    if ((c = utf8_to_codepoint(ctx->bytes, &size)))
        return -1;
    ctx->index += size;
    return c;
}

// https://tc39.es/ecma262/#prod-WhiteSpace
//
//   Whitespace ::
//     <TAB>                ; Character tabulation
//     <VT>                 ; Line tabulation
//     <FF>                 ; Form feed
//     <ZWNBSP>             ; Zero width no-break space
//     <USP>                ; Any code point in general category "Space_Separator"
__attribute__((unused))
static bool is_whitespace(const int cp)
{
    return cp == '\t'
        || cp == '\v'
        || cp == '\f'
        || cp == 0xfeff
        || u_isUWhiteSpace(cp);
}

// https://tc39.es/ecma262/#prod-IdentifierStart
//
//   IdentifierStart ::
//     UnicodeIDStart       ; Any code point with the property "ID_Start"
//     $
//     _
static bool is_identifier_start(const uint32_t cp)
{
    return cp == '$'
        || cp == '_'
        || u_isIDStart(cp);
}

// https://tc39.es/ecma262/#prod-IdentifierPart
//
//   IdentifierPart ::
//     UnicodeIDContinue    ; Any code point with the property "ID_Continue"
//     $
static bool is_identifier_part(const uint32_t cp)
{
    return cp == '$'
        || u_isIDPart(cp);
}

// TODO: Return a status code
static void push_codepoint(uint8_t **buf, uint32_t cp)
{
    if (cp <= 0x7f) {
        vec_push(*buf, cp);
    } else if (cp <= 0x7ff) {
        vec_push(*buf, ((cp >> 6) & 0x1f) | 0xc0);
        vec_push(*buf, (cp        & 0x3f) | 0x80);
    } else if (cp <= 0xffff) {
        vec_push(*buf, ((cp >> 12) & 0x0f) | 0xe0);
        vec_push(*buf, ((cp >> 6)  & 0x3f) | 0x80);
        vec_push(*buf, (cp         & 0x3f) | 0x80);
    } else if (cp <= 0x10ffff) {
        vec_push(*buf, ((cp >> 18) & 0x07) | 0xf0);
        vec_push(*buf, ((cp >> 12) & 0x3f) | 0x80);
        vec_push(*buf, ((cp >> 6)  & 0x3f) | 0x80);
        vec_push(*buf, (cp         & 0x3f) | 0x80);
    }
}

bool ishex(const uint32_t c)
{
    switch (c) {
    case '0' ... '9':
    case 'A' ... 'F':
    case 'a' ... 'f':
        return true;

    default:
        return false;
    }
}

uint32_t hextoi(const uint32_t c)
{
    switch (c) {
    case '0' ... '9':
        return c - '0';

    case 'A' ... 'F':
        return c - 'A' + 10;

    case 'a' ... 'f':
        return c - 'a' + 10;

    default:
        return -1;
    }
}

// https://tc39.es/ecma262/#prod-UnicodeEscapeSequence
//
//   UnicodeEscapeSequence ::
//     u Hex4Digits
//     u{ CodePoint }
//
//   CodePoint ::
//     HexDigits                ; but only if the MV of HexDigits <= 0x10ffff
static uint32_t read_escape_sequence(struct context *ctx)
{
    static const uint32_t powers[] = {65536, 4096, 256, 16, 1};
    size_t cnt;
    uint32_t cp;

    if (read(ctx) != 'u')
        return -1;

    if (peek(ctx) == '{') {
        read(ctx);

        // Skip any leading zeroes
        while (peek(ctx) == '0')
            read(ctx);

        cnt = 0;
        while (ishex(peek_offset(ctx, cnt)))
            cnt++;

        // Count can only be between one and six
        if (cnt == 0 || cnt > 6 || peek_offset(ctx, cnt) != '}')
            return -1;

        cp = 0;
        if (cnt == 6) {
            cp = 1048576; // 1 * 16 ^ 5
            read(ctx);
            cnt--;
        }

        for (size_t i = 0; i < cnt; i++) {
            // printf("%c\n", peek(ctx));
            cp += hextoi(read(ctx)) * powers[5 - cnt + i];
        }
        read(ctx);
    } else {
        cp = 0;
        for (size_t i = 0; i < 4; i++) {
            if (!ishex(peek(ctx)))
                return -1;
            cp += hextoi(read(ctx)) * powers[1 + i];
        }
    }

    if (cp > 0x10ffff)
        return -1;
    return cp;
}

// https://tc39.es/ecma262/#prod-IdentifierName
//
//   PrivateIdentifier ::
//     # IdentifierName
//
//   IdentifierName ::
//     IdentifierStart
//     IdentifierName IdentifierPart
static int read_identifier_name(struct context *ctx, struct token *tok)
{
    uint8_t *buf = NULL;
    uint32_t cp;

    // Include # for private identifiers
    if (peek(ctx) == '#')
        vec_push(buf, read(ctx));

    if (peek(ctx) == '\\') {
        read(ctx);
        if ((cp = read_escape_sequence(ctx)) == (uint32_t)-1)
            return -1;
    } else {
        cp = read_codepoint(ctx);
    }

    if (!is_identifier_start(cp))
        return -1;
    push_codepoint(&buf, cp);

    for (;;) {
        if (peek(ctx) == '\\') {
            read(ctx);
            if ((cp = read_escape_sequence(ctx)) == (uint32_t)-1)
                return -1;
        } else {
            cp = peek_codepoint(ctx);
        }

        if (!is_identifier_part(cp))
            break;

        push_codepoint(&buf, cp);
        read_codepoint(ctx);
    }

    vec_push(buf, 0);
    tok->type = TOKEN_IDENTIFIER;
    tok->id.str = buf;
    tok->id.len = vec_len(buf);

    return 0;
}

int next_token(struct context *ctx, struct token *tok)
{
    assert(ctx && ctx->bytes);

    if (ctx->index >= ctx->size) {
        tok->type = TOKEN_EOF;
        return 0;
    }

    switch (peek(ctx)) {
    case -1:
        tok->type = TOKEN_EOF;
        return 0;

    case '0' ... '9':
        // TODO: Implement parse_numeric_literal
        return -1;

    case '"':
    case '\'':
        // TODO: Implement parse_string_literal
        return -1;

    case '`':
        // TODO: Implement parse_template_literal
        return -1;

    default:
        return read_identifier_name(ctx, tok);

#define F(c, type_)        \
    case c:                \
        tok->type = type_; \
        ctx->index++;      \
        return 0

    F('{', TOKEN_BRACE_LEFT);
    F('}', TOKEN_BRACE_RIGHT);
    F(':', TOKEN_COLON);
    F(',', TOKEN_COMMA);
    F('(', TOKEN_PAREN_LEFT);
    F(')', TOKEN_PAREN_RIGHT);
    F(';', TOKEN_SEMICOLON);
    F('[', TOKEN_SQUARE_LEFT);
    F(']', TOKEN_SQUARE_RIGHT);
    F('~', TOKEN_TILDE);

#undef F

#define F(s, type_)                                              \
    if (ctx->index + strlen(s) <= ctx->size                      \
        && memcmp(&ctx->bytes[ctx->index], s, strlen(s)) == 0) { \
        tok->type = type_;                                       \
        ctx->index += strlen(s);                                 \
        return 0; }

    // &&= && &= &
    case '&':
        F("&&=", TOKEN_AMPERSAND_AMPERSAND_EQUALS);
        F("&&", TOKEN_AMPERSAND_AMPERSAND);
        F("&=", TOKEN_AMPERSAND_EQUALS);

        tok->type = TOKEN_AMPERSAND;
        break;

    // **= ** *= *
    case '*':
        F("**=", TOKEN_ASTERISK_ASTERISK_EQUALS);
        F("**", TOKEN_ASTERISK_ASTERISK);
        F("*=", TOKEN_ASTERISK_EQUALS);

        tok->type = TOKEN_ASTERISK;
        break;

    // ^= ^
    case '^':
        F("^=", TOKEN_CARET_EQUALS);

        tok->type = TOKEN_CARET;
        break;

    // ... .
    case '.':
        F("...", TOKEN_DOT_DOT_DOT);

        tok->type = TOKEN_DOT;
        break;

    // === == => =
    case '=':
        F("===", TOKEN_EQUALS_EQUALS_EQUALS);
        F("==", TOKEN_EQUALS_EQUALS);
        F("=>", TOKEN_ARROW);

        tok->type = TOKEN_EQUALS;
        break;

    // !== != !
    case '!':
        F("!==", TOKEN_EXCLAMATION_EQUALS_EQUALS);
        F("!=", TOKEN_EXCLAMATION_EQUALS);

        tok->type = TOKEN_EXCLAMATION;
        break;

    // >>>= >>> >>= >> >= >
    case '>':
        F(">>>=", TOKEN_GREATER_GREATER_GREATER_EQUALS);
        F(">>>", TOKEN_GREATER_GREATER_GREATER);
        F(">>=", TOKEN_GREATER_GREATER_EQUALS);
        F(">>", TOKEN_GREATER_GREATER);
        F(">=", TOKEN_GREATER_EQUALS);

        tok->type = TOKEN_GREATER;
        break;

    // <<= << <= <
    case '<':
        F("<<=", TOKEN_LESS_LESS_EQUALS);
        F("<<", TOKEN_LESS_LESS);
        F("<=", TOKEN_LESS_EQUALS);

        tok->type = TOKEN_LESS;
        break;

    // -= -- -
    case '-':
        F("-=", TOKEN_MINUS_EQUALS);
        F("--", TOKEN_MINUS_MINUS);

        tok->type = TOKEN_MINUS;
        break;

    // %= %
    case '%':
        F("%=", TOKEN_PERCENT_EQUALS);

        tok->type = TOKEN_PERCENT;
        break;

    // += ++ +
    case '+':
        F("+=", TOKEN_PLUS_EQUALS);
        F("++", TOKEN_PLUS_PLUS);

        tok->type = TOKEN_PLUS;
        break;

    // ??= ?? ?. ?
    case '?':
        F("??=", TOKEN_QUESTION_QUESTION_EQUALS);
        F("??", TOKEN_QUESTION_QUESTION);
        F("?.", TOKEN_QUESTION_DOT);

        tok->type = TOKEN_QUESTION;
        break;

    // /= /
    case '/':
        F("/=", TOKEN_SLASH_EQUALS);

        tok->type = TOKEN_SLASH;
        break;

    // ||= || |= |
    case '|':
        F("||=", TOKEN_VERTICAL_VERTICAL_EQUALS);
        F("||", TOKEN_VERTICAL_VERTICAL);
        F("|=", TOKEN_VERTICAL_EQUALS);

        tok->type = TOKEN_VERTICAL;
        break;

#undef F
    }

    ctx->index++;
    return 0;
}

void print_token(struct token *tok)
{
    assert(tok && tok->type < TOKEN_COUNT);

    printf("%s ", token_strings[tok->type]);
    switch (tok->type) {
    case TOKEN_IDENTIFIER:
        fwrite(tok->id.str, 1, tok->id.len, stdout);
        break;
    }
    printf("\n");
}
