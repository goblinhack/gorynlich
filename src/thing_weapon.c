/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "thing.h"
#include "wid.h"
#include "wid_game_map_server.h"
#include "math.h"

void thing_weapon_swing_offset (thingp t, double *dx, double *dy)
{
    tpp weapon = thing_weapon(t);
    if (!weapon) {
        return;
    }

    *dx = 0;
    *dy = 0;

    double dist_from_player = 
        ((double)tp_get_swing_distance_from_player(weapon)) / 10.0;

    /*
     * Try current direction.
     */
    if (thing_is_dir_tl(t)) {
        *dx = -dist_from_player;
        *dy = -dist_from_player;
        return;
    }

    if (thing_is_dir_tr(t)) {
        *dx = dist_from_player;
        *dy = -dist_from_player;
        return;
    }

    if (thing_is_dir_bl(t)) {
        *dx = -dist_from_player;
        *dy = dist_from_player + 0.3;
        return;
    }

    if (thing_is_dir_br(t)) {
        *dx = dist_from_player;
        *dy = dist_from_player + 0.3;
        return;
    }

    if (thing_is_dir_down(t)) {
        *dy = dist_from_player + 0.3;
        return;
    }

    if (thing_is_dir_up(t)) {
        *dy = -dist_from_player;
        return;
    }

    if (thing_is_dir_right(t)) {
        *dx = dist_from_player;
        return;
    }

    if (thing_is_dir_left(t)) {
        *dx = -dist_from_player;
        return;
    }

    *dy = dist_from_player + 0.3;
}

thingp thing_weapon_carry_anim (thingp t)
{
    thingp weapon_carry_anim = 0;

    if (t->on_server) {
        if (t->stats.weapon_carry_anim_id) {
            weapon_carry_anim = thing_server_id(t->stats.weapon_carry_anim_id);
        }
    } else {
        if (t->stats.weapon_carry_anim_id) {
            weapon_carry_anim = thing_client_id(t->stats.weapon_carry_anim_id);
        }
    }

    return (weapon_carry_anim);
}

thingp thing_weapon_swing_anim (thingp t)
{
    /*
     * If this weapon_swing_anim has its own thing id for animations then destroy that.
     */
    thingp weapon_swing_anim = 0;

    if (t->on_server) {
        if (t->stats.weapon_swing_anim_id) {
            weapon_swing_anim = thing_server_id(t->stats.weapon_swing_anim_id);
        }
    } else {
        if (t->stats.weapon_swing_anim_id) {
            weapon_swing_anim = thing_client_id(t->stats.weapon_swing_anim_id);
        }
    }

    return (weapon_swing_anim);
}

void thing_wield_next_weapon (thingp t)
{
    uint32_t i;

    for (i = 0; i < THING_ACTION_BAR_MAX; i++) {
        uint32_t id = t->stats.action_bar[i].id;
        if (!id) {
            continue;
        }

        tpp weapon = id_to_tp(id);
        if (!tp_is_weapon(weapon)) {
            continue;
        }

        thing_wield(t, weapon);
        break;
    }
}

void thing_unwield (thingp t)
{
    tpp weapon = thing_weapon(t);
    if (!weapon) {
        return;
    }

    if (thing_is_player(t)) {
        THING_LOG(t, "unwield %s", tp_short_name(weapon));
    }

    /*
     * If this weapon has its own thing id for animations then destroy that.
     */
    thingp weapon_carry_anim = thing_weapon_carry_anim(t);
    if (weapon_carry_anim) {
        thing_dead(weapon_carry_anim, 0, "owner weapon");
        t->stats.weapon_carry_anim_id = 0;
    }

    thingp weapon_swing_anim = thing_weapon_swing_anim(t);
    if (weapon_swing_anim) {
        thing_dead(weapon_swing_anim, 0, "owner weapon");
        t->stats.weapon_swing_anim_id = 0;
    }
}

void thing_wield (thingp t, tpp weapon)
{
    thing_unwield(t);

    if (thing_is_player(t)) {
        THING_LOG(t, "unwield %s", tp_short_name(weapon));
    }

    const char *carry_as = tp_weapon_carry_anim(weapon);

    if (!carry_as) {
        ERR("%s could not wield %s", thing_logname(t), tp_short_name(weapon));
        return;
    }

    tpp what = tp_find(carry_as);
    if (!what) {
        ERR("Could not find %s to wield for %s", carry_as, thing_logname(t));
        return;
    }

    widp weapon_carry_anim_wid = wid_game_map_server_replace_tile(
                            wid_game_map_server_grid_container,
                            t->x,
                            t->y,
                            0, /* thing */
                            what,
                            0 /* item */,
                            0 /* stats */);
    /*
     * Save the thing id so the client wid can keep track of the weapon.
     */
    thingp child = wid_get_thing(weapon_carry_anim_wid);
    t->stats.weapon_carry_anim_id = child->thing_id;

    child->dir = t->dir;

    /*
     * Attach to the t thing.
     */
    child->owner_id = t->thing_id;

    thing_update(child);

    t->needs_tx_player_update = true;
}

void thing_swing (thingp t)
{
    if (t->stats.weapon_swing_anim_id) {
        /*
         * Still swinging.
         */
LOG("XXX still swinging");
        return;
    }

    tpp weapon = thing_weapon(t);
    if (!weapon) {
        ERR("No weapon to swing");
        return;
    }

    const char *swung_as = tp_weapon_swing_anim(weapon);
    if (!swung_as) {
        ERR("%s could not swing %s", thing_logname(t),
            tp_short_name(weapon));
        return;
    }

    tpp what = tp_find(swung_as);
    if (!what) {
        ERR("Could not find %s to wield for %s",
            swung_as, thing_logname(t));
        return;
    }

    widp weapon_swing_anim_wid = wid_game_map_server_replace_tile(
                            wid_game_map_server_grid_container,
                            t->x,
                            t->y,
                            0, /* thing */
                            what,
                            0 /* item */,
                            0 /* stats */);

    /*
     * Save the thing id so the client wid can keep track of the weapon.
     */
    thingp child = wid_get_thing(weapon_swing_anim_wid);

    /*
     * Attach to the parent thing.
     */
    child->owner_id = t->thing_id;
    t->stats.weapon_swing_anim_id = child->thing_id;

    /*
     * Destroy the thing quickly. Allow enough time for the client anim
     * to run.
     */
    thing_timer_destroy(child, 200);

LOG("XXX swing");
    thing_update(child);

    t->needs_tx_player_update = true;
}
