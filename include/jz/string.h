//
// Copyright (c) 2024-2025, sonkajarvi
//
// SPDX-License-Identifier: MIT
//

#ifndef JZ_STRING_H
#define JZ_STRING_H

#include <stddef.h>

#ifndef STRING_DEFAULT_CAPACITY
# define STRING_DEFAULT_CAPACITY 16
#endif

struct string
{
    char *data;
    size_t capacity;
    size_t length;
};

void string_free(struct string *str);

int string_reserve(struct string *str, const size_t n);
void string_shrink(struct string *str);

int string_append(struct string *str, char c);
int string_append_str(struct string *str, const char *s);
int string_append_range(struct string *str, const char *s, const size_t n);
int string_append_codepoint(struct string *str, const int cp);

const char *string_ref(struct string *str);
char *string_release(struct string *str);

#endif // JZ_STRING_H
