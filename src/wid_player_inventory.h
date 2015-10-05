/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */

uint8_t wid_player_inventory_init(void);
void wid_player_inventory_fini(void);
void wid_player_inventory_hide(int fast);
void wid_player_inventory_visible(thing_statsp, int fast);
void wid_player_inventory_button_style(widp w, thing_statsp, const item_t,
                                       const int action_bar_item,
                                       const int worn_item,
                                       const int inventory_item,
                                       const int index);
uint8_t wid_player_item_pick_up(widp w, itemp over_item);
int wid_player_inventory_is_visible(void);

extern item_t wid_item;
