/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "string_util.h"
#include "file.h"
#include "mzip_file.h"

int32_t main (int32_t argc, char *argv[])
{
    char *new_filename;
    unsigned char *buf;
    char *filename;
    int32_t len;
    int level = 1;
    int i;

    for (i = 1; i < argc; i++) {
        int val = strtol(argv[i], NULL, 10);
        if (val != 0) {
            level = val;
            if (level < 0) {
                level = -level;
            }
        }
    }

    while (argc-- > 1) {

        int val = strtol(argv[argc], NULL, 10);
        if (val != 0) {
            continue;
        }

        filename = argv[argc];

        if (strstr(filename, ".mz")) {
            fprintf(MY_STDERR, "file %s is already compressed\n", filename);
            exit(1);
        }

        buf = file_read(filename, &len);
        if (!buf) {
            fprintf(MY_STDERR, "failed to read file %s\n", filename);
            exit(1);
            exit(1);
        }

        new_filename = strappend(filename, ".mz");
        if (!new_filename) {
            exit(1);
        }

        printf("\nUncompressed %s len %d\n", filename, len);

        if (mzip_file_write2(new_filename, (unsigned char*)buf, &len,
                             level)) {
            fprintf(MY_STDERR, "failed to write to file %s\n", new_filename);
            exit(1);
        }

        printf("\nCompressed %s to %s len %d\n", filename, new_filename, len);
    }

    exit(0);
}
