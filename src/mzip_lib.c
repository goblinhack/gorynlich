/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#include <string.h>
#include "miniz.h"

#include "main.h"
#include "mzip_lib.h"

unsigned char *miniz_compress (unsigned char *buf, int32_t *uncompressed_len)
{
    unsigned long compressed_len = mz_compressBound(*uncompressed_len);
    unsigned char *buf_compressed;

    buf_compressed = (typeof(buf_compressed))
                    mymalloc((uint32_t)compressed_len, __FUNCTION__);
    if (!buf_compressed) {
        return (0);
    }

    if (mz_compress(buf_compressed, &compressed_len, buf,
                    *uncompressed_len) != MZ_OK) {
        myfree(buf_compressed);
        return (0);
    }

    *uncompressed_len = (uint32_t)compressed_len;

    return (buf_compressed);
}

unsigned char *miniz_compress2 (unsigned char *buf, int32_t *uncompressed_len,
                                int level)
{
    unsigned long compressed_len = mz_compressBound(*uncompressed_len);
    unsigned char *buf_compressed;

    buf_compressed = (typeof(buf_compressed))
                    mymalloc((uint32_t)compressed_len, __FUNCTION__);
    if (!buf_compressed) {
        return (0);
    }

    if (mz_compress2(buf_compressed, &compressed_len, buf,
                     *uncompressed_len, level) != MZ_OK) {
        myfree(buf_compressed);
        return (0);
    }

    *uncompressed_len = (uint32_t)compressed_len;

    return (buf_compressed);
}

static unsigned char *scratch_pad;

void miniz_fini (void) 
{
    if (scratch_pad) {
        myfree(scratch_pad);
        scratch_pad = 0;
    }
}

unsigned char *miniz_uncompress (unsigned char *buf, int32_t *compressed_len)
{
    static unsigned long scratch_pad_size = 16 * 1024;
    unsigned char *buf_uncompressed;
    unsigned long uncompressed_len;

    for (;;) {
        if (scratch_pad == 0) {
            scratch_pad = (typeof(scratch_pad))
                            mymalloc((uint32_t)scratch_pad_size,
                                     "miniz scratchpad");
            if (scratch_pad == 0) {
                fprintf(MY_STDERR, "Failed to alloc scratch pad size %lu\n",
                        scratch_pad_size);
                return (0);
            }
        }

        uncompressed_len = scratch_pad_size;
        if (mz_uncompress(scratch_pad, &uncompressed_len, buf,
                          *compressed_len) == MZ_OK) {
            break;
        }

        if (scratch_pad_size > 100 * 1024 * 1024) {
            fprintf(MY_STDERR, "Failed to uncompress buf in-len %d out-len %lu\n",
                    *compressed_len, uncompressed_len);
            return (0);
        }

        scratch_pad_size += scratch_pad_size / 2;
        scratch_pad = 0;
    }

    buf_uncompressed = (typeof(buf_uncompressed))
                    mymalloc((uint32_t)uncompressed_len, __FUNCTION__);
    if (!buf_uncompressed) {
        return (0);
    }

    memcpy(buf_uncompressed, scratch_pad, uncompressed_len);
    *compressed_len = (uint32_t)uncompressed_len;

    return (buf_uncompressed);
}
