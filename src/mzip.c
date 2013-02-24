/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "string.h"
#include "file.h"
#include "mzip_file.h"

int32_t main (int32_t argc, char *argv[])
{
    char *new_filename;
    unsigned char *buf;
    char *filename;
    int32_t len;

    while (argc-- > 1) {

        filename = argv[argc];

        if (strstr(filename, ".mz")) {
            fprintf(stderr, "file %s is already compressed\n", filename);
            exit(1);
        }

        buf = file_read(filename, &len);
        if (!buf) {
            fprintf(stderr, "failed to read file %s\n", filename);
            exit(1);
            exit(1);
        }

        new_filename = strappend(filename, ".mz");
        if (!new_filename) {
            exit(1);
        }

        if (mzip_file_write(new_filename, (unsigned char*)buf, &len)) {
            fprintf(stderr, "failed to write to file %s\n", new_filename);
            exit(1);
        }

        printf("Compressed %s to %s\n", filename, new_filename);
    }

    exit(0);
}
