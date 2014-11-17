/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

uint8_t wid_player_inventory_init(void);
void wid_player_inventory_fini(void);
void wid_player_inventory_hide(void);
void wid_player_inventory_visible(thing_statsp );
void wid_player_inventory_button_style(widp w, thing_statsp, const item_t,
                                       const int action_bar_item,
                                       const int worn_item,
                                       const int inventory_item,
                                       const int index);
uint8_t wid_player_item_pick_up(widp w, itemp over_item);

extern item_t wid_item;
