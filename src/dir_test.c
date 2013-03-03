/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "dir.h"
#include "string.h"

boolean dir_test (int32_t argc, char *argv[])
{
    tree_root *d;
    tree_file_node *n;

    d = dirlist(".", 0, 0, true /* include dirs */);
    if (!d) {
        exit(1);
    }

    TREE_WALK(d, n) {
        char * file = strsub(n->tree.key, "./", "");
        printf("%s\n", file);
        myfree(file);

        tree_remove(d, &n->tree.node);
        myfree(n);
    }
    myfree(d);

    return (0);
}
