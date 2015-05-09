/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "color.h"
#include "wid_popup.h"
#include "wid_player_stats.h"
#include "string_util.h"
#include "name.h"
#include "thing_template.h"
#include "wid_player_info.h"
#include "math_util.h"
#include "thing.h"
#include "client.h"
#include "socket_util.h"

int32_t thing_stats_get_total_damage (thingp t)
{
    double damage = t->damage;

    if (!damage) {
        damage = tp_get_damage(t->tp);
    }

    double modifier = 0;

    tpp weapon = thing_weapon(t);
    if (weapon) {
        if (!damage) {
            damage = tp_get_damage(weapon);
        }

        /*
         * A wand does no damage, but what if fires does...
         */
        tpp projectile = tp_fires(weapon);
        if (projectile) {
            weapon = projectile;
        }

        if (tp_is_melee_weapon(weapon)) {
            modifier += 
                thing_stats_val_to_modifier(thing_stats_get_attack_melee(t));
        }

        if (tp_is_ranged_weapon(weapon)) {
            modifier += 
                thing_stats_val_to_modifier(thing_stats_get_attack_ranged(t));
        }

        if (tp_is_magical_weapon(weapon)) {
            modifier += 
                thing_stats_val_to_modifier(thing_stats_get_attack_magical(t));
        }
    }

    /*
     * Modifier of 1 maps to +10 % in damage.
     */
    int32_t final_damage = ceil(damage + (damage * (modifier / 10.0)));

    if (thing_is_monst(t) || thing_is_player(t)) {
        if (weapon) {
            THING_LOG(t, "attack damage, modifier %d, damage %d -> %d", 
                      (int) modifier, (int) damage, final_damage);
        } else {
            THING_LOG(t, "attack damage (from %s), modifier %d, damage %d -> %d", 
                      weapon ? tp_name(weapon) : 0,
                      (int) modifier, (int) damage, final_damage);
        }
    }

    return (final_damage);
}
