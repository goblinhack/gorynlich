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

void thing_magic_tick (thingp t)
{
    /*
     * Auto heal
     */
    if (thing_stats_get_magic(t) < thing_stats_get_max_magic(t)) {
        int delta = thing_stats_val_to_modifier(thing_stats_get_healing(t));
        if (delta > 0) {
            thing_stats_modify_magic(t, delta);
        }
    }

    /*
     * If over max magic, return to max.
     */
    int delta = thing_stats_get_magic(t) - thing_stats_get_max_magic(t);
    if (delta > 0) {
        thing_stats_modify_magic(t, -delta / 10);
    }
}
