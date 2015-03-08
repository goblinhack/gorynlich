/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

uint8_t wid_editor_init(void);
void wid_editor_fini(void);
void wid_editor_hide(void);
void wid_editor_visible(level_pos_t);
void wid_editor_save(void);
void wid_editor_title(void);
void wid_editor_load_dialog(void);
void wid_editor_draw(void);
void wid_editor_line(void);
void wid_editor_erase(void);
void wid_editor_fill(void);
void wid_editor_undo(void);
void wid_editor_redo(void);
void wid_editor_clear(void);
void wid_editor_reset_buttons(void);
void wid_editor_save_point(void);
void wid_editor_undo_save_point(void);
void wid_editor_redo_save_point(void);

extern uint8_t wid_editor_mode_draw;
extern uint8_t wid_editor_mode_line;
extern uint8_t wid_editor_mode_fill;
extern uint8_t wid_editor_mode_eraser;

extern widp wid_editor_wid_draw;
extern widp wid_editor_wid_line;
extern widp wid_editor_wid_fill;
extern widp wid_editor_wid_eraser;
extern widp wid_editor_wid_undo;
extern widp wid_editor_wid_redo;
extern void wid_editor_marshal(marshal_p ctx);
extern uint8_t wid_editor_demarshal(demarshal_p ctx);
extern levelp level_ed;

extern widp wid_editor_clear_popup;
extern widp wid_editor_help_popup;
extern widp wid_editor_save_popup;
extern widp wid_editor_title_popup;
extern widp wid_editor_load_popup;
extern widp wid_editor_filename_and_title;

extern uint8_t server_level_is_being_loaded;
extern void wid_editor_load(char *dir_and_file);
