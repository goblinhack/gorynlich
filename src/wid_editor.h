/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

boolean wid_editor_init(void);
void wid_editor_fini(void);
void wid_editor_hide(void);
void wid_editor_visible(void);
void wid_editor_save(void);
void wid_editor_title(void);
void wid_editor_load(void);
void wid_editor_draw(void);
void wid_editor_erase(void);
void wid_editor_fill(void);
void wid_editor_clear(void);
void wid_editor_inc(void);
void wid_editor_dec(void);
void wid_editor_reset_buttons(void);

extern boolean wid_editor_mode_draw;
extern boolean wid_editor_mode_fill;
extern boolean wid_editor_mode_eraser;
extern boolean wid_editor_mode_inc;
extern boolean wid_editor_mode_dec;

extern widp wid_editor_wid_draw;
extern widp wid_editor_wid_fill;
extern widp wid_editor_wid_eraser;
extern widp wid_editor_wid_inc;
extern widp wid_editor_wid_dec;
extern void wid_editor_marshal(marshal_p ctx);
extern boolean wid_editor_demarshal(demarshal_p ctx);
extern levelp level_ed;

extern widp wid_editor_clear_popup;
extern widp wid_editor_help_popup;
extern widp wid_editor_save_popup;
extern widp wid_editor_title_popup;
extern widp wid_editor_load_popup;
extern widp wid_editor_filename_and_title;

