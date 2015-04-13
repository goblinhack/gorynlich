/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "thing.h"
#include "tile.h"
#include "wid.h"
#include "map.h"
#include "math_util.h"
#include "level.h"
#include "wid_game_map_server.h"

typedef struct {
    thingp target;
    const char *reason;
    uint16_t priority;
    uint8_t hitter_killed_on_hitting:1;
} thing_possible_hit;

#define MAX_THING_POSSIBLE_HIT 16

static thing_possible_hit thing_possible_hits[MAX_THING_POSSIBLE_HIT];
static uint32_t thing_possible_hit_size;

/*
 * Add a thing to the list of things that could be hit on this attack.
 */
static void 
thing_possible_hit_add_hitter_killed_on_hitting_ (thingp target,
                                                  const char *reason,
                                                  int hitter_killed_on_hitting)
{
    if (thing_possible_hit_size >= MAX_THING_POSSIBLE_HIT) {
        return;
    }

    thing_possible_hit *h = &thing_possible_hits[thing_possible_hit_size++];
    memset(h, 0, sizeof(*h));
    h->target = target;
    h->priority = tp_get_hit_priority(target->tp);
    h->hitter_killed_on_hitting = hitter_killed_on_hitting;
}

static void 
thing_possible_hit_add (thingp target, const char *reason)
{
    thing_possible_hit_add_hitter_killed_on_hitting_(target,
                                                     reason,
                                                     false);
}

static void 
thing_possible_hit_add_hitter_killed_on_hitting (thingp target,
                                                 const char *reason)
{
    thing_possible_hit_add_hitter_killed_on_hitting_(target,
                                                     reason,
                                                     true);
}

/*
 * Reset the list of things we can possibly hit.
 */
static void thing_possible_init (void)
{
    thing_possible_hit_size = 0;
}

/*
 * Find the thing with the highest priority to hit.
 */
static void thing_possible_hit_do (thingp hitter)
{
    thing_possible_hit *best = 0;
    uint32_t i;

    for (i = 0; i < thing_possible_hit_size; i++) {
        thing_possible_hit *cand = &thing_possible_hits[i];

        /*
         * Don't be silly and hit yourself.
         */
        if (cand->target == hitter) {
            continue;
        }

        /*
         * Skip things that aren't really hitable.
         */
        if (thing_is_animation(cand->target) ||
            thing_is_explosion(cand->target) ||
            thing_is_weapon_swing_effect(cand->target)) {
            continue;
        }

        if (!best) {
            best = cand;
            continue;
        }

        if (cand->priority > best->priority) {
            /*
             * If this target is higher prio, prefer it.
             */
            best = cand;
        } else if (cand->priority == best->priority) {
            /*
             * If this target is closer, prefer it.
             */
            double dist_best = DISTANCE(hitter->x, hitter->y,
                                        best->target->x, best->target->y);
            double dist_cand = DISTANCE(hitter->x, hitter->y,
                                        cand->target->x, cand->target->y);

            if (dist_cand < dist_best) {
                best = cand;
            }
        }
    }

    if (best) {
        if (thing_hit(best->target, hitter, 0)) {
            if (best->hitter_killed_on_hitting) {
                thing_dead(hitter, 0, "hit");
            }
        }
    }

    thing_possible_init();
}

/*
 * On the server, things move in jumps. Find the real position the client
 * will see so collisions look more accurate.
 */
void 
thingp_get_interpolated_position (const thingp t, double *x, double *y)
{
    widp w = thing_wid(t);

    if (!wid_is_moving(w)) {
        *x = t->x;
        *y = t->y;
        return;
    }

    double wdx, wdy;
    double dx = t->x - t->last_x;
    double dy = t->y - t->last_y;

    wid_get_move_interpolated_progress(thing_wid(t), &wdx, &wdy);

    *x = t->last_x + (dx * wdx);
    *y = t->last_y + (dy * wdy);
}

static uint8_t things_overlap (const thingp A, 
                               double nx,
                               double ny,
                               const thingp B)
{
    static tilep wall;
    static double xscale;
    static double yscale;
    static double collision_map_large_x1;
    static double collision_map_large_x2;
    static double collision_map_large_y1;
    static double collision_map_large_y2;
    static double collision_map_medium_x1;
    static double collision_map_medium_x2;
    static double collision_map_medium_y1;
    static double collision_map_medium_y2;
    static double collision_map_small_x1;
    static double collision_map_small_x2;
    static double collision_map_small_y1;
    static double collision_map_small_y2;
    static double collision_map_tiny_x1;
    static double collision_map_tiny_x2;
    static double collision_map_tiny_y1;
    static double collision_map_tiny_y2;

    /*
     * The tiles are considered to be 1 unit wide. However the actual pixels
     * of each tile include shadows. px1/px2 are the bounds and exclude the
     * shadows. So we need to scale up the px1/px2 bounds to 1 as if the 
     * shadow pixels were not there.
     */
    if (!wall) {
        wall = tile_find("wall1_0_0");
        if (!wall) {
            DIE("no wall for collisions");
        }

        xscale = 1.0 / (wall->px2 - wall->px1);
        yscale = 1.0 / (wall->py2 - wall->py1);

        tilep tile = tile_find("large-collision-map");
        if (!tile) {
            DIE("no tile for collisions");
        }

        collision_map_large_x1 = tile->px1 * xscale;
        collision_map_large_x2 = tile->px2 * xscale;
        collision_map_large_y1 = tile->py1 * yscale;
        collision_map_large_y2 = tile->py2 * yscale;

        tile = tile_find("medium-collision-map");
        if (!tile) {
            DIE("no tile for collisions");
        }

        collision_map_medium_x1 = tile->px1 * xscale;
        collision_map_medium_x2 = tile->px2 * xscale;
        collision_map_medium_y1 = tile->py1 * yscale;
        collision_map_medium_y2 = tile->py2 * yscale;

        tile = tile_find("small-collision-map");
        if (!tile) {
            DIE("no tile for collisions");
        }

        collision_map_small_x1 = tile->px1 * xscale;
        collision_map_small_x2 = tile->px2 * xscale;
        collision_map_small_y1 = tile->py1 * yscale;
        collision_map_small_y2 = tile->py2 * yscale;

        tile = tile_find("tiny-collision-map");
        if (!tile) {
            DIE("no tile for collisions");
        }

        collision_map_tiny_x1 = tile->px1 * xscale;
        collision_map_tiny_x2 = tile->px2 * xscale;
        collision_map_tiny_y1 = tile->py1 * yscale;
        collision_map_tiny_y2 = tile->py2 * yscale;
    }

    /*
     * Find out the position of the thing on the client. On the server we move 
     * in jumps, but on the server we want the collision to be more accurate
     * so we use the amount of time passed to interpolate the thing position.
     */
    double Ax, Ay;
    double Bx, By;

    /*
     * If -1, -1 then we are looking at the current position.
     *
     * If not then we are just checking out a future position.
     */
    if ((nx == -1.0) && (ny == -1.0)) {
        thingp_get_interpolated_position(A, &Ax, &Ay);
        thingp_get_interpolated_position(B, &Bx, &By);
    } else {
        Ax = nx;
        Ay = ny;
        Bx = B->x;
        By = B->y;
    }

    widp widA = thing_wid(A);
    widp widB = thing_wid(B);

    double Apx1;
    double Apx2;
    double Apy1;
    double Apy2;

    double Bpx1;
    double Bpx2;
    double Bpy1;
    double Bpy2;

    if (thing_is_collision_map_large(A)) {
        Apx1 = collision_map_large_x1;
        Apx2 = collision_map_large_x2;
        Apy1 = collision_map_large_y1;
        Apy2 = collision_map_large_y2;
    } else if (thing_is_collision_map_medium(A)) {
        Apx1 = collision_map_medium_x1;
        Apx2 = collision_map_medium_x2;
        Apy1 = collision_map_medium_y1;
        Apy2 = collision_map_medium_y2;
    } else if (thing_is_collision_map_small(A)) {
        Apx1 = collision_map_small_x1;
        Apx2 = collision_map_small_x2;
        Apy1 = collision_map_small_y1;
        Apy2 = collision_map_small_y2;
    } else if (thing_is_collision_map_tiny(A)) {
        Apx1 = collision_map_tiny_x1;
        Apx2 = collision_map_tiny_x2;
        Apy1 = collision_map_tiny_y1;
        Apy2 = collision_map_tiny_y2;
    } else {
        /*
         * Just use pixel and alpha values.
         */
        tilep tileA = wid_get_tile(widA);

        Apx1 = tileA->px1 * xscale;
        Apx2 = tileA->px2 * xscale;
        Apy1 = tileA->py1 * yscale;
        Apy2 = tileA->py2 * yscale;
    }

    if (thing_is_collision_map_large(B)) {
        Bpx1 = collision_map_large_x1;
        Bpx2 = collision_map_large_x2;
        Bpy1 = collision_map_large_y1;
        Bpy2 = collision_map_large_y2;
    } else if (thing_is_collision_map_medium(B)) {
        Bpx1 = collision_map_medium_x1;
        Bpx2 = collision_map_medium_x2;
        Bpy1 = collision_map_medium_y1;
        Bpy2 = collision_map_medium_y2;
    } else if (thing_is_collision_map_small(B)) {
        Bpx1 = collision_map_small_x1;
        Bpx2 = collision_map_small_x2;
        Bpy1 = collision_map_small_y1;
        Bpy2 = collision_map_small_y2;
    } else if (thing_is_collision_map_tiny(B)) {
        Bpx1 = collision_map_tiny_x1;
        Bpx2 = collision_map_tiny_x2;
        Bpy1 = collision_map_tiny_y1;
        Bpy2 = collision_map_tiny_y2;
    } else {
        /*
         * Just use pixel and alpha values.
         */
        tilep tileB = wid_get_tile(widB);

        Bpx1 = tileB->px1 * xscale;
        Bpx2 = tileB->px2 * xscale;
        Bpy1 = tileB->py1 * yscale;
        Bpy2 = tileB->py2 * yscale;
    }

    /*
     * Bit of a hack. We need bonepiles to be passable by ghosts, but easy to 
     * hit by missiles.
     */
    if (thing_is_projectile(A) &&
        (thing_is_monst(B) ||
         thing_is_mob_spawner(B))) {

        Bpx1 = collision_map_large_x1;
        Bpx2 = collision_map_large_x2;
        Bpy1 = collision_map_large_y1;
        Bpy2 = collision_map_large_y2;
    }

    /*
     * Find the start of pixels in the tile.
     */
    double Atlx = Ax + Apx1;
    double Abrx = Ax + Apx2;
    double Atly = Ay + Apy1;
    double Abry = Ay + Apy2;

    double Btlx = Bx + Bpx1;
    double Bbrx = Bx + Bpx2;
    double Btly = By + Bpy1;
    double Bbry = By + Bpy2;

    /*
     * The rectangles don't overlap if one rectangle's minimum in some 
     * dimension is greater than the other's maximum in that dimension.
     */
    if ((Atlx < Bbrx) && 
        (Abrx > Btlx) &&
        (Atly < Bbry) && 
        (Abry > Btly)) {

#if 0
    if ((thing_is_projectile(A) || 
         thing_is_projectile(B))) {
LOG("  A %s %f %f %f %f",thing_logname(A),Atlx,Atly,Abrx,Abry);
LOG("    %f %f",Ax,Ay);
LOG("    %f %f %f %f",Apx1,Apy1,Apx2,Apy2);
LOG("  B %s %f %f %f %f",thing_logname(B),Btlx,Btly,Bbrx,Bbry);
LOG("    %f %f",Bx,By);
LOG("    %f %f %f %f",Bpx1,Bpy1,Bpx2,Bpy2);
    }
#endif

        return (true);
    }

    return (false);
}

/*
 * handle a single collision between two things
 */
static void thing_handle_collision (thingp me, thingp it, 
                                    int32_t x, int32_t y)
{
    if (thing_is_dead_or_dying(it)) {
        return;
    }

    if (thing_has_left_level(it)) {
        return;
    }

    /*
     * Filter out boring things.
     */
    if (thing_is_floor(it) ||
        thing_is_animation(it)) {
        return;
    }

    /*
     * Do we overlap with something?
     */
    if (!things_overlap(me, -1.0, -1.0, it)) {
        return;
    }

    if (thing_is_player(me)) {
        /*
         * Player collects keys and other items
         */
        if (thing_is_treasure(it)               ||
            thing_is_weapon(it)                 ||
            thing_is_carryable(it)              ||
            thing_is_food(it)) {

            thing_item_collect(me, it, thing_tp(it));

            thing_dead(it, me, "collected");
            return;
        }

        /*
         * Open doors if you have a key.
         */
        if (thing_is_door(it)) {
            tpp tp;
            tp = thing_is_carrying_thing(me, tp_is_key);
            if (tp) {
                thing_used(me, tp);
                level_open_door(server_level, x, y);
                return;
            }
        }

        /*
         * Player bumped into something.
         */
        if (thing_is_monst(it)                  || 
            thing_is_poison(it)                 ||
            thing_is_weapon_swing_effect(it)    ||
            thing_is_explosion(it)) {
            /*
             * I'm hit!
             */
            thing_possible_hit_add(it, "player hit thing");
            return;
        }

        if (thing_is_exit(it)) {
            thing_reached_exit(me, it);
            return;
        }

        /*
         * An action trigger is only ever used to start an object off as the 
         * initiator of a collision.
         */
        if (thing_is_action_trigger(it)) {
            level_trigger_activate(server_level, 
                                   it->data ? it->data->col_name: 0);
        }
    }

    if (thing_is_monst(me)) {
        /*
         * Monster bumped into something.
         */
        if (thing_is_player(it)                 ||
            thing_is_poison(it)                 ||
            thing_is_weapon_swing_effect(it)    ||
            thing_is_explosion(it)) {
            /*
             * I'm hit!
             */
            thing_possible_hit_add(it, "monst hit thing");
            return;
        }
    }

    if (thing_is_wall(me)) {
        /*
         * Monster bumped into something.
         */
        if (thing_is_player(it)                 ||
            thing_is_treasure(it)               ||
            thing_is_food(it)                   ||
            thing_is_door(it)                   ||
            thing_is_projectile(it)             ||
            thing_is_mob_spawner(it)            ||
            thing_is_monst(it)) {
            /*
             * I'm hit!
             */
            thing_possible_hit_add(it, "monst hit thing");
            return;
        }
    }

    /*
     * Explosion hit something?
     */
    if (thing_is_projectile(me)                 || 
        thing_is_poison(me)                     ||
        thing_is_explosion(me)) {

        if (thing_is_monst(it)                  || 
            thing_is_fragile(it)                ||
            thing_is_door(it)                   ||
            thing_is_wall(it)                   ||
            thing_is_player(it)                 ||
            thing_is_mob_spawner(it)) {
            /*
             * Weapon hits monster or generator.
             */
            thing_possible_hit_add_hitter_killed_on_hitting(
                                            it, "projection hit thing");
        }
    }

    /*
     * Initially we have a wall sitting on a trigger. The wall is inactive
     * and doesn't do collision tests. The trigger might no be active either 
     * and so we n
     */
    if (thing_is_wall(me)) {
        if (thing_is_action_left(it)                ||
            thing_is_action_right(it)               ||
            thing_is_action_up(it)                  ||
            thing_is_action_down(it)) {

            if (level_trigger_is_activated(server_level, 
                                           it->data ? it->data->col_name : 0)) {
                level_trigger_move_thing(thing_tp(it), me);
            }
        }
    }

    /*
     * Sword swing hits?
     */
    if (thing_is_weapon_swing_effect(me)) {

        if (thing_is_monst(it) || 
            thing_is_door(it) ||
            /*
             * Don't hit walls. It's daft.
             */
            thing_is_mob_spawner(it)) {
            /*
             * Weapon hits monster or generator.
             */
            thing_possible_hit_add_hitter_killed_on_hitting(
                                            it, "sword hit thing");
        }
    }
}

/*
 * Have we hit anything?
 */
void thing_handle_collisions (widp grid, thingp me)
{
    int32_t dx, dy;

    if (thing_has_left_level(me)) {
        return;
    }

    thing_map *map = thing_get_map(me);

    for (dx = -1; dx <= 1; dx++) for (dy = -1; dy <= 1; dy++) {
        int32_t x = (int32_t)me->x + dx;
        int32_t y = (int32_t)me->y + dy;

        if ((x < 0) || (y < 0) || (x >= MAP_WIDTH) || (y >= MAP_HEIGHT)) {
            continue;
        }

        thing_map_cell *cell = &map->cells[x][y];

        uint32_t i;
        for (i = 0; i < cell->count; i++) {
            thingp it;
            
            if (me->on_server) {
                it = thing_server_id(cell->id[i]);
            } else {
                it = thing_client_id(cell->id[i]);
            }

            if (me == it) {
                continue;
            }

            thing_handle_collision(me, it, x, y);
        }
    }

    thing_possible_hit_do(me);
}

/*
 * Have we hit anything?
 *
 * No opening of doors in here or other actions. This is just a check.
 */
uint8_t thing_hit_solid_obstacle (widp grid, thingp t, double nx, double ny)
{
    thingp me;
    widp wid_me;

    verify(t);
    wid_me = thing_wid(t);
    verify(wid_me);

    int32_t dx, dy;

    me = wid_get_thing(wid_me);

    thing_map *map = thing_get_map(t);

    for (dx = -1; dx <= 1; dx++) for (dy = -1; dy <= 1; dy++) {
        int32_t x = (int32_t)nx + dx;
        int32_t y = (int32_t)ny + dy;

        if ((x < 0) || (y < 0) || (x >= MAP_WIDTH) || (y >= MAP_HEIGHT)) {
            continue;
        }

        thing_map_cell *cell = &map->cells[x][y];

        uint32_t i;
        for (i = 0; i < cell->count; i++) {
            thingp it;
            
            if (t->on_server) {
                it = thing_server_id(cell->id[i]);
            } else {
                it = thing_client_id(cell->id[i]);
            }

            if (it == t) {
                continue;
            }

            verify(it);

            /*
             * No collisions with the floor!
             */
            if (thing_is_floor(it)          ||
                thing_is_action(it)         ||
                thing_is_animation(it)) {
                continue;
            }

            /*
             * Allow dead ghosts to walk over each other!
             */
            if (thing_is_dead_or_dying(it)) {
                continue;
            }

            if (thing_is_monst(me)) {
                /*
                 * Allow monsters to walk into these things:
                 */
                if (thing_is_player(it)                 ||
                    thing_can_walk_through(it)          ||
                    thing_is_carryable(it)              ||
                    thing_is_weapon_swing_effect(it)    ||
                    thing_is_explosion(it)              ||
                    thing_is_poison(it)                 ||
                    thing_is_projectile(it)             ||
                    thing_is_treasure(it)               ||
                    thing_is_weapon(it)                 ||
                    thing_is_food(it)) {
                    continue;
                }
            }

            if (thing_is_explosion(me)                  ||
                thing_is_projectile(me)                 ||
                thing_is_poison(me)                     ||
                thing_is_weapon_swing_effect(me)) {
                /*
                 * Allow these to pass through anything.
                 */
                continue;
            }

            if (thing_is_player(me)) {
                /*
                 * Allow to walk through doors so we can open them later.
                 */
                if (thing_is_door(it)) {
                    if (thing_is_carrying_thing(me, tp_is_key)) {
                        continue;
                    } else {
                        if (!me->message_open_door_need_key) {
                            me->message_open_door_need_key = 1;
                            MSG_SERVER_SHOUT_AT_PLAYER(INFO, me,
                                           "Collect keys to open doors");
                        }
                    }
                }

                /*
                 * Allow players to collect keys and other junk.
                 */
                if (thing_is_carryable(it)              ||
                    thing_can_walk_through(it)          ||
                    thing_is_food(it)                   ||
                    thing_is_treasure(it)               ||
                    thing_is_weapon(it)                 ||
                    thing_is_exit(it)                   ||
                    thing_is_monst(it)                  ||
                    /*
                     * Walk through friendly fire.
                     */
                    thing_is_projectile(it)             ||
                    thing_is_poison(it)                 ||
                    thing_is_weapon_swing_effect(it)    ||
                    thing_is_explosion(it)) {
                    continue;
                }

                /*
                 * Allow players to walk through other players. Else thay
                 * can spawn on top of each other and get stuck.
                 */
                if (thing_is_player(it)) {
                    continue;
                }
            }

            if (thing_is_wall(me)) {
                /*
                 * Allow moving walls to crush!
                 */
                if (!thing_is_wall(it)) {
                    continue;
                }
            }

            if (!things_overlap(me, nx, ny, it)) {
                continue;
            }

            return (true);
        }
    }

    return (false);
}

/*
 * Have we hit anything?
 *
 * Is there anything other than floor here
 */
uint8_t thing_hit_any_obstacle (widp grid, thingp t, double nx, double ny)
{
    thingp it;
    thingp me;
    widp wid_next;
    widp wid_me;
    widp wid_it;

    verify(t);
    wid_me = thing_wid(t);
    verify(wid_me);

    int32_t dx, dy;

    me = wid_get_thing(wid_me);

    uint8_t z;

    for (dx = -1; dx <= 1; dx++) 
    for (dy = -1; dy <= 1; dy++) {
    for (z = MAP_DEPTH_WALL; z < MAP_DEPTH; z++) {
            int32_t x = (int32_t)nx + dx;
            int32_t y = (int32_t)ny + dy;

            wid_it = wid_grid_find_first(grid, x, y, z);
            while (wid_it) {
                verify(wid_it);

                wid_next = wid_grid_find_next(grid, wid_it, x, y, z);
                if (wid_me == wid_it) {
                    wid_it = wid_next;
                    continue;
                }

                it = wid_get_thing(wid_it);
                if (!it) {
                    wid_it = wid_next;
                    continue;
                }

                /*
                 * No collisions with the floor!
                 */
                if (thing_is_floor(it)) {
                    wid_it = wid_next;
                    continue;
                }

                /*
                 * Allow dead ghosts to walk over each other!
                 */
                if (thing_is_dead_or_dying(it)) {
                    wid_it = wid_next;
                    continue;
                }

                if (!things_overlap(me, nx, ny, it)) {
                    wid_it = wid_next;
                    continue;
                }

                return (true);
            }
        }
    }

    return (false);
}
