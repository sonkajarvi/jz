#include <string.h>

#include <jz/lexer.h>
#include <jz/token.h>
#include <test.h>

#define LEXER_TRY(lx, string, expected) { \
    lexer_init(&lx, string, strlen(string)); \
    struct token *__token = lexer_next(&lx); \
    /* printf("%d \033[90m(%s)\033[0m\n", __token->type, token_stringify(__token)); */ \
    test_assert(__token->type == expected); \
    test_assert(lexer_next(&lx)->type == TOKEN_EOF); }

test_case(lexer_parse_fixed_sized)
{
    struct lexer lx;

    LEXER_TRY(lx, "&", TOKEN_AMPERSAND);
    LEXER_TRY(lx, "&&", TOKEN_AMPERSAND_AMPERSAND);
    LEXER_TRY(lx, "&&=", TOKEN_AMPERSAND_AMPERSAND_EQUALS);
    LEXER_TRY(lx, "&=", TOKEN_AMPERSAND_EQUALS);
    LEXER_TRY(lx, "*", TOKEN_ASTERISK);
    LEXER_TRY(lx, "**", TOKEN_ASTERISK_ASTERISK);
    LEXER_TRY(lx, "**=", TOKEN_ASTERISK_ASTERISK_EQUALS);
    LEXER_TRY(lx, "*=", TOKEN_ASTERISK_EQUALS);
    // LEXER_TRY(lx, "async", TOKEN_ASYNC);
    LEXER_TRY(lx, "=>", TOKEN_ARROW);
    // LEXER_TRY(lx, "await", TOKEN_AWAIT);

    LEXER_TRY(lx, "{", TOKEN_BRACE_LEFT);
    LEXER_TRY(lx, "}", TOKEN_BRACE_RIGHT);
    // LEXER_TRY(lx, "break", TOKEN_BREAK);

    LEXER_TRY(lx, "^", TOKEN_CARET);
    LEXER_TRY(lx, "^=", TOKEN_CARET_EQUALS);
    // LEXER_TRY(lx, "case", TOKEN_CASE);
    // LEXER_TRY(lx, "catch", TOKEN_CATCH);
    // LEXER_TRY(lx, "class", TOKEN_CLASS);
    LEXER_TRY(lx, ":", TOKEN_COLON);
    LEXER_TRY(lx, ",", TOKEN_COMMA);
    // LEXER_TRY(lx, "const", TOKEN_CONST);
    // LEXER_TRY(lx, "continue", TOKEN_CONTINUE);

    // LEXER_TRY(lx, "debugger", TOKEN_DEBUGGER);
    // LEXER_TRY(lx, "default", TOKEN_DEFAULT);
    // LEXER_TRY(lx, "delete", TOKEN_DELETE);
    // LEXER_TRY(lx, "do", TOKEN_DO);
    LEXER_TRY(lx, ".", TOKEN_DOT);
    LEXER_TRY(lx, "...", TOKEN_DOT_DOT_DOT);

    // LEXER_TRY(lx, "else", TOKEN_ELSE);
    // LEXER_TRY(lx, "enum", TOKEN_ENUM);
    LEXER_TRY(lx, "=", TOKEN_EQUALS);
    LEXER_TRY(lx, "==", TOKEN_EQUALS_EQUALS);
    LEXER_TRY(lx, "===", TOKEN_EQUALS_EQUALS_EQUALS);
    LEXER_TRY(lx, "!", TOKEN_EXCLAMATION);
    LEXER_TRY(lx, "!=", TOKEN_EXCLAMATION_EQUALS);
    LEXER_TRY(lx, "!==", TOKEN_EXCLAMATION_EQUALS_EQUALS);
    // LEXER_TRY(lx, "export", TOKEN_EXPORT);
    // LEXER_TRY(lx, "extends", TOKEN_EXTENDS);

    // LEXER_TRY(lx, "false", TOKEN_FALSE);
    // LEXER_TRY(lx, "finally", TOKEN_FINALLY);
    // LEXER_TRY(lx, "for", TOKEN_FOR);
    // LEXER_TRY(lx, "function", TOKEN_FUNCTION);

    LEXER_TRY(lx, ">", TOKEN_GREATER);
    LEXER_TRY(lx, ">=", TOKEN_GREATER_EQUALS);
    LEXER_TRY(lx, ">>", TOKEN_GREATER_GREATER);
    LEXER_TRY(lx, ">>=", TOKEN_GREATER_GREATER_EQUALS);
    LEXER_TRY(lx, ">>>", TOKEN_GREATER_GREATER_GREATER);
    LEXER_TRY(lx, ">>>=", TOKEN_GREATER_GREATER_GREATER_EQUALS);

    // LEXER_TRY(lx, "if", TOKEN_IF);
    // LEXER_TRY(lx, "implements", TOKEN_IMPLEMENTS);
    // LEXER_TRY(lx, "import", TOKEN_IMPORT);
    // LEXER_TRY(lx, "in", TOKEN_IN);
    // LEXER_TRY(lx, "instanceof", TOKEN_INSTANCEOF);
    // LEXER_TRY(lx, "interface", TOKEN_INTERFACE);

    LEXER_TRY(lx, "<", TOKEN_LESS);
    LEXER_TRY(lx, "<=", TOKEN_LESS_EQUALS);
    LEXER_TRY(lx, "<<", TOKEN_LESS_LESS);
    LEXER_TRY(lx, "<<=", TOKEN_LESS_LESS_EQUALS);
    // LEXER_TRY(lx, "let", TOKEN_LET);

    LEXER_TRY(lx, "-", TOKEN_MINUS);
    LEXER_TRY(lx, "-=", TOKEN_MINUS_EQUALS);
    LEXER_TRY(lx, "--", TOKEN_MINUS_MINUS);

    // LEXER_TRY(lx, "new", TOKEN_NEW);
    // LEXER_TRY(lx, "null", TOKEN_NULL);

    // LEXER_TRY(lx, "package", TOKEN_PACKAGE);
    LEXER_TRY(lx, "(", TOKEN_PAREN_LEFT);
    LEXER_TRY(lx, ")", TOKEN_PAREN_RIGHT);
    LEXER_TRY(lx, "%", TOKEN_PERCENT);
    LEXER_TRY(lx, "%=", TOKEN_PERCENT_EQUALS);
    LEXER_TRY(lx, "+", TOKEN_PLUS);
    LEXER_TRY(lx, "+=", TOKEN_PLUS_EQUALS);
    LEXER_TRY(lx, "++", TOKEN_PLUS_PLUS);
    // LEXER_TRY(lx, "private", TOKEN_PRIVATE);
    // LEXER_TRY(lx, "protected", TOKEN_PROTECTED);
    // LEXER_TRY(lx, "public", TOKEN_PUBLIC);

    LEXER_TRY(lx, "?", TOKEN_QUESTION);
    LEXER_TRY(lx, "?.", TOKEN_QUESTION_DOT);
    LEXER_TRY(lx, "??", TOKEN_QUESTION_QUESTION);
    LEXER_TRY(lx, "??=", TOKEN_QUESTION_QUESTION_EQUALS);

    // LEXER_TRY(lx, "return", TOKEN_RETURN);

    LEXER_TRY(lx, ";", TOKEN_SEMICOLON);
    LEXER_TRY(lx, "/", TOKEN_SLASH);
    LEXER_TRY(lx, "/=", TOKEN_SLASH_EQUALS);
    LEXER_TRY(lx, "[", TOKEN_SQUARE_LEFT);
    LEXER_TRY(lx, "]", TOKEN_SQUARE_RIGHT);
    // LEXER_TRY(lx, "static", TOKEN_STATIC);
    // LEXER_TRY(lx, "super", TOKEN_SUPER);
    // LEXER_TRY(lx, "switch", TOKEN_SWITCH);

    LEXER_TRY(lx, "~",  TOKEN_TILDE);
    // LEXER_TRY(lx, "this", TOKEN_THIS);
    // LEXER_TRY(lx, "throw", TOKEN_THROW);
    // LEXER_TRY(lx, "true", TOKEN_TRUE);
    // LEXER_TRY(lx, "try", TOKEN_TRY);
    // LEXER_TRY(lx, "typeof", TOKEN_TYPEOF);

    // LEXER_TRY(lx, "undefined", TOKEN_UNDEFINED);

    // LEXER_TRY(lx, "var", TOKEN_VAR);
    LEXER_TRY(lx, "|", TOKEN_VERTICAL);
    LEXER_TRY(lx, "|=", TOKEN_VERTICAL_EQUALS);
    LEXER_TRY(lx, "||", TOKEN_VERTICAL_VERTICAL);
    LEXER_TRY(lx, "||=", TOKEN_VERTICAL_VERTICAL_EQUALS);
    // LEXER_TRY(lx, "void", TOKEN_VOID);

    // LEXER_TRY(lx, "with", TOKEN_WITH);
    // LEXER_TRY(lx, "while", TOKEN_WHILE);

    // LEXER_TRY(lx, "yield", TOKEN_YIELD);

    test_success();
}
