#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <jz/token.h>

static const char *token_strings[] = {
#define T(x) #x,
    TOKEN_LIST
#undef T
};

const char *token_stringify(struct token *tok)
{
    assert(tok && tok->type < TOKEN_COUNT);
    return token_strings[tok->type];
}

void token_set(struct token *tok, int type)
{
    assert(tok);

    memset(tok, 0, sizeof(*tok));
    tok->type = type;
}

void token_set_numeric_literal(struct token *tok, double number)
{
    token_set(tok, TOKEN_NUMERIC_LITERAL);
    tok->number = number;
}

void token_set_string_literal(struct token *tok, char *s, size_t size)
{
    token_set(tok, TOKEN_STRING_LITERAL);
    tok->string = s;
    tok->size = size;
}

void token_set_identifier(struct token *tok, char *s, size_t size)
{
    token_set(tok, TOKEN_IDENTIFIER);
    tok->string = s;
    tok->size = size;
}

void token_set_boolean(struct token *tok, bool b)
{
    token_set(tok, b ? TOKEN_TRUE : TOKEN_FALSE);
}

void token_print(struct token *tok)
{
    const char *name = token_stringify(tok);

    switch (tok->type) {
    case TOKEN_NUMERIC_LITERAL:
        printf("%s \033[94m%f\033[0m\n", name, tok->number);
        break;

    case TOKEN_STRING_LITERAL:
        printf("%s \033[92m\"%s\"\033[0m\n", name, tok->string);
        break;

    case TOKEN_IDENTIFIER:
        printf("%s \033[90m%s\033[0m\n", name, tok->string);
        break;

    case TOKEN_FALSE:
        printf("%s \033[95mfalse\033[0m\n", name);
        break;

    case TOKEN_TRUE:
        printf("%s \033[95mtrue\033[0m\n", name);
        break;

    default:
        printf("%s\n", name);
    }
}
