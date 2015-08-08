/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */


#include "color.h"
#include "thing.h"
#include "time_util.h"

void thing_health_tick (thingp t)
{
    /*
     * Auto heal
     */
    if (thing_stats_get_hp(t) < thing_stats_get_max_hp(t)) {
        /*
         * Only heal if not being actively attacked.
         */
        if (time_have_x_tenths_passed_since(100,
                                            t->timestamp_last_attacked)) {
            /*
             * Need to allow the player to die!
             */
            if (thing_stats_get_hp(t) > 0) {
                int delta = 
                    thing_stats_val_to_modifier(thing_stats_get_toughness(t));
                if (delta > 0) {
                    thing_stats_modify_hp(t, delta);
                }
            }
        }
    }

    /*
     * Start to croak it if below 0 in health.
     */
    if (thing_stats_get_hp(t) <= 0) {
        THING_LOG(t, "running out of life");

        if (single_player_mode) {
            /*
             * Quicker death in single player mode as there is little chance 
             * of resurrection.
             */
            thing_stats_modify_hp(t, -2);
        } else {
            thing_stats_modify_hp(t, -1);
        }

        thing_update(t);

        if (thing_stats_get_hp(t) <= THING_MIN_HEALTH) {
            thing_dead(t, 0, 0);
        }
    }

    /*
     * If over max health, return to max.
     */
    int delta = thing_stats_get_hp(t) - thing_stats_get_max_hp(t);
    if (delta > 0) {
        THING_LOG(t, "over max hp");

        thing_stats_modify_hp(t, -1);
    }
}
