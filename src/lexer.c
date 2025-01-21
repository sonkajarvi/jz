#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unicode/uchar.h>

#include <jz/lexer.h>
#include <jz/string.h>
#include <jz/token.h>

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

static const char *lexer_position(struct lexer *lx)
{
    return &lx->buffer[lx->index];
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

// https://tc39.es/ecma262/#prod-WhiteSpace
//
//   Whitespace ::
//     <TAB>            ; Character tabulation
//     <VT>             ; Line tabulation
//     <FF>             ; Form feed
//     <ZWNBSP>         ; Zero width no-break space
//     <USP>            ; Any code point in general category "Space_Separator"
static int is_whitespace(const char *bytes)
{
    int c, size;

    // Character is ASCII
    if ((c = bytes[0]) < 0x80)
        return c == ' ' || c == '\t' || c == '\v' || c == '\f';

    if ((c = utf8_to_codepoint(bytes, &size)) == -1)
        return 0;

    return c == 0xffef || u_isUWhiteSpace(c);
}

// https://tc39.es/ecma262/#prod-LineTerminator
//
//   LineTerminator ::
//     <LF>             ; Line feed
//     <CR>             ; Carriage return
//     <LS>             ; Line separator
//     <PS>             ; Paragraph separator
static int is_line_terminator(const char *bytes)
{
    int c, size;

    // Character is ASCII
    if ((c = bytes[0]) < 0x80)
        return c == '\n' || c == '\r';

    if ((c = utf8_to_codepoint(bytes, &size)) == -1)
        return 0;

    switch (c) {
    case 0x2028:
    case 0x2029:
        return size;

    default:
        return 0;
    }
}

__attribute__((const))
static bool ishex(const char c)
{
    switch (c) {
    case '0' ... '9':
    case 'A' ... 'F':
    case 'a' ... 'f':
        return true;
    }

    return false;
}

__attribute__((const))
static int hextoi(const char c)
{
    switch (c) {
    case '0' ... '9':
        return c - '0';
    case 'A' ... 'F':
        return c - 'A' + 10;
    case 'a' ... 'f':
        return c - 'a' + 10;
    }

    return -1;
}

// https://tc39.es/ecma262/#prod-UnicodeEscapeSequence
//
//   UnicodeEscapeSequence ::
//     u Hex4Digits
//     u{ CodePoint }
//
//   Hex4Digits ::
//     HexDigit HexDigit HexDigit HexDigit
//
//   HexDigit :: one of
//     0 1 2 3 4 5 6 7 8 9 a b c d e f A B C D E F
//
//   CodePoint ::
//     HexDigits            ; but only if the MV of HexDigits <= 0x10FFFF
//
//   HexDigits ::
//     HexDigit
//     HexDigits HexDigit
static int maybe_escape_sequence(const char *bytes, int *cp)
{
    static const int powers[] = {1048576, 65536, 4096, 256, 16, 1};
    int i;

    if (bytes[0] != 'u')
        return 0;

    *cp = 0;
    if (bytes[1] != '{')
        goto hex4digits;

    // Count the hex digits in the sequence
    for (i = 0; i < 6; i++) {
        if (!ishex(bytes[2 + i]))
            break;
    }

    // Atleast one hex digit is required, and the sequence must end with '}'
    if (i == 0 || bytes[2 + i] != '}')
        return 0;

    // Calculate the value of the code point
    for (int j = 0; j < i; j++)
        *cp += hextoi(bytes[2 + j]) * powers[6 - i + j];

    return i + 3; // + 3 for "u{}"

hex4digits:
    for (i = 0; i < 4; i++) {
        if (!ishex(bytes[1 + i]))
            return 0;
        *cp += hextoi(bytes[1 + i]) * powers[2 + i];
    }

    return 5;
}

// https://tc39.es/ecma262/#prod-IdentifierStart
//
//   IdentifierStart ::
//     IdentifierStartChar
//     \ UnicodeEscapeSequence
//
//   IdentifierStartChar ::
//     UnicodeIDStart       ; Any Unicode code point with the Unicode property "ID_Start"
//     $
//     _
static int is_identifier_start(const char *bytes, int *cp)
{
    int t;

    if ((*cp = bytes[0]) == '\\') {
        if ((t = maybe_escape_sequence(bytes + 1, cp)) == 0)
            return 0;

        t++; // Skip '\\'
        goto end;
    }

    if (*cp >= 0x80)
        goto not_ascii;

    switch (*cp) {
    case '$':
    case 'A' ... 'Z':
    case '_':
    case 'a' ... 'z':
        return 1;
    }

not_ascii:
    if ((*cp = utf8_to_codepoint(bytes, &t)) == -1)
        return 0;

end:
    if (!u_isIDStart(*cp))
        return 0;

    return t;
}

// https://tc39.es/ecma262/#prod-IdentifierPart
//
//   IdentifierPart ::
//     IdentifierPathChar
//     \ UnicodeEscapeSequence
//
//   IdentifierPathChar ::
//     UnicodeIDContinue    ; Any Unicode code point with the Unicode property "ID_Continue"
//     $
static int is_identifier_part(const char *bytes, int *cp)
{
    int t;

    if ((*cp = bytes[0]) == '\\') {
        if ((t = maybe_escape_sequence(bytes + 1, cp)) == 0)
            return 0;

        t++; // Skip '\\'
        goto end;
    }

    if (*cp >= 0x80)
        goto not_ascii;

    switch (*cp) {
    case '$':
    case '0' ... '9':
    case 'A' ... 'Z':
    case '_':
    case 'a' ... 'z':
        return 1;
    }

not_ascii:
    if ((*cp = utf8_to_codepoint(bytes, &t)) == -1)
        return 0;

end:
    if (!u_isIDPart(*cp))
        return 0;

    return t;
}

static int maybe_keyword(const char *s)
{
#define C(str, tok) \
        if (strcmp(s, str) == 0) return tok;

    switch (*s) {
    case 'a':
        C("await", TOKEN_AWAIT);
        C("async", TOKEN_ASYNC);
        break;

    case 'b':
        C("break", TOKEN_BREAK);
        break;

    case 'c':
        C("case", TOKEN_CASE);
        C("catch", TOKEN_CATCH);
        C("class", TOKEN_CLASS);
        C("const", TOKEN_CONST);
        C("continue", TOKEN_CONTINUE);
        break;

    case 'd':
        C("debugger", TOKEN_DEBUGGER);
        C("default", TOKEN_DEFAULT);
        C("delete", TOKEN_DELETE);
        C("do", TOKEN_DO);
        break;

    case 'e':
        C("else", TOKEN_ELSE);
        C("enum", TOKEN_ENUM);
        C("export", TOKEN_EXPORT);
        C("extends", TOKEN_EXTENDS);
        break;

    case 'f':
        C("false", TOKEN_FALSE);
        C("finally", TOKEN_FINALLY);
        C("for", TOKEN_FOR);
        C("function", TOKEN_FUNCTION);
        break;

    case 'i':
        C("if", TOKEN_IF);
        C("implements", TOKEN_IMPLEMENTS);
        C("import", TOKEN_IMPORT);
        C("in", TOKEN_IN);
        C("instanceof", TOKEN_INSTANCEOF);
        C("interface", TOKEN_INTERFACE);
        break;

    case 'l':
        C("let", TOKEN_LET);
        break;

    case 'n':
        C("new", TOKEN_NEW);
        C("null", TOKEN_NULL);
        break;

    case 'p':
        C("package", TOKEN_PACKAGE);
        C("private", TOKEN_PRIVATE);
        C("protected", TOKEN_PROTECTED);
        C("public", TOKEN_PUBLIC);
        break;

    case 'r':
        C("return", TOKEN_RETURN);
        break;

    case 's':
        C("static", TOKEN_STATIC);
        C("super", TOKEN_SUPER);
        C("switch", TOKEN_SWITCH);
        break;

    case 't':
        C("this", TOKEN_THIS);
        C("throw", TOKEN_THROW);
        C("true", TOKEN_TRUE);
        C("try", TOKEN_TRY);
        C("typeof", TOKEN_TYPEOF);
        break;

    case 'u':
        C("undefined", TOKEN_UNDEFINED);
        break;

    case 'v':
        C("var", TOKEN_VAR);
        C("void", TOKEN_VOID);
        break;

    case 'w':
        C("while", TOKEN_WHILE);
        C("with", TOKEN_WITH);
        break;

    case 'y':
        C("yield", TOKEN_YIELD);
        break;
    }

#undef C

    return -1;
}

// https://tc39.es/ecma262/#sec-identifier-names
//
//   PrivateIdentifier ::
//     # IdentifierName
//
//   IdentifierName ::
//     IdentifierStart
//     IdentifierName IdentifierPart
static int parse_identifier(struct lexer *lx)
{
    struct string buf = {0};
    int cp, t, type;

    if (peek(lx) == '#') {
        string_append(&buf, '#');
        read(lx);
    }

    if ((t = is_identifier_start(lexer_position(lx), &cp)) == 0) {
        token_set(&lx->current, TOKEN_INVALID);
        return -1;
    }

    do {
        string_append_codepoint(&buf, cp);
        lx->index += t;
    } while ((t = is_identifier_part(lexer_position(lx), &cp)) != 0);

    if ((type = maybe_keyword(string_ref(&buf))) != -1) {
        token_set(&lx->current, type);
        string_free(&buf);
        return 0;
    }

    string_shrink(&buf);
    token_set_identifier(&lx->current, buf.data, buf.length);
    return 0;
}

static void skip_whitespace(struct lexer *lx)
{
    int size;

    while ((size = is_whitespace(lexer_position(lx))))
        lx->index += size;
}

void lexer_init(struct lexer *lx, const char *bytes, const size_t size)
{
    if (!lx || !bytes)
        return;

    lx->buffer = bytes;
    lx->index = 0;
    lx->size = size;

    token_set(&lx->current, TOKEN_INVALID);
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

    default:
        lx->index--;
        parse_identifier(lx);
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
