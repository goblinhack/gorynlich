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
#include "time_util.h"

void thing_modify_xp (thingp t, int val)
{
    int old_xp = thing_stats_get_xp(t);

    thing_stats_modify_xp(t, val);

    int xp = thing_stats_get_xp(t);

    int level;
    int spending_points_boost = 0;

    for (level = 1; level < 100; level++) {
        if ((xp > level * 1000) && (old_xp <= level * 1000)) {
            spending_points_boost++;
        }
    }

    if (spending_points_boost) {
        thing_modify_spending_points(t, spending_points_boost);
    }
}
