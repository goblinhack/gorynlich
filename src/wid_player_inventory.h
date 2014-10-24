/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

uint8_t wid_player_inventory_init(void);
void wid_player_inventory_fini(void);
void wid_player_inventory_hide(void);
void wid_player_inventory_visible(player_stats_t *);
void wid_player_inventory_button_style(widp w, player_stats_t *, const item_t);
