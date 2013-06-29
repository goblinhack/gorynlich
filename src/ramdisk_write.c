/*
 * Copyright(C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "zlib.h"

#define CHUNK 16384

#include "ramdisk.h"
#include "ramdisk_files.c"
#include "main.h"
#include "mzip_file.h"
#include "file.h"

static unsigned char out[1024 * 1024 * 64];
static unsigned long outsz;
static unsigned long insz;

/*
 * Compress a file, writes to "out"
 */
static int
docompress (const char *name, FILE *f, int level)
{
#ifdef USE_ZLIB
    int ret, flush;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char tmp[CHUNK];
    unsigned char *outp;

    /* allocate deflate state */
    strm.zalloc = 0;
    strm.zfree = 0;
    strm.opaque = 0;
    ret = deflateInit(&strm, level);
    outp = out;
    outsz = 0;
    insz = 0;

    if (ret != Z_OK) {
        return (ret);
    }

    /* compress until end of file */
    do {
        strm.avail_in = (unsigned int)fread(in, 1, CHUNK, f);
        insz += strm.avail_in;
        if (ferror(f)) {
            (void)deflateEnd(&strm);
            return (Z_ERRNO);
        }

        flush = feof(f) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

        /* run deflate() on input until output buffer not full, finish
           compression if all of f has been read in */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = tmp;
            ret = deflate(&strm, flush);    /* no bad return value */
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_out;

            memcpy(outp, tmp, have);
            outsz += have;
            outp += have;

        } while (strm.avail_out == 0);

        assert(strm.avail_in == 0);     /* all input will be used */

        /* done when last data in file processed */
    } while (flush != Z_FINISH);

    assert(ret == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    (void)deflateEnd(&strm);
#else
    char tmp[MAXSTR];
    uint8_t *data;
    int32_t len;

    /*
     * Read uncompressed.
     */
    data = file_read(name, &len);
    if (!data) {
        return (!Z_OK);
    }

    insz = len;

    snprintf(tmp, sizeof(tmp), "%s.mzip", name);

    /*
     * Write compressed.
     */
#ifdef ENABLE_COMPRESSED_RAMDISK
    mzip_file_write2(tmp, data, &len, level);
    outsz = len;

    /*
     * Read compressed.
     */
    uint8_t *data2 = file_read(tmp, &len);
    memcpy(out, data2, len);

    /*
     * Read compressed and decompress and check.
     */
    uint8_t *data3;
    data3 = mzip_file_read(tmp, &len);
    if (memcmp(data3, data, len)) {
        printf("file differs\n");
        exit(1);
    }

    file_unlink(tmp);

    printf("\n");
#else
    memcpy(out, data, len);
    outsz = len;
#endif

#endif
    return (Z_OK);
}

/*
 * Uncompress memory, in to out.
 */
static int
douncompress (unsigned char *in, unsigned long in_len,
              unsigned char *out, unsigned long out_len)
{
#ifdef ENABLE_COMPRESSED_RAMDISK
    int ret;
    z_stream strm;

    /* allocate inflate state */
    strm.zalloc = 0;
    strm.zfree = 0;
    strm.opaque = 0;
    strm.avail_in = 0;
    strm.next_in = 0;
    ret = inflateInit(&strm);
    if (ret != Z_OK) {
        return (ret);
    }

    strm.avail_in = (unsigned int)in_len;
    strm.next_in = in;
    strm.avail_out = (unsigned int)out_len;
    strm.next_out = out;

    ret = inflate(&strm, Z_NO_FLUSH);
    assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
    switch (ret) {
    case Z_NEED_DICT:
        ret = Z_DATA_ERROR;     /* and fall through */
    case Z_DATA_ERROR:
    case Z_MEM_ERROR:
        (void)inflateEnd(&strm);
        return (ret);
    }

    /* clean up and return */
    (void)inflateEnd(&strm);

    return (ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR);
#else
    memcpy(out, in, in_len);

    return (Z_OK);
#endif
}

/*
 * Replace chars in replace_set with replace_with.
 */
static void
strrepc (char *s, const char *replace_set, char replace_with)
{
    char *c;

    for (c = s; *c; c++) {
        if (strchr(replace_set, *c)) {
            *c = replace_with;
        }
    }
}

int main (int32_t argc, char *argv[])
{
    ramdisk_t *ramfile;
    char *c_array_name;
    unsigned int i;
    FILE *f;
    int ret;
    FILE *fp;
    uint32_t frag;
    char tmp[200];
    uint32_t size;

    size = 0;
    frag = 0;
    snprintf(tmp, sizeof(tmp), "src/ramdisk_data_fragment_%u.c", frag);
    frag++;

    fp = fopen(tmp, "w");
    if (!fp) {
        fprintf(stderr, "could not write to %s\n", tmp);
        exit(1);
    }

    fprintf(fp, "#include <stdint.h>\n");
    fprintf(fp, "#include \"ramdisk.h\"\n\n");

    ramfile = ramdisk_files;
    while (ramfile->filename) {

        f = fopen(ramfile->filename, "r");
        if (!f) {
            fprintf(stderr, "could not open %s\n", ramfile->filename);
            exit(1);
        }

        /* avoid end-of-line conversions */
        SET_BINARY_MODE(f);

        ret = docompress(ramfile->filename, f, 1);
        if (ret != Z_OK) {
            fprintf(stderr, "could not compress %s\n", ramfile->filename);
            exit(1);
        }

        fclose(f);

        fprintf(fp, "/* Compressed %s from %lu to %lu bytes */\n",
               ramfile->filename, insz, outsz);

        c_array_name = strdup(ramfile->filename);

        /*
         * Turn the filename into a safe C var name.
         */
        strrepc(c_array_name, "/.", '_');

        fprintf(fp, "unsigned char %s[] = {\n", c_array_name);

        free(c_array_name);

        fprintf(fp, "    ");

        unsigned char *buf = (typeof(buf)) malloc(insz);
        if (!buf) {
            fprintf(stderr, "uncompress buf\n");
            exit(1);
        }

        if (douncompress(out, outsz, buf, insz)) {
            fprintf(stderr, "uncompress sanity fail\n");
            exit(1);
        }

#ifdef ENABLE_COMPRESSED_RAMDISK
        if (uncompress(buf, &insz, out, outsz)) {
            fprintf(stderr, "zlib uncompress sanity fail\n");
            exit(1);
        }
#endif

        free(buf);

        for (i = 0; i < outsz; i++) {
            size++;
            fprintf(fp, "%u,", out[i]);
        }

        fprintf(fp, "\n};\n\n");

        ramfile++;

        if (size > 1 * 1024 * 1024) {
            size = 0;

            fclose(fp);

            snprintf(tmp, sizeof(tmp), "src/ramdisk_data_fragment_%u.c", frag);
            frag++;

            fp = fopen(tmp, "w");
            if (!fp) {
                fprintf(stderr, "could not write to %s\n", tmp);
                exit(1);
            }
        }
    }

    fclose(fp);

    snprintf(tmp, sizeof(tmp), "src/ramdisk_data_fragment_final.c");

    fp = fopen(tmp, "w");
    if (!fp) {
        fprintf(stderr, "could not write to %s\n", tmp);
        exit(1);
    }

    fprintf(fp, "#include <stdint.h>\n");
    fprintf(fp, "#include \"ramdisk.h\"\n\n");

    ramfile = ramdisk_files;
    while (ramfile->filename) {

        f = fopen(ramfile->filename, "r");
        if (!f) {
            fprintf(stderr, "could not open %s\n", ramfile->filename);
            exit(1);
        }

        /* avoid end-of-line conversions */
        SET_BINARY_MODE(f);

        ret = docompress(ramfile->filename, f, Z_BEST_COMPRESSION);
        if (ret != Z_OK) {
            fprintf(stderr, "could not compress %s\n", ramfile->filename);
            exit(1);
        }

        fclose(f);

        c_array_name = strdup(ramfile->filename);

        /*
         * Turn the filename into a safe C var name.
         */
        strrepc(c_array_name, "/.", '_');

        free(c_array_name);

        fprintf(fp, "extern const unsigned char %s[];\n", c_array_name);

        ramfile++;
    }

    fprintf(fp, "\nramdisk_t ramdisk_data[] = {\n");

    ramfile = ramdisk_files;
    while (ramfile->filename) {

        f = fopen(ramfile->filename, "r");
        if (!f) {
            fprintf(stderr, "could not open %s\n", ramfile->filename);
            exit(1);
        }

        /* avoid end-of-line conversions */
        SET_BINARY_MODE(f);

        ret = docompress(ramfile->filename, f, Z_BEST_COMPRESSION);
        if (ret != Z_OK) {
            fprintf(stderr, "could not compress %s\n", ramfile->filename);
            exit(1);
        }

        fclose(f);

        fprintf(fp, "    {\n");
        fprintf(fp, "        /* filename */ \"%s\",\n", ramfile->filename);
        fprintf(fp, "        /* data     */ 0,\n");
        fprintf(fp, "        /* orig_len */ %lu,\n", insz);
        fprintf(fp, "        /* len      */ %lu,\n", outsz);
#ifdef ENABLE_COMPRESSED_RAMDISK
        fprintf(fp, "        /* uncomprs */ 0,\n");
#else
        fprintf(fp, "        /* uncomprs */ 1,\n");
#endif
        fprintf(fp, "    },\n");

        ramfile++;
    }

    fprintf(fp, "    {0},\n");
    fprintf(fp, "};\n");

    fprintf(fp, "\n\nvoid ramdisk_init (void)\n{\n");

    int cnt = 0;

    ramfile = ramdisk_files;
    while (ramfile->filename) {

        c_array_name = strdup(ramfile->filename);

        /*
         * Turn the filename into a safe C var name.
         */
        strrepc(c_array_name, "/.", '_');

        free(c_array_name);

        fprintf(fp, "    ramdisk_data[%u].data = %s;\n", cnt, c_array_name);
        cnt++;

        ramfile++;
    }

    fprintf(fp, "}\n");

    exit(0);
}
