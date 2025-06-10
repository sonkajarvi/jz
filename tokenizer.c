#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <unicode/uchar.h>

#include "str.h"
#include "token.h"
#include "tokenizer.h"

#define PEEK(c) ((c)->reader.peek(c))
#define READ(c) ((c)->reader.read(c))

static const char *token_strings[] = {
#define F(x) #x,
    TOKEN_LIST(F)
#undef F
};

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
    size_t cnt, idx;
    uint32_t cp;

    if (READ(ctx) != 'u')
        return -1;

    if (PEEK(ctx) == '{') {
        READ(ctx);

        // Skip any leading zeroes
        while (PEEK(ctx) == '0')
            READ(ctx);

        cnt = 0;
        idx = ctx->index;
        while (ishex(PEEK(ctx))) {
            READ(ctx);
            cnt++;
        }

        // Count can only be between one and six
        if (cnt == 0 || cnt > 6 || PEEK(ctx) != '}')
            return -1;

        cp = 0;
        ctx->index = idx;
        if (cnt == 6) {
            cp = 1048576; // 1 * 16 ^ 5
            READ(ctx);
            cnt--;
        }

        for (size_t i = 0; i < cnt; i++)
            cp += hextoi(READ(ctx)) * powers[5 - cnt + i];
        READ(ctx);
    } else {
        cp = 0;
        for (size_t i = 0; i < 4; i++) {
            if (!ishex(PEEK(ctx)))
                return -1;
            cp += hextoi(READ(ctx)) * powers[1 + i];
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
    if (PEEK(ctx) == '#')
        string_push_codepoint(&str, READ(ctx));

    if (PEEK(ctx) == '\\') {
        READ(ctx);
        if ((cp = read_unicode_escape_sequence(ctx)) == (uint32_t)-1)
            return -1;
    } else {
        cp = READ(ctx);
    }

    if (!is_identifier_start(cp))
        return -1;
    string_push_codepoint(&str, cp);

    for (;;) {
        if (PEEK(ctx) == '\\') {
            READ(ctx);
            if ((cp = read_unicode_escape_sequence(ctx)) == (uint32_t)-1)
                return -1;
        } else {
            cp = PEEK(ctx);
        }

        if (!is_identifier_part(cp))
            break;

        // push_codepoint(&buf, cp);
        string_push_codepoint(&str, cp);
        READ(ctx);
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

    switch ((cp = PEEK(ctx))) {
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
        READ(ctx);
        if (!isoct(PEEK(ctx)))
            return octoi(cp);
        cp2 = READ(ctx);
        if (!isoct(PEEK(ctx)) || octoi(cp) >= 4)
            return octoi(cp) * 8 + octoi(cp2);
        return octoi(cp) * 64 + octoi(cp2) * 8 + octoi(READ(ctx));

    // Hex escape sequence
    case 'x':
        READ(ctx);
        if ((cp = READ(ctx)) == (uint32_t)-1 || !ishex(cp))
            return -1;
        if ((cp2 = READ(ctx)) == (uint32_t)-1 || !ishex(cp2))
            return -1;
        return hextoi(cp) * 16 + hextoi(cp2);

    case 'u':
        return read_unicode_escape_sequence(ctx);
    }

    READ(ctx);
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

    if (PEEK(ctx) != (is_single ? '\'' : '"'))
        return -1;
    READ(ctx);

loop:
    for (;;) {
        if (PEEK(ctx) == (is_single ? '\'' : '"')) {
            // printf("end\n");
            READ(ctx);
            break;
        } else if (PEEK(ctx) == '\\') {
            READ(ctx);

            switch (PEEK(ctx)) {
            case '\n':
            case '\r':
            case 0x2028:
            case 0x2029:
                READ(ctx);
                if (PEEK(ctx) == '\n')
                    READ(ctx);
                goto loop;

            default:
                if ((cp = read_escape_sequence(ctx)) == (uint32_t)-1)
                    return -1;
            }
        } else {
            if ((cp = PEEK(ctx)) == (uint32_t)-1)
                return -1;
            if (!is_string_character(cp, is_single))
                break;

            READ(ctx);
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

    switch (PEEK(ctx)) {
    case -1:
        tok->type = TOKEN_EOF;
        return 0;

    case '0' ... '9':
        // TODO: Implement parse_numeric_literal
        return -1;

    case '"':
    case '\'':
        return read_string_literal(ctx, tok, PEEK(ctx) == '\'');

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
