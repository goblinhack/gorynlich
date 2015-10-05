/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */


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
#include "wid_game_map_server.h"

double thing_stats_get_total_vision (thingp t, double vision)
{
    double modifier = thing_stats_val_to_modifier(thing_stats_get_vision(t));
    double final_vision = ceil(vision + (vision * (modifier / 10.0)));

    /*
     * Allow dark vision once modifiers are high enough.
     */
    if (modifier > 0) {
        if (final_vision <= modifier / 2.0) {
            final_vision = modifier / 2.0;
        }
    }

    if (final_vision < 0) {
        final_vision = 0;
    }

    return (final_vision);
}

void level_place_light (levelp level, double x, double y)
{
    int ix = (int)x;
    int iy = (int)y;

    if (level->map_grid.lit[ix][iy]) {
        return;
    }

    level->map_grid.lit[ix][iy] = 1;

    (void) wid_game_map_server_replace_tile(wid_game_map_server_grid_container, x, y,
                                              0, /* thing */
                                              id_to_tp(THING_LIGHT),
                                              0 /* tpp_data */,
                                              0 /* item */,
                                              0 /* stats */);
}
