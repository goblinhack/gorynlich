/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <libgen.h>

#include "main.h"
#include "string_util.h"

// #define HEX_DUMP_WIDTH (MAXSTR / 2)
#define HEX_DUMP_WIDTH (16)

/*
 * Replace chars in replace_set with replace_with.
 */
void
strrepc (char *s, const char *replace_set, char replace_with)
{
    char *c;

    for (c = s; *c; c++) {
        if (strchr(replace_set, *c)) {
            *c = replace_with;
        }
    }
}

/*
 * return a part of a string.
 *
 * substr("foo.zip", -4, 4) -> "zip"
 */
char *substr (const char *in, int32_t pos, int32_t len)
{
    int32_t slen;
    char *out;

    if (!in) {
        return (0);
    }

    slen = (uint32_t)strlen(in);

    /*
     * Negative positions from the end.
     */
    if (pos < 0) {
        pos = slen + pos;
    }

    if (pos < 0) {
        pos = 0;
    }

    if (len < 0) {
        len = 0;
    }

    if (pos > slen) {
        pos = slen;
    }

    if (len > slen - pos) {
        len = slen - pos;
    }

    out = (typeof(out)) mymalloc(len + sizeof((char)'\0'), "substr");
    if (!out) {
        return (0);
    }

    memcpy(out, &(in[pos]), len);
    out[len] = '\0';

    return (out);
}

/*
 * Replace part of a string with another.
 *
 * strsub("foo.zip", ".zip", ""); -> "foo"
 */
char *strsub (const char *in, const char *old, const char *replace_with)
{
    char *buf;
    const char *at;
    int32_t newlen;
    int32_t oldlen;
    int32_t len;

    if (!in || !old || !replace_with) {
        return (0);
    }

    at = strstr(in, old);
    if (!at) {
        buf = dupstr(in, __FUNCTION__);
        return (buf);
    }

    oldlen = (uint32_t)strlen(old);
    newlen = (uint32_t)strlen(replace_with);

    len = (uint32_t)strlen(in) - oldlen + newlen;
    buf = (typeof(buf)) myzalloc(len + sizeof((char)'\0'), "strsub 2");
    if (!buf) {
        return (0);
    }

    *buf = '\0';
    strlcpy(buf, in, at - in + 1);
    strcat(buf, replace_with);
    strcat(buf, at + oldlen);

    return (buf);
}

/*
 * Add onto the end of a string.
 *
 * strsub("foo", ".zip"); -> "foo.zip"
 */
char *strappend (const char *in, const char *append)
{
    char *buf;
    int32_t newlen;
    int32_t len;

    if (!in || !append) {
        return (0);
    }

    newlen = (uint32_t)strlen(append);
    len = (uint32_t)strlen(in) + newlen;
    buf = (typeof(buf)) myzalloc(len + sizeof((char)'\0'), "strappend");
    if (!buf) {
        return (0);
    }

    strcpy(buf, in);
    strcat(buf, append);

    return (buf);
}

/*
 * Add onto the start of a string.
 *
 * strsub("foo", "bar"); -> "barfoo"
 */
char *strprepend (const char *in, const char *prepend)
{
    char *buf;
    int32_t newlen;
    int32_t len;

    if (!in || !prepend) {
        return (0);
    }

    newlen = (uint32_t)strlen(prepend);
    len = (uint32_t)strlen(in) + newlen;
    buf = (typeof(buf)) myzalloc(len + sizeof((char)'\0'), "strprepend");
    if (!buf) {
        return (0);
    }

    strcpy(buf, prepend);
    strcat(buf, in);

    return (buf);
}

/*
 * Return the common matching length of two strings.
 */
uint32_t
strcommon (const char *a, const char *b)
{
    const char *o;

    o = a;

    while ((*a == *b) && *a) { a++; b++; }

    return (uint32_t)(a - o);
}

/*
 * Removes trailing whitespace.
 */
void strchop (char *s)
{
    uint32_t size;
    char *end;

    size = (uint32_t)strlen(s);
    if (!size) {
        return;
    }

    end = s + size - 1;
    while ((end >= s) && (*end == ' ')) {
        end--;
    }

    *(end + 1) = '\0';
}

/*
 * Removes trailing characters.
 */
void strchopc (char *s, char c)
{
    uint32_t size;
    char *end;

    size = (uint32_t)strlen(s);
    if (!size) {
        return;
    }

    end = s + size - 1;
    while ((end >= s) && (*end == c)) {
        end--;
    }

    *(end + 1) = '\0';
}

int32_t strisregexp (const char *in)
{
    const char *a = in;
    char c;

    while ((c = *a++)) {
        switch (c) {
        case '[': return (1);
        case ']': return (1);
        case '{': return (1);
        case '}': return (1);
        case '+': return (1);
        case '$': return (1);
        case '^': return (1);
        case '-': return (1);
        case '*': return (1);
        }
    }

    return (false);
}

#if defined(__MINGW32__) || defined(__linux__)
uint32_t strlcpy (char *dst, const char *src, uint32_t maxlen)
{
    uint32_t srclen = strlen(src);
    if ( maxlen > 0 ) {
        uint32_t len = min(srclen, maxlen-1);
        memcpy(dst, src, len);
        dst[len] = '\0';
    }
    return (srclen);
}

uint32_t strlcat (char *dst, const char *src, uint32_t maxlen)
{
    uint32_t dstlen = strlen(dst);
    uint32_t srclen = strlen(src);
    if ( dstlen < maxlen ) {
        strlcpy(dst+dstlen, src, maxlen-dstlen);
    }
    return (dstlen+srclen);
}
#endif

/*
 * Shrink a string by the space needed for its escape characters.
 */
void strnoescape (char *uncompressed)
{
    char * t = uncompressed;
    char * s = uncompressed;
    char c;

    while ((c = *s++)) {
        if (c != '\\') {
            *t++ = c;
            continue;
        }

        switch (*s++) {
            case 'n':
                *t++ = '\n';
                break;
            case 't':
                *t++ = '\t';
                break;
            case '%':
                *t++ = '%';
                break;
            case '#':
                *t++ = '@';
                break;
            case '\"':
                *t++ = '\"';
                break;
            case '\'':
                *t++ = '\'';
                break;
            case '\\':
                *t++ = '\\';
                break;
            default: {
                char *o = s - 1;
                char orig;

                while (isdigit(*s)) { s++; }

                orig = *s;
                *s = '\0';
                *t++ = atoi(o);
                *s = orig;
                break;
            }
        }
    }
    *t = '\0';
}

static const char *dynvprintf_ (const char *fmt, va_list args)
{
    static char buf[MAXSTR];

    buf[0] = '\0';
    vsnprintf(buf, sizeof(buf), fmt, args);

    return (buf);
}

/*
 * dynamically allocate a printf formatted string. e.g.
 * dynprintf("%sbar", "foo"); => "foobar"
 */
char *dynprintf (const char *fmt, ...)
{
    const char *ret;
    va_list args;

    va_start(args, fmt);
    ret = dynvprintf_(fmt, args);
    va_end(args);

    return (dupstr(ret, __FUNCTION__));
}

char *dynvprintf (const char *fmt, va_list args)
{
    static char buf[MAXSTR];

    buf[0] = '\0';
    vsnprintf(buf, sizeof(buf), fmt, args);

    return (dupstr(buf, __FUNCTION__));
}

/*
 * mybasename
 *
 * A safe wrapper for basename to avoid modifications to the input string.
 */
char *mybasename (const char *in, const char *who)
{
    char *tmp = dupstr(in, who);
    char *tmp2 = dupstr(basename(tmp), who);
    myfree(tmp);

    return (tmp2);
}

/*
 * hex_dump
 *
 * Returns false if we finished with an empty block.
 */
uint8_t hex_dump (void *addr, uint64_t offset, uint64_t len)
{
    uint8_t skipping_blanks = false;
    uint8_t empty[HEX_DUMP_WIDTH] = {0};
    uint8_t buf[HEX_DUMP_WIDTH + 1];
    uint8_t *pc = (typeof(pc)) addr;
    uint64_t i;
    uint32_t x;

    if (!len) {
        return (false);
    }

    for (i = 0, x = 0; i < len; i++, x++) {
        if ((i % HEX_DUMP_WIDTH) == 0) {
            if (!skipping_blanks) {
                if (i != 0) {
                    printf(" |%*s|\n", HEX_DUMP_WIDTH, buf);
                }
            }

            /*
             * Skip blank blocks.
             */
            if (!memcmp(pc + i, empty, sizeof(empty))) {
                i += HEX_DUMP_WIDTH - 1;
                skipping_blanks = true;
                buf[0] = '\0';
                continue;
            }

            printf("  %08X ", (uint32_t)(i + offset));

            x = 0;
        }

        if (x && (((i % (HEX_DUMP_WIDTH/2))) == 0)) {
            printf(" ");
        }

        skipping_blanks = false;

        printf(" %02X", pc[i]);

        if ((pc[i] < ' ') || (pc[i] > '~')) {
            buf[i % HEX_DUMP_WIDTH] = '.';
        } else {
            buf[i % HEX_DUMP_WIDTH] = pc[i];
        }

        buf[(i % HEX_DUMP_WIDTH) + 1] = '\0';
    }

    if (!buf[0]) {
        if (skipping_blanks) {
            printf("  *\n");
        }

        return (false);
    }

    while ((i % HEX_DUMP_WIDTH) != 0) {
        printf ("   ");

        if (i && (((i % (HEX_DUMP_WIDTH/2))) == 0)) {
            printf (" ");
        }

        i++;
    }

    printf(" |%*s|\n", -HEX_DUMP_WIDTH, buf);

    return (true);
}

/*
 * hex_dump_log
 *
 * As above but logs to the buffer
 */
uint8_t hex_dump_log (void *addr, uint64_t offset, uint64_t len)
{
    uint8_t skipping_blanks = false;
    uint8_t empty[HEX_DUMP_WIDTH] = {0};
    uint8_t buf[HEX_DUMP_WIDTH + 1];
    uint8_t *pc = (typeof(pc)) addr;
    uint64_t i;
    uint32_t x;

    if (!len) {
        return (false);
    }

    for (i = 0, x = 0; i < len; i++, x++) {
        if ((i % HEX_DUMP_WIDTH) == 0) {
            if (!skipping_blanks) {
                if (i != 0) {
                    LOGS(" |%*s|\n", HEX_DUMP_WIDTH, buf);
                }
            }

            /*
             * Skip blank blocks.
             */
            if (!memcmp(pc + i, empty, sizeof(empty))) {
                i += HEX_DUMP_WIDTH - 1;
                skipping_blanks = true;
                buf[0] = '\0';
                continue;
            }

            LOGS("  %08X ", (uint32_t)(i + offset));

            x = 0;
        }

#if 0
        if (x && (((i % (HEX_DUMP_WIDTH/2))) == 0)) {
            LOGS(" ");
        }
#endif

        skipping_blanks = false;

        LOGS(" %02X", pc[i]);

        if ((pc[i] < ' ') || (pc[i] > '~')) {
            buf[i % HEX_DUMP_WIDTH] = '.';
        } else {
            buf[i % HEX_DUMP_WIDTH] = pc[i];
        }

        buf[(i % HEX_DUMP_WIDTH) + 1] = '\0';
    }

    if (!buf[0]) {
        if (skipping_blanks) {
            LOGS("  *\n");
        }

        return (false);
    }

    while ((i % HEX_DUMP_WIDTH) != 0) {
        LOGS ("   ");

        if (i && (((i % (HEX_DUMP_WIDTH/2))) == 0)) {
            LOGS (" ");
        }

        i++;
    }

    LOGS(" |%*s|\n", -HEX_DUMP_WIDTH, buf);

    return (true);
}

/*-
 * Copyright (c) 1990, 1993
 *      The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Find the first occurrence of find in s, ignore case.
 */
char *
strcasestr_ (const char *s, const char*find)
{
        char c, sc;
        size_t len;

        if ((c = *find++) != 0) {
                c = tolower((unsigned char)c);
                len = strlen(find);
                do {
                        do {
                                if ((sc = *s++) == 0)
                                        return (NULL);
                        } while ((char)tolower((unsigned char)sc) != c);
                } while (strncasecmp(s, find, len) != 0);
                s--;
        }
        return ((char *)s);
}
