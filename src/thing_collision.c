/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "tree.h"
#include "thing.h"
#include "thing_private.h"
#include "tile.h"
#include "tile_private.h"
#include "thing_template.h"
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
    static double xscale;
    static double yscale;

    /*
     * The tiles are considered to be 1 unit wide. However the actual pixels
     * of each tile include shadows. px1/px2 are the bounds and exclude the
     * shadows. So we need to scale up the px1/px2 bounds to 1 as if the 
     * shadow pixels were not there.
     */
    if (!wall) {
        wall = tile_find("wall_0_0");

        xscale = 1.0 / (wall->px2 - wall->px1);
        yscale = 1.0 / (wall->py2 - wall->py1);
    }

    widp widA = thing_wid(A);
    widp widB = thing_wid(B);

    tilep tileA = wid_get_tile(widA);
    tilep tileB = wid_get_tile(widB);

    double Apx1 = tileA->px1 * xscale;
    double Apx2 = tileA->px2 * xscale;
    double Apy1 = tileA->py1 * yscale;
    double Apy2 = tileA->py2 * yscale;

    double Bpx1 = tileB->px1 * xscale;
    double Bpx2 = tileB->px2 * xscale;
    double Bpy1 = tileB->py1 * yscale;
    double Bpy2 = tileB->py2 * yscale;

    double xoverlap = 0.1;
    double yoverlap = 0.3;

    if (thing_is_monst(A) || thing_is_player(A)) {
        Apx1 += xoverlap;
        Apx2 -= xoverlap;
        Apy1 += yoverlap;
    }

    if (thing_is_monst(B) || thing_is_player(B)) {
        Bpx1 += xoverlap;
        Bpx2 -= xoverlap;
        Bpy1 += yoverlap;
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
    bool no_overlap = (Atlx > Bbrx) ||
                      (Btlx > Abrx) ||
                      (Atly > Bbry) ||
                      (Btly > Abry);

    return (!no_overlap);
}

/*
 * Have we hit anything?
 */
void thing_collision (thingp t, int32_t x, int32_t y)
{
    thing_templatep me;
    thing_templatep it;
    widp wid_next;
    widp wid_me;
    widp wid_it;
    thingp thing_it;

    verify(t);

    me = thing_get_template(t);

    if (!thing_template_is_player(me) &&
        !thing_template_is_monst(me)) {
        return;
    }

    /*
     * Popped from the level?
     */
    wid_me = thing_wid(t);
    if (!wid_me) {
        return;
    }

    verify(wid_me);

    wid_it = wid_grid_find_first(wid_game_map_server_grid_container, x, y);
    while (wid_it) {
        verify(wid_it);

        wid_next = wid_grid_find_next(wid_game_map_server_grid_container,
                                      wid_it, x, y);
        if (wid_me == wid_it) {
            wid_it = wid_next;
            continue;
        }

        if (!wids_overlap(wid_me, wid_it)) {
            wid_it = wid_next;
            continue;
        }

        it = wid_get_thing_template(wid_it);
        if (!it) {
            wid_it = wid_next;
            continue;
        }

        thing_it = wid_get_thing(wid_it);
        if (!thing_it) {
            wid_it = wid_next;
            continue;
        }

        verify(thing_it);

        /*
         * Try to ignore the dead!
         */
        if (thing_is_dead(thing_it)) {
            wid_it = wid_next;
            continue;
        }

        wid_it = wid_next;
    }
}

/*
 * Have we hit anything?
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

            if (thing_is_floor(it)) {
                wid_it = wid_next;
                continue;
            }

            if (!thing_is_wall(it) && !thing_is_door(it)) {
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
