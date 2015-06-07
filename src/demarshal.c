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

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "main.h"
#include "string_util.h"
#include "marshal.h"
#include "ramdisk.h"

typedef enum {
    MARSHAL_NONE,
    MARSHAL_INT,
    MARSHAL_FLOAT,
    MARSHAL_STRING,
    MARSHAL_NAME,
    MARSHAL_PTR,
    MARSHAL_PTR_REF,
    MARSHAL_BRA,
    MARSHAL_KET,
} demarshal_type;

typedef enum {
    MARSHAL_PARSE_STATE_NONE,
    MARSHAL_PARSE_STATE_INT,
    MARSHAL_PARSE_STATE_HEX,
    MARSHAL_PARSE_STATE_STRING,
    MARSHAL_PARSE_STATE_NAME,
} demarshal_parse_state;

/*
 * Tree of pointers that get remapped.
 */
typedef struct tree_demarshal_ptr_node_ {
    struct tree_demarshal_ptr_key_ {
        tree_node node;
        void *key;
    } tree;
    void **ptr;
} tree_demarshal_ptr_node;

typedef struct tree_demarshal_ptr_ {
    tree_root *root;
    tree_demarshal_ptr_node *node;
} tree_demarshal_ptr;

/*
 * Tree of pointers referring to the above pointers that need to be changed
 * to point to the new pointer.
 */
typedef struct tree_demarshal_ptr_ref_node_ {
    tree_key_int tree;
    void *ptr_ref;
    void **pptr_ref;
} tree_demarshal_ptr_ref_node;

typedef struct tree_demarshal_ptr_ref_ {
    tree_root *root;
    tree_demarshal_ptr_ref_node *node;
} tree_demarshal_ptr_ref;

/*
 * Tree of demarshal nodes.
 */
typedef struct tree_demarshal_node_ {
    tree_key_int tree;

    union {
        float v_float;
        int64_t v_int;
        char *v_string;
        char *v_name;
        void *v_ptr;
        void *v_ptr_ref;
    } val;

    tree_demarshal_ptr_ref_node *ptr_ref_node;
    tree_demarshal_ptr_node *ptr_node;
    int8_t depth;
    int8_t type;
    int16_t line;
} tree_demarshal_node;

typedef struct tree_demarshal_ {
    tree_root *root;
    tree_demarshal_node *node;
    tree_demarshal_ptr ptr_tree;
    tree_demarshal_ptr_ref ptr_ref_tree;
    /*
     * Used during parsing to indicate an optional field was read.
     */
    int8_t gotone;
    int8_t peek;
} tree_demarshal;

static tree_demarshal_ptr_node *
demarshal_ptr_alloc_node (tree_demarshal *ctx, void *ptr)
{
    tree_demarshal_ptr_node *node;

    node = (typeof(node)) myzalloc(sizeof(*node), "demarshal ptr");
    node->tree.key = ptr;

    if (!tree_insert(ctx->ptr_tree.root, &node->tree.node)) {
        ERR("insert ptr node %p fail", ptr);
    }

    return (node);
}

static tree_demarshal_ptr_ref_node *
demarshal_ptr_ref_alloc_node (tree_demarshal *ctx, void *ptr_ref)
{
    static int32_t key;

    tree_demarshal_ptr_ref_node *node;

    node = (typeof(node)) myzalloc(sizeof(*node), "demarshal ptr ref");
    node->tree.key = key++;;
    node->ptr_ref = ptr_ref;

    if (!tree_insert(ctx->ptr_ref_tree.root, &node->tree.node)) {
        ERR("insert ptr ref %p fail", ptr_ref);
    }

    return (node);
}

static const char *demarshal_parse_filename;
static int32_t demarshal_parse_line;
static char *demarshal_buf_end;

static tree_demarshal_node *demarshal_alloc_node (tree_demarshal *ctx,
                                                  const int8_t depth,
                                                  const int8_t type)
{
    static int32_t key;
    tree_demarshal_node *node;

    node = (typeof(node)) myzalloc(sizeof(*node), "demarshal node");
    node->tree.key = key++;
    node->depth = depth;
    node->type = type;
    node->line = demarshal_parse_line;

    if (!tree_insert(ctx->root, &node->tree.node)) {
        ERR("insert demarshal node %d fail", key);
    }

    return (node);
}

static tree_demarshal_node *demarshal_push_float (tree_demarshal *ctx,
                                                  const int8_t depth,
                                                  const float val)
{
    tree_demarshal_node *node;

    node = demarshal_alloc_node(ctx, depth, MARSHAL_FLOAT);
    if (!node) {
        return (0);
    }

    node->val.v_float = val;

    return (node);
}

static tree_demarshal_node *demarshal_push_int64 (tree_demarshal *ctx,
                                                  const int8_t depth,
                                                  const int64_t val)
{
    tree_demarshal_node *node;

    node = demarshal_alloc_node(ctx, depth, MARSHAL_INT);
    if (!node) {
        return (0);
    }

    node->val.v_int = val;

    return (node);
}

static tree_demarshal_node *demarshal_push_ptr (tree_demarshal *ctx,
                                                const int8_t depth,
                                                void *val)
{
    tree_demarshal_node *node;

    node = demarshal_alloc_node(ctx, depth, MARSHAL_PTR);
    if (!node) {
        return (0);
    }

    node->val.v_ptr = val;

    node->ptr_node = demarshal_ptr_alloc_node(ctx, val);

    return (node);
}

static tree_demarshal_node *demarshal_push_ptr_ref (tree_demarshal *ctx,
                                                    const int8_t depth,
                                                    void *val)
{
    tree_demarshal_node *node;

    node = demarshal_alloc_node(ctx, depth, MARSHAL_PTR_REF);
    if (!node) {
        return (0);
    }

    node->val.v_ptr_ref = val;

    node->ptr_ref_node = demarshal_ptr_ref_alloc_node(ctx, val);

    return (node);
}

static tree_demarshal_node *demarshal_push_name (tree_demarshal *ctx,
                                                 const int8_t depth,
                                                 char *val)
{
    tree_demarshal_node *node;

    node = demarshal_alloc_node(ctx, depth, MARSHAL_NAME);
    if (!node) {
        return (0);
    }

    node->val.v_name = dupstr(val, "demarshal name");

    return (node);
}

static tree_demarshal_node *demarshal_push_string (tree_demarshal *ctx,
                                                   const int8_t depth,
                                                   char *val)
{
    tree_demarshal_node *node;

    node = demarshal_alloc_node(ctx, depth, MARSHAL_STRING);
    if (!node) {
        return (0);
    }

    node->val.v_string = dupstr(val, "demarshal string");

    return (node);
}

static tree_demarshal_node *demarshal_push_node (tree_demarshal *ctx,
                                                 const int8_t depth,
                                                 const int8_t type)
{
    tree_demarshal_node *node;

    node = demarshal_alloc_node(ctx, depth, type);
    if (!node) {
        return (0);
    }

    return (node);
}

void demarshal_fini (tree_demarshal *ctx)
{
    tree_demarshal_node *n;
    tree_demarshal_ptr_ref_node *r;

    /*
     * Walk all pointer refs, find the pointer they were referring to and
     * change them to point at the new real pointer.
     */
    tree_demarshal_ptr_node target;
    tree_demarshal_ptr_node *found;

    memset(&target, 0, sizeof(target));

    { TREE_WALK(ctx->ptr_ref_tree.root, r) {
        target.tree.key = r->ptr_ref;

        found = (tree_demarshal_ptr_node*)
            tree_find(ctx->ptr_tree.root, (tree_node*)&target);

        if (!found) {
            ERR("pointer ref %p not found", target.tree.key);
        }

        (*r->pptr_ref) = (*found->ptr);
    } }

    TREE_WALK(ctx->root, n) {
        switch (n->type) {
        case MARSHAL_NONE:
            break;
        case MARSHAL_INT:
            break;
        case MARSHAL_FLOAT:
            break;
        case MARSHAL_STRING:
            myfree(n->val.v_string);
            break;
        case MARSHAL_NAME:
            myfree(n->val.v_name);
            break;
        case MARSHAL_PTR:
            break;
        case MARSHAL_PTR_REF:
            break;
        case MARSHAL_BRA:
            break;
        case MARSHAL_KET:
            break;
        }

        tree_remove(ctx->root, &n->tree.node);
        myfree(n);
    }

    tree_destroy(&ctx->root, 0);
    tree_destroy(&ctx->ptr_tree.root, 0);
    tree_destroy(&ctx->ptr_ref_tree.root, 0);

    myfree(ctx);
}

static void demarshal_print_node (tree_demarshal_node *n)
{
#ifndef ENABLE_DEMARSHAL_DEBUG
    return;
#endif

    int8_t depth = n->depth;

    while (depth--) {
        putchar(' ');
    }

    switch (n->type) {
    case MARSHAL_NONE:
        break;
    case MARSHAL_INT:
        printf("[int] %d", (int)n->val.v_int);
        break;
    case MARSHAL_FLOAT:
        printf("[float] %f", n->val.v_float);
        break;
    case MARSHAL_STRING:
        printf("[string] \"%s\"", n->val.v_string);
        break;
    case MARSHAL_PTR:
        printf("[ptr] *%p", n->val.v_ptr);
        break;
    case MARSHAL_PTR_REF:
        printf("[ptr-ref] @%p", n->val.v_ptr_ref);
        break;
    case MARSHAL_NAME:
        printf("[name] \"%s\"", n->val.v_name);
        break;
    case MARSHAL_BRA:
        putchar('{');
        break;
    case MARSHAL_KET:
        putchar('}');
        break;
    }
    putchar('\n');
}

static const char *demarshal_node2str (tree_demarshal_node *n)
{
    static char buf[MAXSTR];

    if (!n) {
        return ("[nil]");
    }

    switch (n->type) {
    case MARSHAL_NONE:
        break;
    case MARSHAL_INT:
        snprintf(buf, sizeof(buf), "[int] %d", (int)n->val.v_int);
        break;
    case MARSHAL_FLOAT:
        snprintf(buf, sizeof(buf), "[float] %f", n->val.v_float);
        break;
    case MARSHAL_STRING:
        snprintf(buf, sizeof(buf), "[string] \"%s\"", n->val.v_string);
        break;
    case MARSHAL_PTR:
        snprintf(buf, sizeof(buf), "[ptr] *%p", n->val.v_ptr);
        break;
    case MARSHAL_PTR_REF:
        snprintf(buf, sizeof(buf), "[ptr-ref] @%p", n->val.v_ptr_ref);
        break;
    case MARSHAL_NAME:
        snprintf(buf, sizeof(buf), "[name] \"%s\"", n->val.v_name);
        break;
    case MARSHAL_BRA:
        snprintf(buf, sizeof(buf), "{");
        break;
    case MARSHAL_KET:
        snprintf(buf, sizeof(buf), "}");
        break;
    default:
        snprintf(buf, sizeof(buf), "unknown");
        break;
    }

    return (buf);
}

void demarshal_print (tree_demarshal *ctx)
{
    tree_demarshal_node *n;

    TREE_WALK(ctx->root, n) {
        demarshal_print_node(n);
    }
}

static int8_t tree_demarshal_ptr_compare_func (const tree_node *a,
                                               const tree_node *b)
{
    tree_demarshal_ptr_node *A = (typeof(A))a;
    tree_demarshal_ptr_node *B = (typeof(B))b;

    if (A->tree.key < B->tree.key) {
        return (-1);
    } else if (A->tree.key > B->tree.key) {
        return (1);
    } else {
        return (0);
    }
}

tree_demarshal *demarshal (const char *filename)
{
    demarshal_parse_state state;
    int8_t depth;
    tree_demarshal *ctx;
    uint8_t newline;
    char *next;
    char *buf;
    int32_t size;
    char *at;
    float tmp_float;
    char *tmp_at = 0;
    int64_t tmp_int = 0;
    uint64_t tmp_hex = 0;
    int64_t tmp_mul = 0;
    uint8_t compress;
    uint8_t reading_ptr;
    uint8_t reading_ptr_ref;

    reading_ptr = 0;
    reading_ptr_ref = 0;

    demarshal_parse_filename = filename;
    demarshal_parse_line = 1;

    if (!filename) {
        ERR("no filename");
    }

    buf = (char*)ramdisk_load(filename, &size);
    if (!buf) {
        return (0);
    }

    ctx = (typeof(ctx)) myzalloc(sizeof(*ctx), "demarshal ctx");
    ctx->root = tree_alloc(TREE_KEY_INTEGER, "TREE ROOT: demarshal");
    ctx->ptr_tree.root = tree_alloc_custom(tree_demarshal_ptr_compare_func,
                                           "TREE ROOT: demarshal ptr");
    ctx->ptr_ref_tree.root = tree_alloc(TREE_KEY_INTEGER,
                                        "TREE ROOT: demarshal ptr ref");

    demarshal_buf_end = buf + size;

    next = buf;
    at = buf;
    depth = 0;
    state = MARSHAL_PARSE_STATE_NONE;
    newline = 1;

    while (at < demarshal_buf_end) {
        at = next;

        if (newline) {
            while (at < demarshal_buf_end) {
                if (*at == '#') {
                    while ((at < demarshal_buf_end) && (*at != '\n')) { at++; }
                    at++;
                } else {
                    while ((at < demarshal_buf_end) && isspace(*at)) { at++; }
                    break;
                }
            }

            newline = 0;
        }

        if (at >= demarshal_buf_end) {
            break;
        }

        char c = *at;

        next = at + 1;

        if (at >= demarshal_buf_end) {
            ERR("Unexpected end in \"%s\"", filename);
        }

        switch (state) {
        case MARSHAL_PARSE_STATE_NONE:
            switch (c) {
            case ' ':
            case '\t':
                continue;

            case '#':
                while ((at < demarshal_buf_end) && (*at != '\n')) { at++; }
                next = at;
                continue;

            default:
                state = MARSHAL_PARSE_STATE_NAME;
                tmp_at = at;
                continue;

            case '*':
                reading_ptr = 1;
                continue;

            case '@':
                reading_ptr_ref = 1;
                continue;

            case '0':
                if (*next == 'x') {
                    next++;
                    state = MARSHAL_PARSE_STATE_HEX;
                    tmp_at = at;
                    continue;
                }

            case '1': case '2': case '3': case '4': case '5':
            case '6': case '7': case '8': case '9':
                state = MARSHAL_PARSE_STATE_INT;
                tmp_int = c - '0';
                tmp_mul = 1;
                tmp_at = at;
                continue;

            case '\"':
                state = MARSHAL_PARSE_STATE_STRING;
                tmp_at = at + 1;
                continue;

            case '[':
            case '{':
                demarshal_push_node(ctx, depth, MARSHAL_BRA);
                depth++;
                continue;

            case ']':
            case '}':
                depth--;
                demarshal_push_node(ctx, depth, MARSHAL_KET);
                continue;

            case 13:
                demarshal_parse_line++;
                newline = 1;
                continue;

            case '\n':
                demarshal_parse_line++;
                newline = 1;
                continue;

            case '-':
                state = MARSHAL_PARSE_STATE_INT;
                tmp_int = 0;
                tmp_at = at;
                tmp_mul = -1;
                continue;

            case '.':
            case '+':
                state = MARSHAL_PARSE_STATE_INT;
                tmp_int = 0;
                tmp_at = at;
                tmp_mul = 1;
                continue;
            }
            continue;

        case MARSHAL_PARSE_STATE_INT:
            while (isdigit(c)) {
                tmp_int *= 10;
                tmp_int += c - '0';
                c = *++at;

                if (at >= demarshal_buf_end) {
                    ERR("Unexpected end in \"%s\" when parsing int", filename);
                }
            }

            if (c == '.') {

                uint8_t need_lex = 0;

                tmp_float = (float)tmp_int;

                float mul = 10.0f;

                c = *++at;

                while (!isspace(c)) {
                    if (at >= demarshal_buf_end) {
                        ERR("Unexpected end in \"%s\" when parsing float",
                            filename);
                    }

                    if (need_lex) {
                        c = *++at;
                        continue;
                    }

                    if (c == 'e') {
                        c = *++at;
                        while (!isspace(c)) {
                            c = *++at;
                        }
                        break;
                    }

                    tmp_float += (float)(c - '0') / mul;
                    mul *= 10.0f;
                    c = *++at;
                }

                if (need_lex) {
                    *at++ = '\0';
                    tmp_float = strtof(tmp_at, 0);
                } else {
                    tmp_float *= (float)tmp_mul;
                }

                demarshal_push_float(ctx, depth, tmp_float);
            } else {
                tmp_int *= tmp_mul;

                demarshal_push_int64(ctx, depth, tmp_int);
            }

            state = MARSHAL_PARSE_STATE_NONE;
            next = at;
            continue;

        case MARSHAL_PARSE_STATE_HEX:
            tmp_hex = 0;

            c = *at;

            while (isdigit(c) ||
                   ((c >= 'a') && (c <='f')) ||
                   ((c >= 'A') && (c <='F'))) {

                tmp_hex *= 16;

                if ((c >= 'a') && (c <='z')) {
                    tmp_hex += c - 'a' + 10;
                } else if ((c >= 'A') && (c <='Z')) {
                    tmp_hex += c - 'A' + 10;
                } else {
                    tmp_hex += c - '0';
                }

                c = *++at;

                if (at >= demarshal_buf_end) {
                    ERR("Unexpected end in \"%s\" when parsing int", filename);
                }
            }

            if (reading_ptr) {
                demarshal_push_ptr(ctx, depth, (void*)(uintptr_t)tmp_hex);
            } else if (reading_ptr_ref) {
                demarshal_push_ptr_ref(ctx, depth, (void*)(uintptr_t)tmp_hex);
            } else {
                demarshal_push_int64(ctx, depth, (int64_t)tmp_hex);
            }

            reading_ptr = 0;
            reading_ptr_ref = 0;

            state = MARSHAL_PARSE_STATE_NONE;
            next = at;
            continue;

        case MARSHAL_PARSE_STATE_NAME:
            while ((c != '=') && (c != ' ') && (c != '\t') && (c != '\n')) {
                c = *++at;

                if (at >= demarshal_buf_end) {
                    ERR("Unexpected end in \"%s\" when parsing name",
                        filename);
                }
            }

            if (c == '\n') {
                newline = 1;
            }

            state = MARSHAL_PARSE_STATE_NONE;
            *at = '\0';
            next = at + 1;
            demarshal_push_name(ctx, depth, tmp_at);
            continue;

        case MARSHAL_PARSE_STATE_STRING:
            compress = 0;

            while (c != '\"') {
                c = *++at;

                if (c == '\\') {
                    at++;
                    compress = 1;
                }

                if (at >= demarshal_buf_end) {
                    ERR("Unexpected end in \"%s\" when parsing string",
                        filename);
                }
            }

            state = MARSHAL_PARSE_STATE_NONE;
            *at = '\0';
            next = at + 1;

            if (compress) {
                strnoescape(tmp_at);
            }

            demarshal_push_string(ctx, depth, tmp_at);
            continue;
        }
    }

    if (state != MARSHAL_PARSE_STATE_NONE) {
        ERR("Error in \"%s\" line %u",
            demarshal_parse_filename,
            demarshal_parse_line);
    }

    ctx->node = (typeof(ctx->node)) tree_root_first(ctx->root);

    myfree(buf);

#ifdef ENABLE_DEMARSHAL_DEBUG
    demarshal_print(ctx);
#endif

    return (ctx);
}

//
// Look at the next node without stepping forward.
//
static uint8_t demarshal_internal_name (tree_demarshal *ctx, const char *want)
{
    tree_root *root = ctx->root;
    tree_demarshal_node **n = &ctx->node;
    tree_demarshal_node *node = *n;

    if (!node) {
        if (ctx->peek) {
            return (0);
        }

        MSG_BOX("Error at file %s, line %d, "
            "expecting a name when looking for \"%s\", got \"%s\"",
            demarshal_parse_filename,
            demarshal_parse_line,
            want,
            demarshal_node2str(node));

        return (0);
    }

    //
    // Make sure we've got a string and not some junk.
    //
    switch (node->type) {
        case MARSHAL_NAME:
            break;
        //
        // Hit the end of a class or the start of a sub class?
        //
        case MARSHAL_KET:
        case MARSHAL_BRA:
            return (0);

        default:
            if (ctx->peek) {
                return (0);
            }

            MSG_BOX("Error at file %s, line %d, "
                "expecting type name when looking for \"%s\", got \"%s\"",
                demarshal_parse_filename,
                node->line,
                want,
                demarshal_node2str(node));
            return (0);
    }

    const char *name = node->val.v_name;
    if (!name) {
        if (ctx->peek) {
            return (0);
        }

        MSG_BOX("Error at file %s, line %d, "
            "looking for \"%s\", got \"%s\"",
            demarshal_parse_filename,
            node->line,
            want,
            demarshal_node2str(node));
        return (0);
    }

    if (strcasecmp(name, want)) {
        if (ctx->peek) {
            return (0);
        }

        MSG_BOX("Error at file %s, line %d, "
            "looking for \"%s\", got %s:\"%s\"",
            demarshal_parse_filename,
            node->line,
            want,
            demarshal_node2str(node),
            name);
        return (0);
    }

    if (!ctx->peek) {
        *n = (tree_demarshal_node*) tree_get_next(root,
                                                  root->node,
                                                  &node->tree.node);
    }

    ctx->gotone = true;

    return (1);
}

//
// Get or peek at the next node
//
static uint8_t demarshal_internal_string (tree_demarshal *ctx, char **out)
{
    tree_root *root = ctx->root;
    tree_demarshal_node **n = &ctx->node;
    tree_demarshal_node *node = *n;

    if (!node) {
        if (ctx->peek) {
            return (0);
        }

        MSG_BOX("Error at file %s, line %d, "
            "expecting string, got %s",
            demarshal_parse_filename,
            demarshal_parse_line,
            demarshal_node2str(node));
        return (0);
    }

    //
    // Make sure we've got a string and not some junk.
    //
    switch (node->type) {
        case MARSHAL_STRING:
            break;
        //
        // Hit the end of a class or the start of a sub class?
        //
        case MARSHAL_KET:
        case MARSHAL_BRA:
            return (0);

        default:
            if (ctx->peek) {
                return (0);
            }

            MSG_BOX("Error at file %s, line %d, "
                "expecting type string, got %s",
                demarshal_parse_filename,
                node->line,
                demarshal_node2str(node));
            return (0);
    }

    if (!ctx->peek) {
        *out = dupstr(node->val.v_string, "demarshal string");

        *n = (tree_demarshal_node*) tree_get_next(root,
                                                  root->node,
                                                  &node->tree.node);
    }

    ctx->gotone = true;

    return (1);
}

//
// Get or peek at the next node
//
static uint8_t demarshal_internal_int (tree_demarshal *ctx, int64_t *out)
{
    tree_root *root = ctx->root;
    tree_demarshal_node **n = &ctx->node;
    tree_demarshal_node *node = *n;

    if (!node) {
        if (ctx->peek) {
            return (0);
        }

        MSG_BOX("Error at file %s, line %d, no node",
            demarshal_parse_filename,
            demarshal_parse_line);

        return (0);
    }

    //
    // Make sure we've got a string and not some junk.
    //
    switch (node->type) {
        case MARSHAL_INT:
            break;
        //
        // Hit the end of a class or the start of a sub class?
        //
        case MARSHAL_KET:
        case MARSHAL_BRA:
            return (0);

        default:
            if (ctx->peek) {
                return (0);
            }

            MSG_BOX("Error at file %s, line %d, "
                "expecting type int, got %s",
                demarshal_parse_filename,
                node->line,
                demarshal_node2str(node));

            return (0);
    }

    if (!ctx->peek) {
        *out = node->val.v_int;

        *n = (tree_demarshal_node*) tree_get_next(root,
                                                  root->node,
                                                  &node->tree.node);
    }

    ctx->gotone = true;

    return (1);
}

//
// Get or peek at the next node
//
static uint8_t demarshal_internal_float (tree_demarshal *ctx, float *out)
{
    tree_root *root = ctx->root;
    tree_demarshal_node **n = &ctx->node;
    tree_demarshal_node *node = *n;

    if (!node) {
        if (ctx->peek) {
            return (0);
        }

        MSG_BOX("Error at file %s, line %d, "
            "expecting float, got %s",
            demarshal_parse_filename,
            demarshal_parse_line,
            demarshal_node2str(node));

        return (0);
    }

    //
    // Make sure we've got a string and not some junk.
    //
    switch (node->type) {
        case MARSHAL_FLOAT:
            break;
        //
        // Hit the end of a class or the start of a sub class?
        //
        case MARSHAL_KET:
        case MARSHAL_BRA:
            return (0);

        default:
            if (ctx->peek) {
                return (0);
            }

            MSG_BOX("Error at file %s, line %d, "
                "expecting type float, got %s",
                demarshal_parse_filename,
                node->line,
                demarshal_node2str(node));

            return (0);
    }

    if (!ctx->peek) {
        *out = node->val.v_float;

        *n = (tree_demarshal_node*) tree_get_next(root,
                                                  root->node,
                                                  &node->tree.node);
    }

    ctx->gotone = true;

    return (1);
}

//
// Get or peek at the next node
//
static uint8_t demarshal_internal_ptr (tree_demarshal *ctx, void **out)
{
    tree_root *root = ctx->root;
    tree_demarshal_node **n = &ctx->node;
    tree_demarshal_node *node = *n;

    if (!node) {
        if (ctx->peek) {
            return (0);
        }

        MSG_BOX("Error at file %s, line %d, no node",
            demarshal_parse_filename,
            demarshal_parse_line);

        return (0);
    }

    //
    // Make sure we've got a string and not some junk.
    //
    switch (node->type) {
        case MARSHAL_PTR:
            break;
        //
        // Hit the end of a class or the start of a sub class?
        //
        case MARSHAL_KET:
        case MARSHAL_BRA:
            return (0);

        default:
            if (ctx->peek) {
                return (0);
            }

            MSG_BOX("Error at file %s, line %d, "
                "expecting type ptr, got %s",
                demarshal_parse_filename,
                node->line,
                demarshal_node2str(node));

            return (0);
    }

    if (!ctx->peek) {
        node->ptr_node->ptr = out;

        *n = (tree_demarshal_node*) tree_get_next(root,
                                                  root->node,
                                                  &node->tree.node);
    }

    ctx->gotone = true;

    return (1);
}

//
// Get or peek at the next node
//
static uint8_t demarshal_internal_ptr_ref (tree_demarshal *ctx, void **out)
{
    tree_root *root = ctx->root;
    tree_demarshal_node **n = &ctx->node;
    tree_demarshal_node *node = *n;

    if (!node) {
        if (ctx->peek) {
            return (0);
        }

        MSG_BOX("Error at file %s, line %d, no node",
            demarshal_parse_filename,
            demarshal_parse_line);

        return (0);
    }

    //
    // Make sure we've got a string and not some junk.
    //
    switch (node->type) {
        case MARSHAL_PTR_REF:
            break;
        //
        // Hit the end of a class or the start of a sub class?
        //
        case MARSHAL_KET:
        case MARSHAL_BRA:
            return (0);

        default:
            if (ctx->peek) {
                return (0);
            }

            MSG_BOX("Error at file %s, line %d, "
                "expecting type ptr-ref, got %s",
                demarshal_parse_filename,
                node->line,
                demarshal_node2str(node));

            return (0);
    }

    if (!ctx->peek) {
        node->ptr_ref_node->pptr_ref = out;

        *n = (tree_demarshal_node*) tree_get_next(root,
                                                  root->node,
                                                  &node->tree.node);
    }

    ctx->gotone = true;

    return (1);
}
//
// Get or peek at the next node
//
static uint8_t demarshal_internal_bra (tree_demarshal *ctx)
{
    tree_root *root = ctx->root;
    tree_demarshal_node **n = &ctx->node;
    tree_demarshal_node *node = *n;

    if (!node) {
        if (ctx->peek) {
            return (0);
        }

        MSG_BOX("Error at file %s, line %d, "
            "expecting {, got %s",
            demarshal_parse_filename,
            demarshal_parse_line,
            demarshal_node2str(node));

        return (0);
    }

    //
    // Make sure we've got a string and not some junk.
    //
    switch (node->type) {
        case MARSHAL_BRA:
            break;

        default:
            if (ctx->peek) {
                return (0);
            }

            MSG_BOX("Error at file %s, line %d, "
                "expecting type {, got %s",
                demarshal_parse_filename,
                node->line,
                demarshal_node2str(node));

            return (0);
    }

    if (!ctx->peek) {
        *n = (tree_demarshal_node*) tree_get_next(root,
                                                  root->node,
                                                  &node->tree.node);
    }

    ctx->gotone = true;

    return (1);
}

//
// Get or peek at the next node
//
static uint8_t demarshal_internal_ket (tree_demarshal *ctx)
{
    tree_root *root = ctx->root;
    tree_demarshal_node **n = &ctx->node;
    tree_demarshal_node *node = *n;

    if (!node) {
        if (ctx->peek) {
            return (0);
        }

        MSG_BOX("Error at file %s, line %d, "
            "expecting }, got %s",
            demarshal_parse_filename,
            demarshal_parse_line,
            demarshal_node2str(node));

        return (0);
    }

    //
    // Make sure we've got a string and not some junk.
    //
    switch (node->type) {
        case MARSHAL_KET:
            break;

        default:
            if (ctx->peek) {
                return (0);
            }

            MSG_BOX("Error at file %s, line %d, "
                "expecting type }, got %s",
                demarshal_parse_filename,
                node->line,
                demarshal_node2str(node));

            return (0);
    }

    if (!ctx->peek) {
        *n = (tree_demarshal_node*) tree_get_next(root,
                                                  root->node,
                                                  &node->tree.node);
    }

    ctx->gotone = true;

    return (1);
}

uint8_t demarshal_name (tree_demarshal *ctx, const char *want)
{
    ctx->peek = 0;

    return (demarshal_internal_name(ctx, want));
}

uint8_t demarshal_peek_name (tree_demarshal *ctx, const char *want)
{
    ctx->peek = 1;

    return (demarshal_internal_name(ctx, want));
}

uint8_t demarshal_string (tree_demarshal *ctx, char **out)
{
    ctx->peek = 0;

    return (demarshal_internal_string(ctx, out));
}

uint8_t demarshal_named_string (tree_demarshal *ctx, const char *name, char **out)
{
    ctx->peek = 0;

    if (!demarshal_internal_name(ctx, name)) {
        return (0);
    }

    return (demarshal_internal_string(ctx, out));
}

uint8_t demarshal_opt_named_string (tree_demarshal *ctx, const char *name,
                                char **out)
{
    ctx->peek = 1;

    if (!demarshal_internal_name(ctx, name)) {
        return (0);
    }

    return (demarshal_named_string(ctx, name, out));
}

uint8_t demarshal_opt_def_named_string (tree_demarshal *ctx, const char *name,
                                    char **out, const char *def)
{
    if (!demarshal_opt_named_string(ctx, name, out)) {
        *out = dupstr(def, "demarshal named string");
        return (0);
    }

    return (1);
}

uint8_t demarshal_peek_string (tree_demarshal *ctx, char **out)
{
    ctx->peek = 1;

    return (demarshal_internal_string(ctx, out));
}

uint8_t demarshal_int64 (tree_demarshal *ctx, int64_t *out)
{
    ctx->peek = 0;

    return (demarshal_internal_int(ctx, out));
}

uint8_t demarshal_named_int64 (tree_demarshal *ctx, const char *name,
                           int64_t *out)
{
    ctx->peek = 0;

    if (!demarshal_internal_name(ctx, name)) {
        return (0);
    }

    return (demarshal_internal_int(ctx, out));
}

uint8_t demarshal_opt_named_int64 (tree_demarshal *ctx, const char *name,
                               int64_t *out)
{
    ctx->peek = 1;

    if (!demarshal_internal_name(ctx, name)) {
        return (0);
    }

    return (demarshal_named_int64(ctx, name, out));
}

uint8_t demarshal_opt_def_named_int64 (tree_demarshal *ctx, const char *name,
                                   int64_t *out, int64_t def)
{
    if (!demarshal_opt_named_int64(ctx, name, out)) {
        *out = def;
        return (0);
    }
    return (1);
}

uint8_t demarshal_int32 (tree_demarshal *ctx, int32_t *out)
{
    int64_t tmp = 0;
    uint8_t rc;

    ctx->peek = 0;
    rc = demarshal_internal_int(ctx, &tmp);
    *out = (int32_t)tmp;

    return (rc);
}

uint8_t demarshal_named_int32 (tree_demarshal *ctx, const char *name,
                           int32_t *out)
{
    int64_t tmp = 0;
    uint8_t rc;

    ctx->peek = 0;

    if (!demarshal_internal_name(ctx, name)) {
        return (0);
    }

    rc = demarshal_internal_int(ctx, &tmp);
    *out = (int32_t)tmp;

    return (rc);
}

uint8_t demarshal_opt_named_int32 (tree_demarshal *ctx, const char *name,
                               int32_t *out)
{
    ctx->peek = 1;

    if (!demarshal_internal_name(ctx, name)) {
        return (0);
    }

    return (demarshal_named_int32(ctx, name, out));
}

uint8_t demarshal_opt_def_named_int32 (tree_demarshal *ctx, const char *name,
                                   int32_t *out, int32_t def)
{
    if (!demarshal_opt_named_int32(ctx, name, out)) {
        *out = def;
        return (0);
    }
    return (1);
}

uint8_t demarshal_int16 (tree_demarshal *ctx, int16_t *out)
{
    int64_t tmp = 0;
    uint8_t rc;

    ctx->peek = 0;
    rc = demarshal_internal_int(ctx, &tmp);
    *out = (int16_t)tmp;

    return (rc);
}

uint8_t demarshal_named_int16 (tree_demarshal *ctx, const char *name,
                           int16_t *out)
{
    int64_t tmp = 0;
    uint8_t rc;

    ctx->peek = 0;

    if (!demarshal_internal_name(ctx, name)) {
        return (0);
    }

    rc = demarshal_internal_int(ctx, &tmp);
    *out = (int16_t)tmp;

    return (rc);
}

uint8_t demarshal_opt_named_int16 (tree_demarshal *ctx, const char *name,
                               int16_t *out)
{
    ctx->peek = 1;

    if (!demarshal_internal_name(ctx, name)) {
        return (0);
    }

    return (demarshal_named_int16(ctx, name, out));
}

uint8_t demarshal_opt_def_named_int16 (tree_demarshal *ctx, const char *name,
                                   int16_t *out, int16_t def)
{
    if (!demarshal_opt_named_int16(ctx, name, out)) {
        *out = def;
        return (0);
    }
    return (1);
}

uint8_t demarshal_int8 (tree_demarshal *ctx, int8_t *out)
{
    int64_t tmp = 0;
    uint8_t rc;

    ctx->peek = 0;
    rc = demarshal_internal_int(ctx, &tmp);
    *out = (int8_t)tmp;

    return (rc);
}

uint8_t demarshal_named_int8 (tree_demarshal *ctx, const char *name,
                          int8_t *out)
{
    int64_t tmp = 0;
    uint8_t rc;

    ctx->peek = 0;

    if (!demarshal_internal_name(ctx, name)) {
        return (0);
    }

    rc = demarshal_internal_int(ctx, &tmp);
    *out = (int8_t)tmp;

    return (rc);
}

uint8_t demarshal_opt_named_int8 (tree_demarshal *ctx, const char *name,
                              int8_t *out)
{
    ctx->peek = 1;

    if (!demarshal_internal_name(ctx, name)) {
        return (0);
    }

    return (demarshal_named_int8(ctx, name, out));
}

uint8_t demarshal_opt_def_named_int8 (tree_demarshal *ctx, const char *name,
                                  int8_t *out, int8_t def)
{
    if (!demarshal_opt_named_int8(ctx, name, out)) {
        *out = def;
        return (0);
    }
    return (1);
}

uint8_t demarshal_uint64 (tree_demarshal *ctx, uint64_t *out)
{
    int64_t tmp = 0;
    uint32_t rc;

    ctx->peek = 0;
    rc = demarshal_internal_int(ctx, &tmp);
    *out = (uint64_t)tmp;

    return (rc);
}

uint8_t demarshal_named_uint64 (tree_demarshal *ctx, const char *name,
                            uint64_t *out)
{
    int64_t tmp = 0;
    uint32_t rc;

    ctx->peek = 0;

    if (!demarshal_internal_name(ctx, name)) {
        return (0);
    }

    rc = demarshal_internal_int(ctx, &tmp);
    *out = (uint64_t)tmp;

    return (rc);
}

uint8_t demarshal_opt_named_uint64 (tree_demarshal *ctx, const char *name,
                                uint64_t *out)
{
    ctx->peek = 1;

    if (!demarshal_internal_name(ctx, name)) {
        return (0);
    }

    return (demarshal_named_uint64(ctx, name, out));
}

uint8_t demarshal_opt_def_named_uint64 (tree_demarshal *ctx, const char *name,
                                    uint64_t *out, uint64_t def)
{
    if (!demarshal_opt_named_uint64(ctx, name, out)) {
        *out = def;
        return (0);
    }
    return (1);
}

uint8_t demarshal_uint32 (tree_demarshal *ctx, uint32_t *out)
{
    int64_t tmp = 0;
    uint32_t rc;

    ctx->peek = 0;
    rc = demarshal_internal_int(ctx, &tmp);
    *out = (uint32_t)tmp;

    return (rc);
}

uint8_t demarshal_named_uint32 (tree_demarshal *ctx, const char *name,
                            uint32_t *out)
{
    int64_t tmp = 0;
    uint32_t rc;

    ctx->peek = 0;

    if (!demarshal_internal_name(ctx, name)) {
        return (0);
    }

    rc = demarshal_internal_int(ctx, &tmp);
    *out = (uint32_t)tmp;

    return (rc);
}

uint8_t demarshal_opt_named_uint32 (tree_demarshal *ctx, const char *name,
                                uint32_t *out)
{
    ctx->peek = 1;

    if (!demarshal_internal_name(ctx, name)) {
        return (0);
    }

    return (demarshal_named_uint32(ctx, name, out));
}

uint8_t demarshal_opt_def_named_uint32 (tree_demarshal *ctx, const char *name,
                                    uint32_t *out, uint32_t def)
{
    if (!demarshal_opt_named_uint32(ctx, name, out)) {
        *out = def;
        return (0);
    }
    return (1);
}

uint8_t demarshal_uint16 (tree_demarshal *ctx, uint16_t *out)
{
    int64_t tmp = 0;
    uint32_t rc;

    ctx->peek = 0;
    rc = demarshal_internal_int(ctx, &tmp);
    *out = (uint16_t)tmp;

    return (rc);
}

uint8_t demarshal_named_uint16 (tree_demarshal *ctx, const char *name,
                            uint16_t *out)
{
    int64_t tmp = 0;
    uint32_t rc;

    ctx->peek = 0;

    if (!demarshal_internal_name(ctx, name)) {
        return (0);
    }

    rc = demarshal_internal_int(ctx, &tmp);
    *out = (uint16_t)tmp;

    return (rc);
}

uint8_t demarshal_opt_named_uint16 (tree_demarshal *ctx, const char *name,
                                    uint16_t *out)
{
    ctx->peek = 1;

    if (!demarshal_internal_name(ctx, name)) {
        return (0);
    }

    return (demarshal_named_uint16(ctx, name, out));
}

uint8_t demarshal_opt_def_named_uint16 (tree_demarshal *ctx, const char *name,
                                        uint16_t *out, uint16_t def)
{
    if (!demarshal_opt_named_uint16(ctx, name, out)) {
        *out = def;
        return (0);
    }
    return (1);
}

uint8_t demarshal_uint8 (tree_demarshal *ctx, uint8_t *out)
{
    int64_t tmp = 0;
    uint32_t rc;

    ctx->peek = 0;
    rc = demarshal_internal_int(ctx, &tmp);
    *out = (uint8_t)tmp;

    return (rc);
}

uint8_t demarshal_named_uint8 (tree_demarshal *ctx, const char *name,
                               uint8_t *out)
{
    int64_t tmp = 0;
    uint32_t rc;

    ctx->peek = 0;

    if (!demarshal_internal_name(ctx, name)) {
        return (0);
    }

    rc = demarshal_internal_int(ctx, &tmp);
    *out = (uint8_t)tmp;

    return (rc);
}

uint8_t demarshal_opt_named_uint8 (tree_demarshal *ctx, const char *name,
                                   uint8_t *out)
{
    ctx->peek = 1;

    if (!demarshal_internal_name(ctx, name)) {
        return (0);
    }

    return (demarshal_named_uint8(ctx, name, out));
}

uint8_t demarshal_opt_def_named_uint8 (tree_demarshal *ctx, const char *name,
                                   uint8_t *out, uint8_t def)
{
    if (!demarshal_opt_named_uint8(ctx, name, out)) {
        *out = def;
        return (0);
    }
    return (1);
}

uint8_t demarshal_peek_float (tree_demarshal *ctx)
{
    ctx->peek = 1;

    return (demarshal_internal_float(ctx, 0));
}

uint8_t demarshal_float (tree_demarshal *ctx, float *out)
{
    ctx->peek = 0;

    return (demarshal_internal_float(ctx, out));
}

uint8_t demarshal_named_float (tree_demarshal *ctx, const char *name,
                           float *out)
{
    ctx->peek = 0;

    if (!demarshal_internal_name(ctx, name)) {
        return (0);
    }

    return (demarshal_internal_float(ctx, out));
}

uint8_t demarshal_opt_named_float (tree_demarshal *ctx, const char *name,
                               float *out)
{
    ctx->peek = 1;

    if (!demarshal_internal_name(ctx, name)) {
        return (0);
    }

    return (demarshal_named_float(ctx, name, out));
}

uint8_t demarshal_opt_def_named_float (tree_demarshal *ctx, const char *name,
                                   float *out, float def)
{
    if (!demarshal_opt_named_float(ctx, name, out)) {
        *out = def;
        return (0);
    }
    return (1);
}

uint8_t demarshal_named_ptr (tree_demarshal *ctx, const char *name,
                         void **out)
{
    ctx->peek = 0;

    if (!demarshal_internal_name(ctx, name)) {
        return (0);
    }

    return (demarshal_internal_ptr(ctx, out));
}

uint8_t demarshal_opt_named_ptr (tree_demarshal *ctx, const char *name,
                             void **out)
{
    ctx->peek = 1;

    if (!demarshal_internal_name(ctx, name)) {
        return (0);
    }

    return (demarshal_named_ptr(ctx, name, out));
}

uint8_t demarshal_named_ptr_ref (tree_demarshal *ctx, const char *name,
                             void **out)
{
    ctx->peek = 0;

    if (!demarshal_internal_name(ctx, name)) {
        return (0);
    }

    return (demarshal_internal_ptr_ref(ctx, out));
}

uint8_t demarshal_opt_named_ptr_ref (tree_demarshal *ctx, const char *name,
                                 void **out)
{
    ctx->peek = 1;

    if (!demarshal_internal_name(ctx, name)) {
        return (0);
    }

    return (demarshal_named_ptr_ref(ctx, name, out));
}

uint8_t demarshal_bra (tree_demarshal *ctx)
{
    ctx->peek = 0;

    return (demarshal_internal_bra(ctx));
}

uint8_t demarshal_peek_bra (tree_demarshal *ctx)
{
    ctx->peek = 1;

    return (demarshal_internal_bra(ctx));
}

uint8_t demarshal_ket (tree_demarshal *ctx)
{
    ctx->peek = 0;

    return (demarshal_internal_ket(ctx));
}

uint8_t demarshal_gotone (tree_demarshal *ctx)
{
    uint8_t gotone;

    gotone = ctx->gotone;

    ctx->gotone = 0;

    return (gotone);
}
