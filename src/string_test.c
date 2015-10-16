/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#include "main.h"
#include "string.h"
#include "string_ext.h"

uint8_t string_test (int32_t argc, char *argv[])
{
#define SPLIT_TEST
#ifdef SPLIT_TEST
    split("\nTo support multiple\nversions,\n\nthe program named perl now just selects the real version of Perl to run, and the program named a2p now just selects the real version of the awk-to-perl translator to run, depending on various settings.  The current supported versions are 5.8.8 (provided for back-ward-compatibility with the Perl 5.8 family) and 5.10.0, with the default being 5.10.0.\n", 80);
    return (0);
#endif

#ifdef SPRINTF_REALLOC_TEST
    char *tmp;
    int size;
    int used;
    int i;

    tmp = 0;
    size = 1024;

    for (i = 0; i < 100; i++) {
        snprintf_realloc(&tmp, &size, &used, "");
        snprintf_realloc(&tmp, &size, &used, " ");
        snprintf_realloc(&tmp, &size, &used, " . ");
        snprintf_realloc(&tmp, &size, &used, "abcdefgh");

        printf("\nsize %u used %u len %lu\n\n[%s]\n",
            size, used, strlen(tmp), tmp);
    }

    myfree(tmp);
#endif

    return (0);
}
