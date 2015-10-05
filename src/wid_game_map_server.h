/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */

extern uint8_t wid_game_map_server_init(void);
extern void wid_game_map_server_fini(void);
extern void wid_game_map_server_hide(void);
extern void wid_game_map_server_visible(void);
extern void wid_game_map_server_save(void);
extern void wid_game_map_server_load(void);

extern void wid_game_map_server_wid_create(void);
extern void wid_game_map_server_wid_destroy(uint8_t keep_players);
extern void wid_game_map_server_score_update(levelp);
extern void wid_game_map_server_item_update(levelp);

extern widp
wid_game_map_server_replace_tile(widp, 
                                 double x, 
                                 double y,
                                 thingp, 
                                 tpp,
                                 tpp_data,
                                 itemp,
                                 thing_statsp );

extern widp wid_game_map_server_window;
extern widp wid_game_map_server_grid_container;
extern widp wid_game_map_server_selected;
extern widp wid_scoreline_container;
extern widp wid_score;
extern widp wid_level;

extern uint32_t server_tile_width;
extern uint32_t server_tile_height;
extern int server_level_is_being_loaded;
