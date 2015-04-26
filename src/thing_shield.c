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

void thing_set_shield_anim_id (thingp t,
                                     uint32_t shield_anim_id)
{
    thingp shield_anim;

    if (!shield_anim_id) {
        thing_set_shield_anim(t, 0);
        return;
    }

    if (t->on_server) {
        shield_anim = thing_server_find(shield_anim_id);
    } else {
        shield_anim = thing_client_find(shield_anim_id);
    }

    thing_set_shield_anim(t, shield_anim);
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
            THING_LOG(t, "weapon carry changed, %s->%s",
                      thing_logname(old_shield_anim),
                      thing_logname(shield_anim));
        } else {
            THING_LOG(t, "remove weapon carry animation, %s",
                      thing_logname(old_shield_anim));
        }
    } else {
        if (shield_anim) {
            THING_LOG(t, "weapon carry anim nos, %s",
                      thing_logname(shield_anim));
        }
    }

    if (shield_anim) {
        t->shield_anim_thing_id = shield_anim->thing_id;
    } else {
        t->shield_anim_thing_id = 0;
    }
}

thingp thing_shield_anim (thingp t)
{
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

void thing_shield_sheath (thingp t)
{
    tpp shield = thing_shield(t);
    if (!shield) {
        return;
    }

    THING_LOG(t, "sheathing shield %s", tp_short_name(shield));

    /*
     * If this shield has its own thing id for animations then destroy that.
     */
    thingp shield_anim = thing_shield_anim(t);
    if (shield_anim) {
        THING_LOG(t, "unwield, carry anim shield %s", 
                  thing_logname(shield_anim));
        thing_dead(shield_anim, 0, "owner shield");
        thing_set_shield_anim(t, 0);
    }

    t->shield = 0;
}

void thing_unwield_shield (thingp t)
{
    tpp shield = thing_shield(t);
    if (!shield) {
        return;
    }

    THING_LOG(t, "unwielding weapon shield %s", tp_short_name(shield));

    thing_shield_sheath(t);
}

void thing_wield_shield (thingp t, tpp shield)
{
    if (t->shield == shield) {
        return;
    }

    thing_unwield_shield(t);

    if (thing_is_player(t)) {
        THING_LOG(t, "unwield weapon shield %s", tp_short_name(shield));
    }

    const char *as = tp_shield_anim(shield);

    if (!as) {
        ERR("%s could not wield shield %s", thing_logname(t), tp_short_name(shield));
        return;
    }

    tpp what = tp_find(as);
    if (!what) {
        ERR("Could not find %s to wield for %s", as, thing_logname(t));
        return;
    }

    t->shield = shield;

    widp shield_anim_wid;

    if (t->on_server) {
        shield_anim_wid = wid_game_map_server_replace_tile(
                                wid_game_map_server_grid_container,
                                t->x,
                                t->y,
                                0, /* thing */
                                what,
                                0, /* tpp data */
                                0 /* item */,
                                0 /* stats */);
    } else {
        shield_anim_wid = wid_game_map_client_replace_tile(
                                wid_game_map_client_grid_container,
                                t->x, t->y, 0, what);
    }

    /*
     * Save the thing id so the client wid can keep track of the shield.
     */
    thingp child = wid_get_thing(shield_anim_wid);
    thing_set_shield_anim(t, child);

    child->dir = t->dir;

    /*
     * Attach to the thing.
     */
    thing_set_owner(child, t);

    if (t->on_server) {
        thing_update(t);
    }
}

widp thing_get_shield_anim_wid (thingp t)
{
    thingp shield = thing_shield_anim(t);
    if (!shield) {
        return (0);
    }

    return (thing_wid(shield));
}
