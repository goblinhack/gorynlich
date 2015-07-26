/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "thing.h"
#include "thing_timer.h"
#include "timer.h"
#include "wid.h"
#include "wid_game_map_client.h"
#include "wid_game_map_server.h"
#include "level.h"
#include "time_util.h"
#include "socket_util.h"

/*
 * If something changed in the player that we need to update the server, do so 
 * now.
 */
void thing_torch_update_count (thingp t, int force)
{
    if (force || time_have_x_secs_passed_since(60, t->timestamp_torch)) {
        t->timestamp_torch = time_get_time_ms();

        /*
         * Use up one torch unit.
         */
        tpp tp = thing_is_carrying_thing(t, tp_is_torch);
        if (tp) {
            thing_used(t, tp);
            tp = thing_is_carrying_thing(t, tp_is_torch);
            if (!tp) {
                MSG_SERVER_SHOUT_AT(INFO, t, 0, 0,
                                           "Your light fizzles out");
            }
        }
    }
}

void thing_torch_tick (thingp t)
{
    /*
     * Work out the torch light radius. Each torch lights 0.5 radius units.
     */
    float torch_light_radius = 
        (double) thing_is_carrying_thing_count(t, tp_is_torch) / 2.0;

    if (torch_light_radius > tp_get_light_radius(t->tp)) {
        torch_light_radius = tp_get_light_radius(t->tp);
    }

    torch_light_radius = thing_stats_get_total_vision(t, torch_light_radius);

    if (torch_light_radius != t->torch_light_radius) {
        t->torch_light_radius = torch_light_radius;
        t->torch_light_radius_set = true;
        thing_update(t);
    }
}
