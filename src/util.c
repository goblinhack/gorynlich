/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <stdlib.h>
#include <string.h>

#include "main.h"

void *myzalloc_ (uint32_t size,
                 const char *what,
                 const char *file,
                 const char *func,
                 const uint32_t line)
{
    void *ptr = calloc(1, size);

    if (!ptr) {
        DIE("No memory, %s:%s():%u", file, func, line);
    }

    ptrcheck_alloc(ptr, what, size, file, func, line);

    return (ptr);
}

void *mymalloc_ (uint32_t size,
                 const char *what,
                 const char *file,
                 const char *func,
                 const uint32_t line)
{
    void *ptr = malloc(size);

    if (!ptr) {
        DIE("No memory, %s:%s():%u", file, func, line);
    }

    ptrcheck_alloc(ptr, what, size, file, func, line);

    return (ptr);
}

void *myrealloc_ (void *ptr,
                  uint32_t size,
                  const char *what,
                  const char *file,
                  const char *func,
                  const uint32_t line)
{
    ptrcheck_free(ptr, file, func, line);

    ptr = realloc(ptr, size);
    if (!ptr) {
        DIE("No memory, %s:%s():%u", file, func, line);
    }

    ptrcheck_alloc(ptr, what, size, file, func, line);

    return (ptr);
}

void myfree_ (void *ptr,
              const char *file,
              const char *func,
              const uint32_t line)
{
    ptrcheck_free(ptr, file, func, line);

    free(ptr);
}

char *dupstr_ (const char *in,
               const char *what,
               const char *file,
               const char *func,
               const uint32_t line)
{
    char *ptr = strdup(in);
    uint32_t size = (typeof(size)) strlen(in);

    if (!ptr) {
        DIE("No memory, %s:%s():%u", file, func, line);
    }

    ptrcheck_alloc(ptr, what, size, file, func, line);

    return (ptr);
}
