/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */

#define __STDC_LIMIT_MACROS
#include <pthread.h>

#include "main.h"
#include "thing.h"
#include "wid_game_map_server.h"
#include "map.h"
#include "math_util.h"

static FILE *fp;
static const int8_t is_a_wall = 63;
static const int8_t not_preferred = 62;

/*
 * We use two threads one to calculate dmaps whenever the player moves
 *
 * The other is uses to create dmaps at level start for every position.
 * This takes a long time.
 */
static pthread_t dmap_thread1;
static pthread_mutex_t dmap_thread1_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t dmap_thread1_cond = PTHREAD_COND_INITIALIZER;

static pthread_t dmap_thread2;
static pthread_mutex_t dmap_thread2_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t dmap_thread2_cond = PTHREAD_COND_INITIALIZER;

/*
 * Final dmaps
 */
static level_walls dmap_map_player_target_treat_doors_as_passable;
static level_walls dmap_map_player_target_treat_doors_as_walls;
static level_walls dmap_map_treasure_target_treat_doors_as_passable;
static level_walls dmap_map_treasure_target_treat_doors_as_walls;

/*
 * Scratch pad dmaps are updated to the above dmaps when complete so the 
 * monster never sees a partail dmap.
 */
static level_walls dmap_map_player_target_treat_doors_as_passable_scratchpad;
static level_walls dmap_map_player_target_treat_doors_as_walls_scratchpad;
static level_walls dmap_map_treasure_target_treat_doors_as_passable_scratchpad;
static level_walls dmap_map_treasure_target_treat_doors_as_walls_scratchpad;

/*
 * For individual players. Updated each time we need it.
 */
static level_walls dmap_map_player_target_treat_doors_as_walls_scratchpad;
static level_walls dmap_map_treasure_target_treat_doors_as_walls_scratchpad;

static int update_treasure_dmap;
static int update_player_dmap;

/*
 * Djkstra maps so we can quickly tell the next hop.
 *
 * This is for every single map position, how to get there.
 */
static level_walls dmap_map_wander[MAP_WIDTH][MAP_HEIGHT];

/*
 * Non straight line distance, avoiding walls.
 *
 * TAKE CARE: takes some seconds for all points to be updated at level start.
 */
int dmap_distance_between_points (int target_x, int target_y, int source_x, int source_y)
{
    if ((target_x >= MAP_WIDTH) || (target_x < 0)) {
        return (-1);
    }

    if ((target_y >= MAP_HEIGHT) || (target_y < 0)) {
        return (-1);
    }

    if ((source_x >= MAP_WIDTH) || (source_x < 0)) {
        return (-1);
    }

    if ((source_y >= MAP_HEIGHT) || (source_y < 0)) {
        return (-1);
    }

    int distance = dmap_map_wander[target_x][target_y].walls[source_x][source_y];

    if (distance >= not_preferred) {
        return (-1);
    }

    return (distance);
}

/*
 * Non straight line distance to player, avoiding walls.
 *
 * Is updated each player move to a new cell
 */
int dmap_distance_to_player (int source_x, int source_y)
{
    if ((source_x >= MAP_WIDTH) || (source_x < 0)) {
        return (-1);
    }

    if ((source_y >= MAP_HEIGHT) || (source_y < 0)) {
        return (-1);
    }

    int distance = (dmap_map_player_target_treat_doors_as_walls.walls[source_x][source_y]);

    if (distance >= not_preferred) {
        return (-1);
    }

    return (distance);
}

/*
 * Print the Dijkstra map scores shared by all things of the same type.
 */
static void dmap_print (levelp level, level_walls *dmap)
{
    int8_t x;
    int8_t y;

    if (!fp) {
        fp = fopen("map.txt", "w");
    }

    for (y = 0; y < MAP_HEIGHT; y++) {
        for (x = 0; x < MAP_WIDTH; x++) {
            if (map_is_exit_at(level, x, y)) {
                fprintf(fp, " Ex ");
                continue;
            }
            if (map_is_door_at(level, x, y)) {
                fprintf(fp, " Do ");
                continue;
            }
            if (map_is_mob_spawner_at(level, x, y)) {
                fprintf(fp, " Gn ");
                continue;
            }
            if (map_is_food_at(level, x, y)) {
                fprintf(fp, " Fd ");
                continue;
            }

            if (!map_is_floor_at(level, x, y)) {
                fprintf(fp, " -- ");
                continue;
            }

            fprintf(fp, "%4d", dmap->walls[x][y]);
        }

        fprintf(fp, "\n");
    }

    fprintf(fp, "\n");
}

/*
 * Print the Dijkstra map scores shared by all things of the same type.
 */
static void dmap_thing_print (thingp t, 
                              int8_t nexthop_x,
                              int8_t nexthop_y)
{
    int8_t x;
    int8_t y;

    if (!fp) {
        fp = fopen("map.txt", "w");
    }

    int8_t tx;
    int8_t ty;

    tx = rintf(t->x);
    ty = rintf(t->y);

    for (y = 0; y < MAP_HEIGHT; y++) {
        for (x = 0; x < MAP_WIDTH; x++) {
            if ((nexthop_x == x) && (nexthop_y == y)) {
                fprintf(fp, " Nh ");
            } else {
                if ((x == tx) && (y == ty)) {
                    fprintf(fp, " Mo ");
                } else {
                    fprintf(fp, "%4d", 
                            dmap_map_player_target_treat_doors_as_walls.walls[x][y]);
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
static void dmap_process (level_walls *dmap, level_walls *dmap_final)
{
    uint8_t x;
    uint8_t y;
    int8_t a;
    int8_t b;
    int8_t c;
    int8_t d;
    int8_t *e;
    int8_t f;
    int8_t g;
    int8_t h;
    int8_t i;
    int8_t lowest;
    uint8_t changed;

    do {
        changed = false;

        for (x = 1; x < MAP_WIDTH - 1; x++) {
            for (y = 1; y < MAP_HEIGHT - 1; y++) {
                e = &dmap->walls[x  ][y];
                if (*e == is_a_wall) {
                    continue;
                }

                a =  dmap->walls[x-1][y-1] * 2;
                b =  dmap->walls[x  ][y-1];
                c =  dmap->walls[x+1][y-1] * 2;

                d =  dmap->walls[x-1][y];
                f =  dmap->walls[x+1][y];
                 
                g =  dmap->walls[x-1][y+1] * 2;
                h =  dmap->walls[x  ][y+1];
                i =  dmap->walls[x+1][y+1] * 2;

#define THIS_IS_FASTER
#ifdef THIS_IS_FASTER
                if (a < b) {
                    lowest = a;
                } else {
                    lowest = b;
                }

                if (c < lowest) { lowest = c; }
                if (d < lowest) { lowest = d; }
                if (f < lowest) { lowest = f; }
                if (g < lowest) { lowest = g; }
                if (h < lowest) { lowest = h; }
                if (i < lowest) { lowest = i; }
#else
                lowest = min(a, 
                             min(b, 
                                 min(c, 
                                     min(d, 
                                         min(f, 
                                             min(g, 
                                                 min(h,i)))))));
#endif

                if (*e - lowest >= 2) {
                    *e = lowest + 1;
                    changed = true;
                }
            }
        }
    } while (changed);

#ifdef GORY_DEBUG
    do {
        for (x = 1; x < MAP_WIDTH - 1; x++) {
            for (y = 1; y < MAP_HEIGHT - 1; y++) {
                e = &dmap->walls[x  ][y];
                if (*e == is_a_wall) {
                    printf("  XX");
                    continue;
                }

                printf("%4d", *e);
            }
            printf("\n");
        }
        printf("\n");
    } while (changed);
#endif

    /*
     * Only update the map when done so the monsters never see a map work
     * in progress.
     */
    memcpy(dmap_final, dmap, sizeof(level_walls));
}

/*
 * Generate goal points with a low value.
 */
static uint32_t dmap_generate_for_player_target_set_goals (uint8_t test, level_walls *dmap)
{
    uint32_t checksum = 0;
    thingp thing_it;
    int8_t x;
    int8_t y;

    { TREE_OFFSET_WALK_UNSAFE(server_player_things, thing_it) {
        /*
         * Only chase players.
         */
        if (!thing_is_player_noverify(thing_it)) {
            continue;
        }

        /*
         * Ignore dead players.
         */
        if (thing_is_dead_or_dying_noverify(thing_it)) {
            continue;
        }

        /*
         * Aim for center of tile.
         */
        x = rintf(thing_it->x);
        y = rintf(thing_it->y);

        if (!test) {
            dmap->walls[x][y] = 0;
        }

        checksum ^= x | (y << 16);
        checksum = checksum << 1;
    } }

    return (checksum);
}

/*
 * Generate goal points with a low value.
 */
static uint32_t dmap_generate_for_treasure_target_set_goals (uint8_t test, level_walls *dmap)
{
    uint32_t checksum = 0;
    thingp thing_it;
    int8_t x;
    int8_t y;

    { TREE_OFFSET_WALK_UNSAFE(server_boring_things, thing_it) {
        /*
         * Only chase treasures.
         */
        if (!thing_is_treasure_noverify(thing_it)) {
            continue;
        }

        /*
         * Aim for center of tile.
         */
        x = rintf(thing_it->x);
        y = rintf(thing_it->y);

        if (!test) {
            dmap->walls[x][y] = 0;
        }

        checksum ^= x | (y << 16);
        checksum = checksum << 1;
    } }

    return (checksum);
}

/*
 * Initialize the djkstra map with high values.
 */
static void dmap_init (level_walls *dmap, const level_walls *map)
{
    int8_t x;
    int8_t y;

    for (x = 0; x < MAP_WIDTH; x++) {
        for (y = 0; y < MAP_HEIGHT; y++) {
            if (map->walls[x][y] != ' ') {
                dmap->walls[x][y] = is_a_wall;
                continue;
            }

            dmap->walls[x][y] = not_preferred;
        }
    }
}

/*
 * Run forever waiting to be woken up and then running the djkstra map.
 */
static void *dmap_thread1_func (void *context)
{
    for (;;) {
        pthread_mutex_lock(&dmap_thread1_mutex);

        pthread_cond_wait(&dmap_thread1_cond, &dmap_thread1_mutex );

        /*
         * Start with a clean dmap for each set of obstacles to consider.
         */
        if (update_player_dmap) {
            dmap_process(&dmap_map_player_target_treat_doors_as_passable_scratchpad,
                         &dmap_map_player_target_treat_doors_as_passable);
            dmap_process(&dmap_map_player_target_treat_doors_as_walls_scratchpad,
                         &dmap_map_player_target_treat_doors_as_walls);
            update_player_dmap = false;
        }

        if (update_treasure_dmap) {
            dmap_process(&dmap_map_treasure_target_treat_doors_as_passable_scratchpad,
                         &dmap_map_treasure_target_treat_doors_as_passable);
            dmap_process(&dmap_map_treasure_target_treat_doors_as_walls_scratchpad,
                         &dmap_map_treasure_target_treat_doors_as_walls);
            update_treasure_dmap = false;
        }

        pthread_mutex_unlock(&dmap_thread1_mutex);
    }

    return (0);
}

static int dmap_generate_for_player_target (levelp level, int force)
{
    if (!force) {
        static uint32_t dmap_checksum;

        /*
        * Only reprocess the djkstra map if something has changed on the map
        * We use a checksum of the goals to indicate this with reasonable 
        * certainty.
        */
        uint32_t checksum = dmap_generate_for_player_target_set_goals(true /* test */, 0);

        if (dmap_checksum == checksum) {
            return (0);
        }

        dmap_checksum = checksum;
    }

    update_player_dmap = true;

    dmap_init(&dmap_map_player_target_treat_doors_as_passable_scratchpad,
              &level->map_player_target_treat_doors_as_passable);
    dmap_generate_for_player_target_set_goals(false /* test */,
                   &dmap_map_player_target_treat_doors_as_passable_scratchpad);

    dmap_init(&dmap_map_player_target_treat_doors_as_walls_scratchpad,
              &level->map_player_target_treat_doors_as_walls);
    dmap_generate_for_player_target_set_goals(false /* test */,
                   &dmap_map_player_target_treat_doors_as_walls_scratchpad);

    return (1);
}

static int dmap_generate_for_treasure_target (levelp level)
{
    static uint32_t dmap_checksum;

    /*
     * Only reprocess the djkstra map if something has changed on the map
     * We use a checksum of the goals to indicate this with reasonable 
     * certainty.
     */
    uint32_t checksum = dmap_generate_for_treasure_target_set_goals(true /* test */, 0);

    if (dmap_checksum == checksum) {
        return (0);
    }

    dmap_checksum = checksum;

    update_treasure_dmap = true;

    dmap_init(&dmap_map_treasure_target_treat_doors_as_passable_scratchpad,
              &level->map_treasure_target_treat_doors_as_passable);
    dmap_generate_for_treasure_target_set_goals(false /* test */,
                   &dmap_map_treasure_target_treat_doors_as_passable_scratchpad);

    dmap_init(&dmap_map_treasure_target_treat_doors_as_walls_scratchpad,
              &level->map_treasure_target_treat_doors_as_walls);
    dmap_generate_for_treasure_target_set_goals(false /* test */,
                   &dmap_map_treasure_target_treat_doors_as_walls_scratchpad);

    return (1);
}

/*
 * Wake up the thread that creates the djkstra map.
 */
static void dmap_thread1_wake (levelp level, int force)
{
    if (!pthread_mutex_trylock(&dmap_thread1_mutex)) {
        return;
    }

    int modified;
    
    modified = dmap_generate_for_player_target(level, force);
    modified += dmap_generate_for_treasure_target(level);

    /*
     * Now wake the dmap processor.
     */
    if (modified) {
        pthread_cond_signal(&dmap_thread1_cond);
    }

    pthread_mutex_unlock(&dmap_thread1_mutex);
}

/*
 * Run forever waiting to be woken up and then running the djkstra map.
 */
static void *dmap_thread2_func (void *context)
{
    for (;;) {

        pthread_mutex_lock(&dmap_thread2_mutex);

        pthread_cond_wait(&dmap_thread2_cond, &dmap_thread2_mutex);

        for (;;) {
            if (!server_level) {
                /*
                * Happens whilst we load the level and before we set the 
                * level pointer.
                */
                SDL_Delay(1000);
                continue;
            }

            server_level->locked++;

            level_walls tmp;
            uint32_t x, y;

            for (x = 0; x < MAP_WIDTH; x++) {
                for (y = 0; y < MAP_HEIGHT; y++) {
                    dmap_init(&tmp, &server_level->map_player_target_treat_doors_as_walls);

                    /*
                    * If a wall then we can't get to it, period.
                    */
                    if (server_level->
                            map_player_target_treat_doors_as_walls.walls[x][y] != ' ') {
                        continue;
                    }

                    /*
                    * Set the goal.
                    */
                    tmp.walls[x][y] = 0;

                    dmap_process(&tmp, &dmap_map_wander[x][y]);
                }

                if (server_level->exit_request) {
                    break;
                }
            }

            server_level->locked--;

            break;
        }

        pthread_mutex_unlock(&dmap_thread2_mutex);
    }

    return (0);
}

/*
 * Wake up the thread that creates the djkstra map.
 */
static void dmap_thread2_wake (void)
{
    /*
     * Now wake the dmap processor.
     */
    pthread_cond_signal(&dmap_thread2_cond);
}

static void dmap_thread1_init (void)
{
    int rc = pthread_create(&dmap_thread1, NULL, &dmap_thread1_func, NULL);

    if (rc != 0) {
        ERR("no dmap thread %s", strerror(rc));
    }
}

static void dmap_thread1_fini (void)
{
    return;
}

static void dmap_thread2_init (void)
{
    int rc = 
        pthread_create(&dmap_thread2, NULL, &dmap_thread2_func, NULL);

    if (rc != 0) {
        ERR("no dmap thread %s", strerror(rc));
    }
}

static void dmap_thread2_fini (void)
{
    return;
}

void dmap_process_init (void)
{
    dmap_thread1_init();
    dmap_thread2_init();
}

void dmap_process_fini (void)
{
    dmap_thread1_fini();
    dmap_thread2_fini();
}

/*
 * Generate maps to allow things to wander to any location.
 */
void dmap_generate_map_wander (levelp level)
{
    dmap_thread2_wake();
}

/*
 * Generate a djkstra map for the thing.
 */
void dmap_generate (levelp level, int force)
{
    dmap_thread1_wake(level, force);

#ifdef ENABLE_MAP_DEBUG
    if (1)
#else
    if (/* DISABLES CODE */ (0))
#endif
    dmap_print(level, &dmap_map_player_target_treat_doors_as_passable);
}

static uint8_t thing_find_nexthop_dmap (thingp t, 
                                        level_walls *dmap,
                                        int32_t *nexthop_x, 
                                        int32_t *nexthop_y)
{
    int8_t x;
    int8_t y;

    x = rintf(t->x);
    y = rintf(t->y);

    int8_t a;
    int8_t b;
    int8_t c;
    int8_t d;
    int8_t e;
    int8_t f;
    int8_t g;
    int8_t h;
    int8_t i;
    int lowest;

    a = dmap->walls[x-1][y-1];
    b = dmap->walls[x  ][y-1];
    c = dmap->walls[x+1][y-1];

    d = dmap->walls[x-1][y];
    e = dmap->walls[x  ][y];
    f = dmap->walls[x+1][y];
        
    g = dmap->walls[x-1][y+1];
    h = dmap->walls[x  ][y+1];
    i = dmap->walls[x+1][y+1];

    /*
     * Some things like archers want to stay a distance away
     * from the player.
     */
    tpp tp;
    tp = thing_tp(t);
    uint32_t min_distance = tp_get_approach_distance(tp);
    if (min_distance) {
        if (a < min_distance) { a += not_preferred; }
        if (b < min_distance) { b += not_preferred; }
        if (c < min_distance) { c += not_preferred; }
        if (d < min_distance) { d += not_preferred; }
        if (e < min_distance) { e += not_preferred; }
        if (f < min_distance) { f += not_preferred; }
        if (g < min_distance) { g += not_preferred; }
        if (h < min_distance) { h += not_preferred; }
        if (i < min_distance) { i += not_preferred; }
    }

    lowest = min(a, min(b, min(c, min(d, min(e, min(f, min(g, min(h,i))))))));

    if (a != lowest) { a += not_preferred; }
    if (b != lowest) { b += not_preferred; }
    if (c != lowest) { c += not_preferred; }
    if (d != lowest) { d += not_preferred; }
    if (e != lowest) { e += not_preferred; }
    if (f != lowest) { f += not_preferred; }
    if (g != lowest) { g += not_preferred; }
    if (h != lowest) { h += not_preferred; }
    if (i != lowest) { i += not_preferred; }

    lowest = min(a, min(b, min(c, min(d, min(e, min(f, min(g, min(h,i))))))));

    int8_t dx = 0;
    int8_t dy = 0;

    if (a == lowest) { dx = -1; dy = -1; }
    else if (b == lowest) { dx =  0; dy = -1; }
    else if (c == lowest) { dx = +1; dy = -1; }
    else if (d == lowest) { dx = -1; dy =  0; }
    else if (e == lowest) { dx =  0; dy =  0; }
    else if (f == lowest) { dx = +1; dy =  0; }
    else if (g == lowest) { dx = -1; dy =  1; }
    else if (h == lowest) { dx =  0; dy =  1; }
    else if (i == lowest) { dx = +1; dy =  1; }
    else {
        return (false);
    }

    /*
     * Best hop is into something we can't move onto?
     */
    if (lowest == not_preferred) {
        return (false);
    }

    if (lowest == is_a_wall) {
        return (false);
    }

    if (lowest > (int) tp_get_vision_distance(tp)) {
        return (false);
    }

    /*
     * Success.
     */
    *nexthop_x = x + dx;
    *nexthop_y = y + dy;

#ifdef ENABLE_MAP_DEBUG
    if (1)
#else
    if (/* DISABLES CODE */ (0))
#endif
    dmap_thing_print(t, *nexthop_x, *nexthop_y);

    return (true);
}

static uint8_t thing_try_nexthop (thingp t,
                                  int32_t *nexthop_x, 
                                  int32_t *nexthop_y,
                                  uint8_t can_change_dir_without_moving)
{
    if (!can_change_dir_without_moving) {
        if (thing_hit_solid_obstacle(wid_game_map_server_grid_container, 
                                        t,
                                        *nexthop_x, *nexthop_y)) {
            return (false);
        }
    }

    if (thing_server_move(t, *nexthop_x, *nexthop_y,
                          *nexthop_y < t->y, *nexthop_y > t->y,
                          *nexthop_x < t->x, *nexthop_x > t->x, 
                          false, /* fire */
                          false  /* magic */)) {
        return (true);
    }

    return (false);
}

static uint8_t thing_dmap_try_nexthop (thingp t,
                                       level_walls *dmap,
                                       int32_t *nexthop_x, 
                                       int32_t *nexthop_y,
                                       uint8_t can_change_dir_without_moving)
{
    if (thing_find_nexthop_dmap(t, dmap, nexthop_x, nexthop_y)) {

        if (!can_change_dir_without_moving) {
            if (thing_hit_solid_obstacle(wid_game_map_server_grid_container, 
                                         t,
                                         *nexthop_x, *nexthop_y)) {
                return (false);
            }
        }

        if (thing_server_move(t, *nexthop_x, *nexthop_y,
                              *nexthop_y < t->y, *nexthop_y > t->y,
                              *nexthop_x < t->x, *nexthop_x > t->x, 
                              false, /* fire */
                              false  /* magic */)) {
            return (true);
        }
    }

    return (false);
}

static int thing_chase_closest_player (thingp t, int32_t *nexthop_x, int32_t *nexthop_y)
{
    double distance = 9999;
    thingp best = 0;
    thingp thing_it;

    { TREE_OFFSET_WALK_UNSAFE(server_player_things, thing_it) {
        /*
         * Only chase players.
         */
        if (!thing_is_player_noverify(thing_it)) {
            continue;
        }

        /*
         * Ignore dead players.
         */
        if (thing_is_dead_or_dying_noverify(thing_it)) {
            continue;
        }

        if (DISTANCE(thing_it->x, thing_it->y, t->x, t->y) < distance) {
            best = thing_it;
        }
    } }

    if (!best) {
        return (false);
    }

    *nexthop_x = t->x;
    *nexthop_y = t->y;

    if (best->x > t->x) {
        *nexthop_x = t->x + 1.0;
    }

    if (best->x < t->x) {
        *nexthop_x = t->x - 1.0;
    }

    if (best->y > t->y) {
        *nexthop_y = t->y + 1.0;
    }

    if (best->y < t->y) {
        *nexthop_y = t->y - 1.0;
    }

    return (true);
}

uint8_t thing_find_nexthop (thingp t, int32_t *nexthop_x, int32_t *nexthop_y)
{
    /*
     * Walk through walls to get to the player?
     */
    if (thing_is_ethereal(t)) {
        if (thing_chase_closest_player(t, nexthop_x, nexthop_y)) {

            if (thing_try_nexthop(t, nexthop_x, nexthop_y,
                                  false /* can_change_dir_without_moving */)) {
                return (true);
            }

            return (true);
        }
    }

    /*
     * Start out with treating doors as passable.
     */
    if (!t->dmap) {
        if (thing_is_shopkeeper(t)) {
            if (thing_is_angry(t)) {
                t->dmap = &dmap_map_player_target_treat_doors_as_passable;
            } else {
                t->dmap = 0;
            }
        } else if (thing_is_treasure_eater(t)) {
            t->dmap = &dmap_map_treasure_target_treat_doors_as_passable;
        } else {
            t->dmap = &dmap_map_player_target_treat_doors_as_passable;
        }
    }

    /*
     * Try the current map.
     */
    if (t->dmap) {
        if (thing_dmap_try_nexthop(t, t->dmap, nexthop_x, nexthop_y,
                                   true /* can_change_dir_without_moving */)) {
            return (true);
        }
    }

    /*
     * Try the alternative map.
     */
    if (thing_is_shopkeeper(t)) {
        /*
         * Chase players only if angry.
         */
        if (thing_is_angry(t)) {
            if (t->dmap == &dmap_map_player_target_treat_doors_as_passable) {
                t->dmap = &dmap_map_player_target_treat_doors_as_walls;
            } else {
                t->dmap = &dmap_map_player_target_treat_doors_as_passable;
            }
        } else {
            t->dmap = 0;
        }
    } else if (thing_is_treasure_eater(t)) {
        if (t->dmap == &dmap_map_treasure_target_treat_doors_as_passable) {
            t->dmap = &dmap_map_treasure_target_treat_doors_as_walls;
        } else {
            t->dmap = &dmap_map_treasure_target_treat_doors_as_passable;
        }
    } else {
        if (t->dmap == &dmap_map_player_target_treat_doors_as_passable) {
            t->dmap = &dmap_map_player_target_treat_doors_as_walls;
        } else {
            t->dmap = &dmap_map_player_target_treat_doors_as_passable;
        }
    }

    if (t->dmap) {
        if (thing_dmap_try_nexthop(t, t->dmap, nexthop_x, nexthop_y,
                                   false /* can_change_dir_without_moving */)) {
            return (true);
        }
    }

    /*
     * Still can't move, if we're a treasure eater, now consider the player as 
     * a secondary target.
     */
    if (thing_is_treasure_eater(t)) {
        if (t->dmap == &dmap_map_player_target_treat_doors_as_passable) {
            t->dmap = &dmap_map_player_target_treat_doors_as_walls;
        } else {
            t->dmap = &dmap_map_player_target_treat_doors_as_passable;
        }

        if (t->dmap) {
            if (thing_dmap_try_nexthop(t, t->dmap, nexthop_x, nexthop_y,
                                       false /* can_change_dir_without_moving */)) {
                return (true);
            }
        }
    }

    /*
     * Still nowhere to move? Try the wander map.
     */
    if (t->dmap_wander) {
        if (!t->dmap_wander->walls[(int)t->x][(int)t->y]) {
            /*
             * Reached the destination. Try a new map.
             */
            t->dmap_wander = 0;
        }
    }

    /*
     * Find a wander destination.
     */
    uint32_t x;
    uint32_t y;
    uint32_t tries = 0;

    thing_map *map = thing_get_map(t);

    for (;;) {
        x = myrand() % MAP_WIDTH;
        y = myrand() % MAP_HEIGHT;

        tries++;
        if (tries > 100) {
            break;
        }

        /*
         * Make sure we can at least reach the target we chose
         */
        int cost_to_this_target = dmap_map_wander[x][y].walls[(int)t->x][(int)t->y];
        if (cost_to_this_target >= not_preferred) {
            continue;
        }

        thing_map_cell *cell = &map->cells[x][y];
        int done = false;
        uint32_t i;
        for (i = 0; i < cell->count; i++) {
            thingp it;
            
            if (t->on_server) {
                it = thing_server_id(cell->id[i]);
            } else {
                it = thing_client_id(cell->id[i]);
            }

            if (thing_is_shopkeeper(t)) {
                if (!thing_is_angry(t)) {
                    /*
                     * Not angry. Just wander the shop floor.
                     */
                    if (thing_is_shop_floor(it)) {
                        done = true;
                        break;
                    }

                    /*
                     * Make sure we stay in the same shop.
                     */
                    if (DISTANCE(x, y, t->x, t->y) > 4.0) {
                        continue;
                    }

                    /*
                     * Keep looking for a shop floor.
                     */
                    continue;
                } else {
                    /*
                     * Lost sight of the player. Hunting.
                     */
                    done = true;
                    break;
                }
            } else {
                /*
                 * Monsters just wander anywhere.
                 */
                done = true;
                break;
            }

            done = true;
            break;
        }

        if (done) {
            break;
        }
    }

    if (!t->dmap_wander) {
        t->dmap_wander = &dmap_map_wander[x][y];
    }

    if (thing_dmap_try_nexthop(t, t->dmap_wander, nexthop_x, nexthop_y,
                               false /* can_change_dir_without_moving */)) {
        return (true);
    }

    /*
     * Try a new wander map next time.
     */
//    t->dmap_wander = &dmap_map_wander[x][y];
//    shouldn't this be 0 to get a new map?
    t->dmap_wander = 0;

    return (false);
}
