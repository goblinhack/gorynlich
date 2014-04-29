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

static boolean things_overlap (const thingp A, 
                               double nx,
                               double ny,
                               const thingp B)
{
    static tilep wall;
    static tilep monst;
    static double xscale;
    static double yscale;
    static double Mpx1;
    static double Mpx2;
    static double Mpy1;
    static double Mpy2;

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

        monst = tile_find("monst-overlap");
        if (!monst) {
            DIE("no monst for collisions");
        }

        Mpx1 = monst->px1 * xscale;
        Mpx2 = monst->px2 * xscale;
        Mpy1 = monst->py1 * yscale;
        Mpy2 = monst->py2 * yscale;
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

    if (thing_is_collision_map_small(A)) {
        Apx1 = Mpx1;
        Apx2 = Mpx2;
        Apy1 = Mpy1;
        Apy2 = Mpy2;
    } else {
        tilep tileA = wid_get_tile(widA);

        Apx1 = tileA->px1 * xscale;
        Apx2 = tileA->px2 * xscale;
        Apy1 = tileA->py1 * yscale;
        Apy2 = tileA->py2 * yscale;
    }

    if (thing_is_collision_map_small(B)) {
        Bpx1 = Mpx1;
        Bpx2 = Mpx2;
        Bpy1 = Mpy1;
        Bpy2 = Mpy2;
    } else {
        tilep tileB = wid_get_tile(widB);

        Bpx1 = tileB->px1 * xscale;
        Bpx2 = tileB->px2 * xscale;
        Bpy1 = tileB->py1 * yscale;
        Bpy2 = tileB->py2 * yscale;
    }

    /*
     * Find the start of pixels in the tile.
     */
    double Atlx = nx + Apx1;
    double Abrx = nx + Apx2;
    double Atly = ny + Apy1;
    double Abry = ny + Apy2;

    double Btlx = B->x + Bpx1;
    double Bbrx = B->x + Bpx2;
    double Btly = B->y + Bpy1;
    double Bbry = B->y + Bpy2;

    /*
     * The rectangles don't overlap if one rectangle's minimum in some 
     * dimension is greater than the other's maximum in that dimension.
     */
    boolean no_overlap = (Atlx > Bbrx) ||
                      (Btlx > Abrx) ||
                      (Atly > Bbry) ||
                      (Btly > Abry);

    return (!no_overlap);
}

/*
 * Have we hit anything?
 */
void thing_handle_collisions (widp grid, thingp t)
{
    thingp it;
    thingp me;
    widp wid_next;
    widp wid_me;
    widp wid_it;

    verify(t);
    wid_me = thing_wid(t);
    verify(wid_me);

    double nx = t->x;
    double ny = t->y;

    int32_t dx, dy;

    me = wid_get_thing(wid_me);

    for (dx = -1; dx <= 1; dx++) for (dy = -1; dy <= 1; dy++) {
        int32_t x = (int32_t)nx + dx;
        int32_t y = (int32_t)ny + dy;

        wid_it = wid_grid_find_first(grid, x, y);
        while (wid_it) {
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
             * Filter out boring things.
             */
            if (thing_is_floor(it)) {
                wid_it = wid_next;
                continue;
            }

            /*
             * Do we overlap with something?
             */
            if (!things_overlap(me, nx, ny, it)) {
                wid_it = wid_next;
                continue;
            }

            if (thing_is_player(me)) {
                /*
                 * Collect keys
                 */
                if (thing_is_key(it)) {
                    thing_collect(me, thing_get_template(it));
                    thing_dead(it, t, "collected");
                    wid_it = wid_next;
                    continue;
                }

                /*
                 * Open doors if you have a key.
                 */
                if (thing_is_door(it) && thing_has(me, THING_KEYS1)) {
                    level_open_door(server_level, x, y);
                    wid_it = wid_next;
                    continue;
                }

                /*
                 * Bumped into a monster.
                 */
                if (thing_is_monst(it)) {
                    /*
                     * Monster dies in the collision but steals hitpoints.
                     */
                    thing_dead(it, t, "hit");

                    wid_it = wid_next;
                    continue;
                }
            }

            /*
             * Weapon hit something?
             */
            if (thing_is_projectile(me)) {
                if (thing_is_monst(it) ||
                    thing_is_wall(it) ||
                    thing_is_door(it)) {
                    /*
                     * Weapon dies in the collision but steals hitpoints.
                     */
                    thing_dead(me, t, "hit");
                    break;
                }
            }

            /*
             * Bumped into a monster.
             */
            if (thing_is_monst(me)) {
                if (thing_is_player(it)) {
                    /*
                     * Monster dies in the collision but steals hitpoints.
                     */
                    thing_dead(me, t, "hit");
                    break;
                }
            }

            wid_it = wid_next;
        }
    }
}

/*
 * Have we hit anything?
 *
 * No open doors in here.
 */
boolean thing_hit_solid_obstacle (widp grid, thingp t, double nx, double ny)
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

            if (thing_is_floor(it)) {
                wid_it = wid_next;
                continue;
            }

            if (!thing_is_wall(it) && 
                !thing_is_door(it) && 
                !thing_is_monst(t)) {
                wid_it = wid_next;
                continue;
            }

            /*
             * Allow monsters to walk into the player.
             */
            if (thing_is_monst(me)) {
                if (thing_is_player(it)) {
                    wid_it = wid_next;
                    continue;
                }
            }

            /*
             * Allow projectiles to "walk" into the anything.
             */
            if (thing_is_projectile(me)) {
                wid_it = wid_next;
                continue;
            }

            /*
             * Allow to walk through doors so we can open them later.
             */
            if (thing_is_player(me)) {
                if (thing_is_door(it) && thing_has(me, THING_KEYS1)) {
                    wid_it = wid_next;
                    continue;
                }
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
