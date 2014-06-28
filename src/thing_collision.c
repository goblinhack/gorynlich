/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "tree.h"
#include "thing.h"
#include "tile.h"
#include "tile_private.h"
#include "wid.h"
#include "map.h"
#include "wid_tooltip.h"
#include "wid_game_map_server.h"
#include "wid_game_map_client.h"
#include "sound.h"
#include "timer.h"

/*
 * On the server, things move in jumps. Find the real position the client
 * will see so collisions look more accurate.
 */
static void 
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
        wall = tile_find("wall_0_0");
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

#ifdef DEBUG
    if ((thing_is_monst(A) || 
         thing_is_monst(B)) &&
        (thing_is_weapon_swing_effect(A) || 
         thing_is_weapon_swing_effect(B))) {
CON("  A %s %f %f %f %f",thing_logname(A),Atlx,Atly,Abrx,Abry);
CON("    %f %f",Ax,Ay);
CON("    %f %f %f %f",Apx1,Apy1,Apx2,Apy2);
CON("  B %s %f %f %f %f",thing_logname(B),Btlx,Btly,Bbrx,Bbry);
CON("    %f %f",Bx,By);
CON("    %f %f %f %f",Bpx1,Bpy1,Bpx2,Bpy2);
    }
#endif

    /*
     * The rectangles don't overlap if one rectangle's minimum in some 
     * dimension is greater than the other's maximum in that dimension.
     */
    if ((Atlx < Bbrx) && 
        (Abrx > Btlx) &&
        (Atly < Bbry) && 
        (Abry > Btly)) {
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
    if (thing_is_dead(it)) {
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
#ifdef DEBUG
if ((thing_is_monst(me) || thing_is_monst(it)) &&
    (thing_is_weapon_swing_effect(me) || thing_is_weapon_swing_effect(it))) {
CON("no overlap %s %s",thing_logname(me),thing_logname(it));
}
#endif
        return;
    }
#ifdef DEBUG
if ((thing_is_monst(me) || thing_is_monst(it)) &&
    (thing_is_weapon_swing_effect(me) || thing_is_weapon_swing_effect(it))) {
CON("HIT %s %s",thing_logname(me),thing_logname(it));
}
#endif

    if (thing_is_player(me)) {
        /*
         * Player collects keys and other items
         */
        if (thing_is_key(it) ||
            thing_is_treasure(it) ||
            thing_is_weapon(it) ||
            thing_is_food(it)) {

            thing_item_collect(me, it, thing_get_template(it));

            thing_dead(it, me, "collected");
            return;
        }

        /*
         * Open doors if you have a key.
         */
        if (thing_is_door(it)) {
            if (thing_is_carrying(me, THING_KEY)) {
                thing_used(me, id_to_thing_template(THING_KEY));
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
            thing_hit(me, it, 0, "monst");
            return;
        }

        if (thing_is_exit(it)) {
            thing_leave_level(me);
            return;
        }
    }

    if (thing_is_monst(me)) {
        /*
         * Monster bumped into something.
         */
        if (thing_is_poison(it)                 ||
            thing_is_weapon_swing_effect(it)    ||
            thing_is_explosion(it)) {
            /*
             * I'm hit!
             */
            thing_hit(me, it, 0, "monst");
            return;
        }
    }

    /*
     * Weapon or explosion hit something?
     */
    if (thing_is_projectile(me)                 || 
        thing_is_poison(me)                     ||
        thing_is_explosion(me)) {

        if (thing_is_monst(it) || thing_is_mob_spawner(it)) {
            /*
             * Weapon hits monster or generator.
             */
            thing_hit(it, me, 0, "hit");

            thing_dead(me, 0, "hit monst");
            return;
        }

        if (thing_is_fragile(it)) {
            /*
             * Weapon hits food or similar?
             */
            thing_hit(it, me, 0, "hit");

            thing_dead(me, 0, "hit item");
            return;
        }

        if (thing_is_door(it) || thing_is_wall(it)) {
            /*
             * Weapon hits a wall. Slim chance of wall being destroyed.
             */
            thing_hit(it, me, 0, "hit");

            thing_dead(me, 0, "hit wall");
            return;
        }
    }
}

/*
 * Have we hit anything?
 */
void thing_handle_collisions (widp grid, thingp me)
{
    int32_t dx, dy;

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
                it = thing_server_ids[cell->id[i]];
            } else {
                it = thing_client_ids[cell->id[i]];
            }

            if (me == it) {
                continue;
            }

            thing_handle_collision(me, it, x, y);
        }
    }
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
                it = thing_server_ids[cell->id[i]];
            } else {
                it = thing_client_ids[cell->id[i]];
            }

            if (it == t) {
                continue;
            }

            verify(it);

            /*
             * No collisions with the floor!
             */
            if (thing_is_floor(it) ||
                thing_is_animation(it)) {
                continue;
            }

            /*
             * Allow dead ghosts to walk over each other!
             */
            if (thing_is_dead(it)) {
                continue;
            }

            if (thing_is_monst(me)) {
                /*
                 * Allow monsters to walk into these things:
                 */
                if (thing_is_player(it)                 ||
                    thing_is_key(it)                    ||
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
                    if (thing_is_carrying(me, THING_KEY)) {
                        continue;
                    } else {
                        if (!me->message_open_door_need_key) {
                            me->message_open_door_need_key = 1;
                            THING_SHOUT_AT(me, INFO,
                                           "Collect keys to open doors");
                        }
                    }
                }

                /*
                 * Allow players to collect keys and other junk.
                 */
                if (thing_is_key(it)                    ||
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

    for (dx = -1; dx <= 1; dx++) for (dy = -1; dy <= 1; dy++) {
        int32_t x = (int32_t)nx + dx;
        int32_t y = (int32_t)ny + dy;

        wid_it = wid_grid_find_first(grid, x, y);
        while (wid_it) {
            verify(wid_it);

            wid_next = wid_grid_find_next(grid, wid_it, x, y);
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
            if (thing_is_dead(it)) {
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

    return (false);
}
