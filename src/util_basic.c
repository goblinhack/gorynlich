/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <stdarg.h>

#include "main.h"
#include "string.h"

#undef DIE
#define DIE printf

#define ptrcheck_alloc(val, what, size, file, func, line)
#define ptrcheck_free(val, file, func, line)

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

static void log_ (const char *fmt, va_list args)
{
    char buf[MAXSTR];
    uint32_t len;

    buf[0] = '\0';
    len = (uint32_t)strlen(buf);
    vsnprintf(buf + len, sizeof(buf) - len, fmt, args);

    printf("%s", buf);
    fflush(stdout);
}

void LOG (const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    log_(fmt, args);
    va_end(args);

    printf("\n");
}
