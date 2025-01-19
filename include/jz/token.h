#ifndef TOKEN_H
#define TOKEN_H

#include <stddef.h>
#include <stdint.h>

#define TOKEN_LIST                    \
    T(AMPERSAND)                      /* & */ \
    T(AMPERSAND_AMPERSAND)            /* && */ \
    T(AMPERSAND_AMPERSAND_EQUALS)     /* &&= */ \
    T(AMPERSAND_EQUALS)               /* &= */ \
    T(ASTERISK)                       /* * */ \
    T(ASTERISK_ASTERISK)              /* ** */ \
    T(ASTERISK_ASTERISK_EQUALS)       /* **= */ \
    T(ASTERISK_EQUALS)                /* *= */ \
    T(ASYNC)                          /* async */ \
    T(ARROW)                          /* => */ \
    T(AWAIT)                          /* await */ \
    \
    T(BIGINT_LITERAL)                 \
    T(BRACE_LEFT)                     /* { */ \
    T(BRACE_RIGHT)                    /* } */ \
    T(BREAK)                          /* break */ \
    \
    T(CARET)                          /* ^ */ \
    T(CARET_EQUALS)                   /* ^= */ \
    T(CASE)                           /* case */ \
    T(CATCH)                          /* catch */ \
    T(CLASS)                          /* class */ \
    T(COLON)                          /* : */ \
    T(COMMA)                          /* , */ \
    T(CONST)                          /* const */ \
    T(CONTINUE)                       /* continue */ \
    \
    T(DEBUGGER)                       /* debugger */ \
    T(DEFAULT)                        /* default */ \
    T(DELETE)                         /* delete */ \
    T(DO)                             /* do */ \
    T(DOT)                            /* . */ \
    T(DOT_DOT_DOT)                    /* ... */ \
    \
    T(ELSE)                           /* else */ \
    T(ENUM)                           /* Reserved for future use */ \
    T(EOF)                            /* For internal use */ \
    T(EQUALS)                         /* = */ \
    T(EQUALS_EQUALS)                  /* == */ \
    T(EQUALS_EQUALS_EQUALS)           /* === */ \
    T(EXCLAMATION)                    /* ! */ \
    T(EXCLAMATION_EQUALS)             /* != */ \
    T(EXCLAMATION_EQUALS_EQUALS)      /* !== */ \
    T(EXPORT)                         /* export */ \
    T(EXTENDS)                        /* extends */ \
    \
    T(FALSE)                          /* false */ \
    T(FINALLY)                        /* finally */ \
    T(FOR)                            /* for */ \
    T(FUNCTION)                       /* function */ \
    \
    T(GREATER)                        /* > */ \
    T(GREATER_EQUALS)                 /* >= */ \
    T(GREATER_GREATER)                /* >> */ \
    T(GREATER_GREATER_EQUALS)         /* >>= */ \
    T(GREATER_GREATER_GREATER)        /* >>> */ \
    T(GREATER_GREATER_GREATER_EQUALS) /* >>>= */ \
    \
    T(IF)                             /* if */ \
    T(IDENTIFIER)                     \
    T(IMPLEMENTS)                     /* Reserved for future use (strict) */ \
    T(IMPORT)                         /* import */ \
    T(IN)                             /* in */ \
    T(INSTANCEOF)                     /* instanceof */ \
    T(INTERFACE)                      /* Reserved for future use (strict) */ \
    T(INVALID)                        /* For internal use */ \
    \
    T(LESS)                           /* < */ \
    T(LESS_EQUALS)                    /* <= */ \
    T(LESS_LESS)                      /* << */ \
    T(LESS_LESS_EQUALS)               /* <<= */ \
    T(LET)                            /* let */ \
    \
    T(MINUS)                          /* - */ \
    T(MINUS_EQUALS)                   /* -= */ \
    T(MINUS_MINUS)                    /* -- */ \
    \
    T(NEW)                            /* new */ \
    T(NULL)                           /* null */\
    T(NUMERIC_LITERAL)                \
    \
    T(PACKAGE)                        /* Reserved for future use (strict) */ \
    T(PAREN_LEFT)                     /* ( */ \
    T(PAREN_RIGHT)                    /* ) */ \
    T(PERCENT)                        /* % */ \
    T(PERCENT_EQUALS)                 /* %= */ \
    T(PLUS)                           /* + */ \
    T(PLUS_EQUALS)                    /* += */ \
    T(PLUS_PLUS)                      /* ++ */ \
    T(PRIVATE)                        /* Reserved for future use (strict) */ \
    T(PROTECTED)                      /* Reserved for future use (strict) */ \
    T(PUBLIC)                         /* Reserved for future use (strict) */ \
    \
    T(QUESTION)                       /* ? */ \
    T(QUESTION_DOT)                   /* ?. */ \
    T(QUESTION_QUESTION)              /* ?? */ \
    T(QUESTION_QUESTION_EQUALS)       /* ??= */ \
    \
    T(REGEX_LITERAL)                  \
    T(RETURN)                         /* return */ \
    \
    T(SEMICOLON)                      /* ; */ \
    T(SLASH)                          /* / */ \
    T(SLASH_EQUALS)                   /* /= */ \
    T(SQUARE_LEFT)                    /* [ */ \
    T(SQUARE_RIGHT)                   /* ] */ \
    T(STATIC)                         /* static */ \
    T(STRING_LITERAL)                 \
    T(SUPER)                          /* super */ \
    T(SWITCH)                         /* switch */ \
    \
    T(TEMPLATE_HEAD)                  \
    T(TEMPLATE_MIDDLE)                \
    T(TEMPLATE_TAIL)                  \
    T(TILDE)                          /* ~ */ \
    T(THIS)                           /* this */ \
    T(THROW)                          /* throw */ \
    T(TRUE)                           /* true */ \
    T(TRY)                            /* try */ \
    T(TYPEOF)                         /* typeof */ \
    \
    T(UNDEFINED)                      /* undefined */ \
    \
    T(VAR)                            /* var */ \
    T(VERTICAL)                       /* | */ \
    T(VERTICAL_EQUALS)                /* |= */ \
    T(VERTICAL_VERTICAL)              /* || */ \
    T(VERTICAL_VERTICAL_EQUALS)       /* ||= */ \
    T(VOID)                           /* void */ \
    \
    T(WITH)                           /* with */ \
    T(WHILE)                          /* while */ \
    \
    T(YIELD)                          /* yield */ 

enum token_type : uint8_t
{
#define T(x) TOKEN_##x,
    TOKEN_LIST
#undef T
    TOKEN_COUNT,
};

struct token
{
    int type;

    size_t start;
    size_t end;

    double number;
    char *string;
};

void token_set(struct token *tok, int type);
void token_set_numeric_literal(struct token *tok, double number);
void token_set_string_literal(struct token *tok, char *s);
void token_set_identifier(struct token *tok, char *s);

const char *token_stringify(struct token *tok);
void token_print(struct token *tok);

#endif // TOKEN_H
