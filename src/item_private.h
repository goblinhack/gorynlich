/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */

typedef struct tree_item_key_ {
    tree_node node;

    thingp thing;
} tree_item_key;

typedef struct item_ {
    tree_item_key tree;

    /*
     * Name for debugging.
     */
    char *logname;
} item;
