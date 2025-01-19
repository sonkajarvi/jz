#include <string.h>

#include <jz/lexer.h>
#include <jz/token.h>
#include <test.h>

// Assert the next token is @expected, and EOF the one after that
#define ASSERT_NEXT(lx, string, expected) {         \
    lexer_init(&lx, string, strlen(string));        \
    test_assert(lexer_next(&lx)->type == expected); \
    test_assert(lexer_next(&lx)->type == TOKEN_EOF); }

test_case(lexer_next_punctuation)
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
