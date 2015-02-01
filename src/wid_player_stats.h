/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

uint8_t wid_thing_stats_init(void);
void wid_player_stats_fini(void);
void wid_player_stats_hide(int fast);
void wid_player_stats_visible(thing_statsp, int fast);
void wid_player_stats_redraw(int fast);
extern widp wid_player_stats;
