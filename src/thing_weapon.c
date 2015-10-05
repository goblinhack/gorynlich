/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
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

void thing_set_weapon_carry_anim_id (thingp t,
                                     uint32_t weapon_carry_anim_id)
{
    thingp weapon_carry_anim;

    if (!weapon_carry_anim_id) {
        thing_set_weapon_carry_anim(t, 0);
        return;
    }

    if (t->on_server) {
        weapon_carry_anim = thing_server_find(weapon_carry_anim_id);
    } else {
        weapon_carry_anim = thing_client_find(weapon_carry_anim_id);
    }

    thing_set_weapon_carry_anim(t, weapon_carry_anim);
}

void thing_set_weapon_carry_anim (thingp t, thingp weapon_carry_anim)
{
    if (weapon_carry_anim) {
        verify(weapon_carry_anim);
    }

    thingp old_weapon_carry_anim = thing_weapon_carry_anim(t);

    if (old_weapon_carry_anim) {
        if (old_weapon_carry_anim == weapon_carry_anim) {
            return;
        }

        if (weapon_carry_anim) {
            THING_LOG(t, "weapon carry changed, %s->%s",
                      thing_logname(old_weapon_carry_anim),
                      thing_logname(weapon_carry_anim));
        } else {
            THING_LOG(t, "remove weapon carry animation, %s",
                      thing_logname(old_weapon_carry_anim));
        }
    } else {
        if (weapon_carry_anim) {
            THING_LOG(t, "weapon carry anim now, %s",
                      thing_logname(weapon_carry_anim));
        }
    }

    if (weapon_carry_anim) {
        t->weapon_carry_anim_thing_id = weapon_carry_anim->thing_id;

        /*
         * Scale up weapons so they look the same size as the carryer.
         */
        double scale = tp_get_scale(t->tp);
        if (scale != 1.0) {
            wid_scaling_blit_to_pct_in(weapon_carry_anim->wid, scale, scale, 500, 9999999);
        }
    } else {
        t->weapon_carry_anim_thing_id = 0;
    }
}

void thing_set_weapon_swing_anim_id (thingp t,
                                     uint32_t weapon_swing_anim_id)
{
    thingp weapon_swing_anim;

    if (!weapon_swing_anim_id) {
        thing_set_weapon_swing_anim(t, 0);
        return;
    }

    if (t->on_server) {
        weapon_swing_anim = thing_server_find(weapon_swing_anim_id);
    } else {
        weapon_swing_anim = thing_client_find(weapon_swing_anim_id);
    }

    thing_set_weapon_swing_anim(t, weapon_swing_anim);
}

void thing_set_weapon_swing_anim (thingp t, thingp weapon_swing_anim)
{
    if (weapon_swing_anim) {
        verify(weapon_swing_anim);
    }

    thingp old_weapon_swing_anim = thing_weapon_swing_anim(t);

    if (old_weapon_swing_anim) {
        if (old_weapon_swing_anim == weapon_swing_anim) {
            return;
        }

        if (weapon_swing_anim) {
            THING_LOG(t, "weapon_swing_anim change %s->%s",
                      thing_logname(old_weapon_swing_anim),
                      thing_logname(weapon_swing_anim));
        } else {
#ifdef THING_DEBUG
            THING_LOG(t, "remove weapon_swing_anim %s", thing_logname(old_weapon_swing_anim));
#endif
        }
    } else {
        if (weapon_swing_anim) {
#ifdef THING_DEBUG
            THING_LOG(t, "weapon_swing_anim %s", thing_logname(weapon_swing_anim));
#endif
        }
    }

    if (weapon_swing_anim) {
        t->weapon_swing_anim_thing_id = weapon_swing_anim->thing_id;

        /*
         * Scale up weapons so they look the same size as the carryer.
         */
        double scale = tp_get_scale(t->tp);
        if (scale != 1.0) {
            wid_scaling_blit_to_pct_in(weapon_swing_anim->wid, scale, scale, 500, 9999999);
        }
    } else {
        t->weapon_swing_anim_thing_id = 0;
    }
}

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

    if (thing_has_ability_reverse_swing(t)) {

        if ((myrand() % 10) < 5) {

            if (thing_is_dir_tl(t)) {
                *dx = +dist_from_player;
                *dy = +dist_from_player;
                return;
            }

            if (thing_is_dir_tr(t)) {
                *dx = -dist_from_player;
                *dy = +dist_from_player;
                return;
            }

            if (thing_is_dir_bl(t)) {
                *dx = +dist_from_player;
                *dy = -dist_from_player;
                return;
            }

            if (thing_is_dir_br(t)) {
                *dx = -dist_from_player;
                *dy = -dist_from_player;
                return;
            }

            if (thing_is_dir_down(t)) {
                *dy = -dist_from_player;
                return;
            }

            if (thing_is_dir_up(t)) {
                *dy = +dist_from_player;
                return;
            }

            if (thing_is_dir_right(t)) {
                *dx = -dist_from_player;
                return;
            }

            if (thing_is_dir_left(t)) {
                *dx = +dist_from_player;
                return;
            }
        }
    }

    /*
     * Try current direction.
     */
    if (thing_is_dir_tl(t)) {
        *dx = -dist_from_player;
        *dy = -dist_from_player;
        return;
    }

    /*
     * Careful here - change dy too much and you hit through walls
     */

    if (thing_is_dir_tr(t)) {
        *dx = dist_from_player;
        *dy = -dist_from_player;
        return;
    }

    if (thing_is_dir_bl(t)) {
        *dx = -dist_from_player;
        *dy = dist_from_player;
        return;
    }

    if (thing_is_dir_br(t)) {
        *dx = dist_from_player;
        *dy = dist_from_player;
        return;
    }

    if (thing_is_dir_down(t)) {
        *dy = dist_from_player;
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
        if (t->weapon_carry_anim_thing_id) {
            weapon_carry_anim = thing_server_id(t->weapon_carry_anim_thing_id);
        }
    } else {
        if (t->weapon_carry_anim_thing_id) {
            weapon_carry_anim = thing_client_id(t->weapon_carry_anim_thing_id);
        }
    }

    return (weapon_carry_anim);
}

thingp thing_weapon_swing_anim (thingp t)
{
    /*
     * If this weapon_swing_anim has its own thing id for animations then 
     * destroy that.
     */
    thingp weapon_swing_anim = 0;

    if (t->on_server) {
        if (t->weapon_swing_anim_thing_id) {
            weapon_swing_anim = thing_server_id(t->weapon_swing_anim_thing_id);
        }
    } else {
        if (t->weapon_swing_anim_thing_id) {
            weapon_swing_anim = thing_client_id(t->weapon_swing_anim_thing_id);
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

    THING_LOG(t, "unwielding %s", tp_short_name(weapon));

    thing_sheath(t);

    /*
     * Update the thing stats so the weapon inventory changes.
     */
    thing_stats_set_action_bar_index(t, 0);
}

void thing_sheath (thingp t)
{
    tpp weapon = thing_weapon(t);
    if (!weapon) {
        return;
    }

    THING_LOG(t, "sheathing %s", tp_short_name(weapon));

    /*
     * If this weapon has its own thing id for animations then destroy that.
     */
    thingp weapon_carry_anim = thing_weapon_carry_anim(t);
    if (weapon_carry_anim) {
        THING_LOG(t, "unwield, carry anim %s", 
                  thing_logname(weapon_carry_anim));
        thing_dead(weapon_carry_anim, 0, "owner weapon");
        thing_set_weapon_carry_anim(t, 0);
    }

    thingp weapon_swing_anim = thing_weapon_swing_anim(t);
    if (weapon_swing_anim) {
        THING_LOG(t, "unwield, swing anim %s", 
                  thing_logname(weapon_swing_anim));
        thing_dead(weapon_swing_anim, 0, "owner weapon");
        thing_set_weapon_swing_anim(t, 0);
    }

    t->weapon = 0;
}

void thing_wield (thingp t, tpp weapon)
{
    if (t->weapon == weapon) {
        return;
    }

    thing_unwield(t);

    if (thing_is_player(t)) {
        THING_LOG(t, "unwield weapon %s", tp_short_name(weapon));
    }

    const char *carry_as = tp_weapon_carry_anim(weapon);

    if (!carry_as) {
        THING_ERR(t, "could not wield weapon %s", tp_short_name(weapon));
        return;
    }

    tpp what = tp_find(carry_as);
    if (!what) {
        THING_ERR(t, "could not find %s to wield", carry_as);
        return;
    }

    t->weapon = weapon;

    /*
     * Update the thing stats so the weapon inventory changes.
     */
    int32_t action_bar_index;

    if (thing_stats_has_action_bar_item(&t->stats,
                                        tp_to_id(weapon),
                                        &action_bar_index)) {
        thing_stats_set_action_bar_index(t, action_bar_index);
    }

    widp weapon_carry_anim_wid;

    if (t->on_server) {
        weapon_carry_anim_wid = wid_game_map_server_replace_tile(
                                wid_game_map_server_grid_container,
                                t->x,
                                t->y,
                                0, /* thing */
                                what,
                                0, /* tpp data */
                                0 /* item */,
                                0 /* stats */);
    } else {
        weapon_carry_anim_wid = wid_game_map_client_replace_tile(
                                wid_game_map_client_grid_container,
                                t->x, t->y, 0, what);
    }

    /*
     * Save the thing id so the client wid can keep track of the weapon.
     */
    thingp child = wid_get_thing(weapon_carry_anim_wid);
    thing_set_weapon_carry_anim(t, child);

    child->dir = t->dir;

    /*
     * Attach to the thing.
     */
    thing_set_owner(child, t);

    if (t->on_server) {
        thing_update(t);
    }
}

void thing_swing (thingp t)
{
    if (t->weapon_swing_anim_thing_id) {
        /*
         * Still swinging.
         */
        return;
    }

    tpp weapon = thing_weapon(t);
    if (!weapon) {
        THING_ERR(t, "No weapon to swing");
        return;
    }

    const char *swung_as = tp_weapon_swing_anim(weapon);
    if (!swung_as) {
        THING_ERR(t, "could not swing %s", tp_short_name(weapon));
        return;
    }

    tpp what = tp_find(swung_as);
    if (!what) {
        THING_ERR(t, "could not find %s to wield", swung_as);
        return;
    }

    /*
     * Put the swing anim on the map
     */
    widp weapon_swing_anim_wid;

    if (t->on_server) {
        weapon_swing_anim_wid = wid_game_map_server_replace_tile(
                                    wid_game_map_server_grid_container,
                                    t->x,
                                    t->y,
                                    0, /* thing */
                                    what,
                                    0, /* tpp data */
                                    0 /* item */,
                                    0 /* stats */);
    } else {
        weapon_swing_anim_wid = wid_game_map_client_replace_tile(
                                    wid_game_map_client_grid_container,
                                    t->x, 
                                    t->y, 
                                    0,  /* thing */
                                    what);
    }

    /*
     * Save the thing id so the client wid can keep track of the weapon.
     */
    thingp child = wid_get_thing(weapon_swing_anim_wid);

    /*
     * Attach to the parent thing.
     */
    thing_set_owner(child, t);

    child->dir = t->dir;

    thing_set_weapon_swing_anim(t, child);

    if (t->on_server) {
        thing_update(t);

        t->needs_tx_weapon_swung = true;

        thing_timer_destroy(child, 50);
    } else {
        /*
         * Hide the weapon too or it just floats in the air.
         */
        thingp weapon_carry_anim = thing_weapon_carry_anim(t);
        if (weapon_carry_anim) {
            thing_hide(weapon_carry_anim);
        }
    }
}

widp thing_get_weapon_carry_anim_wid (thingp t)
{
    thingp weapon = thing_weapon_carry_anim(t);
    if (!weapon) {
        return (0);
    }

    return (thing_wid(weapon));
}

/*
 * If a player carrying a weapon moves around, adjust the placement of the 
 * weapon so it is not always in front.
 */
void thing_set_weapon_placement (thingp t)
{
    widp owner_wid = thing_wid(t);
    if (!owner_wid) {
        return;
    }

    widp weapon_wid = thing_get_weapon_carry_anim_wid(t);
    if (!weapon_wid) {
        return;
    }

    int depth_modifier = 0;

    if (thing_is_dead_or_dying(t)) {
        depth_modifier = -1;
    } else switch (t->dir) {
        case THING_DIR_LEFT:
            depth_modifier = 1;
            break;
        case THING_DIR_RIGHT:
            depth_modifier = 1;
            break;
        case THING_DIR_UP:
            depth_modifier = 0;
            break;
        case THING_DIR_DOWN:
            depth_modifier = 1;
            break;
        case THING_DIR_TL:
            depth_modifier = 0;
            break;
        case THING_DIR_BL:
            depth_modifier = 0;
            break;
        case THING_DIR_TR:
            depth_modifier = 1;
            break;
        case THING_DIR_BR:
            depth_modifier = 1;
            break;
    }

    int weapon_depth = wid_get_z_depth(weapon_wid);
    int player_depth = wid_get_z_depth(owner_wid);
    int new_weapon_depth = player_depth + depth_modifier;

    if (new_weapon_depth == weapon_depth) {
        return;
    }

    wid_set_z_depth(weapon_wid, new_weapon_depth);
}

/*
 * Weapon is destroyed due to damage.
 */
void thing_weapon_worn_out (thingp t, tpp weapon)
{
    thing_unwield(t);

    if (thing_is_player(t)) {
        MSG_SERVER_SHOUT_AT(POPUP, t, 0, 0,
                                   "%%%%fg=red$"
                                   "Your %s crumbles to dust",
                                   tp_short_name(weapon));
    }

    thing_wield_next_weapon(t);
}

/*
 * Check for the weapon being damaged.
 */
void thing_weapon_check_for_wear_damage (thingp target, 
                                        thingp hitter, 
                                        tpp weapon)
{
    /*
     * Hitting soft things doesn't do damage.
     */
    if (!thing_is_hard(target)) {
        return;
    }

    /*
     * If we've already checked for damage on this thing when we used it, 
     * don't check again on impact. Makes no sense for bows anyway.
     */
    if (tp_is_damaged_on_firing(weapon)) {
        return;
    }

    /*
     * What are the odds of it being damaged?
     */
    uint32_t d10000_chance_of_breaking = 
            tp_get_d10000_chance_of_breaking(weapon);

    int r = (myrand() % 10000);
    if (r >= d10000_chance_of_breaking) {
        return;
    }

    /*
     * Take a notch off of the weapons quality.
     */
    if (!thing_wear_out(hitter, weapon)) {
        if (thing_is_player(hitter)) {
            MSG_SERVER_SHOUT_AT(
                POPUP, hitter,  0, 0,
                "%%%%fg=orange$"
                "Your %s is damaged", 
                tp_short_name(weapon));
        }
    }
}

/*
 * For things like bows that wear out
 */
void thing_weapon_check_for_damage_on_firing (thingp hitter,
                                              tpp weapon)
{
    if (!tp_is_damaged_on_firing(weapon)) {
        return;
    }

    /*
     * What are the odds of it being damaged?
     */
    uint32_t d10000_chance_of_breaking = 
            tp_get_d10000_chance_of_breaking(weapon);

    int r = (myrand() % 10000);
    if (r >= d10000_chance_of_breaking) {
        return;
    }

    /*
     * Take a notch off of the weapons quality.
     */
    if (!thing_wear_out(hitter, weapon)) {
        if (thing_is_player(hitter)) {
            MSG_SERVER_SHOUT_AT(
                POPUP, hitter,  0, 0,
                "%%%%fg=orange$"
                "Your %s is damaged", 
                tp_short_name(weapon));
        }
    }
}
