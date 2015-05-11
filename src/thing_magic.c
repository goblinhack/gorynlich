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
#include "time_util.h"

/*
 * Build up magic to be fired later.
 */
void thing_server_magic_powerup (thingp t)
{
    int magic = thing_stats_get_magic(t);

    if (magic > 1) {
        magic = 1;
    }

    if (magic == 0) {
        return;
    }

    t->magic_powerup += magic;

    thing_stats_modify_magic(t, -magic);

    t->timestamp_magic_powerup = time_get_time_ms();
}

/*
 * If enough time has passed since the last press of the m key on the client, 
 * fire!
 */
void thing_server_magic_fire (thingp t)
{
    if (!t->magic_powerup) {
        return;
    }

    if (!time_have_x_tenths_passed_since(2, t->timestamp_magic_powerup)) {
        return;
    }

    /*
     * Cannot magic until we're on a level.
     */
    if (!t->wid) {
        THING_LOG(t, "cannot magic yet, not on the level");
        return;
    }

    /*
     * Use the currently wielded weapon.
     */
    tpp weapon = thing_weapon(t);
    if (!weapon) {
        THING_LOG(t, "has no weapon, cannot magic");
        return;
    }

    /*
     * Does it potentially break each use?
     */
    thing_weapon_check_for_damage_on_firing(t, weapon);

    double dx, dy;

    /*
     * Any smaller than this and diagonal shots collide with adjoining walls.
     */
    double dist_from_player = 0.7;

    /*
     * Try current direction.
     */
    dx = 0.0;
    dy = 0.0;

    if (thing_is_dir_down(t)) {
        dy = dist_from_player;
    }

    if (thing_is_dir_up(t)) {
        dy = -dist_from_player;
    }

    if (thing_is_dir_right(t)) {
        dx = dist_from_player;
    }

    if (thing_is_dir_left(t)) {
        dx = -dist_from_player;
    }

    if (thing_is_dir_tl(t)) {
        dx = -dist_from_player;
        dy = -dist_from_player;
    }

    if (thing_is_dir_tr(t)) {
        dx = dist_from_player;
        dy = -dist_from_player;
    }

    if (thing_is_dir_bl(t)) {
        dx = -dist_from_player;
        dy = dist_from_player;
    }

    if (thing_is_dir_br(t)) {
        dx = dist_from_player;
        dy = dist_from_player;
    }

    /*
     * magic from the player position plus the initial delta so it looks like 
     * it comes from outside of the body.
     */
    double x = t->x;
    double y = t->y;

    x += dx;
    y += dy;

    tpp projectile = tp_find("data/things/magic1");
    if (!projectile) {
        /*
         * Might be a sword.
         */
        thing_swing(t);
        return;
    }

    widp w = wid_game_map_server_replace_tile(
                                    wid_game_map_server_grid_container,
                                    x,
                                    y,
                                    0, /* thing */
                                    projectile,
                                    0, /* tpp data */
                                    0, /* item */
                                    0 /* stats */);

    thingp p = wid_get_thing(w);

    /*
     * Make sure we keep track of who magicd so we can award scores.
     */
    thing_set_owner(p, t);

    double scale = 0.02 * (double) t->magic_powerup;
    p->scale = scale;

    /*
     * Set up the modifier damage if this is say a magicball or bow for ex.
     */
    p->damage = t->magic_powerup;;

    t->magic_powerup = 0;

    /*
     * Round up say -0.7 to -1.0
     */
    dx *= 10.0;
    dy *= 10.0;
    dx /= (dist_from_player * 10.0);
    dy /= (dist_from_player * 10.0);

    p->dx = dx;
    p->dy = dy;
    p->dir = t->dir;

    /*
     * Check for immediate collision with a wall
     */
    thing_handle_collisions(wid_game_map_server_grid_container, p);
    if (thing_is_dead_or_dying(p)) {
        return;
    }

    double fnexthop_x = p->x + p->dx;
    double fnexthop_y = p->y + p->dy;

    thing_server_move(p,
                      fnexthop_x,
                      fnexthop_y,
                      fnexthop_y < p->y,
                      fnexthop_y > p->y,
                      fnexthop_x < p->x,
                      fnexthop_x > p->x,
                      false, /* magic */
                      false  /* magic */);
}
