/*
 * Example implementation of a binary bheap - useful in A * star searches
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

typedef unsigned int bheap_idx;

typedef struct {
    /*
     * We save 50 ms on 2 million entries by using shorts!
     */
    unsigned short sort_key;
    struct dmap_astar_node_ *user_data;
} bheap_data;

typedef void (*bheap_print_func)(const bheap_data *);

typedef struct bheap_ {
#ifdef BHEAP_INCLUDE_PRINTER
    /*
     * How to print elements.
     */
    bheap_print_func printer;
#endif
    /*
     * Size of malloced array.
     */
    bheap_idx max_size;
    /*
     * Number of elements in use.
     */
    bheap_idx in_use;
    /*
     * Contiguous array of data.
     */
    bheap_data data[0];
} bheap;

bheap *bheap_malloc(const bheap_idx max_size,
                    bheap_print_func printer);

void bheap_free(bheap *h);

bheap *bheap_insert(bheap *h, const bheap_data *insert_data);

bheap_data bheap_pop(bheap *h);

static inline unsigned int bheap_empty (bheap *h)
{
    return (!h->in_use);
}
