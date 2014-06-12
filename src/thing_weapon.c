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

thingp thing_weapon_anim (thingp t)
{
    /*
     * If this weapon_anim has its own thing id for animations then destroy that.
     */
    thingp weapon_anim = 0;

    if (t->on_server) {
        if (t->weapon_anim_id) {
            weapon_anim = thing_server_ids[t->weapon_anim_id];
        }
    } else {
        if (t->weapon_anim_id) {
            weapon_anim = thing_client_ids[t->weapon_anim_id];
        }
    }

    return (weapon_anim);
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
    thingp weapon_anim = thing_weapon_anim(t);
    if (weapon_anim) {
        thing_dead(weapon_anim, 0, "owner weapon");
        t->weapon_anim_id = 0;
    }
}

void thing_wield (thingp t, thing_templatep tmp)
{
    thing_unwield(t);

    t->weapon = tmp;

    if (thing_is_player(t)) {
        THING_SHOUT_AT(t, INFO,
                       "You wield the %s", thing_template_short_name(tmp));

        t->needs_tx_player_update = true;
    }

    const char *weapon_anim = thing_template_weapon_anim(tmp);

    if (weapon_anim) {
        thing_templatep what = thing_template_find(weapon_anim);
        if (!what) {
            DIE("could now find %s to wield for %s",
                weapon_anim, thing_logname(t));
        }

        widp weapon_anim_wid = wid_game_map_server_replace_tile(
                                wid_game_map_server_grid_container,
                                t->x,
                                t->y,
                                what);
        /*
         * Save the thing id so the client wid can keep track of the weapon.
         */
        thingp weapon_anim = wid_get_thing(weapon_anim_wid);
        t->weapon_anim_id = weapon_anim->thing_id;
    }
}
