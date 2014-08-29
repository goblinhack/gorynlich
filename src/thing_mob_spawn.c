/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "thing.h"
#include "wid_game_map_server.h"
#include "map.h"

uint8_t thing_mob_spawn (thingp t)
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

        /*
         * Round down so mob spawners on an in between position will not
         * get mixed up. This can happen when we place a mob spawner where
         * a player died, overlapping a wall and we then try to spawn into
         * a cell we cannot move in.
         */
        x = floor(x);
        y = floor(y);

        /*
         * Things not to spawn onto.
         */
        if (map_is_wall_at(server_level, x, y)          ||
            map_is_monst_at(server_level, x, y)         ||
            map_is_pipe_at(server_level, x, y)          ||
            map_is_door_at(server_level, x, y)          ||
            map_is_player_at(server_level, x, y)        ||
            map_is_treasure_at(server_level, x, y)      ||
            map_is_food_at(server_level, x, y)          ||
            map_is_mob_spawner_at(server_level, x, y)   ||
            map_is_exit_at(server_level, x, y)) {
            continue;
        }

        wid_game_map_server_replace_tile(wid_game_map_server_grid_container,
                                         x,
                                         y,
                                         0, /* thing */
                                         what);
        return (true);
    }

    return (false);
}

uint8_t thing_mob_spawn_on_death (thingp t)
{
    const char *mob_spawn = thing_template_spawn_on_death(t->thing_template);
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
        uint32_t r = rand() % 200;

        if (r < 25) {
            x -= 1.0;
        } else if (r < 50) {
            x += 1.0;
        } else if (r < 75) {
            y -= 1.0;
        } else if (r < 100) {
            y += 1.0;
        } else {
            // centered overl player
        }

        /*
         * Round down so mob spawners on an in between position will not
         * get mixed up. This can happen when we place a mob spawner where
         * a player died, overlapping a wall and we then try to spawn into
         * a cell we cannot move in.
         */
        x = floor(x);
        y = floor(y);

        /*
         * Things not to spawn onto.
         */
        if (map_is_wall_at(server_level, x, y)          ||
            map_is_pipe_at(server_level, x, y)          ||
            map_is_door_at(server_level, x, y)          ||
            map_is_exit_at(server_level, x, y)) {
            continue;
        }

        wid_game_map_server_replace_tile(wid_game_map_server_grid_container,
                                         x,
                                         y,
                                         0, /* thing */
                                         what);
        return (true);
    }

    return (false);
}
