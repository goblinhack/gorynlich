/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "tree.h"
#include "thing.h"
#include "thing_timer.h"
#include "timer.h"
#include "wid.h"
#include "wid_game_map_client.h"
#include "wid_game_map_server.h"
#include "marshal.h"
#include "map.h"
#include "level.h"
#include "wid_console.h"
#include "time.h"
#include "string.h"
#include "color.h"
#include "sound.h"
#include "socket.h"
#include "math.h"

boolean thing_mob_spawn (thingp t)
{
    const char *mob_spawn = thing_template_mob_spawn(t->thing_template);
    if (!mob_spawn) {
        return (false);
    }

    thing_templatep what = thing_template_find(mob_spawn);
    if (!what) {
        return (false);
    }

    uint32_t tries = 10;

    while (tries-- > 0) {
        double x = t->x;
        double y = t->y;
        uint32_t r = rand() % 100;

        if (r < 25) {
            x -= 1.0;
        } else if (r < 50) {
            x += 1.0;
        } else if (r < 75) {
            y -= 1.0;
        } else {
            y += 1.0;
        }

        if (map_is_wall_at(server_level, x, y)) {
            continue;
        }

        if (map_is_monst_at(server_level, x, y)) {
            continue;
        }

        if (map_is_pipe_at(server_level, x, y)) {
            continue;
        }

        if (map_is_player_at(server_level, x, y)) {
            continue;
        }

        if (map_is_exit_at(server_level, x, y)) {
            continue;
        }

        if (!map_is_floor_at(server_level, x, y)) {
            continue;
        }

        wid_game_map_server_replace_tile(wid_game_map_server_grid_container,
                                         x,
                                         y,
                                         what);
        return (true);
    }

    return (false);
}
