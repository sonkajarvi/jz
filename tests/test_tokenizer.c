#include <string.h>

#include <str.h>
#include <token.h>
#include <tokenizer.h>
#include <utf8_reader.h>
#include <vec.h>

#include "test.h"

#define ASSERT_TOKEN(str, expected) do {       \
    struct utf8_reader rdr = UTF8_READER_INIT; \
    struct context ctx;                        \
    struct token tok;                          \
    ctx.reader = rdr.reader;                   \
    ctx.bytes = (void *)str;                   \
    ctx.size = strlen(str);                    \
    ctx.index = 0;                             \
    ASSERT_EQ(next_token(&ctx, &tok), 0);      \
    ASSERT_EQ(tok.type, expected); } while (0)

#define ASSERT_IDENTIFIER(src, expected) do {  \
    struct utf8_reader rdr = UTF8_READER_INIT; \
    struct context ctx;                        \
    struct token tok;                          \
    struct string str;                         \
    ctx.reader = rdr.reader;                   \
    ctx.bytes = (void *)src;                   \
    ctx.size = strlen(src);                    \
    ctx.index = 0;                             \
    ASSERT_EQ(next_token(&ctx, &tok), 0);      \
    ASSERT_EQ(tok.type, TOKEN_IDENTIFIER);     \
    S(&str, expected);                         \
    ASSERT_EQ(string_eq(&tok.str, &str), 1);   \
    string_free(&tok.str);                     \
    string_free(&str); } while (0)

#define ASSERT_STRING(src, expected) do {      \
    struct utf8_reader rdr = UTF8_READER_INIT; \
    struct context ctx;                        \
    struct token tok;                          \
    struct string str;                         \
    ctx.reader = rdr.reader;                   \
    ctx.bytes = (void *)src;                   \
    ctx.size = strlen(src);                    \
    ctx.index = 0;                             \
    ASSERT_EQ(next_token(&ctx, &tok), 0);      \
    ASSERT_EQ(tok.type, TOKEN_STRING_LITERAL); \
    S(&str, expected);                         \
    ASSERT_EQ(string_eq(&tok.str, &str), 1);   \
    string_free(&tok.str);                     \
    string_free(&str); } while (0)

TEST(tokenizer_next_token_eof)
{
    ASSERT_TOKEN("", TOKEN_EOF);
}

TEST(tokenizer_next_token_punctuator)
{
    ASSERT_TOKEN("&", TOKEN_AMPERSAND);
    ASSERT_TOKEN("&&", TOKEN_AMPERSAND_AMPERSAND);
    ASSERT_TOKEN("&&=", TOKEN_AMPERSAND_AMPERSAND_EQUALS);
    ASSERT_TOKEN("&=", TOKEN_AMPERSAND_EQUALS);
    ASSERT_TOKEN("*", TOKEN_ASTERISK);
    ASSERT_TOKEN("**", TOKEN_ASTERISK_ASTERISK);
    ASSERT_TOKEN("**=", TOKEN_ASTERISK_ASTERISK_EQUALS);
    ASSERT_TOKEN("*=", TOKEN_ASTERISK_EQUALS);
    ASSERT_TOKEN("=>", TOKEN_ARROW);
    ASSERT_TOKEN("{", TOKEN_BRACE_LEFT);
    ASSERT_TOKEN("}", TOKEN_BRACE_RIGHT);
    ASSERT_TOKEN("^", TOKEN_CARET);
    ASSERT_TOKEN("^=", TOKEN_CARET_EQUALS);
    ASSERT_TOKEN(":", TOKEN_COLON);
    ASSERT_TOKEN(",", TOKEN_COMMA);
    ASSERT_TOKEN(".", TOKEN_DOT);
    ASSERT_TOKEN("...", TOKEN_DOT_DOT_DOT);
    ASSERT_TOKEN("=", TOKEN_EQUALS);
    ASSERT_TOKEN("==", TOKEN_EQUALS_EQUALS);
    ASSERT_TOKEN("===", TOKEN_EQUALS_EQUALS_EQUALS);
    ASSERT_TOKEN("!", TOKEN_EXCLAMATION);
    ASSERT_TOKEN("!=", TOKEN_EXCLAMATION_EQUALS);
    ASSERT_TOKEN("!==", TOKEN_EXCLAMATION_EQUALS_EQUALS);
    ASSERT_TOKEN(">", TOKEN_GREATER);
    ASSERT_TOKEN(">=", TOKEN_GREATER_EQUALS);
    ASSERT_TOKEN(">>", TOKEN_GREATER_GREATER);
    ASSERT_TOKEN(">>=", TOKEN_GREATER_GREATER_EQUALS);
    ASSERT_TOKEN(">>>", TOKEN_GREATER_GREATER_GREATER);
    ASSERT_TOKEN(">>>=", TOKEN_GREATER_GREATER_GREATER_EQUALS);
    ASSERT_TOKEN("<", TOKEN_LESS);
    ASSERT_TOKEN("<=", TOKEN_LESS_EQUALS);
    ASSERT_TOKEN("<<", TOKEN_LESS_LESS);
    ASSERT_TOKEN("<<=", TOKEN_LESS_LESS_EQUALS);
    ASSERT_TOKEN("-", TOKEN_MINUS);
    ASSERT_TOKEN("-=", TOKEN_MINUS_EQUALS);
    ASSERT_TOKEN("--", TOKEN_MINUS_MINUS);
    ASSERT_TOKEN("(", TOKEN_PAREN_LEFT);
    ASSERT_TOKEN(")", TOKEN_PAREN_RIGHT);
    ASSERT_TOKEN("%", TOKEN_PERCENT);
    ASSERT_TOKEN("%=", TOKEN_PERCENT_EQUALS);
    ASSERT_TOKEN("+", TOKEN_PLUS);
    ASSERT_TOKEN("+=", TOKEN_PLUS_EQUALS);
    ASSERT_TOKEN("++", TOKEN_PLUS_PLUS);
    ASSERT_TOKEN("?", TOKEN_QUESTION);
    ASSERT_TOKEN("?.", TOKEN_QUESTION_DOT);
    ASSERT_TOKEN("??", TOKEN_QUESTION_QUESTION);
    ASSERT_TOKEN("??=", TOKEN_QUESTION_QUESTION_EQUALS);
    ASSERT_TOKEN(";", TOKEN_SEMICOLON);
    ASSERT_TOKEN("/", TOKEN_SLASH);
    ASSERT_TOKEN("/=", TOKEN_SLASH_EQUALS);
    ASSERT_TOKEN("[", TOKEN_SQUARE_LEFT);
    ASSERT_TOKEN("]", TOKEN_SQUARE_RIGHT);
    ASSERT_TOKEN("~",  TOKEN_TILDE);
    ASSERT_TOKEN("|", TOKEN_VERTICAL);
    ASSERT_TOKEN("|=", TOKEN_VERTICAL_EQUALS);
    ASSERT_TOKEN("||", TOKEN_VERTICAL_VERTICAL);
    ASSERT_TOKEN("||=", TOKEN_VERTICAL_VERTICAL_EQUALS);
}

// TEST(tokenizer_next_token_keyword)
// {
//     ASSERT_TOKEN("async", TOKEN_ASYNC);
//     ASSERT_TOKEN("await", TOKEN_AWAIT);
//     ASSERT_TOKEN("break", TOKEN_BREAK);
//     ASSERT_TOKEN("case", TOKEN_CASE);
//     ASSERT_TOKEN("catch", TOKEN_CATCH);
//     ASSERT_TOKEN("class", TOKEN_CLASS);
//     ASSERT_TOKEN("const", TOKEN_CONST);
//     ASSERT_TOKEN("continue", TOKEN_CONTINUE);
//     ASSERT_TOKEN("debugger", TOKEN_DEBUGGER);
//     ASSERT_TOKEN("default", TOKEN_DEFAULT);
//     ASSERT_TOKEN("delete", TOKEN_DELETE);
//     ASSERT_TOKEN("do", TOKEN_DO);
//     ASSERT_TOKEN("else", TOKEN_ELSE);
//     ASSERT_TOKEN("enum", TOKEN_ENUM);
//     ASSERT_TOKEN("export", TOKEN_EXPORT);
//     ASSERT_TOKEN("extends", TOKEN_EXTENDS);
//     ASSERT_TOKEN("false", TOKEN_FALSE);
//     ASSERT_TOKEN("finally", TOKEN_FINALLY);
//     ASSERT_TOKEN("for", TOKEN_FOR);
//     ASSERT_TOKEN("function", TOKEN_FUNCTION);
//     ASSERT_TOKEN("if", TOKEN_IF);
//     // ASSERT_TOKEN("implements", TOKEN_IMPLEMENTS);
//     ASSERT_TOKEN("import", TOKEN_IMPORT);
//     ASSERT_TOKEN("in", TOKEN_IN);
//     ASSERT_TOKEN("instanceof", TOKEN_INSTANCEOF);
//     // ASSERT_TOKEN("interface", TOKEN_INTERFACE);
//     ASSERT_TOKEN("let", TOKEN_LET);
//     ASSERT_TOKEN("new", TOKEN_NEW);
//     ASSERT_TOKEN("null", TOKEN_NULL);
//     // ASSERT_TOKEN("package", TOKEN_PACKAGE);
//     // ASSERT_TOKEN("private", TOKEN_PRIVATE);
//     // ASSERT_TOKEN("protected", TOKEN_PROTECTED);
//     // ASSERT_TOKEN("public", TOKEN_PUBLIC);
//     ASSERT_TOKEN("return", TOKEN_RETURN);
//     ASSERT_TOKEN("static", TOKEN_STATIC);
//     ASSERT_TOKEN("super", TOKEN_SUPER);
//     ASSERT_TOKEN("switch", TOKEN_SWITCH);
//     ASSERT_TOKEN("this", TOKEN_THIS);
//     ASSERT_TOKEN("throw", TOKEN_THROW);
//     ASSERT_TOKEN("true", TOKEN_TRUE);
//     ASSERT_TOKEN("try", TOKEN_TRY);
//     ASSERT_TOKEN("typeof", TOKEN_TYPEOF);
//     ASSERT_TOKEN("undefined", TOKEN_UNDEFINED);
//     ASSERT_TOKEN("var", TOKEN_VAR);
//     ASSERT_TOKEN("void", TOKEN_VOID);
//     ASSERT_TOKEN("with", TOKEN_WITH);
//     ASSERT_TOKEN("while", TOKEN_WHILE);
//     ASSERT_TOKEN("yield", TOKEN_YIELD);
// }

TEST(tokenizer_next_token_identifier)
{
    ASSERT_IDENTIFIER("id", "id");
    ASSERT_IDENTIFIER("#id", "#id");

    ASSERT_IDENTIFIER("\\u0061a", "aa");
    ASSERT_IDENTIFIER("\\u{61}a", "aa");
    ASSERT_IDENTIFIER("\\u{0000000061}a", "aa");

    ASSERT_IDENTIFIER("a\\u0061", "aa");
    ASSERT_IDENTIFIER("a\\u{61}", "aa");
}

// https://www.cogsci.ed.ac.uk/~richard/utf-8.cgi
TEST(tokenizer_next_token_string)
{
    ASSERT_STRING("\"\"", "");
    ASSERT_STRING("\'\'", "");

    ASSERT_STRING("\"'\"", "'");
    ASSERT_STRING("'\"'", "\"");

    ASSERT_STRING("\"\x61\"", "a");
    ASSERT_STRING("\"\xc2\xa2\"", "¢");
    ASSERT_STRING("\"\xe2\x82\xac\"", "€");
    ASSERT_STRING("\"\xf0\x9f\x9e\x85\"", "🞅");

    // Line terminators
    ASSERT_STRING("\"a\\\na\"", "aa");
    ASSERT_STRING("\"a\\\ra\"", "aa");
    ASSERT_STRING("\"a\\\xe2\x80\xa8g\"", "ag"); // <LS>
    ASSERT_STRING("\"a\\\xe2\x80\xa9g\"", "ag"); // <PS>
    ASSERT_STRING("\"a\\\r\na\"", "aa");

    // Character escape sequences
    ASSERT_STRING("\"a\\'a\"", "a'a");
    ASSERT_STRING("\"a\\\"a\"", "a\"a");
    ASSERT_STRING("\"a\\\\a\"", "a\\a");
    ASSERT_STRING("\"a\\ba\"", "a\ba");
    ASSERT_STRING("\"a\\fa\"", "a\fa");
    ASSERT_STRING("\"a\\na\"", "a\na");
    ASSERT_STRING("\"a\\ra\"", "a\ra");
    ASSERT_STRING("\"a\\ta\"", "a\ta");
    ASSERT_STRING("\"a\\va\"", "a\va");

    // Legacy octal escape sequence
    ASSERT_STRING("\"\\7\"", "\x07");
    ASSERT_STRING("\"\\07\"", "\x07");
    ASSERT_STRING("\"\\077\"", "?");
    ASSERT_STRING("\"\\777\"", "?7");

    // Non-octal decimal escape sequence
    ASSERT_STRING("\"\\8\"", "8");
    ASSERT_STRING("\"\\9\"", "9");

    // Hex escape sequence
    ASSERT_STRING("\"\\x3f\"", "?");
    ASSERT_STRING("\"\\x3F\"", "?");

    // Unicode escape sequence
    ASSERT_STRING("\"a\\u0062c\"", "abc");
    ASSERT_STRING("\"a\\u{62}c\"", "abc");
    ASSERT_STRING("\"a\\u{0000000062}c\"", "abc");
}
