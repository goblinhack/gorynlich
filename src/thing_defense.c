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

int32_t thing_stats_get_total_damage_minus_defense (thingp t, 
                                                    thingp hitter, 
                                                    int32_t damage)
{
    double d = damage;
    double modifier = thing_stats_val_to_modifier(thing_stats_get_defense(t));

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
