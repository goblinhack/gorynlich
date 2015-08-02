/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */

#include "main.h"
#include "thing.h"
#include "level.h"
#include "sound.h"

void thing_reached_exit (thingp t, thingp exit)
{
    /*
     * First to the exit?
     */
    if (!level_exit_reached_when_open(server_level)) {
        level_set_exit_reached_when_open(server_level, true);

        if (server_level->game_over) {
            return;
        }

        thing_set_got_to_exit_first(t, true);

        THING_LOG(t, "reached exit first");
    } else {
        THING_LOG(t, "reached exit");
    }

    if (exit->data && exit->data->exit_set) {
        global_config.server_level_pos.y = exit->data->exit.y;
        global_config.server_level_pos.x = exit->data->exit.x;

        THING_LOG(t, "exit jump to level %d.%d",
                  global_config.server_level_pos.y,
                  global_config.server_level_pos.x);
    } else {
        /*
         * Jump a few levels at at time.
         */
        global_config.server_level_pos.x += (myrand() % LEVEL_RANDOM_JUMP) + 1;

        if (global_config.server_level_pos.x >= LEVELS_ACROSS) {
            if (global_config.server_level_pos.y == LEVELS_DOWN) {
                /*
                 * Game end level
                 */
                global_config.server_level_pos.x = LEVELS_ACROSS;
            } else {
                /*
                 * Next block of levels.
                 */
                global_config.server_level_pos.x = 1;
                global_config.server_level_pos.y++;
            }
        }

        THING_LOG(t, "exit to next consecutive level %d.%d",
                  global_config.server_level_pos.y,
                  global_config.server_level_pos.x);
    }

    thing_leave_level(t);

    thing_modify_xp(t, 1000);

    sound_play_level_end();
}
