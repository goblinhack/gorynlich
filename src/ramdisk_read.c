/*
 * Copyright(C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#include <libgen.h>
#include "string_util.h"

#define nUSE_ZLIB
#define nUSE_MINIZ
#define nUSE_STB_IMAGE

#ifdef USE_ZLIB
#include <stdlib.h>
#include <zlib.h>
#endif

#ifdef USE_MINIZ
#include "miniz.h"
#endif

#ifdef USE_STB_IMAGE
#endif

#include "main.h"
#include "file.h"
#include "string.h"

#include "ramdisk_data.c"

extern ramdisk_t ramdisk_data[];

unsigned char *ramdisk_load (const char *filename, int32_t *outlen)
{
    ramdisk_t *ramfile = ramdisk_data;
    unsigned char *out;
    char *alt_filename;

    alt_filename = 0;

    /*
     * If the file is on disk and is newer than the program, use that in
     * preference.
     */
    if (file_exists(filename)) {
        if (strstr(filename, "data/")) {
            if (file_exists_and_is_newer_than(filename,
                                              EXEC_FULL_PATH_AND_NAME)) {
                out = file_read_if_exists(filename, outlen);
                if (out) {
                    DBG("Locdisk %s (newer than exec)", filename);
                    return (out);
                }
            }

            if (file_exists_and_is_newer_than(filename, ".o/ramdisk_data.o")) {
                out = file_read_if_exists(filename, outlen);
                if (out) {
                    DBG("Locdisk %s (newer than build)", filename);
                    return (out);
                }
            }

            if (file_exists_and_is_newer_than(filename, "src/.o/ramdisk_data.o")) {
                out = file_read_if_exists(filename, outlen);
                if (out) {
                    DBG("Locdisk %s (newer than src build)", filename);
                    return (out);
                }
            }
        } else {
            out = file_read_if_exists(filename, outlen);
            if (out) {
                DBG("Locdisk %s (exists locally)", filename);
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

        if (outlen) {
            *outlen = (int)ramfile->len;
        }

        DBG("Ramdisk %s, %d bytes", filename, ramfile->len);

        if (alt_filename) {
            myfree(alt_filename);
        }

        uint8_t *copy = (typeof(copy)) 
                        mymalloc((int)ramfile->len + 1, "ramdisk load");
        if (!copy) {
            DBG("no memory for loading ramdisk copy, %s", filename);
            return (0);
        }

        memcpy(copy, (unsigned char*)ramfile->data, (int)ramfile->len);
        *(copy + (int)ramfile->len) = 0;

        return (copy);

#ifdef USE_ZLIB
        int32_t err;

        out = mymalloc(ramfile->orig_len, "RAMDISK scratchpad");
        if (!out) {
            ERR("no memory for ramdisk, %s", filename);
        }

        outlenl = ramfile->orig_len;
        err = uncompress(out, &outlenl,
                         (unsigned char *)ramfile->data,
                         ramfile->len);
        if (err) {
            ERR("file failed to decompress from ramdisk, %s, "
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
            ERR("no memory for ramdisk, %s", filename);
        }

        outlenl = ramfile->orig_len;
        err = mz_uncompress(out, &outlenl,
                            (unsigned char *)ramfile->data,
                            ramfile->len);
        if (err) {
            ERR("file failed to decompress from ramdisk, %s, err %d",
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
            ERR("file failed to decompress from ramdisk, %s", filename);
        }

        if (outlen) {
            *outlen = (int)outlenl;
        }
#endif

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

    if (alt_filename) {
        myfree(alt_filename);
    }

    alt_filename = mybasename(filename, "strip dir");

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

    MSG_BOX("%s", popup_str);
    myfree(popup_str);
     */
    LOG("File not found \"%s\"", filename);

    if (alt_filename) {
        myfree(alt_filename);
    }

    return (0);
}

ramdisk_t *ramdisk_get_data (void)
{
    return (ramdisk_data);
}
