/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */


#include "main.h"
#include "thing.h"
#include "timer.h"
#include "level.h"

uint8_t thing_jesus_spawn (void)
{
    if (level_is_jesus_summoned(server_level)) {
        MSG_SERVER_SHOUT_AT_ALL_PLAYERS(POPUP, 
                                        0, 0,
                                        "Jesus is among you!");
        return (false);
    }

    MSG_SERVER_SHOUT_AT_ALL_PLAYERS(POPUP, 
                                    0, 0,
                                    "Big Jesus is coming!!!");

    tpp what = id_to_tp(THING_JESUS);
    if (!what) {
        ERR("cannot spawn jesus");
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

        thing_place_context_t *context;

        context = (typeof(context)) myzalloc(sizeof(*context), "place thing");

        context->x = x;
        context->y = y;
        context->level = server_level;
        context->destroy_in = 60000;
        context->tp = what;
        context->on_server = 1;

        action_timer_create(
                &server_timers,
                (action_timer_callback)
                    thing_timer_place_and_destroy_callback,
                (action_timer_callback)
                    thing_timer_place_and_destroy_destroy_callback,
                context,
                "place and destroy thing",
                0,
                0 /* jitter */);

        level_set_is_jesus_summoned(server_level, true);

        return (true);
    }

    return (false);
}
