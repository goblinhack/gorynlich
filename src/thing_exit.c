/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include "main.h"
#include "thing.h"
#include "thing_timer.h"
#include "wid_game_map_client.h"
#include "wid_game_map_server.h"
#include "map.h"
#include "level.h"
#include "time_util.h"
#include "string_util.h"
#include "sound.h"
#include "socket_util.h"
#include "client.h"
#include "timer.h"
#include "tile.h"
#include "wid_hiscore.h"
#include "wid_player_info.h"
#include "wid_player_inventory.h"
#include "wid_player_action.h"
#include "wid_choose_stats.h"

void thing_reached_exit (thingp t, thingp exit)
{
    verify(t);

    /*
     * First to the exit?
     */
    if (!level_exit_reached_when_open(server_level)) {
        level_set_exit_reached_when_open(server_level, true);

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
        global_config.server_level_pos.x++;
        if (global_config.server_level_pos.x >= LEVELS_ACROSS) {
            global_config.server_level_pos.x = 1;
            global_config.server_level_pos.y++;
        }

        THING_LOG(t, "exit to next consecutive level %d.%d",
                  global_config.server_level_pos.y,
                  global_config.server_level_pos.x);
    }

    thing_leave_level(t);

    thing_modify_xp(t, 1000);

    sound_play_level_end();
}
