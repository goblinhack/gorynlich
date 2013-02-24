/*
 * Copyright(C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <libgen.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define nUSE_ZLIB
#define USE_MINIZ
#define nUSE_STB_IMAGE

#ifdef USE_ZLIB
#include <zlib.h>
#endif

#ifdef USE_MINIZ
#include "miniz.h"
#endif

#ifdef USE_STB_IMAGE
#include "stb_image.h"
#endif

#include "main.h"
#include "wid_popup.h"
#include "file.h"
#include "string.h"

#include "ramdisk.h"
#include "ramdisk_files.c"

extern ramdisk_t ramdisk_data[];

unsigned char *ramdisk_load (const char *filename, int32_t *outlen)
{
    ramdisk_t *ramfile = ramdisk_data;
    unsigned long outlenl;
    unsigned char *out;
    char *alt_filename;

    alt_filename = 0;

    /*
     * If the file is on disk and is newer than the program, use that in
     * preference.
     */
    if (file_exists(filename)) {
        if (file_exists_and_is_newer_than(filename,
                                          EXEC_FULL_PATH_AND_NAME)) {
            out = file_read_if_exists(filename, outlen);
            if (out) {
                DBG("Locdisk %s", filename);
                return (out);
            }
        }

        if (file_exists_and_is_newer_than(filename, ".o/ramdisk_data.o")) {
            out = file_read_if_exists(filename, outlen);
            if (out) {
                DBG("Locdisk %s", filename);
                return (out);
            }
        }

        if (file_exists_and_is_newer_than(filename, "src/.o/ramdisk_data.o")) {
            out = file_read_if_exists(filename, outlen);
            if (out) {
                DBG("Locdisk %s", filename);
                return (out);
            }
        }
    }

    if (EXEC_DIR) {
        alt_filename = strprepend(filename, EXEC_DIR);

        if (file_exists(alt_filename)) {
            if (file_exists_and_is_newer_than(alt_filename,
                                            EXEC_FULL_PATH_AND_NAME)) {
                out = file_read_if_exists(alt_filename, outlen);
                if (out) {
                    DBG("Locdisk %s", filename);
                    myfree(alt_filename);

                    return (out);
                }
            }

            if (file_exists_and_is_newer_than(alt_filename,
                                            ".o/ramdisk_data.o")) {
                out = file_read_if_exists(alt_filename, outlen);
                if (out) {
                    DBG("Locdisk %s", filename);
                    myfree(alt_filename);

                    return (out);
                }
            }

            if (file_exists_and_is_newer_than(alt_filename,
                                            "src/.o/ramdisk_data.o")) {
                out = file_read_if_exists(alt_filename, outlen);
                if (out) {
                    DBG("Locdisk %s", filename);
                    myfree(alt_filename);

                    return (out);
                }
            }
        }
    }

    while (ramfile->filename) {
        if (strcmp(ramfile->filename, filename)) {
            ramfile++;
            continue;
        }

#ifdef USE_ZLIB
        int32_t err;

        out = mymalloc(ramfile->orig_len);
        if (!out) {
            DIE("no memory for ramdisk, %s", filename);
        }

        outlenl = ramfile->orig_len;
        err = uncompress(out, &outlenl,
                         (unsigned char *)ramfile->data,
                         ramfile->len);
        if (err) {
            DIE("file failed to decompress from ramdisk, %s, "
                "orig len %d, compressed len %d, err %d",
                filename, ramfile->orig_len, ramfile->len, err);
        }

        if (outlen) {
            *outlen = (int)outlenl;
        }
#endif

#ifdef USE_MINIZ
        int32_t err;

        out = (typeof(out)) mymalloc(ramfile->orig_len, "RAMDISK scratchpad");
        if (!out) {
            DIE("no memory for ramdisk, %s", filename);
        }

        outlenl = ramfile->orig_len;
        err = mz_uncompress(out, &outlenl,
                            (unsigned char *)ramfile->data,
                            ramfile->len);
        if (err) {
            DIE("file failed to decompress from ramdisk, %s, err %d",
                filename, err);
        }

        if (outlen) {
            *outlen = (int32_t)outlenl;
        }
#endif

#ifdef USE_STB_IMAGE
        out = (typeof(out))
            stbi_zlib_decode_malloc((const char *)ramfile->data,
                                    ramfile->len, &outlenl);
        if (!out) {
            DIE("file failed to decompress from ramdisk, %s", filename);
        }

        if (outlen) {
            *outlen = (int)outlenl;
        }
#endif

        DBG("Ramdisk %s, %d->%lu bytes", filename, ramfile->len, outlenl);

        if (alt_filename) {
            myfree(alt_filename);
        }

        return (out);
    }

    /*
     * Fallback to the disk.
     */
    out = file_read_if_exists(filename, outlen);
    if (out) {
        DBG("Locdisk %s", filename);

        if (alt_filename) {
            myfree(alt_filename);
        }

        return (out);
    }

    out = file_read_if_exists(alt_filename, outlen);
    if (out) {
        DBG("Locdisk %s", filename);

        if (alt_filename) {
            myfree(alt_filename);
        }

        return (out);
    }

    char *tmp = dupstr(filename, "strip dir");
    alt_filename = dupstr(basename(tmp), "strip dir");
    myfree(tmp);

    out = file_read_if_exists(alt_filename, outlen);
    if (out) {
        DBG("Locdisk %s", alt_filename);

        if (alt_filename) {
            myfree(alt_filename);
        }

        return (out);
    }

    /*
     * Fail. Caller should whinge.
     *
    char *popup_str = dynprintf("Filename was not found on ramdisk or "
                                "on the local disk, %s", filename);

    (void) wid_popup_error(popup_str, 0.5f, 0.5f);
    myfree(popup_str);
     */
    ERR("Filename was not found on ramdisk or "
        "on the local disk, %s", filename);

    if (alt_filename) {
        myfree(alt_filename);
    }

    return (0);
}
