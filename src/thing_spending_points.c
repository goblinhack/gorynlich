/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */


#include "color.h"
#include "thing.h"

void thing_modify_spending_points (thingp t, int val)
{
    thing_stats_modify_spending_points(t, val);

    if (val > 0) {
        MSG_SERVER_SHOUT_AT(POPUP, t, 0, 0,
                            "%%%%font=%s$%%%%fg=%s$+%d spending points",
                            "vlarge", "green", val);

        MSG_SERVER_SHOUT_AT(INFO, t,  0, 0, "%%%%fg=green$Press s to spend points");
    }
}
