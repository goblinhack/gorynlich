/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */


#include "color.h"
#include "thing.h"

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
     * Melee
     */
    tpp helmet = thing_helmet(t);
    if (helmet) {
        modifier += thing_stats_val_to_modifier(tp_get_stats_attack_melee(helmet));
    }

    tpp armor = thing_armor(t);
    if (armor) {
        modifier += thing_stats_val_to_modifier(tp_get_stats_attack_melee(armor));
    }

    tpp left_arm = thing_arm_left(t);
    if (left_arm) {
        modifier += thing_stats_val_to_modifier(tp_get_stats_attack_melee(left_arm));
    }

    tpp right_arm = thing_arm_right(t);
    if (right_arm) {
        modifier += thing_stats_val_to_modifier(tp_get_stats_attack_melee(right_arm));
    }

    /*
     * Magical
     */
    helmet = thing_helmet(t);
    if (helmet) {
        modifier += thing_stats_val_to_modifier(tp_get_stats_attack_magical(helmet));
    }

    armor = thing_armor(t);
    if (armor) {
        modifier += thing_stats_val_to_modifier(tp_get_stats_attack_magical(armor));
    }

    left_arm = thing_arm_left(t);
    if (left_arm) {
        modifier += thing_stats_val_to_modifier(tp_get_stats_attack_magical(left_arm));
    }

    right_arm = thing_arm_right(t);
    if (right_arm) {
        modifier += thing_stats_val_to_modifier(tp_get_stats_attack_magical(right_arm));
    }

    /*
     * Ranged
     */
    helmet = thing_helmet(t);
    if (helmet) {
        modifier += thing_stats_val_to_modifier(tp_get_stats_attack_ranged(helmet));
    }

    armor = thing_armor(t);
    if (armor) {
        modifier += thing_stats_val_to_modifier(tp_get_stats_attack_ranged(armor));
    }

    left_arm = thing_arm_left(t);
    if (left_arm) {
        modifier += thing_stats_val_to_modifier(tp_get_stats_attack_melee(left_arm));
        modifier += thing_stats_val_to_modifier(tp_get_stats_attack_ranged(left_arm));
        modifier += thing_stats_val_to_modifier(tp_get_stats_attack_magical(left_arm));
    }

    right_arm = thing_arm_right(t);
    if (right_arm) {
        modifier += thing_stats_val_to_modifier(tp_get_stats_attack_melee(right_arm));
        modifier += thing_stats_val_to_modifier(tp_get_stats_attack_ranged(right_arm));
        modifier += thing_stats_val_to_modifier(tp_get_stats_attack_magical(right_arm));
    }

    /*
     * Modifier of 1 maps to +10 % in damage.
     */
    int32_t final_damage = ceil(damage + (damage * (modifier / 10.0)));

    if (thing_is_monst(t) || thing_is_player(t)) {
        if (weapon) {
            THING_LOG(t, "attack damage (from %s), modifier %d, damage %d -> %d", 
                      weapon ? tp_name(weapon) : 0,
                      (int) modifier, (int) damage, final_damage);
        } else {
            THING_LOG(t, "attack damage, modifier %d, damage %d -> %d", 
                      (int) modifier, (int) damage, final_damage);
        }
    }

    if (!final_damage) {
        return (final_damage);
    }

    /*
     * 1dx damage.
     */
    return (myrand() % final_damage);
}
