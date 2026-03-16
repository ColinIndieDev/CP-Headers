#pragma once

#include "cpmemory.h"

#define ARR_DEF(type, name)                                                    \
    typedef struct {                                                           \
        type *data;                                                            \
        u32 size;                                                              \
    } name;                                                                    \
    void name##_init(name *arr, u32 size) {                                    \
        arr->data = cp_malloc(size * sizeof(type));                            \
        arr->size = size;                                                      \
    }                                                                          \
    type *name##_at(name *arr, u32 i) { return &arr->data[i]; }                \
    void name##_destroy(name *arr) {                                           \
        cp_free(arr->data);                                                    \
        arr->size = 0;                                                         \
    }                                                                          \
    type *name##_begin(name *arr) { return arr->data; }                        \
    type *name##_end(name *arr) { return arr->data + arr->size; }              \
    type *name##_front(name *arr) { return &arr->data[0]; }                    \
    type *name##_back(name *arr) { return &arr->data[arr->size - 1]; }         \
    b8 name##_empty(name *arr) { return arr->size <= 0; }

#define FOREACH_ARR(type, arrname, it, arrptr)                                 \
    for (type *it = arrname##_begin(arrptr); it != arrname##_end(arrptr); it++)
