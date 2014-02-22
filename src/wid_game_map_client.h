/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

boolean wid_game_map_client_init(void);
void wid_game_map_client_fini(void);
void wid_game_map_client_hide(void);
void wid_game_map_client_visible(void);
void wid_game_map_client_save(void);
void wid_game_map_client_load(void);

void wid_game_map_client_wid_create(void);
void wid_game_map_client_wid_destroy(void);
void wid_game_map_client_score_update(levelp);
void wid_game_map_client_item_update(levelp);
extern widp
wid_game_map_client_replace_tile(widp, int32_t x, int32_t y, thingp t);

extern widp wid_game_map_client_window;
extern widp wid_game_map_client_grid_container;
extern widp wid_game_map_client_selected;
extern widp wid_scoreline_container;
extern widp wid_score;
extern widp wid_level;
extern levelp client_level;

extern uint32_t client_tile_width;
extern uint32_t client_tile_height;
