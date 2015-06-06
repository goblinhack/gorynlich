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
#include "math_util.h"

void thing_reached_teleport (thingp t, thingp teleport)
{
    int tx[MAP_WIDTH*MAP_HEIGHT];
    int ty[MAP_WIDTH*MAP_HEIGHT];
    int poss = 0;

    int x, y;

    int delay = 20;

    if (!thing_is_player(t)) {
        delay = 100;

        /*
         * Only move other things sometimes.
         */
        if ((myrand() % 100) < 90) {
            return;
        }
    }

    if (!time_have_x_tenths_passed_since(delay, 
                                         t->timestamp_last_teleport)) {
        return;
    }

    if (!time_have_x_tenths_passed_since(delay, 
                                         teleport->timestamp_last_teleport)) {
        return;
    }

    for (x = 0; x < MAP_WIDTH; x++) {
        for (y = 0; y < MAP_HEIGHT; y++) {

            level_map_tile *tile = 
                &server_level->map_grid.tile[x][y][MAP_DEPTH_EXPLOSION_TOP];

            tpp it = tile->tp;
            if (!it) {
                continue;
            }

            if (!tp_is_teleport(it)) {
                continue;
            }

            if (DISTANCE(t->x, t->y, x, y) < 2.0) {
                continue;
            }

            const char *tcol = teleport->data->col_name;
            const char *col = tile->data.col_name;

            if (!col || !tcol) {
                tx[poss] = x;
                ty[poss] = y;
                poss++;
                continue;
            }

            if (!strcmp(col, tcol)) {
                tx[poss] = x;
                ty[poss] = y;
                poss++;
                continue;
            }
        }
    }

    if (!poss) {
        return;
    }

    teleport->timestamp_last_teleport = 
    t->timestamp_last_teleport = time_get_time_ms();

    poss = myrand() % poss;

    double nx = tx[poss];
    double ny = ty[poss];

    wid_move_end(t->wid);

    thing_server_wid_update(t, nx, ny, true /* is_new */);
    thing_update(t);

    thing_handle_collisions(wid_game_map_server_grid_container, t);

    sound_play_level_end();
}
