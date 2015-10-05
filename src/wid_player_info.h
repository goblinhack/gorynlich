/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */

uint8_t wid_player_info_init(void);
void wid_player_info_fini(void);
void wid_player_info_hide(int fast);
void wid_player_info_visible(thing_statsp, int fast);
int wid_player_info_is_visible(void);
extern int wid_player_info_set_name;
