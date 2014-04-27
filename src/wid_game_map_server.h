/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

extern boolean wid_game_map_server_init(void);
extern void wid_game_map_server_fini(void);
extern void wid_game_map_server_hide(void);
extern void wid_game_map_server_visible(void);
extern void wid_game_map_server_save(void);
extern void wid_game_map_server_load(void);

extern void wid_game_map_server_wid_create(void);
extern void wid_game_map_server_wid_destroy(void);
extern void wid_game_map_server_score_update(levelp);
extern void wid_game_map_server_item_update(levelp);

extern widp
wid_game_map_server_replace_tile(widp, double x, double y,
                                 thing_templatep thing_template);

extern widp wid_game_map_server_window;
extern widp wid_game_map_server_grid_container;
extern widp wid_game_map_server_selected;
extern widp wid_scoreline_container;
extern widp wid_score;
extern widp wid_level;
extern levelp server_level;

extern uint32_t server_tile_width;
extern uint32_t server_tile_height;
