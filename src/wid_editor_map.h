/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

void wid_editor_map_wid_create(void);
void wid_editor_map_wid_destroy (void);

extern widp wid_editor_map_window;
extern widp wid_editor_map_grid_container;
extern uint8_t wid_editor_got_line_start;

widp wid_editor_map_thing_replace_template(widp,
                                           double x, double y,
                                           thing_templatep);

void wid_editor_map_thing_flood_fill_template(int32_t x, int32_t y,
                                              thing_templatep);
void wid_game_set_count(widp, uint32_t count);

void wid_editor_add_grid(void);
