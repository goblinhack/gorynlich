/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#pragma once

#ifndef TREE_H
#define TREE_H

#include <strings.h>
#include <stdlib.h>

typedef enum { RB_RED, RB_BLACK } node_color;

typedef struct tree_node_ {
    struct tree_node_ *right;
    struct tree_node_ *left;
    struct tree_node_ *parent;
    uint16_t color;
    uint16_t is_static_mem;
} tree_node;

typedef int8_t (*tree_key_func)(const tree_node *, const tree_node *);
typedef uint8_t (*tree_destroy_func)(tree_node *);

typedef enum {
    TREE_KEY_CUSTOM,
    TREE_KEY_INTEGER,
    TREE_KEY_TWO_INTEGER,
    TREE_KEY_THREE_INTEGER,
    TREE_KEY_FOUR_INTEGER,
    TREE_KEY_STRING,
    TREE_KEY_POINTER,
} tree_key_type;

typedef struct tree_root_ {
    tree_key_func compare_func;

    tree_node *node;

    uint32_t size;

    /*
     * Negative offset to start of base structure.
     */
    uint16_t offset;

    /*
     * TREE_KEY_CUSTOM, etc...
     */
    tree_key_type type;
} tree_root;

typedef struct {
    tree_node node;
    char *key;
} tree_key_string;

typedef struct {
    tree_node node;
    void *key;
} tree_key_pointer;

typedef struct {
    tree_node node;
    int32_t key;
} tree_key_int;

typedef struct {
    tree_node node;
    int32_t key2;
    int32_t key3;
} tree_key_two_int;

typedef struct {
    tree_node node;
    int32_t key2;
    int32_t key3;
    int32_t key4;
} tree_key_three_int;

typedef struct {
    tree_node node;
    int32_t key2;
    int32_t key3;
    int32_t key4;
    int32_t key5;
} tree_key_four_int;

tree_root *tree_alloc(tree_key_type, const char *name);
tree_root *tree_alloc_custom(tree_key_func, const char *name);
tree_node *tree_root_top(tree_root *);
tree_node *tree_top(tree_node *);
tree_node *tree_root_first(tree_root *);
tree_node *tree_root_get_nth(tree_root *, uint32_t n);
tree_node *tree_root_get_random(tree_root *);
tree_node *tree_get_prev(tree_root *, tree_node *top, tree_node *);
uint32_t tree_root_size(tree_root *);
uint32_t tree_size(const tree_node *top);
tree_node *tree_find(tree_root *, const tree_node *);

typedef uint8_t (*tree_walker_func)(const tree_node *, void *arg);

uint8_t tree_walk(tree_root *, tree_walker_func, void *arg);
uint8_t tree_walk_reverse(tree_root *, tree_walker_func, void *arg);

uint8_t tree_insert(tree_root *, tree_node *);
uint8_t tree_insert_static(tree_root *, tree_node *);
uint8_t tree_remove(tree_root *, tree_node *);
uint8_t tree_remove_found_node(tree_root *, tree_node *);
uint8_t tree_root_is_empty(tree_root *root);
void tree_empty(tree_root *, tree_destroy_func);
void tree_destroy(tree_root **, tree_destroy_func);

/*
 * Convert a tree node back into the base structure that contains it.
 */
static inline void *retnode2ctx (tree_root *root, tree_node *node)
{
    if (!node) {
        return (0);
    }

    return ((void*) (((char*)node) - root->offset));
}

static inline void *getnode2ctx (tree_root *root, tree_node **out,
                                 tree_node *node)
{
    if (!node) {
        return (0);
    }

    *out = (tree_node*) ((void*) (((char*)node) - root->offset));

    return (node);
}

/*
 * Walk all nodes. Not safe if next node is destroyed. Use TREE_WALK instead.
 */
#define TREE_NEXT_INLINE(compare_func)                                      \
static inline                                                               \
tree_node *tree_next_ ## compare_func (tree_root *root, tree_node *node)    \
{                                                                           \
    tree_node *next;                                                        \
    int32_t compare;                                                        \
                                                                            \
    if (!node) {                                                            \
        return (0);                                                         \
    }                                                                       \
                                                                            \
    if (node->right) {                                                      \
        next = tree_first_noverify(node->right);                                \
    } else if (node->parent) {                                              \
        next = node;                                                        \
        do {                                                                \
            next = next->parent;                                            \
            if (!next) {                                                    \
                break;                                                      \
            }                                                               \
                                                                            \
            compare = (compare_func)(next, node);                           \
        } while (compare <= 0);                                             \
    } else {                                                                \
        next = 0;                                                           \
    }                                                                       \
                                                                            \
    return (next);                                                          \
}                                                                           \
    
#define TREE_PREV_INLINE(compare_func)                                      \
static inline                                                               \
tree_node *tree_prev_ ## compare_func (tree_root *root, tree_node *node)    \
{                                                                           \
    tree_node *next;                                                        \
    int32_t compare;                                                        \
                                                                            \
    if (!node) {                                                            \
        return (0);                                                         \
    }                                                                       \
                                                                            \
    if (node->left) {                                                       \
        next = tree_last(node->left);                                       \
    } else if (node->parent) {                                              \
        next = node;                                                        \
        do {                                                                \
            next = next->parent;                                            \
            if (!next) {                                                    \
                break;                                                      \
            }                                                               \
                                                                            \
            compare = (compare_func)(next, node);                           \
        } while (compare > 0);                                              \
    } else {                                                                \
        next = 0;                                                           \
    }                                                                       \
                                                                            \
    return (next);                                                          \
}                                                                           \

#define TREE_WALK(ROOT, NODE)                                               \
    typeof((NODE)->tree) saved_key;                                         \
    extern int32_t gcc_no_warn;                                             \
                                                                            \
    if (ROOT)                                                               \
                                                                            \
    for ((NODE) = (typeof(NODE)) tree_first((ROOT)->node);                  \
                                                                            \
        gcc_no_warn =                                                       \
            (NODE) ? saved_key = (NODE)->tree, 0 : 0, (NODE);               \
                                                                            \
        (NODE) = (typeof(NODE))                                             \
            tree_get_next((ROOT), (ROOT)->node, &saved_key.node))

#define TREE_WALK_INLINE(ROOT, NODE, tree_get_next_func)                    \
    typeof((NODE)->tree) saved_key;                                         \
    extern int32_t gcc_no_warn;                                             \
                                                                            \
    if (ROOT)                                                               \
                                                                            \
    for ((NODE) = (typeof(NODE)) tree_first((ROOT)->node);                  \
                                                                            \
        gcc_no_warn =                                                       \
            (NODE) ? saved_key = (NODE)->tree, 0 : 0, (NODE);               \
                                                                            \
        (NODE) = (typeof(NODE))                                             \
            (tree_get_next_func)((ROOT), (ROOT)->node, &saved_key.node))


#define TREE_WALK_REVERSE(ROOT, NODE)                                       \
    typeof((NODE)->tree) saved_key;                                         \
    extern int32_t gcc_no_warn;                                             \
                                                                            \
    if (ROOT)                                                               \
                                                                            \
    for ((NODE) = (typeof(NODE)) tree_last((ROOT)->node);                   \
                                                                            \
        gcc_no_warn =                                                       \
            (NODE) ? saved_key = (NODE)->tree, 0 : 0, (NODE);               \
                                                                            \
        (NODE) = (typeof(NODE))                                             \
            tree_get_prev((ROOT), (ROOT)->node, &saved_key.node))

#define TREE_WALK_UNSAFE(ROOT, NODE)                                        \
    typeof(NODE) NEXT;                                                      \
                                                                            \
    for (NEXT = (NODE) = (typeof(NODE)) tree_root_first((ROOT));            \
         (NODE = (NEXT)) != 0;                                              \
         NEXT = (typeof(NODE)) tree_next((ROOT), (tree_node*)(NODE)))       \

#define TREE_WALK_REVERSE_UNSAFE(ROOT, NODE)                                \
    typeof(NODE) PREV;                                                      \
                                                                            \
    for (PREV = (NODE) = (typeof(NODE)) tree_root_last((ROOT));             \
         (NODE = (PREV)) != 0;                                              \
         PREV = (typeof(NODE)) tree_prev((ROOT), (tree_node*)(NODE)))       \

#define TREE_WALK_REVERSE_UNSAFE_INLINE(ROOT, NODE, tree_prev_func)         \
    typeof(NODE) PREV;                                                      \
                                                                            \
    for (PREV = (NODE) = (typeof(NODE)) tree_root_last((ROOT));             \
         (NODE = (PREV)) != 0;                                              \
         PREV = (typeof(NODE)) (tree_prev_func)((ROOT), (tree_node*)(NODE))) \

#define TREE_OFFSET_WALK_UNSAFE(ROOT, OUT)                                 \
    typeof(OUT) NEXT;                                                      \
    typeof(OUT) NODE;                                                      \
                                                                           \
    for (NEXT = (NODE) = (typeof(NODE))                                    \
         getnode2ctx((ROOT), (tree_node**) &(OUT), tree_root_first((ROOT))); \
         (NODE = (NEXT)) != 0;                                             \
         NEXT = (typeof(NODE))                                             \
         getnode2ctx((ROOT), (tree_node**) &(OUT),                         \
                     tree_next((ROOT), (tree_node*)(NODE))))               \
                                                                            
#define TREE_OFFSET_WALK_REVERSE_UNSAFE(ROOT, OUT)                         \
    typeof(OUT) PREV;                                                      \
    typeof(OUT) NODE;                                                      \
                                                                           \
    for (PREV = (NODE) = (typeof(NODE))                                    \
         getnode2ctx((ROOT), (tree_node**) &(OUT), tree_root_last((ROOT))); \
         (NODE = (PREV)) != 0;                                             \
         PREV = (typeof(NODE))                                             \
         getnode2ctx((ROOT), (tree_node**) &(OUT),                         \
                     tree_prev((ROOT), (tree_node*)(NODE))))               \

#define TREE_OFFSET_WALK(ROOT, NODE, NAME)                                  \
    typeof((NODE)->NAME) saved_key;                                         \
    extern int32_t gcc_no_warn;                                             \
                                                                            \
    if (ROOT)                                                               \
                                                                            \
    for ((NODE) = (typeof(NODE))                                            \
         retnode2ctx(ROOT, tree_first((ROOT)->node));                       \
                                                                            \
        gcc_no_warn =                                                       \
            (NODE) ? saved_key = (NODE)->NAME, 0 : 0, (NODE);               \
                                                                            \
        (NODE) = (typeof(NODE))                                             \
            retnode2ctx(ROOT,                                               \
                tree_get_next((ROOT), (ROOT)->node, &saved_key.node)))

static inline int8_t
tree_key_int32_compare_func (const tree_node *a, const tree_node *b)
{
    tree_key_int *A = (typeof(A))a;
    tree_key_int *B = (typeof(B))b;

    if (A->key < B->key) {
        return (-1);
    }

    if (A->key > B->key) {
        return (1);
    }

    return (0);
}

static inline int8_t
tree_key_two_int32_compare_func (const tree_node *a, const tree_node *b)
{
    tree_key_two_int *A = (typeof(A))a;
    tree_key_two_int *B = (typeof(B))b;

    if (A->key2 < B->key2) {
        return (-1);
    }

    if (A->key2 > B->key2) {
        return (1);
    }

    if (A->key3 < B->key3) {
        return (-1);
    }

    if (A->key3 > B->key3) {
        return (1);
    }

    return (0);
}

static inline int8_t
tree_key_three_int32_compare_func (const tree_node *a, const tree_node *b)
{
    tree_key_three_int *A = (typeof(A))a;
    tree_key_three_int *B = (typeof(B))b;

    if (A->key2 < B->key2) {
        return (-1);
    }

    if (A->key2 > B->key2) {
        return (1);
    }

    if (A->key3 > B->key3) {
        return (1);
    }

    if (A->key3 < B->key3) {
        return (-1);
    }

    if (A->key4 > B->key4) {
        return (1);
    }

    if (A->key4 < B->key4) {
        return (-1);
    }

    return (0);
}

static inline int8_t
tree_key_four_int32_compare_func (const tree_node *a, const tree_node *b)
{
    tree_key_four_int *A = (typeof(A))a;
    tree_key_four_int *B = (typeof(B))b;

    if (A->key2 < B->key2) {
        return (-1);
    }

    if (A->key2 > B->key2) {
        return (1);
    }

    if (A->key3 > B->key3) {
        return (1);
    }

    if (A->key3 < B->key3) {
        return (-1);
    }

    if (A->key4 > B->key4) {
        return (1);
    }

    if (A->key4 < B->key4) {
        return (-1);
    }

    if (A->key5 > B->key5) {
        return (1);
    }

    if (A->key5 < B->key5) {
        return (-1);
    }

    return (0);
}

static inline int8_t tree_key_string_compare_func (const tree_node *a,
                                             const tree_node *b)
{
    tree_key_string *A = (typeof(A))a;
    tree_key_string *B = (typeof(B))b;

    return (strcasecmp(A->key, B->key));
}

static inline int8_t tree_key_pointer_compare_func (const tree_node *a,
                                                     const tree_node *b)
{
    tree_key_pointer *A = (typeof(A))a;
    tree_key_pointer *B = (typeof(B))b;

    if (A->key < B->key) {
        return (-1);
    }

    if (A->key > B->key) {
        return (1);
    }

    return (0);
}

static inline int8_t tree_node_compare_func (const tree_root *root,
                                             const tree_node *a,
                                             const tree_node *b)
{
    switch (root->type) {
    case TREE_KEY_CUSTOM:
        return (*(root->compare_func))(a, b);
    case TREE_KEY_INTEGER:
        return (tree_key_int32_compare_func(a, b));
    case TREE_KEY_TWO_INTEGER:
        return (tree_key_two_int32_compare_func(a, b));
    case TREE_KEY_THREE_INTEGER:
        return (tree_key_three_int32_compare_func(a, b));
    case TREE_KEY_FOUR_INTEGER:
        return (tree_key_four_int32_compare_func(a, b));
    case TREE_KEY_STRING:
        return (tree_key_string_compare_func(a, b));
    case TREE_KEY_POINTER:
        return (tree_key_pointer_compare_func(a, b));
    }

    return (0);
}

/*
 * Get the least node in the subtree.
 */
static inline tree_node *tree_first_noverify (tree_node *top)
{
    while (top->left) {
        top = top->left;
    }

    return (top);
}

/*
 * Find the next highest node.
 */
static inline tree_node *tree_get_next (tree_root *root,
                                        tree_node *top,
                                        tree_node *node)
{
    tree_node *subtree;
    int8_t compare;

    if (!top) {
        return (0);
    }

    compare = tree_node_compare_func(root, top, node);

    if (compare == 0) {
        /*
         * top == node
         *
         * Dive into the right tree and return the least node.
         *
         *        4   (top 4, node 4, look at 7)
         *       / \
         *      3   8
         *     /   / \
         *    1   7   9
         */
        if (!top->right) {
            return (0);
        }

	return (tree_first_noverify(top->right));
    }

    if (compare < 0) {
        /*
         * top < node
         *
         * Dive into the right tree.
         *
         *        4   (top 4, node 5, look at 8)
         *       / \
         *      3   8
         *     /   / \
         *    1   7   9
         */
	return (tree_get_next(root, top->right, node));
    }

    /*
     * top > node
     *
     * Dive into the left tree.
     *
     *        4   (top 4, node 3, look at 1)
     *       / \
     *      3   8
     *     /   / \
     *    1   7   9
     */
    subtree = tree_get_next(root, top->left, node);
    if (subtree) {
        return (subtree);
    }

    /*
     * top > node
     *
     * If there is no subtree.
     *
     *        4   (top 1, node 0, look at 1)
     *       / \
     *      3   8
     *     /   / \
     *    1   7   9
     */
    return (top);
}

#define TREE_GET_NEXT_INLINE(compare_func)                              \
/*                                                                      \
 * Find the next highest node.						\
 */									\
static inline tree_node *tree_get_next_ ## compare_func (tree_root *root, \
                                                         tree_node *top,  \
                                                         tree_node *node) \
{									\
    tree_node *subtree;							\
    int8_t compare;							\
									\
    if (!top) {								\
        return (0);							\
    }									\
									\
    compare = (compare_func)(top, node);				\
									\
    if (compare == 0) {							\
        /*								\
         * top == node							\
         *								\
         * Dive into the right tree and return the least node.		\
         *								\
         *        4   (top 4, node 4, look at 7)			\
         *       / \							\
         *      3   8							\
         *     /   / \							\
         *    1   7   9							\
         */								\
        if (!top->right) {						\
            return (0);							\
        }								\
									\
	return (tree_first_noverify(top->right));				\
    }									\
									\
    if (compare < 0) {							\
        /*								\
         * top < node							\
         *								\
         * Dive into the right tree.					\
         *								\
         *        4   (top 4, node 5, look at 8)			\
         *       / \							\
         *      3   8							\
         *     /   / \							\
         *    1   7   9							\
         */								\
	return ((tree_get_next_ ## compare_func)(root, top->right, node)); \
    }									\
									\
    /*									\
     * top > node							\
     *									\
     * Dive into the left tree.						\
     *									\
     *        4   (top 4, node 3, look at 1)				\
     *       / \							\
     *      3   8							\
     *     /   / \							\
     *    1   7   9							\
     */									\
    subtree = (tree_get_next_ ## compare_func)(root, top->left, node);	\
    if (subtree) {							\
        return (subtree);						\
    }									\
									\
    /*									\
     * top > node							\
     *									\
     * If there is no subtree.						\
     *									\
     *        4   (top 1, node 0, look at 1)				\
     *       / \							\
     *      3   8							\
     *     /   / \							\
     *    1   7   9							\
     */									\
    return (top);							\
}									\

/*
 * Get the highest node in the subtree.
 */
static inline tree_node *tree_root_last (tree_root *root)
{
    tree_node *top;

    if (unlikely(!root)) {
        return (0);
    }

    top = root->node;
    if (unlikely(!top)) {
        return (0);
    }

    while (top->right) {
        top = top->right;
    }

    return (top);
}

/*
 * Get the least node in the subtree.
 */
static inline tree_node *tree_first (tree_node *top)
{
    if (unlikely(!top)) {
        return (0);
    }

    while (top->left) {
        top = top->left;
    }

    return (top);
}

/*
 * Get the highest node in the subtree.
 */
static inline tree_node *tree_last (tree_node *top)
{
    if (unlikely(!top)) {
        return (0);
    }

    while (top->right) {
        top = top->right;
    }

    return (top);
}

/*
 * Walk all nodes. Not safe if next node is destroyed. Use TREE_WALK instead.
 */
static inline tree_node *tree_next (tree_root *root, tree_node *node)
{
    tree_node *next;
    int32_t compare;

    if (!node) {
        return (0);
    }

    if (node->right) {
        next = tree_first_noverify(node->right);
    } else if (node->parent) {
        next = node;
        do {
            next = next->parent;
            if (!next) {
                break;
            }

            compare = tree_node_compare_func(root, next, node);
        } while (compare <= 0);
    } else {
        next = 0;
    }

    return (next);
}

static inline tree_node *tree_prev (tree_root *root, tree_node *node)
{
    tree_node *next;
    int32_t compare;

    if (!node) {
        return (0);
    }

    if (node->left) {
        next = tree_last(node->left);
    } else if (node->parent) {
        next = node;
        do {
            next = next->parent;
            if (!next) {
                break;
            }

            compare = tree_node_compare_func(root, next, node);
        } while (compare > 0);
    } else {
        next = 0;
    }

    return (next);
}
#endif
