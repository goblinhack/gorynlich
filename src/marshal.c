/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 *
 * Routines to allow the dumping of structures to streams, files and then
 * reading back in from a (zipped) file. e.g format:
 *
 *    a=1 c=24 b="hello" d={
 *     a=12 b={
 *      a="long string" b=[ {
 *        a="string" b=[ 1 2 3 ]
 *       } {
 *        a="long string" b=[ 4 5 6 ]
 *       } {
 *        a="longer string" b=[ 7 8 9 ]
 *       } ]
 *     }
 *
 * The aim is to be *fast* at the expense of robustness.
 */

#include <string.h>
#include <math.h>
#include <stdio.h>

#include "main.h"
#include "file.h"
#include "mzip_file.h"
#include "marshal.h"
#include "string_ext.h"

typedef struct tree_marshal_ {
    char *filename;
    char *buf;
    int32_t buf_used;
    int32_t buf_size;
    int32_t indent;
} tree_marshal;

tree_marshal *marshal (const char *filename)
{
    tree_marshal *ctx;

    ctx = (typeof(ctx)) myzalloc(sizeof(*ctx), "marshal ctr");

    if (filename) {
        ctx->filename = dupstr(filename, "marshal name");
    } else {
        ctx->filename = 0;
    }

    return (ctx);
}

boolean marshal_name (tree_marshal *ctx, const char *name)
{
    snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                     "\n%*s", ctx->indent, "");

    return (snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                             "%s=", name));
}

boolean marshal_name_only (tree_marshal *ctx, const char *name)
{
    snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                     "\n%*s", ctx->indent, "");

    return (snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                             "%s", name));
}

boolean marshal_string (tree_marshal *ctx, const char *out)
{
    return (snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                             "\"%s\" ", out));
}

boolean marshal_named_string (tree_marshal *ctx, const char *name,
                          const char *out)
{
    snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                     "\n%*s", ctx->indent, "");

    return (snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                             "%s=\"%s\" ", name, out));
}

boolean marshal_int8 (tree_marshal *ctx, int8_t out)
{
    return (snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                             "%d ", out));
}

boolean marshal_named_int8 (tree_marshal *ctx, const char *name, int8_t out)
{
    snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                     "\n%*s", ctx->indent, "");

    return (snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                             "%s=%d ", name, out));
}

boolean marshal_int16 (tree_marshal *ctx, int16_t out)
{
    return (snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                             "%d ", out));
}

boolean marshal_named_int16 (tree_marshal *ctx, const char *name, int16_t out)
{
    snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                     "\n%*s", ctx->indent, "");

    return (snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                             "%s=%d ", name, out));
}

boolean marshal_int32 (tree_marshal *ctx, int32_t out)
{
    return (snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                             "%d ", out));
}

boolean marshal_named_int32 (tree_marshal *ctx, const char *name, int32_t out)
{
    snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                     "\n%*s", ctx->indent, "");

    return (snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                             "%s=%d ", name, out));
}

boolean marshal_int64 (tree_marshal *ctx, int64_t out)
{
    return (snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                             "%lld ", out));
}

boolean marshal_named_int64 (tree_marshal *ctx, const char *name, int64_t out)
{
    snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                     "\n%*s", ctx->indent, "");

    return (snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                             "%s=%ld ", name, out));
}

boolean marshal_uint8 (tree_marshal *ctx, uint8_t out)
{
    return (snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                             "%u ", out));
}

boolean marshal_named_uint8 (tree_marshal *ctx, const char *name, uint8_t out)
{
    snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                     "\n%*s", ctx->indent, "");

    return (snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                             "%s=%u ", name, out));
}

boolean marshal_uint16 (tree_marshal *ctx, uint16_t out)
{
    return (snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                             "%u ", out));
}

boolean marshal_named_uint16 (tree_marshal *ctx, const char *name, uint16_t out)
{
    snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                     "\n%*s", ctx->indent, "");

    return (snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                             "%s=%u ", name, out));
}

boolean marshal_uint32 (tree_marshal *ctx, uint32_t out)
{
    return (snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                             "%lu ", out));
}

boolean marshal_named_uint32 (tree_marshal *ctx, const char *name, uint32_t out)
{
    snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                     "\n%*s", ctx->indent, "");

    return (snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                             "%s=%lu ", name, out));
}

boolean marshal_uint64 (tree_marshal *ctx, uint64_t out)
{
    return (snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                             "%llu ", out));
}

boolean marshal_named_uint64 (tree_marshal *ctx, const char *name, uint64_t out)
{
    snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                     "\n%*s", ctx->indent, "");

    return (snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                             "%s=%llu ", name, out));
}

boolean marshal_float (tree_marshal *ctx, float out)
{
    if (isnan(out) || isinf(out)) {
        out = 0.0f;
    }

    return (snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                             "%f ", out));
}

boolean marshal_named_float (tree_marshal *ctx, const char *name, float out)
{
    snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                     "\n%*s", ctx->indent, "");

    if (isnan(out) || isinf(out)) {
        out = 0.0f;
    }

    return (snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                             "%s=%f ", name, out));
}

boolean marshal_named_ptr (tree_marshal *ctx, const char *name,
                       void *out)
{
    snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                     "\n%*s", ctx->indent, "");

    return (snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                             "%s=*%p ", name, out));
}

boolean marshal_named_ptr_ref (tree_marshal *ctx, const char *name,
                           void *out)
{
    snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                     "\n%*s", ctx->indent, "");

    return (snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                             "%s=@%p ", name, out));
}

boolean marshal_bra (tree_marshal *ctx)
{
    snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used, "{");
    ctx->indent++;

    return (snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                             "%*s", ctx->indent, ""));
}

boolean marshal_ket (tree_marshal *ctx)
{
    snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used, "\n");

    ctx->indent--;
    snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used,
                     "%*s", ctx->indent, "");

    return (snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used, "} "));
}

boolean marshal_array_bra (tree_marshal *ctx)
{
    return (snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used, "[ "));
}

boolean marshal_array_ket (tree_marshal *ctx)
{
    return (snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used, "] "));
}

boolean marshal_fini (tree_marshal *ctx)
{
    char *buf;
    boolean rc;

    if (ctx->buf_used) {
        ctx->buf_used--; /* to remote trailing space */
        snprintf_realloc(&ctx->buf, &ctx->buf_size, &ctx->buf_used, "\n");
    }

    rc = 0;
    buf = ctx->buf;
    if (buf) {
        if (ctx->buf_used && (*ctx->buf == '\n')) {
            ctx->buf++; /* remove leading \n */
            ctx->buf_used--;
        }

        if (!ctx->filename) {
            printf("%s", ctx->buf);
        } else if (strstr(ctx->filename, ".mz")) {
            rc = mzip_file_write(ctx->filename, (unsigned char*)ctx->buf,
                                 &ctx->buf_used);
            myfree(ctx->filename);
        } else {
            rc = file_write(ctx->filename, (unsigned char*)ctx->buf,
                            ctx->buf_used);
            myfree(ctx->filename);
        }

        myfree(buf);
    }

    myfree(ctx);

    return (rc);
}
