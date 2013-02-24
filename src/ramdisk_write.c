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

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

#include "zlib.h"

#define CHUNK 16384

#include "ramdisk.h"
#include "ramdisk_files.c"

static unsigned char out[1024 * 1024 * 16];
static unsigned long outsz;
static unsigned long insz;

/*
 * Compress a file, writes to "out"
 */
static int
docompress (FILE *f, int level)
{
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
    return (Z_OK);
}

/*
 * Uncompress memory, in to out.
 */
static int
douncompress (unsigned char *in, unsigned long in_len,
              unsigned char *out, unsigned long out_len)
{
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

    printf("#include <stdint.h>\n");
    printf("#include \"ramdisk.h\"\n\n");

    ramfile = ramdisk_files;
    while (ramfile->filename) {

        f = fopen(ramfile->filename, "r");
        if (!f) {
            fprintf(stderr, "could not open %s\n", ramfile->filename);
            exit(1);
        }

        /* avoid end-of-line conversions */
        SET_BINARY_MODE(f);

        ret = docompress(f, Z_BEST_COMPRESSION);
        if (ret != Z_OK) {
            fprintf(stderr, "could not compress %s\n", ramfile->filename);
            exit(1);
        }

        fclose(f);

        printf("/* Compressed %s from %lu to %lu bytes */\n",
               ramfile->filename, insz, outsz);

        c_array_name = strdup(ramfile->filename);

        /*
         * Turn the filename into a safe C var name.
         */
        strrepc(c_array_name, "/.", '_');

        printf("unsigned char %s[] = {\n", c_array_name);

        free(c_array_name);

        printf("    ");

        unsigned char *buf = (typeof(buf)) malloc(insz);
        if (!buf) {
            fprintf(stderr, "uncompress buf\n");
            exit(1);
        }

        if (douncompress(out, outsz, buf, insz)) {
            fprintf(stderr, "uncompress sanity fail\n");
            exit(1);
        }

        if (uncompress(buf, &insz, out, outsz)) {
            fprintf(stderr, "zlib uncompress sanity fail\n");
            exit(1);
        }

        free(buf);

        for (i = 0; i < outsz; i++) {
            printf("%u,", out[i]);
        }

        printf("\n};\n\n");

        ramfile++;
    }

    printf("ramdisk_t ramdisk_data[] = {\n");

    ramfile = ramdisk_files;
    while (ramfile->filename) {

        f = fopen(ramfile->filename, "r");
        if (!f) {
            fprintf(stderr, "could not open %s\n", ramfile->filename);
            exit(1);
        }

        /* avoid end-of-line conversions */
        SET_BINARY_MODE(f);

        ret = docompress(f, Z_BEST_COMPRESSION);
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

        printf("    {\n");
        printf("        /* filename */ \"%s\",\n", ramfile->filename);
        printf("        /* data     */ %s,\n", c_array_name);
        printf("        /* orig_len */ %lu,\n", insz);
        printf("        /* len      */ %lu,\n", outsz);
        printf("    },\n");

        ramfile++;
    }

    printf("    {0},\n");
    printf("};\n");

    exit(0);
}
