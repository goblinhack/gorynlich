/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "main.h"
#include "file.h"
#include "string_util.h"
#include "mzip_file.h"
#include "mzip_lib.h"
#ifndef MINIMAL
#include "ramdisk.h"
#endif

unsigned char *mzip_file_read (const char *filename, int32_t *out_len)
{
    unsigned char *buf_compressed;
    unsigned char *buf;
    int32_t len;

#ifndef MINIMAL
    buf_compressed = ramdisk_load(filename, &len);
    if (!buf_compressed) {
        fprintf(MY_STDERR,
                "Failed to read compressed file \"%s\" for reading: %s\n",
                filename, strerror(errno));
        return (0);
    }
#else
    buf_compressed = file_read(filename, &len);
    if (!buf_compressed) {
        fprintf(MY_STDERR,
                "Failed to read compressed file \"%s\" for reading: %s\n",
                filename, strerror(errno));
        return (0);
    }
#endif

    buf = miniz_uncompress(buf_compressed, &len);
    if (!buf) {
        fprintf(MY_STDERR,
                "Failed to read compressed data for file \"%s\" len %d: %s\n",
                filename, len, strerror(errno));
        myfree(buf_compressed);
        return (0);
    }

    if (out_len) {
        *out_len = len;
    }

    LOG("Uncompressed %s, %d bytes", filename, len);

    myfree(buf_compressed);

    return (buf);
}

int32_t mzip_file_write (const char *filename, unsigned char *buf, 
                         int32_t *len)
{
    unsigned char *buf_compressed;

    buf_compressed = miniz_compress(buf, len);
    if (!buf_compressed) {
        fprintf(MY_STDERR,
                "Failed to compress file \"%s\" len %d for writing: %s\n",
                filename, *len, strerror(errno));
        return (-1);
    }

    if (file_write(filename, buf_compressed, *len)) {
        fprintf(MY_STDERR,
                "Failed to write compressed file \"%s\" len %d: %s\n",
                filename, *len, strerror(errno));
        myfree(buf_compressed);
        return (-1);
    }

    myfree(buf_compressed);

    return (0);
}

int32_t mzip_file_write2 (const char *filename, unsigned char *buf, 
                          int32_t *len, int level)
{
    unsigned char *buf_compressed;

    buf_compressed = miniz_compress2(buf, len, level);
    if (!buf_compressed) {
        fprintf(MY_STDERR,
                "Failed to compress file \"%s\" len %d for writing: %s\n",
                filename, *len, strerror(errno));
        return (-1);
    }

    if (file_write(filename, buf_compressed, *len)) {
        fprintf(MY_STDERR,
                "Failed to write compressed file \"%s\" len %d: %s\n",
                filename, *len, strerror(errno));
        myfree(buf_compressed);
        return (-1);
    }

    myfree(buf_compressed);

    return (0);
}
