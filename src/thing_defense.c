/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */


#include "color.h"
#include "thing.h"

int32_t thing_stats_get_total_damage_minus_defense (thingp t, 
                                                    thingp hitter, 
                                                    int32_t damage)
{
    double d = damage;
    double modifier = thing_stats_val_to_modifier(thing_stats_get_defense(t));

    tpp helmet = thing_helmet(t);
    if (helmet) {
        modifier += thing_stats_val_to_modifier(tp_get_stats_defense(helmet));
    }

    tpp boots = thing_boots(t);
    if (boots) {
        modifier += thing_stats_val_to_modifier(tp_get_stats_defense(boots));
    }

    tpp armor = thing_armor(t);
    if (armor) {
        modifier += thing_stats_val_to_modifier(tp_get_stats_defense(armor));
    }

    tpp left_arm = thing_arm_left(t);
    if (left_arm) {
        modifier += thing_stats_val_to_modifier(tp_get_stats_defense(left_arm));
    }

    tpp right_arm = thing_arm_right(t);
    if (right_arm) {
        modifier += thing_stats_val_to_modifier(tp_get_stats_defense(right_arm));
    }

    if (modifier < 0) {
        d *= -(modifier - 1.0);
    } else if (modifier == 0) {
        /*
         * No change.
         */
    } else {
        d /= (modifier + 0.5);
    }

    if (d < 1) {
        d = 1;
    }

    THING_LOG(t, "damage modified from %d to %d, modifier %f", 
              damage, (int)d, modifier);

    return ((int)d);
}
