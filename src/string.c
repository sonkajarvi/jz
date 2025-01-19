//
// Copyright (c) 2024-2025, sonkajarvi
//
// SPDX-License-Identifier: MIT
//

#include <stdlib.h>
#include <string.h>

#include <jz/string.h>

void string_free(struct string *str)
{
    if (!str)
        return;

    free(str->data);
    memset(str, 0, sizeof(*str));
}

int string_append(struct string *str, char c)
{
    if (!str)
        return -1;

    char *tmp = str->data;
    if (str->length < str->capacity)
        goto skip_realloc;

    str->capacity = 2 * str->capacity ?: STRING_DEFAULT_CAPACITY;
    if ((tmp = realloc(tmp, str->capacity)) == NULL)
        return -1;
    str->data = tmp;

skip_realloc:
    str->data[str->length++] = c;
    return 0;
}

int string_append_str(struct string *str, const char *s)
{
    return string_append_range(str, s, strlen(s));
}

int string_append_range(struct string *str, const char *s, const size_t n)
{
    if (!str || !s || !s[0] || !n)
        return -1;

    char *tmp = str->data;
    if (str->length + n < str->capacity)
        goto skip_realloc;

    while (str->capacity < str->length + n)
        str->capacity = 2 * str->capacity ?: STRING_DEFAULT_CAPACITY;

    if ((tmp = realloc(tmp, str->capacity)) == NULL)
        return -1;
    str->data = tmp;

skip_realloc:
    memcpy(&str->data[str->length], s, n);
    str->length += n;
    return 0;
}

int string_append_codepoint(struct string *str, const int cp)
{
    switch (cp) {
    case 0 ... 0x7f:
        string_append(str, cp);
        return 0;

    case 0x80 ... 0x7ff:
        string_append(str, ((cp >> 6) & 0x1f) | 0xc0);
        string_append(str, (cp        & 0x3f) | 0x80);
        return 0;

    case 0x800 ... 0xd7ff:
    case 0xe000 ... 0xffff:
        string_append(str, ((cp >> 12) & 0x0f) | 0xe0);
        string_append(str, ((cp >> 6)  & 0x3f) | 0x80);
        string_append(str, (cp         & 0x3f) | 0x80);
        return 0;

    case 0x10000 ... 0x10ffff:
        string_append(str, ((cp >> 18) & 0x07) | 0xf0);
        string_append(str, ((cp >> 12) & 0x3f) | 0x80);
        string_append(str, ((cp >> 6)  & 0x3f) | 0x80);
        string_append(str, (cp         & 0x3f) | 0x80);
        return 0;
    }

    return -1;
}

int string_reserve(struct string *str, const size_t n)
{
    if (!str || n <= str->capacity)
        return -1;

    char *tmp;
    if ((tmp = realloc(str->data, n)) == NULL)
        return -1;

    str->data = tmp;
    str->capacity = n;
    return 0;
}


const char *string_ref(struct string *str)
{
    if (!str || !str->data)
        return NULL;

    // Add null terminator
    if (str->capacity == str->length || str->data[str->length] > 0)
        string_append(str, '\0');

    return str->data;
}

char *string_release(struct string *str)
{
    if (!str || !str->data)
        return NULL;

    // Add null terminator
    if (str->capacity == str->length || str->data[str->length] > 0)
        string_append(str, '\0');

    char *tmp = str->data;
    memset(str, 0, sizeof(*str));
    return tmp;
}

void string_shrink(struct string *str)
{
    if (!str || !str->data || str->length == str->capacity)
        return;

    char *tmp;
    if ((tmp = realloc(str->data, str->length + 1)) == NULL)
        return;

    str->data = tmp;
    str->capacity = str->length + 1;
}
