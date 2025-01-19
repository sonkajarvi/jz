#include <string.h>

#include <jz/lexer.h>
#include <jz/token.h>
#include <test.h>

// Asserts the next token is @expected, and EOF the one after that
#define ASSERT_NEXT(lx, string, expected) {         \
    lexer_init(&lx, string, strlen(string));        \
    test_assert(lexer_next(&lx)->type == expected); \
    test_assert(lexer_next(&lx)->type == TOKEN_EOF); }

// Asserts the nexst token is an identifier with @expected as value,
// and EOF the one after that
#define ASSERT_IDENTIFIER(lx, _string, expected) {  \
    lexer_init(&lx, _string, strlen(_string));      \
    struct token *tk = lexer_next(&lx);             \
    test_assert(tk->type == TOKEN_IDENTIFIER);      \
    test_assert(strcmp(tk->string, expected) == 0); \
    test_assert(lexer_next(&lx)->type == TOKEN_EOF); }

test_case(lexer_next_punctuator)
{
    struct lexer lx;

    ASSERT_NEXT(lx, "&", TOKEN_AMPERSAND);
    ASSERT_NEXT(lx, "&&", TOKEN_AMPERSAND_AMPERSAND);
    ASSERT_NEXT(lx, "&&=", TOKEN_AMPERSAND_AMPERSAND_EQUALS);
    ASSERT_NEXT(lx, "&=", TOKEN_AMPERSAND_EQUALS);
    ASSERT_NEXT(lx, "*", TOKEN_ASTERISK);
    ASSERT_NEXT(lx, "**", TOKEN_ASTERISK_ASTERISK);
    ASSERT_NEXT(lx, "**=", TOKEN_ASTERISK_ASTERISK_EQUALS);
    ASSERT_NEXT(lx, "*=", TOKEN_ASTERISK_EQUALS);
    ASSERT_NEXT(lx, "=>", TOKEN_ARROW);
    ASSERT_NEXT(lx, "{", TOKEN_BRACE_LEFT);
    ASSERT_NEXT(lx, "}", TOKEN_BRACE_RIGHT);
    ASSERT_NEXT(lx, "^", TOKEN_CARET);
    ASSERT_NEXT(lx, "^=", TOKEN_CARET_EQUALS);
    ASSERT_NEXT(lx, ":", TOKEN_COLON);
    ASSERT_NEXT(lx, ",", TOKEN_COMMA);
    ASSERT_NEXT(lx, ".", TOKEN_DOT);
    ASSERT_NEXT(lx, "...", TOKEN_DOT_DOT_DOT);
    ASSERT_NEXT(lx, "=", TOKEN_EQUALS);
    ASSERT_NEXT(lx, "==", TOKEN_EQUALS_EQUALS);
    ASSERT_NEXT(lx, "===", TOKEN_EQUALS_EQUALS_EQUALS);
    ASSERT_NEXT(lx, "!", TOKEN_EXCLAMATION);
    ASSERT_NEXT(lx, "!=", TOKEN_EXCLAMATION_EQUALS);
    ASSERT_NEXT(lx, "!==", TOKEN_EXCLAMATION_EQUALS_EQUALS);
    ASSERT_NEXT(lx, ">", TOKEN_GREATER);
    ASSERT_NEXT(lx, ">=", TOKEN_GREATER_EQUALS);
    ASSERT_NEXT(lx, ">>", TOKEN_GREATER_GREATER);
    ASSERT_NEXT(lx, ">>=", TOKEN_GREATER_GREATER_EQUALS);
    ASSERT_NEXT(lx, ">>>", TOKEN_GREATER_GREATER_GREATER);
    ASSERT_NEXT(lx, ">>>=", TOKEN_GREATER_GREATER_GREATER_EQUALS);
    ASSERT_NEXT(lx, "<", TOKEN_LESS);
    ASSERT_NEXT(lx, "<=", TOKEN_LESS_EQUALS);
    ASSERT_NEXT(lx, "<<", TOKEN_LESS_LESS);
    ASSERT_NEXT(lx, "<<=", TOKEN_LESS_LESS_EQUALS);
    ASSERT_NEXT(lx, "-", TOKEN_MINUS);
    ASSERT_NEXT(lx, "-=", TOKEN_MINUS_EQUALS);
    ASSERT_NEXT(lx, "--", TOKEN_MINUS_MINUS);
    ASSERT_NEXT(lx, "(", TOKEN_PAREN_LEFT);
    ASSERT_NEXT(lx, ")", TOKEN_PAREN_RIGHT);
    ASSERT_NEXT(lx, "%", TOKEN_PERCENT);
    ASSERT_NEXT(lx, "%=", TOKEN_PERCENT_EQUALS);
    ASSERT_NEXT(lx, "+", TOKEN_PLUS);
    ASSERT_NEXT(lx, "+=", TOKEN_PLUS_EQUALS);
    ASSERT_NEXT(lx, "++", TOKEN_PLUS_PLUS);
    ASSERT_NEXT(lx, "?", TOKEN_QUESTION);
    ASSERT_NEXT(lx, "?.", TOKEN_QUESTION_DOT);
    ASSERT_NEXT(lx, "??", TOKEN_QUESTION_QUESTION);
    ASSERT_NEXT(lx, "??=", TOKEN_QUESTION_QUESTION_EQUALS);
    ASSERT_NEXT(lx, ";", TOKEN_SEMICOLON);
    ASSERT_NEXT(lx, "/", TOKEN_SLASH);
    ASSERT_NEXT(lx, "/=", TOKEN_SLASH_EQUALS);
    ASSERT_NEXT(lx, "[", TOKEN_SQUARE_LEFT);
    ASSERT_NEXT(lx, "]", TOKEN_SQUARE_RIGHT);
    ASSERT_NEXT(lx, "~",  TOKEN_TILDE);
    ASSERT_NEXT(lx, "|", TOKEN_VERTICAL);
    ASSERT_NEXT(lx, "|=", TOKEN_VERTICAL_EQUALS);
    ASSERT_NEXT(lx, "||", TOKEN_VERTICAL_VERTICAL);
    ASSERT_NEXT(lx, "||=", TOKEN_VERTICAL_VERTICAL_EQUALS);

    test_success();
}

test_case(lexer_next_keyword)
{
    struct lexer lx;

    ASSERT_NEXT(lx, "async", TOKEN_ASYNC);
    ASSERT_NEXT(lx, "await", TOKEN_AWAIT);
    ASSERT_NEXT(lx, "break", TOKEN_BREAK);
    ASSERT_NEXT(lx, "case", TOKEN_CASE);
    ASSERT_NEXT(lx, "catch", TOKEN_CATCH);
    ASSERT_NEXT(lx, "class", TOKEN_CLASS);
    ASSERT_NEXT(lx, "const", TOKEN_CONST);
    ASSERT_NEXT(lx, "continue", TOKEN_CONTINUE);
    ASSERT_NEXT(lx, "debugger", TOKEN_DEBUGGER);
    ASSERT_NEXT(lx, "default", TOKEN_DEFAULT);
    ASSERT_NEXT(lx, "delete", TOKEN_DELETE);
    ASSERT_NEXT(lx, "do", TOKEN_DO);
    ASSERT_NEXT(lx, "else", TOKEN_ELSE);
    ASSERT_NEXT(lx, "enum", TOKEN_ENUM);
    ASSERT_NEXT(lx, "export", TOKEN_EXPORT);
    ASSERT_NEXT(lx, "extends", TOKEN_EXTENDS);
    ASSERT_NEXT(lx, "false", TOKEN_FALSE);
    ASSERT_NEXT(lx, "finally", TOKEN_FINALLY);
    ASSERT_NEXT(lx, "for", TOKEN_FOR);
    ASSERT_NEXT(lx, "function", TOKEN_FUNCTION);
    ASSERT_NEXT(lx, "if", TOKEN_IF);
    ASSERT_NEXT(lx, "implements", TOKEN_IMPLEMENTS);
    ASSERT_NEXT(lx, "import", TOKEN_IMPORT);
    ASSERT_NEXT(lx, "in", TOKEN_IN);
    ASSERT_NEXT(lx, "instanceof", TOKEN_INSTANCEOF);
    ASSERT_NEXT(lx, "interface", TOKEN_INTERFACE);
    ASSERT_NEXT(lx, "let", TOKEN_LET);
    ASSERT_NEXT(lx, "new", TOKEN_NEW);
    ASSERT_NEXT(lx, "null", TOKEN_NULL);
    ASSERT_NEXT(lx, "package", TOKEN_PACKAGE);
    ASSERT_NEXT(lx, "private", TOKEN_PRIVATE);
    ASSERT_NEXT(lx, "protected", TOKEN_PROTECTED);
    ASSERT_NEXT(lx, "public", TOKEN_PUBLIC);
    ASSERT_NEXT(lx, "return", TOKEN_RETURN);
    ASSERT_NEXT(lx, "static", TOKEN_STATIC);
    ASSERT_NEXT(lx, "super", TOKEN_SUPER);
    ASSERT_NEXT(lx, "switch", TOKEN_SWITCH);
    ASSERT_NEXT(lx, "this", TOKEN_THIS);
    ASSERT_NEXT(lx, "throw", TOKEN_THROW);
    ASSERT_NEXT(lx, "true", TOKEN_TRUE);
    ASSERT_NEXT(lx, "try", TOKEN_TRY);
    ASSERT_NEXT(lx, "typeof", TOKEN_TYPEOF);
    ASSERT_NEXT(lx, "undefined", TOKEN_UNDEFINED);
    ASSERT_NEXT(lx, "var", TOKEN_VAR);
    ASSERT_NEXT(lx, "void", TOKEN_VOID);
    ASSERT_NEXT(lx, "with", TOKEN_WITH);
    ASSERT_NEXT(lx, "while", TOKEN_WHILE);
    ASSERT_NEXT(lx, "yield", TOKEN_YIELD);

    test_success();
}

test_case(lexer_next_identifier)
{
    struct lexer lx;

    // Starts with keyword
    ASSERT_IDENTIFIER(lx, "constt", "constt");

    test_success();
}

// https://www.cogsci.ed.ac.uk/~richard/utf-8.cgi
test_case(utf8_to_codepoint)
{
    int size;

    test_assert(utf8_to_codepoint("\x61", &size) == 0x61); // a
    test_assert(size == 1);

    test_assert(utf8_to_codepoint("\xc2\xa2", &size) == 0xa2); // ¢
    test_assert(size == 2);

    test_assert(utf8_to_codepoint("\xe2\x82\xac", &size) == 0x20ac); // €
    test_assert(size == 3);

    test_assert(utf8_to_codepoint("\xf0\x9f\x9e\x85", &size) == 0x1f785); // 🞅
    test_assert(size == 4);

    // Lone continuation byte
    test_assert(utf8_to_codepoint("\x80", NULL) == -1);

    // Overlong encodings
    test_assert(utf8_to_codepoint("\xc0\xbf", NULL) == -1);
    test_assert(utf8_to_codepoint("\xe0\x80\xbf", NULL) == -1);
    test_assert(utf8_to_codepoint("\xf0\x80\x80\xbf", NULL) == -1);

    // Surrogates
    test_assert(utf8_to_codepoint("\xed\xa0\x80", NULL) == -1);
    test_assert(utf8_to_codepoint("\xed\xbf\xbf", NULL) == -1);

    // Invalid bytes
    test_assert(utf8_to_codepoint("\xfe", NULL) == -1);
    test_assert(utf8_to_codepoint("\xff", NULL) == -1);

    test_success();
}
