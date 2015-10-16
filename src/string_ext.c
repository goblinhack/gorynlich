/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#include <SDL.h>

#include "main.h"
#include "string_ext.h"
#include "color.h"
#include "tex.h"
#include "tile.h"

/*
 * Split a string "like\nthis" into "like" and "this". e.g.:
 *
 * split("To support multiple versions, the program named perl now just selects the real version of Perl to run, and the program named a2p now just selects the real version of the awk-to-perl translator to run, depending on various settings.  The current supported versions are 5.8.8 (provided for back-ward-compatibility with the Perl 5.8 family) and 5.10.0, with the default being 5.10.0. ", 80);
 *
 * yields:
 *
"To support multiple versions, the program named perl now just selects the real"
"version of Perl to run, and the program named a2p now just selects the real"
"version of the awk-to-perl translator to run, depending on various settings."
"The current supported versions are 5.8.8 (provided for back-ward-compatibility"
"with the Perl 5.8 family) and 5.10.0, with the default being 5.10.0."
 */
tree_root *split (const char *text, uint32_t max_line_len)
{
    tree_string_split_node *node;
    uint8_t found_format_string;
    const char *line_start;
    const char *line_end;
    tree_root *root;
    static int32_t key;
    uint32_t line_len;
    char c;
    const char *const text_start = text;

    if (!text) {
        return (0);
    }

    root = tree_alloc(TREE_KEY_INTEGER, "TREE ROOT: split");

    for (;;) {
        line_len = 0;
        line_start = text;

        /*
         * The first newline is used as a split char. Two newlines means one
         * of them is forced.
         */
        if (text != text_start) {
            if (*text == '\n') {
                text++;
            }
        }

        while (*text == ' ') {
            text++;
        }

        /*
         * Read in as much of the line we can up to the line length.
         */
        found_format_string = false;

        while (line_len < max_line_len) {
            c = *text;
            line_len++;

            if (c == '%') {
                if (!found_format_string) {
                    found_format_string = true;
                } else if (found_format_string) {
                    text++;
                    if (!strncmp(text, "fg=", 3)) {
                        text += 3;
                        line_len -= 2; /* for the %% */
                        (void) string2color(&text);
                        found_format_string = false;
                        continue;
                    } else if (!strncmp(text, "fmt=", 4)) {
                        text += 4;
                        line_len -= 2; /* for the %% */
                        (void) string2fmt(&text);
                        found_format_string = false;
                        continue;
                    } else if (!strncmp(text, "font=", 5)) {
                        text += 5;
                        line_len -= 2; /* for the %% */
                        (void) string2font(&text);

                        found_format_string = false;
                        continue;
                    } else if (!strncmp(text, "tex=", 4)) {
                        text += 4;
                        line_len -= 2; /* for the %% */
                        (void) string2tex(&text);
                        found_format_string = false;
                        continue;
                    } else if (!strncmp(text, "tile=", 5)) {
                        text += 5;
                        line_len -= 2; /* for the %% */
                        (void) string2tile(&text);
                        found_format_string = false;
                        continue;
                    } else {
                        text--;
                    }

                    found_format_string = false;
                }
            } else if (c == '\n') {
                break;
            } else if (c == '\0') {
                break;
            }

            text++;
        }

        line_end = text;

        /*
         * Step back looking for a word break.
         */
        if (line_len >= max_line_len) {
            while ((line_end > line_start) &&
                    ((*line_end != ' ') &&
                    (*line_end != '\0') &&
                    (*line_end != '\n'))) {
                line_end--;
            }
        }

        while (line_end > line_start) {
            if (*(line_end - 1) == ' ') {
                line_end--;
                continue;
            }
            break;
        }

        if (line_end == line_start) {
            line_end = text;
        }

        /*
         * Skip leading junk.
         */
        while (line_start < line_end) {
            if ((*line_start == '\n')) {
                line_start++;
                continue;
            }

            if (*line_start == ' ') {
                line_start++;
                continue;
            }
            break;
        }

        /*
         * Add a node to the tree.
         */
        line_len = (uint32_t)(line_end - line_start);

        node = (typeof(node)) myzalloc(sizeof(*node),
                                        "TREE NODE: split");
        node->tree.key = key++;
        node->line = (char *)myzalloc(line_len + 1, "TREE KEY: split");
        strncpy(node->line, line_start, line_len);

        if (!tree_insert(root, &node->tree.node)) {
            ERR("insert split string %s fail", node->line);
        }

        text = line_end;
        if (!*text) {
            break;
        }

        if (text == text_start) {
            if (line_len == 0) {
                text++;
            }
        }
    }

    return (root);
}

ENUM_DEF_C(ENUM_FMT, enum_fmt)

enum_fmt string2fmt (const char **s)
{
    static char tmp[MAXSTR];
    static const char *eo_tmp = tmp + MAXSTR;
    const char *c = *s;
    char *t = tmp;

    while (t < eo_tmp) {
        if ((*c == '\0') || (*c == '$')) {
            break;
        }

        *t++ = *c++;
    }

    if (c == eo_tmp) {
        return (ENUM_FMT_NONE);
    }

    *t++ = '\0';
    *s += (t - tmp);

    enum_fmt val = enum_fmt_str2val(tmp);

    if ((int) val == -1) {
        ERR("unknown fmt [%s]", tmp);
    }

    return (val);
}

ENUM_DEF_C(ENUM_FONT, enum_font)

fontp string2font (const char **s)
{
    static char tmp[MAXSTR];
    static const char *eo_tmp = tmp + MAXSTR;
    const char *c = *s;
    char *t = tmp;

    while (t < eo_tmp) {
        if ((*c == '\0') || (*c == '$')) {
            break;
        }

        *t++ = *c++;
    }

    if (c == eo_tmp) {
        return (0);
    }

    *t++ = '\0';
    *s += (t - tmp);

    enum_font val = enum_font_str2val(tmp);

    if ((int) val == -1) {
        ERR("unknown font [%s]", tmp);
    }

    switch (val) {
    case ENUM_FONT_NONE:    return (0);
    case ENUM_FONT_FIXED:   return (fixed_font);
    case ENUM_FONT_VSMALL:  return (vsmall_font);
    case ENUM_FONT_SMALL:   return (small_font);
    case ENUM_FONT_MED:     return (med_font);
    case ENUM_FONT_LARGE:   return (large_font);
    case ENUM_FONT_VLARGE:  return (vlarge_font);
    case ENUM_FONT_VVLARGE: return (vvlarge_font);
    default: return (0);
    }
}

static void split_free_cb (tree_string_split_node *node)
{
    myfree(node->line);
}

void split_free (tree_root **root)
{
    tree_destroy(root, (tree_destroy_func)split_free_cb);
}

/*
 * size, the returned size of the buffer. Can be given initially to force
 * alloc of a given buffer size. Else a default size is used.
 *
 * used, optional pointer to avoid the need to do strlen when appending to the
 * string.
 *
 * str, returned pointer for the new string
 *
 * returns 0 on success
 */
int32_t snprintf_realloc (char **str,
                          int32_t *size, int32_t *used,
                          const char *fmt, ...)
{
    int32_t freespace;
    int32_t needspace;
    int32_t usedspace;
    va_list ap;
    char *tmp;

    if (!str) {
        return (-1);
    }

    if (!size) {
        return (-1);
    }

    if (!*str) {
        if (!*size) {
            *size = 128;
        }

        if (used) {
            *used = 0;
        }

        *str = (char*) mymalloc(*size, "sprintf alloc");
        if (!*str) {
            *size = 0;
            return (-1);
        }

        *str[0] = '\0';
    }

    if (!used || !*used) {
        usedspace = (uint32_t)strlen(*str);
    } else {
        usedspace = *used;
    }

    char add[MAXSTR];
    va_start(ap, fmt);
    vsnprintf(add, sizeof(add) - 1, fmt, ap);
    va_end(ap);
    add[MAXSTR-1] = '\0';

    needspace = strlen(add);

    for (;;) {
        freespace = *size - usedspace;

        if (needspace < freespace) {
            if (used) {
                *used += needspace;
            }
            strcat(*str, add);

            return (0);
        }

        (*size) *= 2;

        tmp = (char*) myrealloc(*str, *size, "sprintf realloc");
        if (!tmp) {
            free (*str);
            *str = 0;
            *size = 0;
            return (-1);
        }

        *str = tmp;
    }
}
