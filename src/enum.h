/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info.
 */

#pragma once

#ifndef __ENUM_H__
#define __ENUM_H__

#include "tree.h"

/*
 * Enum list macros .h definitions
 */
#define ENUM_LIST_MACRO_VALUE(enum_arg, string_arg)             enum_arg

#define ENUM_DEF_H(ENUM_NAME, enum_name)                        \
                                                                \
typedef enum {                                                  \
    ENUM_NAME(ENUM_LIST_MACRO_VALUE)                            \
} enum_name;                                                    \
                                                                \
extern const char *enum_name ## _val2str(enum_name val);        \
extern enum_name enum_name ## _str2val(const char *val);        \
extern void enum_name ## _destroy(void);                        \

typedef struct {
    tree_key_string tree;
    uint32_t val;
} enum_key;

/*
 * Enum list macros .c definitions
 */
#define ENUM_LIST_MACRO_STRING(enum_arg, string_arg)            string_arg

/*                                                              \
 * Enum to string.                                              \
 */                                                             \
#define ENUM_DEF_C(ENUM_NAME, enum_name)                        \
                                                                \
const char *enum_name ## _val2str (enum_name val)               \
{                                                               \
    static const char *arr[] = {                                \
        ENUM_NAME(ENUM_LIST_MACRO_STRING)                       \
    };                                                          \
                                                                \
    if (val >= (typeof(val)) ARRAY_SIZE(arr)) {                 \
        return ("[bad]");                                       \
    }                                                           \
                                                                \
    if (!arr[val]) {                                            \
        return ("[undefined]");                                 \
    }                                                           \
                                                                \
    return (arr[val]);                                          \
}                                                               \
                                                                \
static tree_root *enum_name ## _tree;                           \
                                                                \
/*                                                              \
 * String to enum.                                              \
 */                                                             \
enum_name enum_name ## _str2val (const char *val)               \
{                                                               \
    static const char *arr[] = {                                \
        ENUM_NAME(ENUM_LIST_MACRO_STRING)                       \
    };                                                          \
                                                                \
    /*                                                          \
     * If no tree, create one and fill it with enums.           \
     */                                                         \
    if (!enum_name ## _tree) {                                  \
        enum_name ## _tree = tree_alloc(TREE_KEY_STRING,        \
                          "TREE ROOT: ENUM " # enum_name);      \
                                                                \
        /*                                                      \
         * Populate the tree with enum values.                  \
         */                                                     \
        FOR_ALL_IN_ARRAY(iter, arr) {                           \
            enum_key *enummap;                                  \
                                                                \
            enummap = (typeof(enummap))                         \
                myzalloc(sizeof(*enummap),                      \
                         "TREE NODE: ENUM map " # enum_name);   \
                                                                \
            enummap->tree.key =                                 \
                dupstr(*iter, "TREE KEY: ENUM val " # enum_name);   \
                                                                \
            enummap->val = (uint32_t)(iter - arr);              \
                                                                \
            if (!tree_insert(enum_name ## _tree,                \
                             &enummap->tree.node)) {            \
                DIE("enum %s insert", *iter);                   \
            }                                                   \
        }                                                       \
    }                                                           \
                                                                \
    /*                                                          \
     * Tree lookup of string.                                   \
     */                                                         \
    enum_key target;                                            \
    enum_key *result;                                           \
                                                                \
    memset(&target, 0, sizeof(target));                         \
    target.tree.key = (char*) val;                              \
                                                                \
    result = (typeof(result))                                   \
        tree_find(enum_name ## _tree, &target.tree.node);       \
    if (!result) {                                              \
        return ((enum_name)-1);                                 \
    }                                                           \
                                                                \
    return ((typeof(enum_name))result->val);                    \
}                                                               \
                                                                \
void enum_name ## _destroy (void)                               \
{                                                               \
    tree_destroy(&enum_name ## _tree, 0);                       \
}

/*
 * string_ext.c
 */
#define ENUM_FMT(list_macro)                                            \
    list_macro(ENUM_FMT_NONE = 0,               "none"),                \
    list_macro(ENUM_FMT_LEFT,                   "left"),                \
    list_macro(ENUM_FMT_RIGHT,                  "right"),               \
    list_macro(ENUM_FMT_CENTERX,                "centerx"),             \
    list_macro(ENUM_FMT_TOP,                    "top"),                 \
    list_macro(ENUM_FMT_BOTTOM,                 "bottom"),              \
    list_macro(ENUM_FMT_CENTERY,                "centery"),             \
    list_macro(ENUM_FMT_RESET,                  "reset"),               \

ENUM_DEF_H(ENUM_FMT, enum_fmt)

#define ENUM_FONT(list_macro)                                           \
    list_macro(ENUM_FONT_NONE = 0,              "none"),                \
    list_macro(ENUM_FONT_FIXED,                 "fixed"),               \
    list_macro(ENUM_FONT_VSMALL,                "vsmall"),              \
    list_macro(ENUM_FONT_SMALL,                 "small"),               \
    list_macro(ENUM_FONT_MED,                   "medium"),              \
    list_macro(ENUM_FONT_LARGE,                 "large"),               \
    list_macro(ENUM_FONT_VLARGE,                "vlarge"),              \
    list_macro(ENUM_FONT_VVLARGE,               "vvlarge"),             \

ENUM_DEF_H(ENUM_FONT, enum_font)

void enum_fmt_destroy(void);

#endif
