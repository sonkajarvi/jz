#ifndef TOKEN_H_
#define TOKEN_H_

#define TOKEN_LIST(F) \
    F(AMPERSAND)                      /* & */ \
    F(AMPERSAND_AMPERSAND)            /* && */ \
    F(AMPERSAND_AMPERSAND_EQUALS)     /* &&= */ \
    F(AMPERSAND_EQUALS)               /* &= */ \
    F(ASTERISK)                       /* * */ \
    F(ASTERISK_ASTERISK)              /* ** */ \
    F(ASTERISK_ASTERISK_EQUALS)       /* **= */ \
    F(ASTERISK_EQUALS)                /* *= */ \
    F(ASYNC)                          /* async */ \
    F(ARROW)                          /* => */ \
    F(AWAIT)                          /* await */ \
    \
    F(BIGINT_LITERAL)                 \
    F(BRACE_LEFT)                     /* { */ \
    F(BRACE_RIGHT)                    /* } */ \
    F(BREAK)                          /* break */ \
    \
    F(CARET)                          /* ^ */ \
    F(CARET_EQUALS)                   /* ^= */ \
    F(CASE)                           /* case */ \
    F(CATCH)                          /* catch */ \
    F(CLASS)                          /* class */ \
    F(COLON)                          /* : */ \
    F(COMMA)                          /* , */ \
    F(CONST)                          /* const */ \
    F(CONTINUE)                       /* continue */ \
    \
    F(DEBUGGER)                       /* debugger */ \
    F(DEFAULT)                        /* default */ \
    F(DELETE)                         /* delete */ \
    F(DO)                             /* do */ \
    F(DOT)                            /* . */ \
    F(DOT_DOT_DOT)                    /* ... */ \
    \
    F(ELSE)                           /* else */ \
    F(ENUM)                           /* (Reserved for future use) */ \
    F(EOF)                            /* (For internal use) */ \
    F(EQUALS)                         /* = */ \
    F(EQUALS_EQUALS)                  /* == */ \
    F(EQUALS_EQUALS_EQUALS)           /* === */ \
    F(EXCLAMATION)                    /* ! */ \
    F(EXCLAMATION_EQUALS)             /* != */ \
    F(EXCLAMATION_EQUALS_EQUALS)      /* !== */ \
    F(EXPORT)                         /* export */ \
    F(EXTENDS)                        /* extends */ \
    \
    F(FALSE)                          /* false */ \
    F(FINALLY)                        /* finally */ \
    F(FOR)                            /* for */ \
    F(FUNCTION)                       /* function */ \
    \
    F(GREATER)                        /* > */ \
    F(GREATER_EQUALS)                 /* >= */ \
    F(GREATER_GREATER)                /* >> */ \
    F(GREATER_GREATER_EQUALS)         /* >>= */ \
    F(GREATER_GREATER_GREATER)        /* >>> */ \
    F(GREATER_GREATER_GREATER_EQUALS) /* >>>= */ \
    \
    F(IF)                             /* if */ \
    F(IDENTIFIER)                     \
    F(IMPLEMENTS)                     /* (Reserved for future use [strict]) */ \
    F(IMPORT)                         /* import */ \
    F(IN)                             /* in */ \
    F(INSTANCEOF)                     /* instanceof */ \
    F(INTERFACE)                      /* (Reserved for future use [strict]) */ \
    \
    F(LESS)                           /* < */ \
    F(LESS_EQUALS)                    /* <= */ \
    F(LESS_LESS)                      /* << */ \
    F(LESS_LESS_EQUALS)               /* <<= */ \
    F(LET)                            /* let */ \
    \
    F(MINUS)                          /* - */ \
    F(MINUS_EQUALS)                   /* -= */ \
    F(MINUS_MINUS)                    /* -- */ \
    \
    F(NEW)                            /* new */ \
    F(NULL)                           /* null */ \
    F(NUMERIC_LITERAL)                \
    \
    F(PACKAGE)                        /* (Reserved for future use [strict]) */ \
    F(PAREN_LEFT)                     /* ( */ \
    F(PAREN_RIGHT)                    /* ) */ \
    F(PERCENT)                        /* % */ \
    F(PERCENT_EQUALS)                 /* %= */ \
    F(PLUS)                           /* + */ \
    F(PLUS_EQUALS)                    /* += */ \
    F(PLUS_PLUS)                      /* ++ */ \
    F(PRIVATE)                        /* (Reserved for future use [strict]) */ \
    F(PROTECTED)                      /* (Reserved for future use [strict]) */ \
    F(PUBLIC)                         /* (Reserved for future use [strict]) */ \
    \
    F(QUESTION)                       /* ? */ \
    F(QUESTION_DOT)                   /* ?. */ \
    F(QUESTION_QUESTION)              /* ?? */ \
    F(QUESTION_QUESTION_EQUALS)       /* ??= */ \
    \
    F(REGEX_LITERAL)                  \
    F(RETURN)                         /* return */ \
    \
    F(SEMICOLON)                      /* ; */ \
    F(SLASH)                          /* / */ \
    F(SLASH_EQUALS)                   /* /= */ \
    F(SQUARE_LEFT)                    /* [ */ \
    F(SQUARE_RIGHT)                   /* ] */ \
    F(STATIC)                         /* static */ \
    F(STRING_LITERAL)                 \
    F(SUPER)                          /* super */ \
    F(SWITCH)                         /* switch */ \
    \
    F(TEMPLATE_HEAD)                  \
    F(TEMPLATE_MIDDLE)                \
    F(TEMPLATE_TAIL)                  \
    F(TILDE)                          /* ~ */ \
    F(THIS)                           /* this */ \
    F(THROW)                          /* throw */ \
    F(TRUE)                           /* true */ \
    F(TRY)                            /* try */ \
    F(TYPEOF)                         /* typeof */ \
    \
    F(UNDEFINED)                      /* undefined */ \
    \
    F(VAR)                            /* var */ \
    F(VERTICAL)                       /* | */ \
    F(VERTICAL_EQUALS)                /* |= */ \
    F(VERTICAL_VERTICAL)              /* || */ \
    F(VERTICAL_VERTICAL_EQUALS)       /* ||= */ \
    F(VOID)                           /* void */ \
    \
    F(WITH)                           /* with */ \
    F(WHILE)                          /* while */ \
    \
    F(YIELD)                          /* yield */

enum token_type
{
#define F(x) TOKEN_##x,
    TOKEN_LIST(F)
#undef F
    TOKEN_COUNT
};

struct token
{
    int type;
};

#endif // TOKEN_H_
