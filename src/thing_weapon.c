/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>
#include <math.h>

#include "main.h"
#include "thing.h"
#include "thing_timer.h"
#include "wid.h"
#include "wid_game_map_client.h"
#include "wid_game_map_server.h"
#include "map.h"
#include "level.h"
#include "time.h"
#include "string.h"
#include "wid_textbox.h"
#include "color.h"
#include "sound.h"
#include "socket.h"
#include "client.h"
#include "timer.h"
#include "math.h"

void thing_weapon_swing_offset (thingp t, double *dx, double *dy)
{
    double dist_from_player = 0.7;

    /*
     * Try current direction.
     */
    *dx = 0.0;
    *dy = 0.0;

    if (thing_is_dir_down(t)) {
        *dy = dist_from_player;
    }

    if (thing_is_dir_up(t)) {
        *dy = -dist_from_player;
    }

    if (thing_is_dir_right(t)) {
        *dx = dist_from_player;
    }

    if (thing_is_dir_left(t)) {
        *dx = -dist_from_player;
    }

    if (thing_is_dir_tl(t)) {
        *dx = -dist_from_player;
        *dy = -dist_from_player;
    }

    if (thing_is_dir_tr(t)) {
        *dx = dist_from_player;
        *dy = -dist_from_player;
    }

    if (thing_is_dir_bl(t)) {
        *dx = -dist_from_player;
        *dy = dist_from_player;
    }

    if (thing_is_dir_br(t)) {
        *dx = dist_from_player;
        *dy = dist_from_player;
    }
}

thingp thing_weapon_carry_anim (thingp t)
{
    /*
     * If this weapon_carry_anim has its own thing id for animations then destroy that.
     */
    thingp weapon_carry_anim = 0;

    if (t->on_server) {
        if (t->weapon_carry_anim_id) {
            weapon_carry_anim = thing_server_ids[t->weapon_carry_anim_id];
        }
    } else {
        if (t->weapon_carry_anim_id) {
            weapon_carry_anim = thing_client_ids[t->weapon_carry_anim_id];
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
        if (t->weapon_swing_anim_id) {
            weapon_swing_anim = thing_server_ids[t->weapon_swing_anim_id];
        }
    } else {
        if (t->weapon_swing_anim_id) {
            weapon_swing_anim = thing_client_ids[t->weapon_swing_anim_id];
        }
    }

    return (weapon_swing_anim);
}

void thing_wield_next_weapon (thingp t)
{
    uint32_t i;

    for (i = 0; i < THING_MAX; i++) {
        if (!thing_is_carrying(t, i)) {
            continue;
        }

        thing_templatep tmp = id_to_thing_template(i);
        if (!thing_template_is_weapon(tmp)) {
            continue;
        }

        thing_wield(t, tmp);
        break;
    }
}

void thing_unwield (thingp t)
{
    if (!t->weapon) {
        return;
    }

    if (thing_is_player(t)) {
        THING_LOG(t, "unwield %s", thing_template_short_name(t->weapon));
    }

    t->weapon = 0;

    /*
     * If this weapon has its own thing id for animations then destroy that.
     */
    thingp weapon_carry_anim = thing_weapon_carry_anim(t);
    if (weapon_carry_anim) {
        thing_dead(weapon_carry_anim, 0, "owner weapon");
        t->weapon_carry_anim_id = 0;
    }

    thingp weapon_swing_anim = thing_weapon_swing_anim(t);
    if (weapon_swing_anim) {
        thing_dead(weapon_swing_anim, 0, "owner weapon");
        t->weapon_swing_anim_id = 0;
    }
}

void thing_wield (thingp parent, thing_templatep tmp)
{
    thing_unwield(parent);

    parent->weapon = tmp;

    if (thing_is_player(parent)) {
        THING_SHOUT_AT(parent, INFO,
                       "You wield the %s", thing_template_short_name(tmp));

        parent->needs_tx_player_update = true;
    }

    const char *child = thing_template_weapon_carry_anim(tmp);

    if (child) {
        thing_templatep what = thing_template_find(child);
        if (!what) {
            DIE("could now find %s to wield for %s",
                child, thing_logname(parent));
        }

        widp weapon_carry_anim_wid = wid_game_map_server_replace_tile(
                                wid_game_map_server_grid_container,
                                parent->x,
                                parent->y,
                                what);
        /*
         * Save the thing id so the client wid can keep track of the weapon.
         */
        thingp child = wid_get_thing(weapon_carry_anim_wid);
        parent->weapon_carry_anim_id = child->thing_id;

        child->dir = parent->dir;

        parent->needs_tx_player_update = true;
    }
}

void thing_swing (thingp parent)
{
    if (parent->weapon_swing_anim_id) {
        return;
    }

    const char *child = thing_template_weapon_swing_anim(parent->weapon);
    if (child) {
        thing_templatep what = thing_template_find(child);
        if (!what) {
            DIE("could now find %s to wield for %s",
                child, thing_logname(parent));
        }

        widp weapon_swing_anim_wid = wid_game_map_server_replace_tile(
                                wid_game_map_server_grid_container,
                                parent->x,
                                parent->y,
                                what);

        /*
         * Save the thing id so the client wid can keep track of the weapon.
         */
        thingp child = wid_get_thing(weapon_swing_anim_wid);

        /*
         * Attach to the parent thing.
         */
        child->owner_id = parent->thing_id;
        parent->weapon_swing_anim_id = child->thing_id;

        /*
         * Destroy the thing quickly. Allow enough time for the client anim
         * to run.
         */
        thing_timer_destroy(child, 100);
LOG("swung %s id %d",thing_logname(child), child->thing_id);
LOG("swung %s id weapon_swing_anim_id %d",thing_logname(parent), parent->weapon_swing_anim_id);

        parent->needs_tx_player_update = true;
    }
}
