/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 *
 * Test tree insert/delete routines.
 *
 * Prints a nice tree too:
 *
 *               .___________________103(+1)_____________.
 *               |                                       |
 *     ._________49________.                   ._________144(+1)___.
 *     |                   |                   |                   |
 * .___16(+2)          .___73(+2)          .___127(+2)         .___165(+1)
 * |       |           |       |           |       |           |       |
 * 7(+2) ._(-2)40      58(+2) _92.         109(+2) 130(+2)   ._149(+1) 169(+2)
 *   |   |   |           |   |   |           |   |   |       |   |   |   |
 *  .(-1)12  42         .(-1)72 +1) +1)      112(+1) 133(+2) 145 157(+1) 187
 *  |   | |             |     |   |           |       |           |     | |
 *  10  21 3            67    85  99          123     135(+1)     160   179 3
 *                                                    |
 *                                                    136
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "tree.h"
#include "time.h"

#undef TREE_DEBUG_PRINT
#undef TREE_DEBUG_PRINT_WALKS

/*
 * How many nodes to add/remove from the tree.
 */
#define TREE_DEBUG_NUM_NODES          100000

/*
 * How large a 2d scratch pad array for printing the tree.
 */
#define TREE_DEBUG_BUF_MAX            1000

typedef struct tree_debug_node_ {
    tree_key_int tree;
    int32_t test;
} tree_debug_node;

static tree_debug_node tree_debug_nodes[TREE_DEBUG_NUM_NODES];

static int8_t tree_debug_compare_func(const tree_node *, const tree_node *);

tree_root tree_debug_root = {
    tree_debug_compare_func
};

static int8_t tree_debug_compare_func (const tree_node *a, const tree_node *b)
{
    tree_debug_node *A = (typeof(A))a;
    tree_debug_node *B = (typeof(B))b;

    if (A->tree.key < B->tree.key) {
        return (-1);
    }

    if (A->tree.key > B->tree.key) {
        return (1);
    }

    return (0);
}

#ifdef TREE_DEBUG_PRINT
static char tree_debug_buf[TREE_DEBUG_BUF_MAX][TREE_DEBUG_BUF_MAX];

static void tree_debug_print_node (tree_root root,
                                   tree_node *node,
                                   int32_t depth,
                                   int32_t x, int32_t y)
{
    int32_t span = 50 / ((depth * depth)+1);
    int32_t i;

    if (!node) {
        sprintf(tree_debug_buf[y] + x, "[]");
        return;
    }

    if (node->color == RB_BLACK) {
        sprintf(tree_debug_buf[y] + x, "%u(B)",
                ((tree_debug_node*)node)->tree.key);
    } else {
        sprintf(tree_debug_buf[y] + x, "%u(R)",
                ((tree_debug_node*)node)->tree.key);
    }

    if (node->right) {
        if ((tree_debug_buf[y][x + span] == '\0') ||
            (tree_debug_buf[y][x + span] == ' ')) {
            tree_debug_buf[y][x + span] = '.';
        }

        for (i = 0; i <= span; i++) {
            if ((tree_debug_buf[y][x + i] == '\0') ||
                (tree_debug_buf[y][x + i] == ' ')) {
                tree_debug_buf[y][x + i] = '_';
            }
        }

        if ((tree_debug_buf[y+1][x + span] == '\0') ||
            (tree_debug_buf[y+1][x + span] == ' ')) {
            tree_debug_buf[y+1][x + span] = '|';
        }

        ASSERT(node->right != node);
        tree_debug_print_node(root, node->right, depth + 1, x + span, y + 2);
    }

    if (node->left) {
        if ((tree_debug_buf[y][x - span] == '\0') ||
            (tree_debug_buf[y][x - span] == ' ')) {
            tree_debug_buf[y][x - span] = '.';
        }

        for (i = 0; i <= span; i++) {
            if ((tree_debug_buf[y][x - i] == '\0') ||
                (tree_debug_buf[y][x - i] == ' ')) {
                tree_debug_buf[y][x - i] = '_';
            }
        }

        if ((tree_debug_buf[y+1][x - span] == '\0') ||
            (tree_debug_buf[y+1][x - span] == ' ')) {
            tree_debug_buf[y+1][x - span] = '|';
        }

        ASSERT(node->left != node);
        tree_debug_print_node(root, node->left, depth + 1, x - span, y + 2);
    }
}

static void tree_debug_print_trim_blank_edges (void)
{
    int32_t minx = TREE_DEBUG_BUF_MAX;
    int32_t miny = TREE_DEBUG_BUF_MAX;
    int32_t maxx = 0;
    int32_t maxy = 0;
    int32_t x;
    int32_t y;

    for (x = 0; x < TREE_DEBUG_BUF_MAX; x++) {
        for (y = 0; y < TREE_DEBUG_BUF_MAX; y++) {
            if (tree_debug_buf[y][x] == '\0') {
                tree_debug_buf[y][x] = ' ';
            }

            if (tree_debug_buf[y][x] != ' ') {
                if (x < minx) {
                    minx = x;
                }

                if (y < miny) {
                    miny = y;
                }

                if (x > maxx) {
                    maxx = x;
                }

                if (y > maxy) {
                    maxy = y;
                }
            }
        }
    }

    for (y = miny; y <= maxy; y++) {
        for (x = minx; x <= maxx; x++) {
            putchar(tree_debug_buf[y][x]);
        }
        putchar('\n');
    }

    putchar('\n');
}

static void tree_debug_print (tree_root root)
{
    int32_t depth;
    int32_t x;
    int32_t y;

    memset(tree_debug_buf, ' ', sizeof(tree_debug_buf));

    depth = 0;
    x = TREE_DEBUG_BUF_MAX / 2;
    y = 0;

    tree_debug_print_node(root, root.node, depth, x, y);
    tree_debug_print_trim_blank_edges();
}
#endif

#ifdef TREE_DEBUG_PRINT_WALKS
static uint8_t tree_debug_walk_print (const tree_node *node, void *arg)
{
    tree_debug_node *A = (typeof(A))node;

    printf("%d -> ", A->test);

    return (1);
}
#endif

void tree_debug_test (void)
{
    struct timeval start;
    struct timeval end;
    int32_t i;

    for (;;) {
        int32_t numnodes = TREE_DEBUG_NUM_NODES;

        /*
         * Populate the tree.
         */
        gettimeofday(&start, 0);

        for (i = 0; i < numnodes; i++) {
            memset(&tree_debug_nodes[i], 0, sizeof(tree_debug_nodes[i]));

            for (;;) {
                tree_debug_nodes[i].tree.key = rand() % numnodes;
                tree_debug_nodes[i].test = tree_debug_nodes[i].tree.key;

                if (tree_insert(&tree_debug_root,
                                &tree_debug_nodes[i].tree.node)) {
#ifdef TREE_DEBUG_PRINT
                    tree_debug_print(tree_debug_root);

                    printf("^^^ inserted %u\n", tree_debug_nodes[i].tree.key);
#endif
                    break;
                }
            }
        }
        gettimeofday(&end, 0);

        printf("inserted %u nodes\n", numnodes);
        printf("  (%lld microseconds)\n", timeval_diff(0, &end, &start));

        /*
         * Walk the tree.
         */
        {
            tree_debug_node *node;
            int32_t cnt;
            int32_t val;

            cnt = 0;
            val = -1;
            tree_root *root = &tree_debug_root;

            TREE_WALK(root, node) {

#ifdef TREE_DEBUG_PRINT
                printf("%u[%u] ", node->tree.key, node->test);
#endif
                if (node->test < val) {
                    ERR("walk > got %u vs %u", node->test, val);
                }

                val = node->test;

                cnt++;
            }

            ASSERT(cnt == numnodes);
        }

        /*
         * Walk the tree.
         */
        {
            tree_debug_node *node;
            int32_t cnt;
            int32_t val;

            cnt = 0;
            val = 99999999;
            tree_root *root = &tree_debug_root;

            TREE_WALK_REVERSE(root, node) {

#ifdef TREE_DEBUG_PRINT
                printf("%u[%u] ", node->tree.key, node->test);
#endif
                if (node->test > val) {
                    ERR("walk > got %u vs %u", node->test, val);
                }

                val = node->test;

                cnt++;
            }

            ASSERT(cnt == numnodes);
        }

#ifdef TREE_DEBUG_PRINT_WALKS
        printf("\ntree_walk:\n");
        tree_walk(&tree_debug_root, tree_debug_walk_print, 0);
        printf("\n");

        printf("\nreverse tree_walk:\n");
        tree_walk_reverse(&tree_debug_root, tree_debug_walk_print, 0);
        printf("\n");
#endif

        /*
         * TREE_WALK
         */
        gettimeofday(&start, 0);
        {
            tree_root *root = &tree_debug_root;
            tree_debug_node *node;

            printf("\nTREE_WALK:\n");
            { TREE_WALK(root, node) {
#ifdef TREE_DEBUG_PRINT_WALKS
                printf("%d -> ", node->test);
#endif
            } }

#ifdef TREE_DEBUG_PRINT_WALKS
            printf("\n");
#endif
        }

        gettimeofday(&end, 0);
        printf("  (%lld microseconds)\n", timeval_diff(0, &end, &start));

        /*
         * TREE_WALK_REVERSE
         */
        gettimeofday(&start, 0);
        {
            tree_root *root = &tree_debug_root;
            tree_debug_node *node;

            printf("\nTREE_WALK_REVERSE:\n");

            { TREE_WALK_REVERSE(root, node) {
#ifdef TREE_DEBUG_PRINT_WALKS
                printf("%d -> ", node->test);
#endif
            } }

#ifdef TREE_DEBUG_PRINT_WALKS
            printf("\n");
#endif
        }

        gettimeofday(&end, 0);
        printf("  (%lld microseconds)\n", timeval_diff(0, &end, &start));
 
        /*
         * TREE_WALK_UNSAFE
         */
        gettimeofday(&start, 0);
        {
            const tree_debug_node *node;

            printf("\nTREE_WALK_UNSAFE:\n");
            { TREE_WALK_UNSAFE(&tree_debug_root, node) {
#ifdef TREE_DEBUG_PRINT_WALKS
                printf("%d -> ", node->test);
#endif
            } }

#ifdef TREE_DEBUG_PRINT_WALKS
            printf("\n");
#endif
        }
        gettimeofday(&end, 0);
        printf("  (%lld microseconds)\n", timeval_diff(0, &end, &start));

        /*
         * TREE_WALK_REVERSE_UNSAFE
         */
        gettimeofday(&start, 0);
        {
            const tree_debug_node *node;

            printf("\nTREE_WALK_REVERSE_UNSAFE:\n");
            { TREE_WALK_REVERSE_UNSAFE(&tree_debug_root, node) {
#ifdef TREE_DEBUG_PRINT_WALKS
                printf("%d -> ", node->test);
#endif
            } }

#ifdef TREE_DEBUG_PRINT_WALKS
            printf("\n");
#endif
        }
        gettimeofday(&end, 0);
        printf("  (%lld microseconds)\n", timeval_diff(0, &end, &start));
        printf("\n");

        /*
         * Empty the tree.
         */
        gettimeofday(&start, 0);

        for (i = 0; i < numnodes; i++) {
            tree_remove(&tree_debug_root,
                        &tree_debug_nodes[i].tree.node);

#ifdef TREE_DEBUG_PRINT
            tree_debug_print(tree_debug_root);

            printf("^^^ removed %u\n", tree_debug_nodes[i].tree.key);
#endif
        }

        gettimeofday(&end, 0);
        printf("deleted %u nodes\n", numnodes);
        printf("  (%lld microseconds)\n", timeval_diff(0, &end, &start));

        /*
         * Check the tree is empty.
         */
        ASSERT(tree_size(tree_debug_root.node) == 0);
        break;
    }
}
