/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
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

double thing_stats_get_total_speed (thingp t)
{
    double speed = tp_get_speed(t->tp);
    double modifier = thing_stats_val_to_modifier(thing_stats_get_speed(t));

    tpp helmet = thing_helmet(t);
    if (helmet) {
        modifier += tp_get_stats_speed(helmet);
    }

    tpp boots = thing_boots(t);
    if (boots) {
        modifier += tp_get_stats_speed(boots);
    }

    tpp armour = thing_armour(t);
    if (armour) {
        modifier += tp_get_stats_speed(armour);
    }

    tpp left_arm = thing_arm_left(t);
    if (left_arm) {
        modifier += tp_get_stats_speed(left_arm);
    }

    tpp right_arm = thing_arm_right(t);
    if (right_arm) {
        modifier += tp_get_stats_speed(right_arm);
    }

    /*
     * Modifier of 1 maps to +10 % in speed.
     */
    double final_speed = ceil(speed + (speed * (modifier / 10.0)));

    return (final_speed);
}
