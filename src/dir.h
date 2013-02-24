/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */

#include "tree.h"

typedef struct tree_file_node_ {
    tree_key_string tree;
    int8_t is_file:1;
} tree_file_node;

boolean dir_exists(const char *filename);

tree_root *dirlist(const char *dir,
                   const char *include_suffix,
                   const char *exclude_suffix,
                   boolean include_dirs);

void dirlist_free(tree_root **root);
char *dir_dot(void);
char *dir_dotdot(char *in);
char *dospath2unix(char *in);
