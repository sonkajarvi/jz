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

void token_print(struct token *tok)
{
    const char *name = token_stringify(tok);

    switch (tok->type) {
    case TOKEN_NUMERIC_LITERAL:
        printf("%s %f\n", name, tok->number);
        break;

    case TOKEN_STRING_LITERAL:
        printf("%s \"%s\"\n", name, tok->string);
        break;

    case TOKEN_IDENTIFIER:
        printf("%s %s\n", name, tok->string);
        break;

    default:
        printf("%s\n", name);
    }
}
