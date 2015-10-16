/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
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

#ifdef ENABLE_PTRCHECK
    ptrcheck_alloc(ptr, what, size, file, func, line);
#endif

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

#ifdef ENABLE_PTRCHECK
    ptrcheck_alloc(ptr, what, size, file, func, line);
#endif

    return (ptr);
}

void *myrealloc_ (void *ptr,
                  uint32_t size,
                  const char *what,
                  const char *file,
                  const char *func,
                  const uint32_t line)
{
#ifdef ENABLE_PTRCHECK
    ptrcheck_free(ptr, file, func, line);
#endif

    ptr = realloc(ptr, size);
    if (!ptr) {
        DIE("No memory, %s:%s():%u", file, func, line);
    }

#ifdef ENABLE_PTRCHECK
    ptrcheck_alloc(ptr, what, size, file, func, line);
#endif

    return (ptr);
}

void myfree_ (void *ptr,
              const char *file,
              const char *func,
              const uint32_t line)
{
#ifdef ENABLE_PTRCHECK
    ptrcheck_free(ptr, file, func, line);
#endif

    free(ptr);
}

char *dupstr_ (const char *in,
               const char *what,
               const char *file,
               const char *func,
               const uint32_t line)
{
    if (!in) {
        ERR("no string to duplicate");
        return (0);
    }

    char *ptr = strdup(in);
#ifdef ENABLE_PTRCHECK
    uint32_t size = (typeof(size)) strlen(in);
#endif

    if (!ptr) {
        DIE("No memory, %s:%s():%u", file, func, line);
    }

#ifdef ENABLE_PTRCHECK
    ptrcheck_alloc(ptr, what, size, file, func, line);
#endif

    return (ptr);
}
