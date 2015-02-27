/*
 * Copyright (C) 2011 Neil McGill
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
CON("redraw");
    if (!wid_player_info_is_visible()) {
CON("redraw fail");
        return;
    }

    if (!player_stats) {
CON("redraw fail2");
        return;
    }

    player_inventory_sort(player_stats);

    wid_player_info_hide(fast);
    wid_player_info_visible(player_stats, fast);

    wid_player_inventory_hide(fast);
    wid_player_inventory_visible(player_stats, fast);

    wid_player_action_hide(fast);
    wid_player_action_visible(player_stats, fast);

    wid_raise(wid_mouse_template);
}
