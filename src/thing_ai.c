/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#define __STDC_LIMIT_MACROS
#include <SDL.h>
#include <unistd.h>

#include "main.h"
#include "tree.h"
#include "thing.h"
#include "level_private.h"
#include "map.h"
#include "wid.h"
#include "level.h"
#include "wid_game_map_server.h"
#include "math.h"
#include "term.h"
#include "binary_heap.c" // yep, for inlining

static char walls[TILES_MAP_WIDTH][TILES_MAP_HEIGHT];

/*
 * Used while doing an A* search to keep track of score nodes.
 */
typedef struct dmap_astar_node_ {
    struct dmap_astar_node_ *came_from;
    int16_t cost_from_start_to_goal;
    int16_t cost_from_start_to_here;
    int16_t x;
    int16_t y;
    /*
     * Set if we find a better path to this node and have inserted a copy of 
     * this node into the heap with a preferred cost.
     */
    boolean ignore_this_node;
} dmap_astar_node;

/*
 * Run time heap to avoid the need to allocate A* map nodes. This gets cleaned 
 * out each A* search.
 */
static dmap_astar_node dmap_astar_nodes[TILES_MAP_WIDTH * TILES_MAP_HEIGHT * 200];
static uint32_t dmap_astar_node_count;

/*
 * A goal that we want to head for.
 */
typedef struct dmap_goal_ {
    tree_node node;
    int16_t tiebreak;
    int16_t score;
    int16_t x;
    int16_t y;
} dmap_goal;

/*
 * For breadth first flooding of a value.
 */
typedef struct {
    int16_t x;
    int16_t y;
    int16_t score;
} dmap_floodwalk;

typedef struct dmap_t_ {
    /*
     * Caches.
     */
    levelp level;

    /*
     * Targets we look for, sorted in order of best first.
     */
    tree_rootp goal_nodes;

    /*
     * A* search nodes.
     */
    bheap *open_nodes;

    /*
     * Merged score for all targets.
     */
    int16_t score[TILES_MAP_WIDTH][TILES_MAP_HEIGHT];

    /*
     * For debugging, to see the best path.
     */
#ifdef ENABLE_MAP_DEBUG
    char best[TILES_MAP_WIDTH][TILES_MAP_HEIGHT];
#endif

    dmap_astar_node *open[TILES_MAP_WIDTH][TILES_MAP_HEIGHT];
    uint8_t closed[TILES_MAP_WIDTH][TILES_MAP_HEIGHT];

    int16_t best_score;
    int16_t worst_score;
    int16_t score_range;

    /*
     * A goal we are trying to reach.
     */
    int16_t goal_x;
    int16_t goal_y;

    /*
     * And the next step towards that goal.
     */
    int16_t nexthop_x;
    int16_t nexthop_y;
} dmap;

/*
 * Flood from a source, fill the map and do something.
 */
static uint32_t floodwalk_cnt;

#define MAP_FLOODWALK_BEGIN(_x_, _y_, _score_)                          \
                                                                        \
    static uint32_t floodwalk                                           \
        [TILES_MAP_WIDTH][TILES_MAP_HEIGHT];                            \
                                                                        \
    static dmap_floodwalk                                               \
        queue[TILES_MAP_WIDTH*TILES_MAP_HEIGHT];                        \
                                                                        \
    dmap_floodwalk *queue_limit = &queue[ARRAY_SIZE(queue)];            \
    dmap_floodwalk *queue_end = queue;                                  \
    dmap_floodwalk *queue_at = queue;                                   \
                                                                        \
    floodwalk[_x_][_y_] = ++floodwalk_cnt;                              \
    queue_end->x = _x_;                                                 \
    queue_end->y = _y_;                                                 \
    queue_end->score = _score_;                                         \
    queue_end++;                                                        \
                                                                        \
    for (;;) {                                                          \
        if (queue_at >= queue_end) {                                    \
            break;                                                      \
        }                                                               \
                                                                        \
        _score_ = queue_at->score;                                      \
        _x_ = queue_at->x;                                              \
        _y_ = queue_at->y;                                              \
        queue_at++;                                                     \

#define MAP_FLOODWALK_END(_x_, _y_, _score_)                            \
        /*                                                              \
         * Try adjacent tiles and push at the end of the queue.         \
         */                                                             \
        if (_x_ < TILES_MAP_WIDTH - 1) {                                \
            int16_t dx = _x_ + 1;                                       \
            int16_t dy = _y_;                                           \
            if (walls[dx][dy] == ' ') {                                 \
                if (floodwalk[dx][dy] != floodwalk_cnt) {               \
                    floodwalk[dx][dy] = floodwalk_cnt;                  \
                    queue_end->x = dx;                                  \
                    queue_end->y = dy;                                  \
                    queue_end->score = _score_;                         \
                    queue_end++;                                        \
                }                                                       \
            }                                                           \
        }                                                               \
                                                                        \
        if (_x_ > 0) {                                                  \
            int16_t dx = _x_ - 1;                                       \
            int16_t dy = _y_;                                           \
            if (walls[dx][dy] == ' ') {                                 \
                if (floodwalk[dx][dy] != floodwalk_cnt) {               \
                    floodwalk[dx][dy] = floodwalk_cnt;                  \
                    queue_end->x = dx;                                  \
                    queue_end->y = dy;                                  \
                    queue_end->score = _score_;                         \
                    queue_end++;                                        \
                }                                                       \
            }                                                           \
        }                                                               \
                                                                        \
        if (_y_ < TILES_MAP_HEIGHT - 1) {                               \
            int16_t dx = _x_;                                           \
            int16_t dy = _y_ + 1;                                       \
            if (walls[dx][dy] == ' ') {                                 \
                if (floodwalk[dx][dy] != floodwalk_cnt) {               \
                    floodwalk[dx][dy] = floodwalk_cnt;                  \
                    queue_end->x = dx;                                  \
                    queue_end->y = dy;                                  \
                    queue_end->score = _score_;                         \
                    queue_end++;                                        \
                }                                                       \
            }                                                           \
        }                                                               \
                                                                        \
        if (_y_ > 0) {                                                  \
            int16_t dx = _x_;                                           \
            int16_t dy = _y_ - 1;                                       \
            if (walls[dx][dy] == ' ') {                                 \
                if (floodwalk[dx][dy] != floodwalk_cnt) {               \
                    floodwalk[dx][dy] = floodwalk_cnt;                  \
                    queue_end->x = dx;                                  \
                    queue_end->y = dy;                                  \
                    queue_end->score = _score_;                         \
                    queue_end++;                                        \
                }                                                       \
            }                                                           \
        }                                                               \
                                                                        \
        if (queue_end >= queue_limit) {                                 \
            DIE("overflow");                                            \
        }                                                               \
    }                                                                   \

static void dmap_goal_add(dmap *map, int16_t x, int16_t y, int16_t score);
static void dmap_goal_free(dmap *map, dmap_goal *node);
static void dmap_goals_find(dmap *map, thingp t);

#ifdef ENABLE_MAP_DEBUG
static FILE *fp;

/*
 * Print the score of each cell.
 */
static void dmap_print_visited (dmap *map, thingp t)
{
    char tmp[10];
    int16_t x;
    int16_t y;

    for (y = 0; y < TILES_MAP_HEIGHT; y++) {
        for (x = 0; x < TILES_MAP_WIDTH; x++) {

            /*
             * Skip walls.
             */
            if (walls[x][y] != ' ') {
                sprintf(tmp, "   ");
                fprintf(fp, "%s", tmp);
                continue;
            }

            int16_t score = t->visited[x][y];

            sprintf(tmp, "%3d", score);
            fprintf(fp, "%s", tmp);
        }
        fprintf(fp, "\n");
    }
    fprintf(fp, "\n");
}

/*
 * Print the score of each cell.
 */
static void dmap_print_scores (dmap *map)
{
    char tmp[10];
    int16_t x;
    int16_t y;

    for (y = 0; y < TILES_MAP_HEIGHT; y++) {
        for (x = 0; x < TILES_MAP_WIDTH; x++) {

            /*
             * Skip walls.
             */
            if (walls[x][y] != ' ') {
                sprintf(tmp, "    ");
                fprintf(fp, "%s", tmp);
                continue;
            }

            int16_t score = map->score[x][y];

            sprintf(tmp, "%4d", score);
            fprintf(fp, "%s", tmp);
        }
        fprintf(fp, "\n");
    }
    fprintf(fp, "\n");
}
#endif

static inline void dmap_print_walls (dmap *map)
{
    int16_t x;
    int16_t y;

    for (y = 0; y < TILES_MAP_HEIGHT; y++) {
        for (x = 0; x < TILES_MAP_WIDTH; x++) {
#ifdef ENABLE_MAP_DEBUG_CONSOLE
            term_goto(x, y);
            term_putc(walls[x][y]);
#endif
        }
    }
}

/*
 * Print the map.
 */
#ifdef ENABLE_MAP_DEBUG
static void inline dmap_print_map (dmap *map, 
                                   thingp t,
                                   int16_t found_x, int16_t found_y,
                                   boolean show_best, boolean show_open)
{
    levelp level = map->level;
    int16_t x;
    int16_t y;
    char c;

    if (!fp) {
        fp = fopen("map.txt", "w");
        if (!fp) {
            DIE("no map log file");
        }
    }

    if (0) {
        dmap_print_visited(map, t);
    }

    if (1) {
        dmap_print_scores(map);
    }

    for (y = 0; y < TILES_MAP_HEIGHT; y++) {
        for (x = 0; x < TILES_MAP_WIDTH; x++) {
            c = ' ';

            if (map_is_floor_at(level, x, y)) {
                c = ' ';
            }

            if (map_is_wall_at(level, x, y) ||
                !map_is_floor_at(level, x, y)) {

                if (map_is_door_at(level, x, y)) {
                    c = 'D';
                } else {
                    c = '+';
                }

            } else {
                if (show_open) {
                    if (map->open[x][y]) {
                        c = '.';
                    }

                    if (map->closed[x][y]) {
                        c = '_';
                    }
                }

                if (show_best) {
                    if (map->best[x][y] != ' ') {
                        c = 'b';
                    }
                }

                if ((x == found_x) && (y == found_y)) {
                    c = 'S';
                } else if (map_is_pipe_at(level, x, y)) {
                    if (level->end_pipe[x][y] != ' ') {
                        c = 'P';
                    } else {
                        c = 'p';
                    }
                } else if (map_is_door_at(level, x, y)) {
                    c = '#';
                } else if (map_is_monst_at(level, x, y)) {
                    c = 'm';
                } else if (map_is_food_at(level, x, y)) {
                    c = 'F';
                } else if (map_is_key_at(level, x, y)) {
                    c = 'k';
#if 0
                } else if (map_is_generator_at(level, x, y)) {
                    c = 'G';
#endif
                } else if (map_is_star_at(level, x, y)) {
                    c = 'o';
                } else if (map_is_exit_at(level, x, y)) {
                    c = '<';
                } else if (map_is_player_at(level, x, y)) {
                    c = '@';
                }

                if ((x == found_x) && (y == found_y)) {
                    c = 'S';
                }
            }

#ifdef ENABLE_MAP_DEBUG_CONSOLE
            term_goto(x, y);
            term_putc(c);
#endif
            fprintf(fp, "%c", c);
        }
        fprintf(fp, "\n");
    }
    fprintf(fp, "\n");
}
#endif

/*
 * Do a breadth first flood to all nodes and find the oldest cell.
 */
static void dmap_find_oldest_visited (dmap *map, thingp t,
                                      int16_t *found_x, int16_t *found_y)
{
    int16_t best_visited = INT16_MAX;
    boolean got_one = false;
    int16_t score = 0;
    int16_t x = (int)t->x;
    int16_t y = (int)t->y;

    MAP_FLOODWALK_BEGIN(x, y, score)

        int16_t visited = t->visited[x][y];

        if (visited < best_visited) {
            best_visited = visited;
            got_one = true;
            *found_x = x;
            *found_y = y;
        }

    MAP_FLOODWALK_END(x, y, score)

    if (!got_one) {
        DIE("no oldest cell");
    }
}

/*
 * Do a breadth first flood to all nodes, decrementing the score as we go.
 */
static void dmap_goal_flood (dmap *map, int16_t score, int16_t x, int16_t y)
{
    dmap_goal_add(map, x, y, score);

    MAP_FLOODWALK_BEGIN(x, y, score)

        if (score > 0) {
            score--;
        } else if (score < 0) {
            score++;
        } else {
            break;
        }

        map->score[x][y] += score;

    MAP_FLOODWALK_END(x, y, score)
}

static int8_t dmap_goal_compare (const tree_node *a, const tree_node *b)
{
    dmap_goal *A = (typeof(A))a;
    dmap_goal *B = (typeof(B))b;

    if (A->score < B->score) {
        return (-1);
    }

    if (A->score > B->score) {
        return (1);
    }

    if (A->tiebreak < B->tiebreak) {
        return (-1);
    }

    if (A->tiebreak > B->tiebreak) {
        return (1);
    }

    return (0);
}

static dmap_goal *dmap_goal_alloc (int16_t x, int16_t y, int16_t score)
{
    static int16_t tiebreak;

    tiebreak++;

    dmap_goal *node =
        (typeof(node)) mymalloc(sizeof(*node), "TREE NODE: A* goal node");

    memset(&node->node, 0, sizeof(node->node));
    node->x = x;
    node->y = y;
    node->score = score;
    node->tiebreak = tiebreak;

    return (node);
}

/*
 * Add to the goal set.
 */
static void dmap_goal_add (dmap *map, int16_t x, int16_t y, int16_t score)
{
    if (!map->goal_nodes) {
        map->goal_nodes =
            tree_alloc_custom(dmap_goal_compare, "TREE ROOT: A* goals");
    }

    dmap_goal *node = dmap_goal_alloc(x, y, score);

    if (!tree_insert(map->goal_nodes, &node->node)) {
        DIE("failed to add start to add to goal nodes");
    }
}

/*
 * Remove from the goal set.
 */
static void dmap_goal_free (dmap *map, dmap_goal *node)
{
    if (!tree_remove(map->goal_nodes, &node->node)) {
        DIE("failed to remove from goal nodes");
    }

    myfree(node);
}

static dmap_astar_node *dmap_astar_alloc (int16_t x, int16_t y)
{
    assert(dmap_astar_node_count < ARRAY_SIZE(dmap_astar_nodes));

    dmap_astar_node *node = &dmap_astar_nodes[dmap_astar_node_count++];

    memset(node, 0, sizeof(*node));
    node->x = x;
    node->y = y;

    return (node);
}

/*
 * Add to the open set.
 */
static void dmap_astar_add_to_open (dmap *map, dmap_astar_node *node)
{
    if (map->open[node->x][node->y]) {
        DIE("already in open");
    }

    map->open[node->x][node->y] = node;

    bheap_data data;

    data.sort_key = node->cost_from_start_to_goal;
    data.user_data = node;

    map->open_nodes = bheap_insert(map->open_nodes, &data);
}

/*
 * Add to the closed set. Once added, never removed.
 */
static void dmap_astar_add_to_closed (dmap *map, dmap_astar_node *node)
{
    if (map->closed[node->x][node->y]) {
        DIE("already in closed");
    }

    map->closed[node->x][node->y] = 1;
}

/*
 * A guess cost from here to the goal.
 */
static int16_t dmap_astar_cost_est_from_here_to_goal (dmap *map,
                                                      int16_t x, int16_t y)
{
    /*
     * Lower scores are preferred.
     */
    return (map->score[x][y]);
}

/*
 * Evaluate a neighbor for adding to the open set.
 */
static void dmap_astar_eval_neighbor (dmap *map, dmap_astar_node *current,
                                      int16_t dx, int16_t dy)
{
    int16_t current_x = current->x;
    int16_t current_y = current->y;
    int16_t nexthop_x = current_x + dx;
    int16_t nexthop_y = current_y + dy;

    if (nexthop_x <= 0) {
        return;
    }

    if (nexthop_y <= 0) {
        return;
    }

    if (nexthop_x >= TILES_MAP_WIDTH - 1) {
        return;
    }

    if (nexthop_y >= TILES_MAP_HEIGHT - 1) {
        return;
    }

    /*
     * Ignore walls.
     */
    if (walls[nexthop_x][nexthop_y] != ' ') {
        return;
    }

    /*
     * Lowest score is best.
     */
    int16_t score_nexthop = map->score[nexthop_x][nexthop_y];

    /*
     * End of search limits?
     */
    if (score_nexthop >= 0) {
        return;
    }

    dmap_astar_node *neighbor = map->open[nexthop_x][nexthop_y];

    /*
     * If in the closed set already, ignore.
     */
    if (map->closed[nexthop_x][nexthop_y]) {
        int16_t cost_from_start_to_here = current->cost_from_start_to_here +
                        score_nexthop;

        if (neighbor && 
            (cost_from_start_to_here < neighbor->cost_from_start_to_here)) {
            /*
            * Ignore this node in future path finding.
            */
            neighbor->ignore_this_node = true;

            /*
            * Use this copy of the above node instead. It will have a better 
            * search path. This allows us to avoid needing to resort the above 
            * element which in a binary heap is messy.
            */
            dmap_astar_node *better_neighbor = 
                            dmap_astar_alloc(neighbor->x, neighbor->y);

            /*
            * Now insert a copy of this node with the new path.
            */
            better_neighbor->came_from = current;
            better_neighbor->cost_from_start_to_here = cost_from_start_to_here;
            better_neighbor->cost_from_start_to_goal = cost_from_start_to_here +
                dmap_astar_cost_est_from_here_to_goal(map, nexthop_x, nexthop_y);

            /*
            * Remove it from the open list; prior to adding again.
            */
            map->closed[neighbor->x][neighbor->y] = 0;
            map->open[neighbor->x][neighbor->y] = 0;

            dmap_astar_add_to_open(map, better_neighbor);
        }
        return;
    }

    int16_t cost_from_start_to_here = current->cost_from_start_to_here +
                    score_nexthop;
 
    if (!neighbor) {
        neighbor = dmap_astar_alloc(nexthop_x, nexthop_y);
        neighbor->came_from = current;
        neighbor->cost_from_start_to_here = cost_from_start_to_here;
        neighbor->cost_from_start_to_goal = cost_from_start_to_here +
            dmap_astar_cost_est_from_here_to_goal(map, nexthop_x, nexthop_y);

        dmap_astar_add_to_open(map, neighbor);
        return;
    }

    if (cost_from_start_to_here < neighbor->cost_from_start_to_here) {
        /*
         * Ignore this node in future path finding.
         */
        neighbor->ignore_this_node = true;

        /*
         * Use this copy of the above node instead. It will have a better 
         * search path. This allows us to avoid needing to resort the above 
         * element which in a binary heap is messy.
         */
        dmap_astar_node *better_neighbor = 
                        dmap_astar_alloc(neighbor->x, neighbor->y);

        /*
         * Now insert a copy of this node with the new path.
         */
        better_neighbor->came_from = current;
        better_neighbor->cost_from_start_to_here = cost_from_start_to_here;
        better_neighbor->cost_from_start_to_goal = cost_from_start_to_here +
            dmap_astar_cost_est_from_here_to_goal(map, nexthop_x, nexthop_y);

        /*
         * Remove it from the open list; prior to adding again.
         */
        map->open[neighbor->x][neighbor->y] = 0;

        dmap_astar_add_to_open(map, better_neighbor);
    }
}

static void dmap_astar_reconstruct_path (dmap *map, dmap_astar_node *came_from)
{
    int16_t x;
    int16_t y;

    /*
     * And best path for debugging.
     */
    map->nexthop_x = came_from->x;
    map->nexthop_y = came_from->y;

    while (came_from) {
        x = came_from->x;
        y = came_from->y;

        if (came_from->came_from) {
            map->nexthop_x = x;
            map->nexthop_y = y;
        }

#ifdef ENABLE_MAP_DEBUG
        map->best[x][y] = 'b';
#endif

        came_from = came_from->came_from;
    }
}

/*
 * A* search of the map.
 */
static boolean dmap_astar_best_path (dmap *map, thingp t,
                                     int16_t target_x, int16_t target_y)
{
    int16_t start_x = (int)t->x;
    int16_t start_y = (int)t->y;

    map->goal_x = target_x;
    map->goal_y = target_y;

    /*
     * The set of tentative nodes to be evaluated.
     */
    map->open_nodes = bheap_malloc(
                            TILES_MAP_WIDTH * TILES_MAP_HEIGHT /* elements */,
                            0 /* bheap_print_func */);

    memset(map->open, 0, sizeof(map->open));

    /*
     * The set of nodes already evaluated.
     */
    memset(map->closed, 0, sizeof(map->closed));

#ifdef ENABLE_MAP_DEBUG
    memset(map->best, ' ', sizeof(map->best));
#endif

    /*
     * Create the start node.
     */
    dmap_astar_node_count = 0;
    dmap_astar_node *node = dmap_astar_alloc(start_x, start_y);
    dmap_astar_add_to_open(map, node);

    boolean goal_found = false;

    /*
     * Estimated total cost from start to goal through y.
     */
    node->cost_from_start_to_here = 0;
    node->cost_from_start_to_goal =
            dmap_astar_cost_est_from_here_to_goal(map, start_x, start_y);

    while (!bheap_empty(map->open_nodes)) {
        bheap_data data = bheap_pop(map->open_nodes);

        /*
         * current := the node in openset having the lowest f_score[] value
         */
        dmap_astar_node *current = data.user_data;

        /*
         * Ignore this node?
         */
        if (current->ignore_this_node) {
            continue;
        }

        /*
         * Reached the goal?
         */
        if ((current->x == map->goal_x) && (current->y == map->goal_y)) {
            dmap_astar_reconstruct_path(map, current);
            goal_found = true;
        }

        /*
         * Add current to closedset
         */
        dmap_astar_add_to_closed(map, current);

        dmap_astar_eval_neighbor(map, current, -1, 0);
        dmap_astar_eval_neighbor(map, current,  1, 0);
        dmap_astar_eval_neighbor(map, current,  0, -1);
        dmap_astar_eval_neighbor(map, current,  0,  1);

        int16_t current_x = current->x;
        int16_t current_y = current->y;

        if (walls[current_x - 1][current_y] == ' ') {
            if (walls[current_x][current_y - 1] == ' ') {
                dmap_astar_eval_neighbor(map, current, -1, -1);
            }

            if (walls[current_x][current_y + 1] == ' ') {
                dmap_astar_eval_neighbor(map, current, -1, +1);
            }
        }

        if (walls[current_x + 1][current_y] == ' ') {
            if (walls[current_x][current_y - 1] == ' ') {
                dmap_astar_eval_neighbor(map, current, 1, -1);
            }

            if (walls[current_x][current_y + 1] == ' ') {
                dmap_astar_eval_neighbor(map, current, 1, +1);
            }
        }
    }

    bheap_free(map->open_nodes);

    if (!goal_found) {
        return (false);
    }

    return (true);
}

/*
 * Walk all things and see what we want to walk to.
 */
void dmap_goals_find (dmap *map, thingp t)
{
    levelp level;
    int16_t x;
    int16_t y;

    level = thing_level(t);

    /*
     * Walk the map and find all targets. Flood each target into its own map
     * and merge with the combined map
     */
    thingp thing_it;

//    TREE_WALK_UNSAFE(server_active_things, thing_it) {
    TREE_WALK_UNSAFE(server_boring_things, thing_it) {
        /*
         * Only chase players.
         */
#if 0
        if (!thing_is_player_fast(thing_it)) {
            continue;
        }
#endif
        if (!thing_is_key_fast(thing_it)) {
            continue;
        }

        /*
         * Ignore dead players.
         */
        if (thing_is_dead_fast(thing_it)) {
            continue;
        }

        /*
         * Aim for center of tile.
         */
        x = (int)(thing_it->x + 0.5);
        y = (int)(thing_it->y + 0.5);

        /*
         * Chases.
         */
        dmap_goal_flood(map, -100, x, y);
    }
}

static boolean dmap_move_in_same_dir (dmap *map, levelp level, thingp t,
                                      int32_t *nexthop_x, int32_t *nexthop_y)
{
    boolean moved = false;

    int16_t x = t->x;
    int16_t y = t->y;

    if (t->last_x < t->x) {
        if (walls[x+1][y] == ' ') {
            moved = true;
            *nexthop_x = x+1;
            *nexthop_y = y;
        } else if (walls[x][y+1] == ' ') {
            moved = true;
            *nexthop_x = x;
            *nexthop_y = y+1;
        } else if (walls[x][y-1] == ' ') {
            moved = true;
            *nexthop_x = x;
            *nexthop_y = y-1;
        }
    }

    if (!moved) {
        if (t->last_x > t->x) {
            if (walls[x-1][y] == ' ') {
                moved = true;
                *nexthop_x = x-1;
                *nexthop_y = y;
            } else if (walls[x][y+1] == ' ') {
                moved = true;
                *nexthop_x = x;
                *nexthop_y = y+1;
            } else if (walls[x][y-1] == ' ') {
                moved = true;
                *nexthop_x = x;
                *nexthop_y = y-1;
            }
        }
    }

    if (!moved) {
        if (t->last_y < t->y) {
            if (walls[x][y+1] == ' ') {
                moved = true;
                *nexthop_x = x;
                *nexthop_y = y+1;
            } else if (walls[x+1][y] == ' ') {
                moved = true;
                *nexthop_x = x+1;
                *nexthop_y = y;
            } else if (walls[x-1][y] == ' ') {
                moved = true;
                *nexthop_x = x-1;
                *nexthop_y = y;
            }
        }
    }

    if (!moved) {
        if (t->last_y > t->y) {
            if (walls[x][y-1] == ' ') {
                moved = true;
                *nexthop_x = x;
                *nexthop_y = y-1;
            } else if (walls[x+1][y] == ' ') {
                moved = true;
                *nexthop_x = x+1;
                *nexthop_y = y;
            } else if (walls[x-1][y] == ' ') {
                moved = true;
                *nexthop_x = x-1;
                *nexthop_y = y;
            }
        }
    }

    /*
     * Try the other direction.
     */
    if (!moved) {
        if (t->last_x < t->x) {
            if (walls[x-1][y] == ' ') {
                moved = true;
                *nexthop_x = x-1;
                *nexthop_y = y;
            }
        }
    }

    if (!moved) {
        if (t->last_x > t->x) {
            if (walls[x+1][y] == ' ') {
                moved = true;
                *nexthop_x = x+1;
                *nexthop_y = y;
            }
        }
    }

    if (!moved) {
        if (t->last_y < t->y) {
            if (walls[x][y-1] == ' ') {
                moved = true;
                *nexthop_x = x;
                *nexthop_y = y-1;
            }
        }
    }

    if (!moved) {
        if (t->last_y > t->y) {
            if (walls[x][y+1] == ' ') {
                moved = true;
                *nexthop_x = x;
                *nexthop_y = y+1;
            }
        }
    }

    return (moved);
}

static boolean dmap_move_in_same_door_dir (dmap *map, levelp level, thingp t,
                                           int32_t *nexthop_x, int32_t *nexthop_y)
{
    boolean moved = false;

    int16_t x = t->x;
    int16_t y = t->y;

    if (t->last_x < t->x) {
        if ((rand() % 100) < 50) {
            if (level->roads[x+1][y] != ' ') {
                moved = true;
                *nexthop_x = x+1;
                *nexthop_y = y;
            } else if (level->roads[x][y+1] != ' ') {
                moved = true;
                *nexthop_x = x;
                *nexthop_y = y+1;
            } else if (level->roads[x][y-1] != ' ') {
                moved = true;
                *nexthop_x = x;
                *nexthop_y = y-1;
            }
        } else {
            if (level->roads[x+1][y] != ' ') {
                moved = true;
                *nexthop_x = x+1;
                *nexthop_y = y;
            } else if (level->roads[x][y-1] != ' ') {
                moved = true;
                *nexthop_x = x;
                *nexthop_y = y-1;
            } else if (level->roads[x][y+1] != ' ') {
                moved = true;
                *nexthop_x = x;
                *nexthop_y = y+1;
            }
        }
    }

    if (!moved) {
        if (t->last_x > t->x) {
            if ((rand() % 100) < 50) {
                if (level->roads[x-1][y] != ' ') {
                    moved = true;
                    *nexthop_x = x-1;
                    *nexthop_y = y;
                } else if (level->roads[x][y+1] != ' ') {
                    moved = true;
                    *nexthop_x = x;
                    *nexthop_y = y+1;
                } else if (level->roads[x][y-1] != ' ') {
                    moved = true;
                    *nexthop_x = x;
                    *nexthop_y = y-1;
                }
            } else {
                if (level->roads[x-1][y] != ' ') {
                    moved = true;
                    *nexthop_x = x-1;
                    *nexthop_y = y;
                } else if (level->roads[x][y-1] != ' ') {
                    moved = true;
                    *nexthop_x = x;
                    *nexthop_y = y-1;
                } else if (level->roads[x][y+1] != ' ') {
                    moved = true;
                    *nexthop_x = x;
                    *nexthop_y = y+1;
                }
            }
        }
    }

    if (!moved) {
        if (t->last_y < t->y) {
            if ((rand() % 100) < 50) {
                if (level->roads[x][y+1] != ' ') {
                    moved = true;
                    *nexthop_x = x;
                    *nexthop_y = y+1;
                } else if (level->roads[x+1][y] != ' ') {
                    moved = true;
                    *nexthop_x = x+1;
                    *nexthop_y = y;
                } else if (level->roads[x-1][y] != ' ') {
                    moved = true;
                    *nexthop_x = x-1;
                    *nexthop_y = y;
                }
            } else {
                if (level->roads[x][y+1] != ' ') {
                    moved = true;
                    *nexthop_x = x;
                    *nexthop_y = y+1;
                } else if (level->roads[x-1][y] != ' ') {
                    moved = true;
                    *nexthop_x = x-1;
                    *nexthop_y = y;
                } else if (level->roads[x+1][y] != ' ') {
                    moved = true;
                    *nexthop_x = x+1;
                    *nexthop_y = y;
                }
            }
        }
    }

    if (!moved) {
        if (t->last_y > t->y) {
            if ((rand() % 100) < 50) {
                if (level->roads[x][y-1] != ' ') {
                    moved = true;
                    *nexthop_x = x;
                    *nexthop_y = y-1;
                } else if (level->roads[x+1][y] != ' ') {
                    moved = true;
                    *nexthop_x = x+1;
                    *nexthop_y = y;
                } else if (level->roads[x-1][y] != ' ') {
                    moved = true;
                    *nexthop_x = x-1;
                    *nexthop_y = y;
                }
            } else {
                if (level->roads[x][y-1] != ' ') {
                    moved = true;
                    *nexthop_x = x;
                    *nexthop_y = y-1;
                } else if (level->roads[x-1][y] != ' ') {
                    moved = true;
                    *nexthop_x = x-1;
                    *nexthop_y = y;
                } else if (level->roads[x+1][y] != ' ') {
                    moved = true;
                    *nexthop_x = x+1;
                    *nexthop_y = y;
                }
            }
        }
    }

    /*
     * Try the other direction.
     */
    if (!moved) {
        if (t->last_x < t->x) {
            if (level->roads[x-1][y] != ' ') {
                moved = true;
                *nexthop_x = x-1;
                *nexthop_y = y;
            }
        }
    }

    if (!moved) {
        if (t->last_x > t->x) {
            if (level->roads[x+1][y] != ' ') {
                moved = true;
                *nexthop_x = x+1;
                *nexthop_y = y;
            }
        }
    }

    if (!moved) {
        if (t->last_y < t->y) {
            if (level->roads[x][y-1] != ' ') {
                moved = true;
                *nexthop_x = x;
                *nexthop_y = y-1;
            }
        }
    }

    if (!moved) {
        if (t->last_y > t->y) {
            if (level->roads[x][y+1] != ' ') {
                moved = true;
                *nexthop_x = x;
                *nexthop_y = y+1;
            }
        }
    }

    return (moved);
}

static boolean dmap_find_nexthop (dmap *map, levelp level, thingp t,
                                  int32_t *nexthop_x, int32_t *nexthop_y)
{
    boolean found_goal = false;

    if (thing_is_monst(t)) {
        memcpy(walls, level->monst_walls, sizeof(walls));
    } else {
        memcpy(walls, level->walls, sizeof(walls));
    }

    if (t->x < 0) {
        return (false);
    }

    if (t->y < 0) {
        return (false);
    }

    if (t->x > TILES_MAP_WIDTH - 1) {
        return (false);
    }

    if (t->y > TILES_MAP_HEIGHT - 1) {
        return (false);
    }

    /*
     * Age out adjacent cells so we wander around when bored.
     */
    int dx, dy;

    for (dx = -1; dx <= 1; dx++) {
        for (dy = -1; dy <= 1; dy++) {
            t->visited[(int)t->x + dx][(int)t->y + dy]++;
        }
    }

    /*
     * Find all goals.
     */
    dmap_goals_find(map, t);

    if (!map->goal_nodes) {
        /*
         * If no goal was found, try and keep moving the same way.
         */
        if (thing_has(t, THING_KEYS1)) {
            found_goal = dmap_move_in_same_door_dir(map, level, t, 
                                                    nexthop_x, nexthop_y);
        } else {
            found_goal = dmap_move_in_same_dir(map, level, t, 
                                               nexthop_x, nexthop_y);
        }
    } else {
        /*
         * Try goals in order.
         */
        while (tree_root_size(map->goal_nodes)) {
            dmap_goal *goal =
                    (typeof(goal)) tree_root_first(map->goal_nodes);
            /*
             * Stick with the same next hop if nothing good.
             */
            if ((goal->x == t->x) && (goal->y == t->y)) {
                if (map->nexthop_x && map->nexthop_y) {
                    *nexthop_x = map->nexthop_x;
                    *nexthop_y = map->nexthop_y;
                    found_goal = true;
                    break;
                }
            }

            if (dmap_astar_best_path(map, t, goal->x, goal->y)) {
LOG("goal at %d %d found",goal->x,goal->y);
                *nexthop_x = map->nexthop_x;
                *nexthop_y = map->nexthop_y;
                found_goal = true;
                break;
            }
LOG("goal at %d %d not found",goal->x,goal->y);
        dmap_print_map(map, t, t->x, t->y, true, true);

            dmap_goal_free(map, goal);
        }

        tree_destroy(&map->goal_nodes, 0);
    }

    /*
     * If still no goal, just try and find the oldest cell.
     */
    if (!found_goal) {
        int16_t target_x = -1;
        int16_t target_y = -1;

        dmap_find_oldest_visited(map, t, &target_x, &target_y);
LOG("find oldest %d %d",target_x, target_y);

        if ((target_x != t->x) || (target_y != t->y)) {
            dmap_goal_flood(map, -100, target_x, target_y);

            if (dmap_astar_best_path(map, t, target_x, target_y)) {
                *nexthop_x = map->nexthop_x;
                *nexthop_y = map->nexthop_y;
                found_goal = true;
            }
        }
    }

    if (!found_goal) {
        /*
         * If no goal was found, try and keep moving the same way.
         */
LOG("last resort just keep moving");
        if (thing_has(t, THING_KEYS1)) {
            found_goal = dmap_move_in_same_door_dir(map, level, t, 
                                                    nexthop_x, nexthop_y);
        } else {
            found_goal = dmap_move_in_same_dir(map, level, t, 
                                               nexthop_x, nexthop_y);
        }
    }

    if (!found_goal) {
#ifdef ENABLE_MAP_DEBUG
        dmap_print_map(map, t, t->x, t->y, false, true);
#endif
        return (false);
    }

    if (level->walls[*nexthop_x][*nexthop_y] != ' ') {
        THING_LOG(t, "nexthop %d %d into a wall, found goal %d",
                  *nexthop_x, *nexthop_y, found_goal);

#ifdef ENABLE_MAP_DEBUG
        if (thing_is_monst(t)) {
            dmap_print_map(map, t, t->x, t->y, false, true);
        }
#endif
        return (false);
    }

    t->last_x = t->x;
    t->last_y = t->y;

    /*
     * Show likely best path.
     */
#ifdef ENABLE_MAP_DEBUG
    if (thing_is_monst(t)) {
        dmap_print_map(map, t, t->x, t->y, true, true);
    }
#endif

    return (true);
}

/*
 * Print the Dijkstra map scores shared by all things of the same type.
 */
static void dmap_print (thing_templatep t, levelp level)
{
    int16_t x;
    int16_t y;

    if (!fp) {
        fp = fopen("map.txt", "w");
    }

    for (y = 0; y < TILES_MAP_HEIGHT; y++) {
        for (x = 0; x < TILES_MAP_WIDTH; x++) {
            if (level->monst_walls[x][y] != ' ') {
                fprintf(fp, "   ");
                continue;
            }

            fprintf(fp, "%3d", t->dmap[x][y]);
        }

        fprintf(fp, "\n");
    }

    fprintf(fp, "\n");
}

/*
 * Print the Dijkstra map scores shared by all things of the same type.
 */
static void dmap_thing_print (thingp t, 
                              levelp level,
                              int16_t nexthop_x,
                              int16_t nexthop_y)
{
    thing_templatep temp = t->thing_template;
    int16_t x;
    int16_t y;

    if (!fp) {
        fp = fopen("map.txt", "w");
    }

    int16_t tx;
    int16_t ty;

    tx = (int)(t->x + 0.5);
    ty = (int)(t->y + 0.5);

    LOG("%d %d %d %d",tx,ty,nexthop_x,nexthop_y);
    for (y = 0; y < TILES_MAP_HEIGHT; y++) {
        for (x = 0; x < TILES_MAP_WIDTH; x++) {
            if (level->monst_walls[x][y] != ' ') {
                fprintf(fp, "   ");
                continue;
            }

            if ((nexthop_x == x) && (nexthop_y == y)) {
                fprintf(fp, " N ");
            } else {
                if ((x == tx) && (y == ty)) {
                    fprintf(fp, " @ ");
                } else {
                    fprintf(fp, "%3d", temp->dmap[x][y]);
                }
            }
        }

        fprintf(fp, "\n");
    }

    fprintf(fp, "\n");
}

/*
 * http://www.roguebasin.com/index.php?title=The_Incredible_Power_of_Dijkstra_Maps
 *
 * To get a Dijkstra map, you start with an integer array representing your
 * map, with some set of goal cells set to zero and all the rest set to a very
 * high number. 
 *
 * Iterate through the map's "floor" cells -- skip the impassable wall cells.
 * If any floor tile has a value that is at least 2 greater than its 
 * lowest-value floor neighbor, set it to be exactly 1 greater than its lowest 
 * value neighbor. Repeat until no changes are made. The resulting grid of 
 * numbers represents the number of steps that it will take to get from any 
 * given tile to the nearest goal. 
 */
static void dmap_process (thing_templatep t, levelp level)
{
    int16_t x;
    int16_t y;
    int16_t a;
    int16_t b;
    int16_t c;
    int16_t d;
    int16_t *e;
    int16_t f;
    int16_t g;
    int16_t h;
    int16_t i;
    int16_t lowest;
    boolean changed;

    do {
        changed = false;

        for (x = 1; x < TILES_MAP_WIDTH - 1; x++) {
            for (y = 1; y < TILES_MAP_HEIGHT - 1; y++) {
                if (level->monst_walls[x][y] != ' ') {
                    continue;
                }

                a =  t->dmap[x-1][y-1] * 2;
                b =  t->dmap[x  ][y-1];
                c =  t->dmap[x+1][y-1] * 2;

                d =  t->dmap[x-1][y];
                e = &t->dmap[x  ][y];
                f =  t->dmap[x+1][y];
                 
                g =  t->dmap[x-1][y+1] * 2;
                h =  t->dmap[x  ][y+1];
                i =  t->dmap[x+1][y+1] * 2;

                lowest = min(a, min(b, min(c, min(d, min(f, min(g, min(h,i)))))));

                if (*e - lowest >= 2) {
                    *e = lowest + 1;
                    changed = true;
                }
            }
        }
    } while (changed);
}

/*
 * Generate goal points with a low value.
 */
static void dmap_goals_set (thing_templatep t)
{
    thingp thing_it;
    int16_t x;
    int16_t y;

    { TREE_WALK_UNSAFE(server_active_things, thing_it) {
        /*
         * Only chase players.
         */
        if (!thing_is_player_fast(thing_it)) {
            continue;
        }

        /*
         * Ignore dead players.
         */
        if (thing_is_dead_fast(thing_it)) {
            continue;
        }

        /*
         * Aim for center of tile.
         */
        x = (int)(thing_it->x + 0.5);
        y = (int)(thing_it->y + 0.5);

        t->dmap[x][y] = 0;
    } }

    { TREE_WALK_UNSAFE(server_boring_things, thing_it) {
        if (!thing_is_key_fast(thing_it)) {
            continue;
        }

        /*
         * Aim for center of tile.
         */
        x = (int)(thing_it->x + 0.5);
        y = (int)(thing_it->y + 0.5);

        t->dmap[x][y] = 0;
    } }
}

/*
 * Initialize the djkstra map with high values.
 */
static void dmap_init (thing_templatep t)
{
    int16_t x;
    int16_t y;

    const int16_t not_preferred = 999;

    for (x = 0; x < TILES_MAP_WIDTH; x++) {
        for (y = 0; y < TILES_MAP_HEIGHT; y++) {
            t->dmap[x][y] = not_preferred;
        }
    }
}

/*
 * Generate a djkstra map for the thing.
 */
static void dmap_generate (uint32_t i, levelp level)
{
    thing_templatep t = id_to_thing_template(i);

    if (!server_level) {
        t->dmap_valid = false;
        return;
    }

    t->dmap_valid = true;

    dmap_init(t);
    dmap_goals_set(t);
    dmap_process(t, level);
    if (0) dmap_print(t, level);
}

/*
 * Generate a djkstra map for the thing.
e*/
void thing_generate_dmaps (void)
{
    dmap_generate(THING_GHOST, server_level);
}

boolean thing_find_nexthop (thingp t, int32_t *nexthop_x, int32_t *nexthop_y)
{
    thing_templatep temp = t->thing_template;

    if (!temp->dmap_valid) {
        return (false);
    }

    int16_t x;
    int16_t y;

    x = (int)(t->x + 0.5);
    y = (int)(t->y + 0.5);

    int16_t a;
    int16_t b;
    int16_t c;
    int16_t d;
    int16_t e;
    int16_t f;
    int16_t g;
    int16_t h;
    int16_t i;
    int16_t lowest;

    a = temp->dmap[x-1][y-1];
    b = temp->dmap[x  ][y-1];
    c = temp->dmap[x+1][y-1];

    d = temp->dmap[x-1][y];
    e = temp->dmap[x  ][y];
    f = temp->dmap[x+1][y];
        
    g = temp->dmap[x-1][y+1];
    h = temp->dmap[x  ][y+1];
    i = temp->dmap[x+1][y+1];

    lowest = min(a, min(b, min(c, min(d, min(e, min(f, min(g, min(h,i))))))));

    const int16_t not_preferred = 999;

    if (a != lowest) { a += not_preferred; }
    if (b != lowest) { b += not_preferred; }
    if (c != lowest) { c += not_preferred; }
    if (d != lowest) { d += not_preferred; }
    if (e != lowest) { e += not_preferred; }
    if (f != lowest) { f += not_preferred; }
    if (g != lowest) { g += not_preferred; }
    if (h != lowest) { h += not_preferred; }
    if (i != lowest) { i += not_preferred; }

    a += t->visited[x-1][y-1];
    b += t->visited[x  ][y-1];
    c += t->visited[x+1][y-1];

    d += t->visited[x-1][y];
    e += t->visited[x  ][y];
    f += t->visited[x+1][y];
        
    g += t->visited[x-1][y+1];
    h += t->visited[x  ][y+1];
    i += t->visited[x+1][y+1];

    lowest = min(a, min(b, min(c, min(d, min(e, min(f, min(g, min(h,i))))))));

    int16_t dx = 0;
    int16_t dy = 0;

    if (a == lowest) { dx = -1; dy = -1; }
    else if (b == lowest) { dx =  0; dy = -1; }
    else if (c == lowest) { dx = +1; dy = -1; }
    else if (d == lowest) { dx = -1; dy =  0; }
    else if (e == lowest) { dx =  0; dy =  0; }
    else if (f == lowest) { dx = +1; dy =  0; }
    else if (g == lowest) { dx = -1; dy =  1; }
    else if (h == lowest) { dx =  0; dy =  1; }
    else if (i == lowest) { dx = +1; dy =  1; }

    *nexthop_x = x + dx;
    *nexthop_y = y + dy;

    dmap_thing_print(t, server_level, *nexthop_x, *nexthop_y);

if (0) dmap_find_nexthop(0, 0, t, nexthop_x, nexthop_y);
    return (true);
}
