/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info.
 */

#pragma once

#include "tree.h"

typedef struct tree_file_node_ {
    tree_key_string tree;
    int8_t is_file:1;
} tree_file_node;

uint8_t dir_exists(const char *filename);

tree_root *dirlist(const char *dir,
                   const char *include_suffix,
                   const char *exclude_suffix,
                   uint8_t include_dirs,
                   uint8_t include_ramdisk);

void dirlist_free(tree_root **root);
char *dir_dot(void);
char *dir_dotdot(char *in);
char *dospath2unix(char *in);
