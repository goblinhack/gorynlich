/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */

uint8_t wid_game_map_client_init(void);
void wid_game_map_client_fini(void);
void wid_game_map_client_hide(void);
void wid_game_map_client_visible(void);
void wid_game_map_client_save(void);
void wid_game_map_client_load(void);

void wid_game_map_client_wid_create(void);
void wid_game_map_client_wid_destroy(void);
void wid_game_map_client_score_update(levelp, uint8_t redo);
void wid_game_map_client_scroll_adjust(uint8_t adjust);
void wid_game_map_go_back_to_editor(void);
uint8_t wid_game_map_client_player_move(void);

extern widp
wid_game_map_client_replace_tile(widp, double x, double y, thingp t, tpp);

extern widp wid_game_map_client_window;
extern widp wid_game_map_client_grid_container;
extern widp wid_game_map_client_selected;
extern widp wid_scoreline_container;
extern widp wid_score;
extern widp wid_level;

extern uint32_t client_tile_width;
extern uint32_t client_tile_height;

extern double last_playery;
extern double last_playerx;

extern uint32_t player_action_bar_changed_at;
