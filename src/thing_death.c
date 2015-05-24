/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>
#include <math.h>

#include "main.h"
#include "thing.h"
#include "wid_game_map_server.h"
#include "map.h"
#include "level.h"
#include "time_util.h"

uint8_t thing_death_spawn (void)
{
    tpp what = id_to_tp(THING_DEATH);
    if (!what) {
        DIE("cannot spawn death");
        return (false);
    }

    uint32_t tries = 1000;

    while (tries-- > 0) {
        double x;
        double y;

        x = myrand() % MAP_WIDTH;
        y = myrand() % MAP_HEIGHT;

        switch (myrand() % 4) {
        case 0: x = 0; break;
        case 1: x = MAP_WIDTH - 1; break;
        case 2: y = 0; break;
        case 3: y = MAP_HEIGHT - 1; break;
        }

        /*
         * Round down so mob spawners on an in between position will not
         * get mixed up. This can happen when we place a mob spawner where
         * a player died, overlapping a wall and we then try to spawn into
         * a cell we cannot move in.
         */
        x = floor(x);
        y = floor(y);

        wid_game_map_server_replace_tile(wid_game_map_server_grid_container,
                                         x,
                                         y,
                                         0, /* thing */
                                         what,
                                         0, /* tpp data */
                                         0 /* item */,
                                         0 /* stats */);
        return (true);
    }

    return (false);
}

void thing_death_tick (void)
{
    if (!server_level) {
        return;
    }

    if (!level_death_is_coming_soon(server_level)) {
        if (time_have_x_tenths_passed_since(10 * 300,
                                            level_get_timestamp_started(server_level))) {
            MSG_SERVER_SHOUT_AT_ALL_PLAYERS(POPUP, "Death is coming soon...");

            level_set_death_is_coming_soon(server_level, true);
        }
    }

    if (!level_death_is_coming(server_level)) {
        if (time_have_x_tenths_passed_since((10 * 300) + 30,
                                            level_get_timestamp_started(server_level))) {
            MSG_SERVER_SHOUT_AT_ALL_PLAYERS(POPUP, "Death is coming...");

            thing_death_spawn();

            level_set_death_is_coming(server_level, true);
        }
    }
}