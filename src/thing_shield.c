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
#include "wid_game_map_client.h"
#include "math_util.h"

thingp level_place_shield (levelp level, 
                           thingp owner,
                           double x, double y)
{
    widp w = thing_place_behind_or_under(owner,
                                         id_to_tp(THING_SHIELD1),
                                         0 /* itemp */);
    if (!w) {
        ERR("could not place shield");
        return (0);
    }

    thingp t = wid_get_thing(w);

    thing_set_owner(t, owner);

    thing_wake(t);

    return (t);
}

thingp thing_shield_anim (thingp t)
{
    /*
     * If this shield_anim has its own thing id for animations then 
     * destroy that.
     */
    thingp shield_anim = 0;

    if (t->on_server) {
        if (t->shield_anim_thing_id) {
            shield_anim = thing_server_id(t->shield_anim_thing_id);
        }
    } else {
        if (t->shield_anim_thing_id) {
            shield_anim = thing_client_id(t->shield_anim_thing_id);
        }
    }

    return (shield_anim);
}

void thing_set_shield_anim (thingp t, thingp shield_anim)
{
    if (shield_anim) {
        verify(shield_anim);
    }

    thingp old_shield_anim = thing_shield_anim(t);

    if (old_shield_anim) {
        if (old_shield_anim == shield_anim) {
            return;
        }

        if (shield_anim) {
            THING_LOG(t, "shield_anim change %s->%s", 
                      thing_logname(old_shield_anim), 
                      thing_logname(shield_anim));
        } else {
            if (0) {
                THING_LOG(t, "remove shield_anim %s",
                          thing_logname(old_shield_anim));
            }
        }
    } else {
        if (shield_anim) {
            if (0) {
                THING_LOG(t, "shield_anim %s",
                          thing_logname(shield_anim));
            }
        }
    }

    if (shield_anim) {
        t->shield_anim_thing_id = shield_anim->thing_id;
    } else {
        t->shield_anim_thing_id = 0;
    }
}
