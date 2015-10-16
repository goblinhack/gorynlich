/*
* Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#include "main.h"

#include "socket_util.h"
#include "client.h"
#include "time_util.h"
#include "slre.h"
#include "command.h"
#include "string_util.h"
#include "wid_server_join.h"
#include "wid_game_map_client.h"
#include "wid_player_info.h"
#include "wid_player_inventory.h"
#include "wid_player_action.h"
#include "wid_chat.h"
#include "wid.h"
#include "thing.h"
#include "mzip_lib.h"
#include "wid_dead.h"
#include "level.h"
#include "name.h"
#include "music.h"
#include "map.h"
#include "music.h"

void client_level_ending (void)
{
    LEVEL_LOG(client_level, "Hide level");

    wid_hide(wid_game_map_client_grid_container, wid_hide_delay);

    if (client_level->is_test_level) {
        LEVEL_LOG(client_level,
                    "Test level finished, back to editor");

        wid_game_map_go_back_to_editor();
        return;
    }
}

void client_level_starting (void)
{
    LEVEL_LOG(client_level, "Reveal level");

    wid_visible(wid_game_map_client_grid_container, wid_visible_delay);

    wid_game_map_client_scroll_adjust(1);

    map_fixup(client_level);

    music_play_game(client_level->level_pos);
}

void client_player_fully_dead (msg_server_hiscores *latest_hiscores)
{
    LEVEL_LOG(client_level, "Player dead");

    if (client_level->is_test_level) {
        LEVEL_LOG(client_level,
                    "Test level finished, back to editor");

        wid_game_map_go_back_to_editor();
        return;
    }

    wid_dead_visible(latest_hiscores->players[0].player_name,
                     latest_hiscores->players[0].death_reason,
                     latest_hiscores->rejoin_allowed);
}
