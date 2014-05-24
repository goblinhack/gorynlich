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
    static double collision_map_player_x1;
    static double collision_map_player_x2;
    static double collision_map_player_y1;
    static double collision_map_player_y2;
    static double collision_map_monst_x1;
    static double collision_map_monst_x2;
    static double collision_map_monst_y1;
    static double collision_map_monst_y2;
    static double collision_map_weapon_x1;
    static double collision_map_weapon_x2;
    static double collision_map_weapon_y1;
    static double collision_map_weapon_y2;

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

        tilep player = tile_find("player-collision-map");
        if (!player) {
            DIE("no player for collisions");
        }

        collision_map_player_x1 = player->px1 * xscale;
        collision_map_player_x2 = player->px2 * xscale;
        collision_map_player_y1 = player->py1 * yscale;
        collision_map_player_y2 = player->py2 * yscale;

        tilep monst = tile_find("monst-collision-map");
        if (!monst) {
            DIE("no monst for collisions");
        }

        collision_map_monst_x1 = monst->px1 * xscale;
        collision_map_monst_x2 = monst->px2 * xscale;
        collision_map_monst_y1 = monst->py1 * yscale;
        collision_map_monst_y2 = monst->py2 * yscale;

        tilep weapon = tile_find("weapon-collision-map");
        if (!weapon) {
            DIE("no weapon for collisions");
        }

        collision_map_weapon_x1 = weapon->px1 * xscale;
        collision_map_weapon_x2 = weapon->px2 * xscale;
        collision_map_weapon_y1 = weapon->py1 * yscale;
        collision_map_weapon_y2 = weapon->py2 * yscale;
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

    if (thing_is_collision_map_monst(A)) {
        Apx1 = collision_map_monst_x1;
        Apx2 = collision_map_monst_x2;
        Apy1 = collision_map_monst_y1;
        Apy2 = collision_map_monst_y2;
    } else if (thing_is_collision_map_weapon(A)) {
        if (thing_is_collision_map_player(B)) {
            Apx1 = collision_map_player_x1;
            Apx2 = collision_map_player_x2;
            Apy1 = collision_map_player_y1;
            Apy2 = collision_map_player_y2;
        } else if (thing_is_collision_map_player(B)) {
            Apx1 = collision_map_monst_x1;
            Apx2 = collision_map_monst_x2;
            Apy1 = collision_map_monst_y1;
            Apy2 = collision_map_monst_y2;
        } else {
            Apx1 = collision_map_weapon_x1;
            Apx2 = collision_map_weapon_x2;
            Apy1 = collision_map_weapon_y1;
            Apy2 = collision_map_weapon_y2;
        }
    } else if (thing_is_collision_map_player(A)) {
        Apx1 = collision_map_player_x1;
        Apx2 = collision_map_player_x2;
        Apy1 = collision_map_player_y1;
        Apy2 = collision_map_player_y2;
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

    if (thing_is_collision_map_monst(B)) {
        Bpx1 = collision_map_monst_x1;
        Bpx2 = collision_map_monst_x2;
        Bpy1 = collision_map_monst_y1;
        Bpy2 = collision_map_monst_y2;
    } else if (thing_is_collision_map_weapon(B)) {
        if (thing_is_collision_map_player(A)) {
            Bpx1 = collision_map_player_x1;
            Bpx2 = collision_map_player_x2;
            Bpy1 = collision_map_player_y1;
            Bpy2 = collision_map_player_y2;
        } else if (thing_is_collision_map_player(A)) {
            Bpx1 = collision_map_monst_x1;
            Bpx2 = collision_map_monst_x2;
            Bpy1 = collision_map_monst_y1;
            Bpy2 = collision_map_monst_y2;
        } else {
            Bpx1 = collision_map_weapon_x1;
            Bpx2 = collision_map_weapon_x2;
            Bpy1 = collision_map_weapon_y1;
            Bpy2 = collision_map_weapon_y2;
        }
    } else if (thing_is_collision_map_player(B)) {
        Bpx1 = collision_map_player_x1;
        Bpx2 = collision_map_player_x2;
        Bpy1 = collision_map_player_y1;
        Bpy2 = collision_map_player_y2;
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

    /*
     * The rectangles don't overlap if one rectangle's minimum in some 
     * dimension is greater than the other's maximum in that dimension.
     */
    uint8_t no_overlap = (Atlx > Bbrx) ||
                         (Btlx > Abrx) ||
                         (Atly > Bbry) ||
                         (Btly > Abry);

    return (!no_overlap);
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

    /*
     * Filter out boring things.
     */
    if (thing_is_floor(it)) {
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
         * Player collects keys
         */
        if (thing_is_key(it) ||
            /*
             * And treasure.
             */
            thing_is_treasure(it) ||
            /*
             * And treasure.
             */
            thing_is_weapon(it) ||
            /*
             * And food.
             */
            thing_is_food(it)) {

            thing_collect(me, thing_get_template(it));

            thing_dead(it, me, "collected");
            return;
        }

        /*
         * Open doors if you have a key.
         */
        if (thing_is_door(it) && thing_has(me, THING_KEYS1)) {
            level_open_door(server_level, x, y);
            return;
        }

        /*
         * Player bumped into something.
         */
        if (thing_is_monst(it) || thing_is_explosion(it)) {
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
    if (thing_is_projectile(me) || thing_is_explosion(me)) {
        if (thing_is_monst(it) || thing_is_generator(it)) {
            /*
             * Weapon hits monster or generator.
             */
            thing_hit(it, me, 0, "hit");
            return;
        }

        if (thing_is_destroyed_on_hitting(it)) {
            /*
             * Weapon hits food or similar?
             */
            thing_hit(it, me, 0, "hit");
            return;
        }

        if (thing_is_door(it) || thing_is_wall(it)) {
            /*
             * Weapon hits a wall
             */
            thing_dead(me, 0, "hit solid obstacle");
            return;
        }
    }
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

    int32_t dx, dy;

    me = wid_get_thing(wid_me);

    for (dx = -1; dx <= 1; dx++) for (dy = -1; dy <= 1; dy++) {
        int32_t x = (int32_t)t->x + dx;
        int32_t y = (int32_t)t->y + dy;

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

            thing_handle_collision(me, it, x, y);

            wid_it = wid_next;
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

            if (thing_is_monst(me)) {
                /*
                 * Allow monsters to walk into these things:
                 */
                if (thing_is_player(it)     ||
                    thing_is_key(it)        ||
                    thing_is_explosion(it)  ||
                    thing_is_projectile(it) ||
                    thing_is_treasure(it)   ||
                    thing_is_weapon(it)     ||
                    thing_is_food(it)) {
                    wid_it = wid_next;
                    continue;
                }
            }

            if (thing_is_projectile(me)) {
                /*
                 * Allow projectiles to pass through anything.
                 */
                wid_it = wid_next;
                continue;
            }

            if (thing_is_explosion(me)) {
                /*
                 * Allow explosions to pass through anything.
                 */
                wid_it = wid_next;
                continue;
            }

            if (thing_is_player(me)) {
                /*
                 * Allow to walk through doors so we can open them later.
                 */
                if (thing_is_door(it) && thing_has(me, THING_KEYS1)) {
                    wid_it = wid_next;
                    continue;
                }

                /*
                 * Allow players to collect keys.
                 */
                if (thing_is_key(it) ||
                    /*
                     * And collect food.
                     */
                    thing_is_food(it) ||
                    /*
                     * And collect treasure.
                     */
                    thing_is_treasure(it) ||
                    /*
                     * And walk through monsters.
                     */
                    thing_is_monst(it) ||
                    /*
                     * Or friendly fire.
                     */
                    thing_is_projectile(it) ||
                    /*
                     * Or friendly fire.
                     */
                    thing_is_explosion(it)) {
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
