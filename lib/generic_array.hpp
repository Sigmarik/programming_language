/**
 * @file generic_array.hpp
 * @author Kudryashov Ilya (kudriashov.it@phystech.edu)
 * @brief Generic-type array structure
 * @version 0.1
 * @date 2023-05-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef GENERIC_ARRAY_HPP
#define GENERIC_ARRAY_HPP

#define MAKE_ARRAY_DECLARATION(name, type)                      \
struct name {                                                   \
    size_t size = 0; size_t capacity = 0; type* buffer = NULL;  \
};                                                              \
                                                                \
void name##_ctor(name* array, size_t start_capacity = 1);       \
void name##_dtor(name* array);                                  \
void name##_push(name* array, type element);                    \
void name##_pop(name* array);                                   \
type* name##_at(const name* array, long long index = -1);       \
type* name##_begin(const name* array);                          \
type* name##_end(const name* array);


#define MAKE_ARRAY_DEFINITION(name, type)                                       \
void name##_ctor(name* array, size_t start_capacity) {                          \
    if (array->buffer) return;                                                  \
    array->buffer = (type*) calloc(start_capacity, sizeof(type));               \
    if (!array->buffer) { errno = ENOMEM; return; }                             \
    array->capacity = start_capacity;                                           \
}                                                                               \
                                                                                \
void name##_dtor(name* array) {                                                 \
    free(array->buffer);                                                        \
    array->buffer = NULL;                                                       \
    array->capacity = 0;                                                        \
    array->size = 0;                                                            \
}                                                                               \
                                                                                \
void name##_push(name* array, type element) {                                   \
    if (!array->buffer) return;                                                 \
                                                                                \
    if (array->size + 1 >= array->capacity) {                                   \
        type* new_buffer = (type*) calloc(array->capacity << 1, sizeof(type));  \
        if (!new_buffer) { errno = ENOMEM; return; }                            \
        memcpy(new_buffer, array->buffer, array->size * sizeof(type));          \
        free(array->buffer);                                                    \
        array->buffer = new_buffer;                                             \
        array->capacity <<= 1;                                                  \
    }                                                                           \
                                                                                \
    array->buffer[array->size] = element;                                       \
    ++array->size;                                                              \
}                                                                               \
                                                                                \
void name##_pop(name* array) {                                                  \
    if (array->size == 0) return;                                               \
                                                                                \
    if (array->size * 4 < array->capacity && array->capacity >= 2) {            \
        type* new_buffer = (type*) calloc(array->capacity >> 1, sizeof(type));  \
        if (!new_buffer) { errno = ENOMEM; return; }                            \
        memcpy(new_buffer, array->buffer, array->size * sizeof(type));          \
        free(array->buffer);                                                    \
        array->buffer = new_buffer;                                             \
        array->capacity >>= 1;                                                  \
    }                                                                           \
                                                                                \
    --array->size;                                                              \
}                                                                               \
                                                                                \
type* name##_at(const name* array, long long index) {                           \
    if (!array->buffer) return NULL;                                            \
                                                                                \
    if (index > (long long) array->size || -index > (long long) array->size)    \
        return NULL;                                                            \
                                                                                \
    if (index < 0) index = (long long)array->size - index;                      \
                                                                                \
    return &array->buffer[index];                                               \
}                                                                               \
                                                                                \
type* name##_begin(const name* array) {                                         \
    return array->buffer;                                                       \
}                                                                               \
                                                                                \
type* name##_end(const name* array) {                                           \
    return array->buffer + array->size - 1;                                     \
}

#endif
