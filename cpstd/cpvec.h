#pragma once

#include <assert.h>

#include "cpmemory.h"

#define VEC_DEF(type, name)                                                    \
    typedef struct {                                                           \
        type *data;                                                            \
        u32 size;                                                              \
        u32 capacity;                                                          \
    } name;                                                                    \
    void name##_init(name *vec, u32 size, type val) {                          \
        vec->capacity = size > 10 ? size * 2 : 10;                             \
        vec->data = cp_malloc(vec->capacity * sizeof(type));                   \
        vec->size = size;                                                      \
        for (u32 i = 0; i < vec->size; i++) {                                  \
            vec->data[i] = val;                                                \
        }                                                                      \
    }                                                                          \
    void name##_reserve(name *vec, u32 size) {                                 \
        vec->capacity = size;                                                  \
        vec->data = cp_malloc(vec->capacity * sizeof(type));                   \
        vec->size = 0;                                                         \
    }                                                                          \
    type *name##_at(name *vec, u32 i) {                                        \
        assert("Out of bounds or not initialized" && 0 <= i &&                 \
               i < vec->size && vec->size > 0);                                \
        return &vec->data[i];                                                  \
    }                                                                          \
    type name##_get(name *vec, u32 i) {                                        \
        assert("Out of bounds or not initialized" && 0 <= i &&                 \
               i < vec->size && vec->size > 0);                                \
        return vec->data[i];                                                   \
    }                                                                          \
    void name##_push_back(name *vec, type val) {                               \
        if (vec->size >= vec->capacity) {                                      \
            vec->capacity *= 2;                                                \
            type *temp = cp_realloc(vec->data, vec->capacity * sizeof(type));  \
            assert(temp != NULLPTR);                                           \
            vec->data = temp;                                                  \
        }                                                                      \
        vec->data[vec->size] = val;                                            \
        vec->size++;                                                           \
    }                                                                          \
    void name##_push_front(name *vec, type val) {                              \
        if (vec->size >= vec->capacity) {                                      \
            type *new_data =                                                   \
                cp_realloc(vec->data, (u64)vec->capacity * 2 * sizeof(type));  \
            if (new_data != NULLPTR) {                                         \
                vec->capacity *= 2;                                            \
                vec->data = new_data;                                          \
            }                                                                  \
        }                                                                      \
        memmove(&vec->data[1], &vec->data[0], vec->size * sizeof(type));       \
        vec->data[0] = val;                                                    \
        vec->size++;                                                           \
    }                                                                          \
    void name##_pop_back(name *vec) {                                          \
        assert("Cannot pop element if there are none" && vec->size > 0);       \
        vec->size--;                                                           \
    }                                                                          \
    void name##_pop_front(name *vec) {                                         \
        assert("Cannot pop element if there are none" && vec->size > 0);       \
        memmove(&vec->data[0], &vec->data[1], (vec->size - 1) * sizeof(type)); \
        vec->size--;                                                           \
    }                                                                          \
    void name##_delete(name *vec, u32 i) {                                     \
        assert("Out of bounds or not initialized" && 0 <= i &&                 \
               i < vec->size && vec->size > 0);                                \
        memmove(&vec->data[i], &vec->data[i + 1],                              \
                (vec->size - i - 1) * sizeof(type));                           \
        vec->size--;                                                           \
    }                                                                          \
    void name##_destroy(name *vec) {                                           \
        cp_free(vec->data);                                                    \
        vec->size = 0;                                                         \
        vec->capacity = 0;                                                     \
    }                                                                          \
    void name##_set(name *vec, u32 i, type val) {                              \
        assert("Out of bounds or not initialized" && 0 <= i &&                 \
               i < vec->size && vec->size > 0);                                \
        vec->data[i] = val;                                                    \
    }                                                                          \
    void name##_copy(name *vec, name *dest) {                                  \
        dest->size = vec->size;                                                \
        dest->capacity = vec->capacity;                                        \
        if (dest->capacity < vec->size) {                                      \
            if (dest->data != NULLPTR) {                                       \
                cp_free(dest->data);                                           \
            }                                                                  \
            dest->capacity = vec->size * 2;                                    \
            dest->data = cp_malloc((u64)vec->size * 2 * sizeof(type));         \
        }                                                                      \
        for (int i = 0; i < vec->size; i++) {                                  \
            dest->data[i] = vec->data[i];                                      \
        }                                                                      \
    }                                                                          \
    void name##_clear(name *vec) { vec->size = 0; }                            \
    type *name##_begin(name *v) { return v->data; }                            \
    type *name##_end(name *v) { return v->data + v->size; }                    \
    type *name##_front(name *v) { return &v->data[0]; }                        \
    type *name##_back(name *v) { return &v->data[v->size - 1]; }               \
    b8 name##_empty(name *v) { return v->size <= 0; }

#define FOREACH_VEC(type, vname, it, vptr)                                     \
    for (type *it = vname##_begin(vptr); it != vname##_end(vptr); it++)

#define VEC_ERASE_IF(v, cond)                                                  \
    do {                                                                       \
        u32 w = 0;                                                             \
        for (u32 _i = 0; _i < (v)->size; _i++) {                               \
            __auto_type it = (v)->data[_i];                                    \
            if (!(cond)) {                                                     \
                (v)->data[w++] = (v)->data[_i];                                \
            }                                                                  \
        }                                                                      \
        (v)->size = w;                                                         \
    } while (0);
