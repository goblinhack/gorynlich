/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

boolean wid_game_init(void);
void wid_game_fini(void);
void wid_game_hide(void);
void wid_game_visible(void);
void wid_game_save(void);
void wid_game_load(void);

void wid_game_map_wid_create(void);
void wid_game_map_wid_destroy(void);
void wid_game_map_score_update(levelp);
void wid_game_map_item_update(levelp);
extern widp
wid_game_map_replace_tile(widp, int32_t x, int32_t y, uint32_t count,
                          thing_templatep thing_template);

extern widp wid_game_map_window;
extern widp wid_game_map_grid_container;
extern widp wid_game_map_selected;
extern widp wid_scoreline_container;
extern widp wid_score;
extern widp wid_level;
extern levelp level_game;

extern uint32_t tile_width;
extern uint32_t tile_height;
