/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */


#include "main.h"
#include "thing.h"
#include "wid_game_map_server.h"
#include "wid_game_map_client.h"

thingp thing_shield_carry_anim (thingp t)
{
    thingp shield_carry_anim = 0;

    if (t->on_server) {
        if (t->shield_carry_anim_thing_id) {
            shield_carry_anim = thing_server_id(t->shield_carry_anim_thing_id);
        }
    } else {
        if (t->shield_carry_anim_thing_id) {
            shield_carry_anim = thing_client_id(t->shield_carry_anim_thing_id);
        }
    }

    return (shield_carry_anim);
}

void thing_set_shield_carry_anim_id (thingp t,
                                     uint32_t shield_carry_anim_id)
{
    thingp shield_carry_anim;

    if (!shield_carry_anim_id) {
        thing_set_shield_carry_anim(t, 0);
        return;
    }

    if (t->on_server) {
        shield_carry_anim = thing_server_find(shield_carry_anim_id);
    } else {
        shield_carry_anim = thing_client_find(shield_carry_anim_id);
    }

    thing_set_shield_carry_anim(t, shield_carry_anim);
}

void thing_set_shield_carry_anim (thingp t, thingp shield_carry_anim)
{
    if (shield_carry_anim) {
        verify(shield_carry_anim);
    }

    thingp old_shield_carry_anim = thing_shield_carry_anim(t);

    if (old_shield_carry_anim) {
        if (old_shield_carry_anim == shield_carry_anim) {
            return;
        }

        if (shield_carry_anim) {
            THING_LOG(t, "shield carry changed, %s->%s",
                      thing_logname(old_shield_carry_anim),
                      thing_logname(shield_carry_anim));
        } else {
            THING_LOG(t, "remove shield carry animation, %s",
                      thing_logname(old_shield_carry_anim));
        }
    } else {
        if (shield_carry_anim) {
            THING_LOG(t, "shield carry anim now, %s",
                      thing_logname(shield_carry_anim));
        }
    }

    if (shield_carry_anim) {
        t->shield_carry_anim_thing_id = shield_carry_anim->thing_id;
    } else {
        t->shield_carry_anim_thing_id = 0;
    }
}

void thing_unwield_shield (thingp t)
{
    tpp shield = thing_shield(t);
    if (!shield) {
        return;
    }

    THING_LOG(t, "unwielding shield %s", tp_short_name(shield));

    thing_shield_sheath(t);

    t->shield = 0;
}

void thing_shield_sheath (thingp t)
{
    tpp shield = thing_shield(t);
    if (!shield) {
        return;
    }

    THING_LOG(t, "sheathing %s", tp_short_name(shield));

    /*
     * If this shield has its own thing id for animations then destroy that.
     */
    thingp shield_carry_anim = thing_shield_carry_anim(t);
    if (shield_carry_anim) {
        THING_LOG(t, "unwield, carry anim %s", 
                  thing_logname(shield_carry_anim));
        thing_dead(shield_carry_anim, 0, "owner shield");
        thing_set_shield_carry_anim(t, 0);
    }

    /*
     * Else if vanishes on level changes.
     */
    if (!t->on_server) {
        t->shield = 0;
    }
}

void thing_wield_shield (thingp t, tpp shield)
{
    thingp existing_shield = thing_shield_carry_anim(t);

    if (existing_shield) {
        THING_LOG(t, "wield with existing shield %s",
                  thing_logname(existing_shield));
    }

    if (!shield) {
        THING_ERR(t, "could not wield null pointer!");
        return;
    }

    const char *carry_as = tp_shield_carry_anim(shield);

    if (!carry_as) {
        THING_ERR(t, "could not wield shield %s", tp_short_name(shield));
        return;
    }

    tpp what = tp_find(carry_as);
    if (!what) {
        THING_ERR(t, "Could not find %s to wield", carry_as);
        return;
    }

    if (t->shield == shield) {
        if (t->on_server) {
            if (thing_wid(existing_shield)) {
                /*
                 * Add onto existing shield.
                 */
                thing_stats_modify_hp(existing_shield, 
                                      tp_get_stats_max_hp(what));
                return;
            }
        } else {
            return;
        }
    }

    if (thing_is_player(t)) {
        THING_LOG(t, "wield shield %s", tp_short_name(shield));
    }

    t->shield = shield;

    widp shield_carry_anim_wid;

    if (t->on_server) {
        shield_carry_anim_wid = wid_game_map_server_replace_tile(
                                wid_game_map_server_grid_container,
                                t->x,
                                t->y,
                                existing_shield, /* thing */
                                what,
                                0, /* tpp data */
                                0 /* item */,
                                0 /* stats */);
    } else {
        shield_carry_anim_wid = wid_game_map_client_replace_tile(
                                wid_game_map_client_grid_container,
                                t->x, t->y, 0, what);
    }

    if (!shield_carry_anim_wid) {
        THING_ERR(t, "failed wield shield %s", tp_short_name(shield));
        return;
    }

    /*
     * Save the thing id so the client wid can keep track of the shield.
     */
    thingp child = wid_get_thing(shield_carry_anim_wid);
    thing_set_shield_carry_anim(t, child);

    child->dir = t->dir;

    /*
     * Attach to the thing.
     */
    thing_set_owner(child, t);

    if (t->on_server) {
        thing_update(t);
    }
}

widp thing_get_shield_carry_anim_wid (thingp t)
{
    thingp shield = thing_shield_carry_anim(t);
    if (!shield) {
        return (0);
    }

    return (thing_wid(shield));
}
