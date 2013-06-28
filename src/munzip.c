/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "main.h"
#include "string.h"
#include "file.h"
#include "mzip_lib.h"
#include "mzip_file.h"

int32_t main (int32_t argc, char *argv[])
{
    char *new_filename;
    unsigned char *buf;
    char *filename;
    int32_t len;

    while (argc-- > 1) {

        filename = argv[argc];

        buf = mzip_file_read(filename, &len);
        if (!buf) {
            fprintf(stderr, "failed to uncompress file %s\n", filename);
            exit(1);
        }

        if (!strstr(filename, ".mz")) {
            fprintf(stderr, "expecting .mz suffix for file %s\n", filename);
            exit(1);
        }

        new_filename = strsub(filename, ".mz", "");
        if (!new_filename) {
            exit(1);
        }

        if (!strcmp(new_filename, filename)) {
            fprintf(stderr, "old \"%s\" and new \"%s\" names the same\n",
                    filename, new_filename);
            exit(1);
        }

        if (file_write(new_filename, (unsigned char*)buf, len)) {
            fprintf(stderr, "failed to write to file %s\n", new_filename);
            exit(1);
        }

        printf("\nUncompressed %s to %s len %d\n", filename, new_filename, len);
    }

    exit(0);
}
