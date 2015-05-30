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

uint8_t thing_mob_spawn (thingp t)
{
    const char *mob_spawn = tp_mob_spawn(t->tp);
    if (!mob_spawn) {
        return (false);
    }

    tpp what = tp_find(mob_spawn);
    if (!what) {
        ERR("cannot spawn %s", mob_spawn);
        return (false);
    }

    uint32_t tries = 10;

    while (tries-- > 0) {
        double x = t->x;
        double y = t->y;
        uint32_t r = myrand() % 100;

        if (r < 12) {
            x -= 1.0;
        } else if (r < 50) {
            x += 1.0;
        } else if (r < 75) {
            y -= 1.0;
        } else {
            y += 1.0;
        }

        /*
         * Protect against mobs at the edge!
         */
        if ((x < 0) || (x >= MAP_WIDTH)) {
            continue;
        }

        if ((y < 0) || (y >= MAP_HEIGHT)) {
            continue;
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
        if (thing_is_lava(t)) {
            if (map_is_wall_at(server_level, x, y)   ||
                map_is_lava_at(server_level, x, y)   ||
                map_is_exit_at(server_level, x, y)) {

                uint32_t r = myrand() % 100;

                if (r < 12) {
                    x -= 1.0;
                } else if (r < 50) {
                    x += 1.0;
                } else if (r < 75) {
                    y -= 1.0;
                } else {
                    y += 1.0;
                }

                if (map_is_wall_at(server_level, x, y)   ||
                    map_is_lava_at(server_level, x, y)   ||
                    map_is_exit_at(server_level, x, y)) {
                    continue;
                }
            }
        } else {
            if (map_is_wall_at(server_level, x, y)          ||
                map_is_monst_at(server_level, x, y)         ||
                map_is_cobweb_at(server_level, x, y)        ||
                map_is_pipe_at(server_level, x, y)          ||
                map_is_door_at(server_level, x, y)          ||
                map_is_player_at(server_level, x, y)        ||
                map_is_treasure_at(server_level, x, y)      ||
                map_is_potion_at(server_level, x, y)        ||
                map_is_food_at(server_level, x, y)          ||
                map_is_mob_spawner_at(server_level, x, y)   ||
                map_is_exit_at(server_level, x, y)) {

                uint32_t r = myrand() % 100;

                if (r < 12) {
                    x -= 1.0;
                } else if (r < 50) {
                    x += 1.0;
                } else if (r < 75) {
                    y -= 1.0;
                } else {
                    y += 1.0;
                }

                if (map_is_wall_at(server_level, x, y)          ||
                    map_is_monst_at(server_level, x, y)         ||
                    map_is_cobweb_at(server_level, x, y)        ||
                    map_is_pipe_at(server_level, x, y)          ||
                    map_is_door_at(server_level, x, y)          ||
                    map_is_player_at(server_level, x, y)        ||
                    map_is_treasure_at(server_level, x, y)      ||
                    map_is_potion_at(server_level, x, y)        ||
                    map_is_food_at(server_level, x, y)          ||
                    map_is_mob_spawner_at(server_level, x, y)   ||
                    map_is_exit_at(server_level, x, y)) {

                    continue;
                }
            }
        }

        if (x <= 0) {
            continue;
        }
        if (y <= 0) {
            continue;
        }
        if (x >= MAP_WIDTH-1) {
            continue;
        }
        if (y >= MAP_HEIGHT-1) {
            continue;
        }

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

thingp thing_mob_spawn_on_death (thingp t)
{
    const char *mob_spawn = tp_spawn_on_death(t->tp);
    if (!mob_spawn) {
        THING_ERR(t, "nothing to spawn on death");
        return (0);
    }

    tpp what = tp_find(mob_spawn);
    if (!what) {
        THING_ERR(t, "cannot spawn %s on death", mob_spawn);
        return (0);
    }

    uint32_t tries = 20;
    uint8_t first = 1;

    while (tries-- > 0) {
        double x = t->x;
        double y = t->y;
        uint32_t r = myrand() % 100;

        if (r < 25) {
            x -= 1.0;
        } else if (r < 50) {
            x += 1.0;
        } else if (r < 75) {
            y -= 1.0;
        } else if (r < 100) {
            y += 1.0;
        } else {
            // centered over player
        }

        /*
         * Protect against mobs at the edge!
         */
        if ((x < 0) || (x >= MAP_WIDTH)) {
            continue;
        }

        if ((y < 0) || (y >= MAP_HEIGHT)) {
            continue;
        }

        if (first) {
            x = t->x;
            y = t->y;
            first = 0;
        }

        /*
         * Round down so mob spawners on an in between position will not
         * get mixed up. This can happen when we place a mob spawner where
         * a player died, overlapping a wall and we then try to spawn into
         * a cell we cannot move in.
         */
        x = floor(x);
        y = floor(y);

        if (tp_is_cobweb(what)) {
            /*
             * Don't want too many cobwebs in the same place.
             */
            if (map_is_cobweb_at(server_level, x, y)) {
                continue;
            }
        } else {
            /*
             * Things not to spawn onto.
             */
            if (map_is_wall_at(server_level, x, y)          ||
                map_is_pipe_at(server_level, x, y)          ||
                map_is_door_at(server_level, x, y)          ||
                map_is_exit_at(server_level, x, y)) {
                continue;
            }
        }

        widp w = wid_game_map_server_replace_tile(
                                         wid_game_map_server_grid_container,
                                         x,
                                         y,
                                         0, /* thing */
                                         what,
                                         0, /* tpp data */
                                         0 /* item */,
                                         0 /* stats */);

        THING_LOG(t, "place %s on death", mob_spawn);

        return (wid_get_thing(w));
    }

    return (0);
}
