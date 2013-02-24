/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#define __STDC_LIMIT_MACROS
#include <SDL.h>

#include "main.h"
#include "tree.h"
#include "thing.h"
#include "thing_private.h"
#include "thing_template.h"
#include "level_private.h"
#include "map.h"
#include "wid.h"
#include "level.h"
#include "wid_game_map.h"
#include "item.h"
#include "math.h"

static char walls[TILES_MAP_EDITABLE_WIDTH][TILES_MAP_EDITABLE_HEIGHT];

/*
 * Used while doing an A* search to keep track of score nodes.
 */
typedef struct dmap_astar_node_ {
    tree_node node;
    struct dmap_astar_node_ *came_from;
    int16_t cost_from_start_to_goal;
    int16_t cost_from_start_to_here;
    uint16_t tiebreak;
    int16_t x;
    int16_t y;
} dmap_astar_node;

/*
 * A goal that we want to head for.
 */
typedef struct dmap_goal_ {
    tree_node node;
    uint16_t distance;
    uint16_t tiebreak;
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
     * Merged score for all targets.
     */
    int16_t score[TILES_MAP_EDITABLE_WIDTH][TILES_MAP_EDITABLE_HEIGHT];
    int8_t score_hits[TILES_MAP_EDITABLE_WIDTH][TILES_MAP_EDITABLE_HEIGHT];

    /*
     * Distance from the source for each reachable tile.
     */
    int16_t distance[TILES_MAP_EDITABLE_WIDTH][TILES_MAP_EDITABLE_HEIGHT];

    /*
     * For debugging, to see the best path.
     */
#ifdef ENABLE_MAP_DEBUG
    char best[TILES_MAP_EDITABLE_WIDTH][TILES_MAP_EDITABLE_HEIGHT];
#endif

    /*
     * Targets we look for, sorted in order of best first.
     */
    tree_rootp goal_nodes;
    dmap_goal *goals[TILES_MAP_EDITABLE_WIDTH][TILES_MAP_EDITABLE_HEIGHT];

    /*
     * A* search nodes.
     */
    tree_rootp open_nodes;
    tree_rootp closed_nodes;
    dmap_astar_node *open[TILES_MAP_EDITABLE_WIDTH][TILES_MAP_EDITABLE_HEIGHT];
    dmap_astar_node *closed[TILES_MAP_EDITABLE_WIDTH][TILES_MAP_EDITABLE_HEIGHT];

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
        [TILES_MAP_EDITABLE_WIDTH][TILES_MAP_EDITABLE_HEIGHT];          \
                                                                        \
    static dmap_floodwalk                                               \
        queue[TILES_MAP_EDITABLE_WIDTH*TILES_MAP_EDITABLE_HEIGHT];      \
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

#define MAP_FLOODWALK_END(_x_, _y_, _score_)                                         \
        /*                                                              \
         * Try adjacent tiles and push at the end of the queue.         \
         */                                                             \
        if (_x_ < TILES_MAP_EDITABLE_WIDTH - 1) {                       \
            int16_t dx = _x_ + 1;                                       \
            int16_t dy = _y_;                                           \
            if (walls[dx][dy] == ' ') {                     \
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
            if (walls[dx][dy] == ' ') {                     \
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
        if (_y_ < TILES_MAP_EDITABLE_HEIGHT - 1) {                      \
            int16_t dx = _x_;                                           \
            int16_t dy = _y_ + 1;                                       \
            if (walls[dx][dy] == ' ') {                     \
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
            if (walls[dx][dy] == ' ') {                     \
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

static void dmap_goal_add(dmap *map, int16_t x, int16_t y,
                          int16_t score, uint16_t distance);
static void dmap_goal_free(dmap *map, dmap_goal *node);
static void dmap_goals_find(dmap *map, thingp t);

#ifdef ENABLE_MAP_DEBUG
/*
 * Print the score of each cell.
 */
static void dmap_print_scores (dmap *map)
{
    char tmp[10];
    int16_t x;
    int16_t y;

    for (y = 0; y < TILES_MAP_EDITABLE_HEIGHT; y++) {
        for (x = 0; x < TILES_MAP_EDITABLE_WIDTH; x++) {

            /*
             * Skip walls.
             */
            if (walls[x][y] != ' ') {
                printf("%3s ","");
                continue;
            }

            int16_t score = map->score[x][y];

            sprintf(tmp, "%3d", score);
            printf("%3s ", tmp);
        }

        printf("\n");
    }

    printf("\n");
}
#endif

/*
 * Print the distance from the start of each cell.
 */
static inline void dmap_distance_print (dmap *map)
{
    char tmp[10];
    int16_t x;
    int16_t y;

    for (y = 0; y < TILES_MAP_EDITABLE_HEIGHT; y++) {
        for (x = 0; x < TILES_MAP_EDITABLE_WIDTH; x++) {

            /*
             * Skip walls.
             */
            if (walls[x][y] != ' ') {
                printf("%3s ","");
                continue;
            }

            sprintf(tmp, "%3d", map->distance[x][y]);

            printf("%3s ", tmp);
        }

        printf("\n");
    }

    printf("\n");
}

/*
 * Print the map.
 */
#ifdef ENABLE_MAP_DEBUG
static void inline dmap_print_map (dmap *map, int16_t found_x, int16_t found_y,
                                   boolean show_best, boolean show_open)
{
    levelp level = map->level;
    int16_t x;
    int16_t y;
    char c;

    if (show_best) {
        dmap_print_scores(map);
    }

    for (y = 0; y < TILES_MAP_EDITABLE_HEIGHT; y++) {
        for (x = 0; x < TILES_MAP_EDITABLE_WIDTH; x++) {

            if (map_is_wall_at(level, x, y) ||
                !map_is_floor_at(level, x, y)) {

                if (map_is_road_at(level, x, y)) {
                    c = ' ';
                } else {
                    c = '+';
                }

            } else {
                c = ' ';

                if (show_open) {
                    if (map->closed[x][y]) {
                        c = '_';
                    }

                    if (map->open[x][y]) {
                        c = '.';
                    }
                }

                if (show_best) {
                    if (map->best[x][y] != ' ') {
                        c = 'b';
                    }
                }

                if (map_is_pipe_at(level, x, y)) {
                    if (level->end_pipe[x][y] != ' ') {
                        c = 'P';
                    } else {
                        c = 'p';
                    }
                } else if (map_is_road_at(level, x, y)) {
                    c = '#';
                } else if (map_is_monst_at(level, x, y)) {
                    c = 'm';
                } else if (map_is_esnail_at(level, x, y)) {
                    c = 'E';
                } else if (map_is_food_at(level, x, y)) {
                    c = 'F';
                } else if (map_is_letter_at(level, x, y)) {
                    c = 'L';
                } else if (map_is_star_at(level, x, y)) {
                    c = 'o';
                } else if (map_is_bonus_letter_at(level, x, y)) {
                    c = 'B';
                } else if (map_is_exit_at(level, x, y)) {
                    c = '<';
                } else if (map_is_player_at(level, x, y)) {
                    c = '@';
                }
            }

            putchar(c);
        }

        putchar('\n');
    }

    putchar('\n');
}
#endif

/*
 * Round out the total of all the map scores merged together.
 */
static void dmap_normalize (dmap *map)
{
    int16_t x;
    int16_t y;

    for (y = 0; y < TILES_MAP_EDITABLE_HEIGHT; y++) {
        for (x = 0; x < TILES_MAP_EDITABLE_WIDTH; x++) {

            if (walls[x][y] != ' ') {
                continue;
            }

            if (map->score_hits[x][y]) {
                map->score[x][y] /= (int16_t) map->score_hits[x][y];
            }
        }
    }
}

/*
 * Do a breadth first flood to all nodes and find the oldest cell.
 */
static void dmap_find_oldest_visited (dmap *map, thingp t,
                                      int16_t *found_x, int16_t *found_y)
{
    int16_t best_visited = INT16_MAX;
    boolean got_one = false;
    int16_t score = 0;
    int16_t x = t->grid_x;
    int16_t y = t->grid_y;

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
 * Do a breadth first flood to all nodes and find the oldest cell.
 */
static void dmap_find_best_cell (dmap *map, thingp t,
                                 int16_t *found_x, int16_t *found_y)
{
    int16_t best_score = -INT16_MAX;
    boolean got_one = false;
    int16_t score = 0;
    int16_t x = t->grid_x;
    int16_t y = t->grid_y;

    MAP_FLOODWALK_BEGIN(x, y, score)

        int16_t score = map->score[x][y];

        if (score > best_score) {
            best_score = score;
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
    dmap_goal_add(map, x, y, score, -map->distance[x][y]);

    /*
     * Only flood negative goals to avoid.
     */
    if (score > 0) {
        return;
    }

    MAP_FLOODWALK_BEGIN(x, y, score)

        if (score > 0) {
            score--;
        } else if (score < 0) {
            score++;
        } else {
            break;
        }

        map->score[x][y] += score;
        map->score_hits[x][y]++;

    MAP_FLOODWALK_END(x, y, score)
}

/*
 * Do a breadth first flood to all nodes of the distance from the player.
 */
static void dmap_distance_flood (dmap *map, thingp t)
{
    int16_t distance = 0;
    int16_t x = t->grid_x;
    int16_t y = t->grid_y;

    MAP_FLOODWALK_BEGIN(x, y, distance)

        distance--;

        map->distance[x][y] = distance;

    MAP_FLOODWALK_END(x, y, distance)
}

static int32_t dmap_goal_compare (const tree_node *a, const tree_node *b)
{
    dmap_goal *A = (typeof(A))a;
    dmap_goal *B = (typeof(B))b;

    if (A->distance < B->distance) {
        return (-1);
    }

    if (A->distance > B->distance) {
        return (1);
    }

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

static dmap_goal *dmap_goal_alloc (int16_t x, int16_t y,
                                   int16_t score, uint16_t distance)
{
    static uint16_t tiebreak;

    tiebreak++;

    dmap_goal *node =
        (typeof(node)) myzalloc(sizeof(*node), "TREE NODE: A* goal node");

    node->x = x;
    node->y = y;
    node->score = score;
    node->distance = distance;
    node->tiebreak = tiebreak;

    return (node);
}

/*
 * Add to the goal set.
 */
static void dmap_goal_add (dmap *map, int16_t x, int16_t y,
                           int16_t score, uint16_t distance)
{
    if (!map->goal_nodes) {
        map->goal_nodes =
            tree_alloc_custom(dmap_goal_compare, "TREE ROOT: A* goals");
    }

    dmap_goal *node = dmap_goal_alloc(x, y, score, distance);

    if (!tree_insert(map->goal_nodes, &node->node)) {
        DIE("failed to add start to add to goal nodes");
    }

    map->goals[x][y] = node;
}

/*
 * Remove from the goal set.
 */
static void dmap_goal_free (dmap *map, dmap_goal *node)
{
    verify(node);

    if (!tree_remove(map->goal_nodes, &node->node)) {
        DIE("failed to remove from goal nodes");
    }

    myfree(node);
}

static int32_t dmap_astar_compare (const tree_node *a, const tree_node *b)
{
    dmap_astar_node *A = (typeof(A))a;
    dmap_astar_node *B = (typeof(B))b;

    if (A->cost_from_start_to_goal < B->cost_from_start_to_goal) {
        return (-1);
    }

    if (A->cost_from_start_to_goal > B->cost_from_start_to_goal) {
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

static dmap_astar_node *dmap_astar_alloc (int16_t x, int16_t y)
{
    static uint16_t tiebreak;

    tiebreak++;

    dmap_astar_node *node =
        (typeof(node)) myzalloc(sizeof(*node), "TREE NODE: A* node");

    node->tiebreak = tiebreak++;
    node->x = x;
    node->y = y;

    return (node);
}

/*
 * Add to the open set.
 */
static void dmap_astar_add_to_open (dmap *map, dmap_astar_node *node)
{
    verify(node);

    if (map->open[node->x][node->y]) {
        DIE("already in open");
    }

    map->open[node->x][node->y] = node;

    if (!tree_insert(map->open_nodes, &node->node)) {
        DIE("failed to add start to add to open nodes");
    }
}

/*
 * Remove from the open set.
 */
static void dmap_astar_remove_from_open (dmap *map, dmap_astar_node *node)
{
    verify(node);

    if (!map->open[node->x][node->y]) {
        DIE("not in in open");
    }

    map->open[node->x][node->y] = 0;

    if (!tree_remove(map->open_nodes, &node->node)) {
        DIE("failed to remove from open nodes");
    }
}

/*
 * Add to the closed set. Once added, never removed.
 */
static void dmap_astar_add_to_closed (dmap *map, dmap_astar_node *node)
{
    verify(node);

    if (map->closed[node->x][node->y]) {
        DIE("already in closed");
    }

    map->closed[node->x][node->y] = node;

    if (!tree_insert(map->closed_nodes, &node->node)) {
        DIE("failed to add start to add to closed nodes");
    }
}

/*
 * A guess cost from here to the goal.
 */
static int16_t dmap_astar_cost_est_from_here_to_goal (dmap *map,
                                                      int16_t x, int16_t y)
{
    /*
     * Manhattan distance.
     */
    return (abs(map->goal_x - x) + abs(map->goal_y - y));
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

    if (nexthop_x >= TILES_MAP_EDITABLE_WIDTH - 1) {
        return;
    }

    if (nexthop_y >= TILES_MAP_EDITABLE_HEIGHT - 1) {
        return;
    }

    /*
     * Ignore walls.
     */
    if (walls[nexthop_x][nexthop_y] != ' ') {
        return;
    }

    /*
     * Reached a terminal goal; like an enemy?
     */
    dmap_goal *goal = map->goals[nexthop_x][nexthop_y];
    if (goal && (goal->score < 0)) {
        return;
    }

    /*
     * If in the closed set already, ignore.
     */
    if (map->closed[nexthop_x][nexthop_y]) {
        return;
    }

    int16_t distance_to_nexthop =
        map->score[nexthop_x][nexthop_y];

    /*
     * We use positive scores for good, but want to minimize distance.
     */
    distance_to_nexthop = -distance_to_nexthop;

    int16_t cost_from_start_to_here = current->cost_from_start_to_here +
                    distance_to_nexthop;
 
    dmap_astar_node *neighbor = map->open[nexthop_x][nexthop_y];
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
        dmap_astar_remove_from_open(map, neighbor);

        neighbor->came_from = current;
        neighbor->cost_from_start_to_here = cost_from_start_to_here;
        neighbor->cost_from_start_to_goal = cost_from_start_to_here +
            dmap_astar_cost_est_from_here_to_goal(map, nexthop_x, nexthop_y);

        dmap_astar_add_to_open(map, neighbor);
    }
}

static void dmap_astar_reconstruct_path (dmap *map, dmap_astar_node *came_from)
{
    int16_t x;
    int16_t y;

    /*
     * And best path for debugging.
     */
#ifdef ENABLE_MAP_DEBUG
    memset(map->best, ' ', sizeof(map->best));
#endif

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
    int16_t start_x = t->grid_x;
    int16_t start_y = t->grid_y;

    map->goal_x = target_x;
    map->goal_y = target_y;

    /*
     * The set of tentative nodes to be evaluated.
     */
    map->open_nodes =
            tree_alloc_custom(dmap_astar_compare, "TREE ROOT: A* open");

    map->closed_nodes =
            tree_alloc_custom(dmap_astar_compare, "TREE ROOT: A* closed");

    memset(map->open, 0, sizeof(map->open));

    /*
     * The set of nodes already evaluated.
     */
    memset(map->closed, 0, sizeof(map->closed));

    /*
     * Create the start node.
     */
    dmap_astar_node *node = dmap_astar_alloc(start_x, start_y);
    dmap_astar_add_to_open(map, node);

    boolean goal_found = false;

    /*
     * Estimated total cost from start to goal through y.
     */
    node->cost_from_start_to_here = 0;
    node->cost_from_start_to_goal =
            dmap_astar_cost_est_from_here_to_goal(map, start_x, start_y);

    while (!tree_root_is_empty(map->open_nodes)) {
        /*
         * current := the node in openset having the lowest f_score[] value
         */
        dmap_astar_node *current =
                (typeof(current)) tree_root_first(map->open_nodes);

        /*
         * Reached the goal?
         */
        if ((current->x == map->goal_x) && (current->y == map->goal_y)) {
            dmap_astar_reconstruct_path(map, current);
            goal_found = true;
            break;
        }

        /*
         * Remove current from openset
         */
        dmap_astar_remove_from_open(map, current);

        /*
         * Add current to closedset
         */
        dmap_astar_add_to_closed(map, current);

        dmap_astar_eval_neighbor(map, current, -1, 0);
        dmap_astar_eval_neighbor(map, current,  1, 0);
        dmap_astar_eval_neighbor(map, current,  0, -1);
        dmap_astar_eval_neighbor(map, current,  0,  1);
    }

    tree_destroy(&map->open_nodes, 0);
    tree_destroy(&map->closed_nodes, 0);

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
     * Walk the things a number of times, working out our priorities.
     */
    uint16_t maxpass;
    uint16_t pass;

    if (thing_is_player(t)) {
        maxpass = 2;
    } else if (thing_is_esnail(t)) {
        maxpass = 4;
    } else if (thing_is_car(t)) {
        return;
    } else if (thing_is_plant(t)) {
        maxpass = 2;
    } else {
        DIE("need number of passes");
    }

    boolean running_away = false;
    boolean chasing = false;

    /*
     * Walk the map and find all targets. Flood each target into its own map
     * and merge with the combined map
     */
    for (pass = 0; pass < maxpass; pass++) {
        thingp thing_it;

        TREE_WALK_UNSAFE(things, thing_it) {
            /*
             * Not on the map.
             */
            if (!t->wid) {
                continue;
            }

            /*
             * Ignore carried things.
             */
            if (thing_item_owner(thing_it)) {
                continue;
            }

            /*
             * Try to ignore the dead!
             */
            if (thing_is_dead(thing_it)) {
                continue;
            }

            /*
             * If not on the map, ignore.
             */
            if (!thing_it->wid) {
                continue;
            }

            x = thing_it->grid_x;
            y = thing_it->grid_y;

            if (thing_is_esnail(t)) {
                if (pass == 0) {
                    /*
                     * Top priority.
                     */
                    if (thing_is_player(thing_it)) {
                        if (thing_has_powerup_rocket_count(thing_it)) {
                            /*
                             * If the player has a rocket, avoid!
                             */
                            dmap_goal_flood(map, -5, x, y);
                            running_away = true;
                        }
                    }

                    if (thing_is_bomb(thing_it)) {
                        /*
                         * Avoid bombs!
                         */
                        if (thing_is_open(thing_it)) {
                            dmap_goal_flood(map, -10, x, y);
                            running_away = true;
                        }
                    }
                } else if (pass == 1) {
                    /*
                     * Medium priority.
                     */
                    if (!running_away) {
                        if (thing_is_player(thing_it)) {
                            if (items_has_letter(thing_carried_items(thing_it))) {
                                /*
                                 * If the player has a letter, chase!
                                 */
                                dmap_goal_flood(map, 1, x, y);
                                chasing = true;
                            }
                        }
                    }
                } else if (pass == 2) {
                    /*
                     * Low priority.
                     */
                    if (!running_away) {
                        /*
                         * Chase food.
                         */
                        if (thing_is_food(thing_it)) {
                            dmap_goal_flood(map, 1, x, y);
                        }

                        /*
                         * Chase letter.
                         */
                        if (thing_is_letter(thing_it)) {
                            dmap_goal_flood(map, 1, x, y);
                        }

                        /*
                         * Chase bombs.
                         */
                        if (thing_is_bomb(thing_it)) {
                            if (!thing_is_open(thing_it)) {
                                dmap_goal_flood(map, 2, x, y);
                            }
                        }

                        /*
                         * Chase stars.
                         */
                        if (thing_is_star(thing_it)) {
                            dmap_goal_flood(map, 1, x, y);
                        }

                        /*
                         * Chase bonus letter.
                         */
                        if (thing_is_bonus_letter(thing_it)) {
                            dmap_goal_flood(map, 1, x, y);
                        }

                        /*
                         * Chase end pipe
                         */
                        if (thing_is_pipe(thing_it)) {
                            if ((x != thing_grid_x(thing_it)) &&
                                (y != thing_grid_y(thing_it))) {
                                if (level->end_pipe[x][y] != ' ') {
                                    dmap_goal_flood(map, 1, x, y);
                                }
                            }
                        }

                    }

                    if (thing_is_plant(thing_it)) {
                        if (thing_has_powerup_rocket_count(t)) {
                            /*
                             * If I have a rocket, chase it.
                             */
                            dmap_goal_flood(map, 1, x, y);
                        } else {
                            /*
                             * Avoid plants!
                             */
                            dmap_goal_flood(map, -10, x, y);
                        }
                    }

                    if (thing_is_seedpod(thing_it)) {
                        if (thing_has_powerup_rocket_count(t)) {
                            /*
                             * If I have a rocket, chase it.
                             */
                            dmap_goal_flood(map, 2, x, y);
                        }
                    }

                    if (thing_is_car(thing_it)) {
                        /*
                         * Avoid cars!
                         */
                        dmap_goal_flood(map, -12, x, y);
                    }
                } else if (pass == 3) {
                    /*
                     * Lowest priority.
                     */
                    if (!map->goal_nodes) {
                        /*
                         * Chase the exit if it is open and nothing else
                         * better to do.
                         */
                        if (level_is_exit_open(level)) {
                            if (thing_is_exit(thing_it)) {
                                dmap_goal_flood(map, 1, x, y);
                            }
                        }
                    }
                }
            } else if (thing_is_plant(t)) {
                if (pass == 0) {
                    /*
                     * Top priority.
                     */
                    if (thing_has_powerup_rocket_count(thing_it)) {
                        /*
                         * If the player has a rocket, avoid!
                         */
                        dmap_goal_flood(map, -10, x, y);
                        running_away = true;
                    }
                } else if (pass == 1) {
                    /*
                     * Medium priority.
                     */
                    if (!running_away) {
                        if (thing_is_player(thing_it)) {
                            /*
                             * Chases.
                             */
                            dmap_goal_flood(map, 1, x, y);
                        }

                        if (thing_is_esnail(thing_it)) {
                            /*
                             * Chases.
                             */
                            dmap_goal_flood(map, 1, x, y);
                        }
                    }

                }
            } else if (thing_is_player(t)) {
                if (pass == 0) {
                    /*
                     * Top priority.
                     */
                    if (thing_is_food(thing_it)) {
                        /*
                         * Chase food.
                         */
                        dmap_goal_flood(map, 1, x, y);
                        chasing = true;
                    }

                    /*
                     * If using a rocket, do not head for the exit.
                     */
                    if (thing_has_powerup_rocket_count(t)) {
                        chasing = true;
                    }
                } else if (pass == 1) {

                    /*
                     * Medium priority.
                     */
                    if (!chasing) {
                        /*
                         * If the exit is open, and no food left, head for 
                         * that.
                         */
                        if (thing_is_exit(thing_it)) {
                            if (level_is_exit_open(level)) {
                                if (!level_count_is_food(level)) {
                                    dmap_goal_flood(map, 1, x, y);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    /*
     * Round out the total of all the maps merged together.
     */
    dmap_normalize(map);
}

static boolean dmap_move_in_same_dir (dmap *map, levelp level, thingp t,
                                      int32_t *nexthop_x, int32_t *nexthop_y)
{
    boolean moved = false;

    int16_t x = t->grid_x;
    int16_t y = t->grid_y;

    if (t->last_x < t->grid_x) {
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
        if (t->last_x > t->grid_x) {
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
        if (t->last_y < t->grid_y) {
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
        if (t->last_y > t->grid_y) {
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
        if (t->last_x < t->grid_x) {
            if (walls[x-1][y] == ' ') {
                moved = true;
                *nexthop_x = x-1;
                *nexthop_y = y;
            }
        }
    }

    if (!moved) {
        if (t->last_x > t->grid_x) {
            if (walls[x+1][y] == ' ') {
                moved = true;
                *nexthop_x = x+1;
                *nexthop_y = y;
            }
        }
    }

    if (!moved) {
        if (t->last_y < t->grid_y) {
            if (walls[x][y-1] == ' ') {
                moved = true;
                *nexthop_x = x;
                *nexthop_y = y-1;
            }
        }
    }

    if (!moved) {
        if (t->last_y > t->grid_y) {
            if (walls[x][y+1] == ' ') {
                moved = true;
                *nexthop_x = x;
                *nexthop_y = y+1;
            }
        }
    }

    return (moved);
}

static boolean dmap_move_in_same_road_dir (dmap *map, levelp level, thingp t,
                                           int32_t *nexthop_x, int32_t *nexthop_y)
{
    boolean moved = false;

    int16_t x = t->grid_x;
    int16_t y = t->grid_y;

    if (t->last_x < t->grid_x) {
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
        if (t->last_x > t->grid_x) {
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
        if (t->last_y < t->grid_y) {
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
        if (t->last_y > t->grid_y) {
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
        if (t->last_x < t->grid_x) {
            if (level->roads[x-1][y] != ' ') {
                moved = true;
                *nexthop_x = x-1;
                *nexthop_y = y;
            }
        }
    }

    if (!moved) {
        if (t->last_x > t->grid_x) {
            if (level->roads[x+1][y] != ' ') {
                moved = true;
                *nexthop_x = x+1;
                *nexthop_y = y;
            }
        }
    }

    if (!moved) {
        if (t->last_y < t->grid_y) {
            if (level->roads[x][y-1] != ' ') {
                moved = true;
                *nexthop_x = x;
                *nexthop_y = y-1;
            }
        }
    }

    if (!moved) {
        if (t->last_y > t->grid_y) {
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

    if (thing_is_plant(t)) {
        memcpy(walls, level->monst_walls, sizeof(walls));
    } else {
        memcpy(walls, level->walls, sizeof(walls));
    }

    if (t->grid_x < 0) {
        return (false);
    }

    if (t->grid_y < 0) {
        return (false);
    }

    if (t->grid_x > TILES_MAP_EDITABLE_WIDTH - 1) {
        return (false);
    }

    if (t->grid_y > TILES_MAP_EDITABLE_HEIGHT - 1) {
        return (false);
    }

    /*
     * For finding oldest cells.
     */
    static uint16_t visited;

    t->visited[t->grid_x][t->grid_y] = ++visited;

    /*
     * Flood how far various nodes are from us.
     */
    dmap_distance_flood(map, t);

    /*
     * Find all goals.
     */
    dmap_goals_find(map, t);

    if (!map->goal_nodes) {
        /*
         * If no goal was found, try and keep moving the same way.
         */
        if (thing_is_car(t)) {
            found_goal = dmap_move_in_same_road_dir(map, level, t, nexthop_x, nexthop_y);
        } else {
            found_goal = dmap_move_in_same_dir(map, level, t, nexthop_x, nexthop_y);
        }
    } else {
        /*
         * Try goals in order.
         */
        while (tree_root_size(map->goal_nodes)) {
            dmap_goal *goal =
                    (typeof(goal)) tree_root_first(map->goal_nodes);

            /*
             * Goal is something we want to avoid? Ignore for now.
             */
            if (goal->score < 0) {
                dmap_goal_free(map, goal);
                continue;
            }

            /*
             * Too close to a bad goal? Ignore this goal for now.
             */
            if (map->score[goal->x][goal->y] < -7) {
                dmap_goal_free(map, goal);
                continue;
            }

            /*
             * Stick with the same next hop if nothing good.
             */
            if ((goal->x == t->grid_x) && (goal->y == t->grid_y)) {
                *nexthop_x = map->nexthop_x;
                *nexthop_y = map->nexthop_y;
                found_goal = true;
                break;
            }

            if (dmap_astar_best_path(map, t, goal->x, goal->y)) {
                *nexthop_x = map->nexthop_x;
                *nexthop_y = map->nexthop_y;
                found_goal = true;
                break;
            }

            dmap_goal_free(map, goal);
        }

        tree_destroy(&map->goal_nodes, 0);
    }

    /*
     * If no goal, just try and find the best area to head.
     */
    if (!found_goal) {
        int16_t target_x = -1;
        int16_t target_y = -1;

        dmap_find_best_cell(map, t, &target_x, &target_y);

        if ((target_x != t->grid_x) || (target_y != t->grid_y)) {
            if (dmap_astar_best_path(map, t, target_x, target_y)) {
                *nexthop_x = map->nexthop_x;
                *nexthop_y = map->nexthop_y;
                found_goal = true;
            }
        }
    }

    /*
     * If still no goal, just try and find the oldest cell.
     */
    if (!found_goal) {
        int16_t target_x = -1;
        int16_t target_y = -1;

        dmap_find_oldest_visited(map, t, &target_x, &target_y);

        if ((target_x != t->grid_x) || (target_y != t->grid_y)) {
            if (dmap_astar_best_path(map, t, target_x, target_y)) {
                *nexthop_x = map->nexthop_x;
                *nexthop_y = map->nexthop_y;
                found_goal = true;
            }
        }
    }

    if (!found_goal) {
        THING_LOG(t, "found no goal, try moving the same way");

        /*
         * If no goal was found, try and keep moving the same way.
         */
        if (thing_is_car(t)) {
            found_goal = dmap_move_in_same_road_dir(map, level, t, nexthop_x, nexthop_y);
        } else {
            found_goal = dmap_move_in_same_dir(map, level, t, nexthop_x, nexthop_y);
        }
    }

    if (!found_goal) {
        THING_LOG(t, "no goal");
        return (false);
    }

    if (level->walls[*nexthop_x][*nexthop_y] != ' ') {
        THING_LOG(t, "nexthop %d %d into a wall, found goal %d",
                  *nexthop_x, *nexthop_y, found_goal);

#ifdef ENABLE_MAP_DEBUG
        if (thing_is_esnail(t)) {
            dmap_print_map(map, t->grid_x, t->grid_y, true, true);
        }
#endif
        LOG("nexthop %d %d into a wall", *nexthop_x, *nexthop_y);

        return (false);
    }

    t->last_x = t->grid_x;
    t->last_y = t->grid_y;

    /*
     * Show likely best path.
     */
#ifdef ENABLE_MAP_DEBUG
    if (thing_is_esnail(t)) {
        dmap_print_map(map, t->grid_x, t->grid_y, true, true);
    }
#endif

    return (true);
}

boolean thing_find_nexthop (thingp t, int32_t *nexthop_x, int32_t *nexthop_y)
{
    verify(t);

    /*
     * This is the merged map with scores from all targets.
     */
    static dmap map;
    memset(&map, 0, sizeof(map));
    levelp level = map.level = thing_level(t);

    return (dmap_find_nexthop(&map, level, t, nexthop_x, nexthop_y));
}
