/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#ifndef WIN32
#include <execinfo.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "backtrace.h"

#define MAX_TRACEBACK 16

typedef struct traceback_ {
    void *array[MAX_TRACEBACK];
    uint32_t size;
} traceback;

void backtrace_print (void)
{
#ifndef WIN32
    void *array[10];
    int32_t size;
    char **strings;
    int32_t i;

    size = backtrace(array, ARRAY_SIZE(array));
    strings = backtrace_symbols(array, size);

    for (i = 0; i < size; i++) {
        RAW("%s", strings[i]);
    }

    free(strings);
#endif
}
/*
 * Allocate a new traceback.
 */
traceback *traceback_alloc (void)
{
#ifndef WIN32
    traceback *tb;

    tb = (typeof(tb)) malloc(sizeof(*tb));

    tb->size = backtrace(tb->array, MAX_TRACEBACK);

    return (tb);
#else
    return (0);
#endif
}

/*
 * Free a traceback.
 */
void traceback_free (traceback *tb)
{
#ifndef WIN32
    free(tb);
#endif
}

/*
 * Free a traceback.
 */
void traceback_stdout (traceback *tb)
{
#ifndef WIN32
    uint32_t i;
    char **strings;

    strings = backtrace_symbols(tb->array, tb->size);

    for (i = 0; i < tb->size; i++) {
        RAW("%s", strings[i]);

        if (strstr(strings[i], "main +")) {
            break;
        }
    }

    free(strings);
#endif
}

/*
 * Free a traceback.
 */
void traceback_stderr (traceback *tb)
{
#ifndef WIN32
    uint32_t i;
    char **strings;

    strings = backtrace_symbols(tb->array, tb->size);

    for (i = 0; i < tb->size; i++) {
        RAW("%s", strings[i]);

        if (strstr(strings[i], "main +")) {
            break;
        }
    }

    free(strings);
#endif
}
