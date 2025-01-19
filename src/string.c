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
    if (!str || !c)
        return -1;

    char *tmp = str->data;
    if (str->length + 1 < str->capacity)
        goto skip_realloc;

    str->capacity = 2 * str->capacity ?: STRING_DEFAULT_CAPACITY;
    if ((tmp = realloc(tmp, str->capacity)) == NULL)
        return -1;
    str->data = tmp;

skip_realloc:
    str->data[str->length++] = c;
    str->data[str->length] = '\0';
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
    if (str->length + n + 1 < str->capacity)
        goto skip_realloc;

    while (str->capacity < str->length + n)
        str->capacity = 2 * str->capacity ?: STRING_DEFAULT_CAPACITY;

    if ((tmp = realloc(tmp, str->capacity)) == NULL)
        return -1;
    str->data = tmp;

skip_realloc:
    memcpy(&str->data[str->length], s, n);
    str->length += n;
    str->data[str->length] = '\0';
    return 0;
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
    return str ? str->data : NULL;
}

char *string_release(struct string *str)
{
    if (!str)
        return NULL;

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
