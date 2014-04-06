/*
 * Example implementation of a binary heap - useful in A * star searches
 * 
 * In a sorted list or tree, every element in the tree is in the correct order.
 * However this is not needed for A* where we just want to pull the lowest cost
 * key out of the tree. We don't care if the whole tree is sorted; the rest of
 * the list can be a jumble as long as when we pull items off the list they are
 * sorted.
 * 
 * A binary heap is bunch of items where either the lowest or highest value 
 * item is at the top of the heap. In this demo, I use the lowest value 
 * ordering. Each element in the heap has two children and it must have a 
 * value lower than those two children.
 * 
 * e.g. this linear array (the heap):
 * 
 *     3  7 42 9 12 65 44
 * 
 * can be viewed as a tree
 * 
 *                                 ___ 3 ___
 *                                /         \
 *                               7           42 
 *                             /  \         /  \
 *                            9   12       65   44
 * 
 * 
 * When insterting elements, we add them to the end of the array and then 
 * compare with the parent node ((index - 1) / 2) and swap if the parent is 
 * lower.
 * 
 * When popping from the head, we take the head element out and swap it with the
 * element at the end of the array. Then we compare with each child node 
 * (index * 2) + 1
 * 
 * So each thread of the tree will always be sorted, but the whole tree itself 
 * may not be; which is a lot quicker for A* where once we fill the tree, we 
 * just want to drain it, in sort order.
 * 
 * The example code, heap.c and binary, binary_heap should produce results 
 * like the following when ran.
 * 
inserted 7  (1  in use):7  
inserted 49 (2  in use):7  49 
inserted 73 (3  in use):7  49 73 
inserted 58 (4  in use):7  49 73 58 
inserted 30 (5  in use):7  30 73 58 49 
inserted 72 (6  in use):7  30 72 58 49 73 
inserted 44 (7  in use):7  30 44 58 49 73 72 
inserted 78 (8  in use):7  30 44 58 49 73 72 78 
inserted 23 (9  in use):7  23 44 30 49 73 72 78 58 
inserted 9  (10 in use):7  9  44 30 23 73 72 78 58 49 
inserted 40 (11 in use):7  9  44 30 23 73 72 78 58 49 40 
inserted 65 (12 in use):7  9  44 30 23 65 72 78 58 49 40 73 
inserted 92 (13 in use):7  9  44 30 23 65 72 78 58 49 40 73 92 
inserted 42 (14 in use):7  9  42 30 23 65 44 78 58 49 40 73 92 72 
inserted 87 (15 in use):7  9  42 30 23 65 44 78 58 49 40 73 92 72 87 
inserted 3  (16 in use):3  7  42 9  23 65 44 30 58 49 40 73 92 72 87 78 
inserted 27 (17 in use):3  7  42 9  23 65 44 27 58 49 40 73 92 72 87 78 30 
inserted 29 (18 in use):3  7  42 9  23 65 44 27 29 49 40 73 92 72 87 78 30 58 
inserted 40 (19 in use):3  7  42 9  23 65 44 27 29 49 40 73 92 72 87 78 30 58 40 
inserted 12 (20 in use):3  7  42 9  12 65 44 27 29 23 40 73 92 72 87 78 30 58 40 49 
popped   3  (19 in use):7  9  42 27 12 65 44 30 29 23 40 73 92 72 87 78 49 58 40 
popped   7  (18 in use):9  12 42 27 23 65 44 30 29 40 40 73 92 72 87 78 49 58 
popped   9  (17 in use):12 23 42 27 40 65 44 30 29 58 40 73 92 72 87 78 49 
popped   12 (16 in use):23 27 42 29 40 65 44 30 49 58 40 73 92 72 87 78 
popped   23 (15 in use):27 29 42 30 40 65 44 78 49 58 40 73 92 72 87 
popped   27 (14 in use):29 30 42 49 40 65 44 78 87 58 40 73 92 72 
popped   29 (13 in use):30 40 42 49 40 65 44 78 87 58 72 73 92 
popped   30 (12 in use):40 40 42 49 58 65 44 78 87 92 72 73 
popped   40 (11 in use):40 49 42 73 58 65 44 78 87 92 72 
popped   40 (10 in use):42 49 44 73 58 65 72 78 87 92 
popped   42 (9  in use):44 49 65 73 58 92 72 78 87 
popped   44 (8  in use):49 58 65 73 87 92 72 78 
popped   49 (7  in use):58 73 65 78 87 92 72 
popped   58 (6  in use):65 73 72 78 87 92 
popped   65 (5  in use):72 73 92 78 87 
popped   72 (4  in use):73 78 92 87 
popped   73 (3  in use):78 87 92 
popped   78 (2  in use):87 92 
popped   87 (1  in use):92 
popped   92 (0  in use):
 * 
 * Plese let me know if this is useful to anyone. I wrote this as examples I 
 * found did not quite work, and/or had no demo of it working.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "binary_heap.h"

/*
 * GCC extension for offset
 */
#ifdef __GNUC__
#if defined(__GNUC__) && __GNUC__ > 3
#define STRUCT_OFFSET(STRUCT, MEMBER) __builtin_offsetof(STRUCT, MEMBER)
#else
#define STRUCT_OFFSET(STRUCT, MEMBER) \
                     ((size_t) ( (char *)&((st *)0)->m - (char *)0 ))
#endif
#else
#define STRUCT_OFFSET(STRUCT, MEMBER) \
                     ((size_t) ( (char *)&((st *)0)->m - (char *)0 ))
#endif

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

/*
 * How much contiguous memory to hold the heap struct and data elems?
 */
static size_t bheap_size (const bheap_idx max_size)
{
    /*
     * Get the real sizes, taking into account padding of structs in memory.
     */
    size_t struct_size = STRUCT_OFFSET(bheap, data);
    size_t elem_size = STRUCT_OFFSET(bheap, data[1]) -
                       STRUCT_OFFSET(bheap, data[0]);

    return (struct_size + (elem_size * max_size));
}

/*
 * Allocate contiguous heap memory for all elements.
 */
bheap *bheap_malloc (const bheap_idx max_size,
                     bheap_print_func printer)
{
    bheap *h = malloc(bheap_size(max_size));
    if (!h) {
        return (0);
    }

    h->max_size = max_size;
    h->in_use = 0;
#ifdef BHEAP_INCLUDE_PRINTER
    h->printer = printer;
#endif

    return (h);
}

/*
 * Destroy the contiguous heap memory.
 */
void bheap_free (bheap *h)
{
    free(h);
}

/*
 * Compare two keys for equality. By having the compare function local it is
 * less flexible but now we can inline the check. Most of the time is spent
 * in doing key compares so this has to be fast!
 */
static unsigned char inline bheap_compare_sort_keys (const bheap_data *a,
                                                     const bheap_data *b)
{
    return (a->sort_key <= b->sort_key);
}

/*
 * Insert a new element in to the heap and sort it.
 */
bheap *bheap_insert (bheap *h, const bheap_data *insert_data)
{
    /*
     * Resize the bheap if needed.
     */
    if (h->in_use == h->max_size) {
        h->max_size += (h->max_size + 1) >> 1;

        h = realloc(h, bheap_size(h->max_size));
        if (!h) {
            return (0);
        }
    }

    bheap_idx idx = h->in_use++;
    bheap_data *heap_data = h->data;
    bheap_data *current = &heap_data[idx];

    /*
     * Insert the new element. If there are existing elements, we need to
     * walk backwards from the end of the bheap, swapping elements as we
     * go until the bheap is reordered.
     */
    if (idx) {
        bheap_idx parent_idx;
        bheap_data *parent;

        /*
         * Get the parent idx, which is half the current idx.
         */
        parent_idx = (idx - 1) >> 1;
        parent = &heap_data[parent_idx];

        do {
            /*
             * If the parent is less than the current value, then we are 
             * ordered.
             */
            if (bheap_compare_sort_keys(parent, insert_data)) {
                break;
            }

            /*
             * "Swap" the parent data with the current. The parent data we
             * do not set yet in case there is a further swap.
             */
            *current = *parent;

            /*
             * Parent is now the current idx.
             */
            idx = parent_idx;
            current = &heap_data[idx];

            /*
             * Get the parent idx, which is half the current idx.
             */
            parent_idx = (idx - 1) >> 1;
            parent = &heap_data[parent_idx];

        } while (idx);
    }

    *current = *insert_data;

    return (h);
}

/*
 * Pop the head of the heap and resort.
 */
bheap_data bheap_pop (bheap *h)
{
    /*
     * The head element is what we will return.
     */
    bheap_data *heap_data = h->data;
    bheap_data head = heap_data[0];

    /*
     * First, we remove the item in slot #1, which is now empty. Then we take 
     * the last item in the bheap, and move it up to slot #1.
     */
    bheap_data temp = heap_data[--h->in_use];
    bheap_idx idx = 0;

    for (;;) {
        bheap_idx lowest_child_idx;
        bheap_idx other_child_idx;
        bheap_idx child_idx;

        /*
         * Next we compare the item to each of its two children, which are at 
         * locations (current position * 2) and  (current position * 2 + 1).  
         *
         * If it has a lower score than both of its two children, it stays 
         * where it is. If not, we swap it with the lower of the two children.  
         */
        child_idx = (idx << 1) + 1;
        if (unlikely(child_idx >= h->in_use)) {
            /*
             * If we are past the end of the bheap then it is ordered again.
             */
            break;
        }

        const bheap_data *child = &heap_data[child_idx];
        const bheap_data *lowest_child;

        /*
         * Find the lowest of the two children if there are two children.
         */
        other_child_idx = child_idx + 1;

        if (likely(other_child_idx < h->in_use)) {
            const bheap_data *other_child = child + 1;

            if (bheap_compare_sort_keys(child, other_child)) {
                lowest_child = child;
                lowest_child_idx = child_idx;
            } else {
                lowest_child = other_child;
                lowest_child_idx = other_child_idx;
            }
        } else {
            /*
             * We only have one child? It must be the lowest.
             */
            lowest_child = child;
            lowest_child_idx = child_idx;
        }

        /*
         * If our value is lower than both children, then we are ordered.
         */
        if (unlikely(bheap_compare_sort_keys(&temp, lowest_child))) {
            break; 
        }

        heap_data[idx] = *lowest_child;
        idx = lowest_child_idx;
    }

    /*
     * Re-insert the popped value now.
     */
    heap_data[idx] = temp;

    return (head);
}
