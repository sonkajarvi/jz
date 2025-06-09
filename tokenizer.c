#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <unicode/uchar.h>

#include "str.h"
#include "token.h"
#include "tokenizer.h"

static const char *token_strings[] = {
#define F(x) #x,
    TOKEN_LIST(F)
#undef F
};

__attribute__((__warn_unused_result__))
static uint32_t peek_offset(struct context *ctx, const size_t offset)
{
    assert(ctx && ctx->bytes);

    if (ctx->index + offset >= ctx->size)
        return -1;
    return ctx->bytes[ctx->index + offset];
}

__attribute__((__warn_unused_result__))
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

__attribute__((__warn_unused_result__))
static uint32_t peek_codepoint(struct context *ctx)
{
    uint32_t c;

    // Character is ASCII
    if (peek(ctx) < 0x80)
        return peek(ctx);

    if ((c = utf8_to_codepoint(&ctx->bytes[ctx->index], NULL)) == (uint32_t)-1)
        return -1;
    return c;
}

static uint32_t read_codepoint(struct context *ctx)
{
    uint32_t c;
    int size;

    // Character is ASCII
    if (peek(ctx) < 0x80)
        return read(ctx);

    if ((c = utf8_to_codepoint(&ctx->bytes[ctx->index], &size)) == (uint32_t)-1)
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

bool isoct(const uint32_t cp)
{
    return cp >= '0' && cp <= '7';
}

uint32_t octoi(const uint32_t cp)
{
    return cp - '0';
}

// https://tc39.es/ecma262/#prod-UnicodeEscapeSequence
//
//   UnicodeEscapeSequence ::
//     u Hex4Digits
//     u{ CodePoint }
//
//   CodePoint ::
//     HexDigits                ; but only if the MV of HexDigits <= 0x10ffff
static uint32_t read_unicode_escape_sequence(struct context *ctx)
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

        for (size_t i = 0; i < cnt; i++)
            cp += hextoi(read(ctx)) * powers[5 - cnt + i];
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
    struct string str;
    uint32_t cp;

    string_init(&str);

    // Include # for private identifiers
    if (peek_codepoint(ctx) == '#')
        string_push_codepoint(&str, read_codepoint(ctx));

    if (peek(ctx) == '\\') {
        read(ctx);
        if ((cp = read_unicode_escape_sequence(ctx)) == (uint32_t)-1)
            return -1;
    } else {
        cp = read_codepoint(ctx);
    }

    if (!is_identifier_start(cp))
        return -1;
    string_push_codepoint(&str, cp);

    for (;;) {
        if (peek(ctx) == '\\') {
            read(ctx);
            if ((cp = read_unicode_escape_sequence(ctx)) == (uint32_t)-1)
                return -1;
        } else {
            cp = peek_codepoint(ctx);
        }

        if (!is_identifier_part(cp))
            break;

        // push_codepoint(&buf, cp);
        string_push_codepoint(&str, cp);
        read_codepoint(ctx);
    }

    tok->type = TOKEN_IDENTIFIER;
    tok->str = str;
    return 0;
}

// https://tc39.es/ecma262/#prod-EscapeSequence
//
//   EscapeSequence ::
//     CharacterEscapeSequence
//     0 [lookahead != DecimalDigit]
//     LegacyOctalEscapeSequence
//     NonOctalDecimalEscapeSequence
//     HexEscapeSequence
//     UnicodeEscapeSequence
//
//   CharacterEscapeSequence ::
//     SingleEscapeCharacter
//     SourceCharacter          ; but not one of EscapeCharacter or LineTerminator
//
//   SingleEscapeCharacter :: one of
//     ' " \ b f n r t v
//
//   EscapeCharacter ::
//     SingleEscapeCharacter
//     DecimalDigit
//     x
//     u
//
//   LegacyOctalEscapeSequence ::
//     0 [lookahead == {8, 9}]
//     NonZeroOctalDigit [lookahead != OctalDigit]
//     ZeroToThree OctalDigit [lookahead != OctalDigit]
//     FourToSeven OctalDigit
//     ZeroToThree OctalDigit OctalDigit
//
//   NonOctalDecimalEscapeSequence :: one of
//     8 9
//
//   HexEscapeSequence ::
//     x HexDigit HexDigit
static uint32_t read_escape_sequence(struct context *ctx)
{
    uint32_t cp, cp2;

    switch ((cp = peek_codepoint(ctx))) {
    // Character escape sequence,
    // non-octal decimal escape sequence
    default:
        break;

    // Single escape character
    case '\'':
    case '"':
    case '\\':
        break;
    case 'b': cp = '\b'; break;
    case 'f': cp = '\f'; break;
    case 'n': cp = '\n'; break;
    case 'r': cp = '\r'; break;
    case 't': cp = '\t'; break;
    case 'v': cp = '\v'; break;

    // Legacy octal escape sequence
    case '0' ... '7':
        read_codepoint(ctx);
        if (!isoct(peek(ctx)))
            return octoi(cp);
        cp2 = read(ctx);
        if (!isoct(peek(ctx)) || octoi(cp) >= 4)
            return octoi(cp) * 8 + octoi(cp2);
        return octoi(cp) * 64 + octoi(cp2) * 8 + octoi(read(ctx));

    // Hex escape sequence
    case 'x':
        read_codepoint(ctx);
        if ((cp = read_codepoint(ctx)) == (uint32_t)-1 || !ishex(cp))
            return -1;
        if ((cp2 = read_codepoint(ctx)) == (uint32_t)-1 || !ishex(cp2))
            return -1;
        return hextoi(cp) * 16 + hextoi(cp2);

    case 'u':
        return read_unicode_escape_sequence(ctx);
    }

    read_codepoint(ctx);
    return cp;
}

// https://tc39.es/ecma262/#prod-DoubleStringCharacter
//
//   DoubleStringCharacter ::
//     SourceCharacter          ; but not one of " or \ or LineTerminator
//     <LS>                     ; Line separator
//     <PS>                     ; Paragraph separator
//
//   SingleStringCharacter ::
//     SourceCharacter          ; but not one of ' or \ or LineTerminator
//     <LS>
//     <PS>
static bool is_string_character(const uint32_t cp, const bool is_single)
{
    uint32_t delim = '"';
    if (is_single)
        delim = '\'';
    return cp <= 0x10ffff && cp != '\n' && cp != '\r' && cp != delim && cp != '\\';
}

// https://tc39.es/ecma262/#prod-StringLiteral
//
//   StringLiteral ::
//     " DoubleStringCharacters_opt "
//     ' SingleStringCharacters_opt '
//
//   LineContinuation ::
//     \ LineTerminatorSequence
//
//   LineTerminatorSequence ::
//     <LF>                     ; Line feed
//     <CR> [lookahead != <LF>] ; Carriage return
//     <LS>                     ; Line separator
//     <PS>                     ; Paragraph separator
//     <CR> <LF>
static int read_string_literal(struct context *ctx, struct token *tok, const bool is_single)
{
    struct string str;
    uint32_t cp;

    string_init(&str);

    if (peek(ctx) != (is_single ? '\'' : '"'))
        return -1;
    read(ctx);

loop:
    for (;;) {
        if (peek(ctx) == (is_single ? '\'' : '"')) {
            // printf("end\n");
            read(ctx);
            break;
        } else if (peek(ctx) == '\\') {
            read(ctx);

            switch (peek_codepoint(ctx)) {
            case '\n':
            case '\r':
            case 0x2028:
            case 0x2029:
                read_codepoint(ctx);
                if (peek(ctx) == '\n')
                    read(ctx);
                goto loop;

            default:
                if ((cp = read_escape_sequence(ctx)) == (uint32_t)-1)
                    return -1;
            }
        } else {
            if ((cp = peek_codepoint(ctx)) == (uint32_t)-1)
                return -1;
            if (!is_string_character(cp, is_single))
                break;

            read_codepoint(ctx);
        }

        string_push_codepoint(&str, cp);
    }

    tok->type = TOKEN_STRING_LITERAL;
    tok->str = str;
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
        return read_string_literal(ctx, tok, peek(ctx) == '\'');

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
    case TOKEN_STRING_LITERAL:
        // fwrite(tok->str, 1, tok->len, stdout);
        break;
    }
    printf("\n");
}
