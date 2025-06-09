#ifndef STRING_H_
#define STRING_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define S(str, x) string_from_utf8((str), (void *)x, strlen(x))

// UTF-16 encoded string
struct string
{
    // Vectors are used internally,
    // no capacity or length included here
    uint16_t *data;
};

size_t string_len(struct string *str);
bool string_eq(struct string *str, struct string *other);
void string_init(struct string *str);
void string_free(struct string *str);
void string_from_utf8(struct string *str, uint8_t *utf8, size_t len);
void string_push_codepoint(struct string *str, uint32_t cp);

#endif // STRING_H_
