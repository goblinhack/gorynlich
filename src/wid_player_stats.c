/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "color.h"
#include "wid_player_stats.h"
#include "wid_player_info.h"
#include "wid_player_inventory.h"
#include "wid_player_action.h"
#include "wid_choose_game_type.h"
#include "timer.h"
#include "string_util.h"
#include "client.h"
#include "thing.h"

static thing_statsp player_stats;

void wid_player_stats_redraw (int fast)
{
    if (!wid_player_info_is_visible()) {
        return;
    }

    if (!player_stats) {
        return;
    }

    player_inventory_sort(player_stats);

    wid_player_info_hide(fast);
    wid_player_info_visible(player_stats, fast);

    wid_player_inventory_hide(fast);
    wid_player_inventory_visible(player_stats, fast);

    wid_player_action_hide(fast, false /* player quit */);
    wid_player_action_visible(player_stats, fast);

    wid_raise(wid_mouse_template);
}

void wid_player_stats_visible (thing_statsp s)
{
    player_stats = s;
}

void wid_player_stats_hide (thing_statsp s)
{
}
