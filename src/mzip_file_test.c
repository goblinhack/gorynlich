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
#include "file.h"
#include "mzip_file.h"

boolean mzip_file_test (int32_t argc, char *argv[])
{
    const char *filename = "test.mz";
    char out[] =
    "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson."
    "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson."
    "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson."
    "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson."
    "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson."
    "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson."
    "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson."
    "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson."
    "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson."
    "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson."
    "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson."
    "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson."
    "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson."
    "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson."
    "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson."
    "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson."
    "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson."
    "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson."
    "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson."
    "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson."
    "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson."
    "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson."
    "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson."
    "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson."
    "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson."
    "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson."
    "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson."
    "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson."
    "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson."
    "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson."
    "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson.";
    int32_t out_len;
    unsigned char *in;
    int32_t in_len;

    out_len = (uint32_t)strlen(out);
    printf("Wrote uncompressed len %d\n", out_len);
    mzip_file_write(filename, (unsigned char*)out, &out_len);
    printf("Wrote compressed len %d\n", out_len);

    in = mzip_file_read(filename, &in_len);
    printf("Read uncompressed len %d\n", in_len);
    myfree(in);

    return (0);
}
