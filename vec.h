#ifndef VEC_H_
#define VEC_H_

// vec_isempty
// vec_len
// vec_cap

// vec_begin
// vec_end
// vec_at

// vec_push
// vec_pop
// vec_clear
// vec_free

#include <stdlib.h>

#define vec_empty(v) ((v) ? vec_len(v) == 0 : 1)
#define vec_len(v) ((v) ? vec_header_(v)->len : 0)
#define vec_cap(v) ((v) ? vec_header_(v)->cap : 0)

#define vec_begin(v) (v)
#define vec_end(v) ((v) ? (v) + vec_len(v) - 1 : NULL)

#define vec_push(v, item) ({             \
    vec_maybegrow_(v, 1);                \
    (v)[vec_header_(v)->len++] = (item); \
    vec_end(v); })

#define vec_free(v) ((v) ? free(vec_header_(v)) : 0)

#define vec_header_(v) ((struct vec_header *)(v) - 1)
#define vec_maybegrow_(v, add_len)                         \
    (!(v) || vec_cap(v) < vec_len(v) + (add_len)           \
        ? (v) = vec_realloc_((v), (add_len), sizeof(*(v))) \
        : 0)

struct vec_header
{
    size_t len;
    size_t cap;
};

static inline
size_t calculate_capacity_(size_t cap)
{
    if (cap < 2)
        return 2;

    return cap + (cap >> 1) + (cap >> 3);
}

static inline
void *vec_realloc_(void *v, size_t add_len, size_t width)
{
    size_t new_cap = vec_cap(v);
    while (new_cap < vec_len(v) + add_len)
        new_cap = calculate_capacity_(new_cap);

    struct vec_header *tmp = realloc(v ? vec_header_(v) : NULL,
        sizeof(*tmp) + (new_cap * width));

    if (!v)
        tmp->len = 0;
    tmp->cap = new_cap;

    return (char *)tmp + sizeof(*tmp);
}

#endif // VEC_H_
