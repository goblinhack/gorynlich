/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 *
 * A useful hack to help debug stale/free/bad pointers.
 */

#include "main.h"

uint8_t ptrcheck_test (int32_t argc, char *argv[])
{
    void *p;

    ptrcheck_leak_snapshot();

    p = myzalloc(10, "ptrcheck_test a");
    verify(p);
    myfree(p);

    p = mymalloc(10, "ptrcheck_test b");
    verify(p);

    return (0);
}
