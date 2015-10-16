/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info.
 */

#pragma once

#include "enum.h"

typedef struct tree_string_split_node_ {
    tree_key_int tree;
    char *line;
} tree_string_split_node;

tree_root *split(const char *in, uint32_t max_line_len);
enum_fmt string2fmt(const char **s);
fontp string2font(const char **s);
void split_free(tree_root **root);
int32_t snprintf_realloc(char **str, int32_t *size, int32_t *used,
                         const char *fmt, ...);
