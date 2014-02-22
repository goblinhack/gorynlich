/*
 * Copyright(C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include "sdl.h"

typedef enum {
    WID_COLOR_BG,
    WID_COLOR_TL,
    WID_COLOR_BR,
    WID_COLOR_TEXT,
    WID_COLOR_BLIT_OUTLINE,
    WID_COLOR_MAX,
} wid_color;

typedef enum {
    WID_MODE_NORMAL,
    WID_MODE_OVER,
    WID_MODE_FOCUS,
    WID_MODE_ACTIVE,
    WID_MODE_LAST,
} wid_mode;

#define WID_MODE_FIRST WID_MODE_NORMAL

/*
 * System
 */
boolean wid_init(void);
void wid_fini(void);

/*
 * Create
 */
widp wid_new_window(const char *name);
widp wid_new_container(widp, const char *name);
widp wid_new_plain(widp, const char *name);
widp wid_new_rounded_window(const char *name);
widp wid_new_square_window(const char *name);
widp wid_new_square_button(widp parent, const char *name);
widp wid_new_rounded_button(widp, const char *name);
widp wid_new_rounded_small_button(widp, const char *name);
widp wid_new_vert_scroll_bar(widp parent, widp scrollbar_owner);
widp wid_new_horiz_scroll_bar(widp parent, widp scrollbar_owner);
boolean wid_has_grid(widp);
void wid_new_grid(widp, uint32_t width, uint32_t height,
                  uint32_t pixwidth, uint32_t pixheight);
widp wid_new_tooltip(const char *tooltip);
void wid_destroy_grid(widp);
void wid_detach_from_grid(widp);
void wid_attach_to_grid(widp);
void wid_empty_grid(widp);
void wid_destroy(widp *);
void wid_destroy_nodelay(widp *);
void wid_destroy_in(widp w, uint32_t ms);

/*
 * Accessors.
 */
widp wid_get_top_parent(widp);
widp wid_get_parent(widp);
widp wid_get_scrollbar_vert(widp);
widp wid_get_scrollbar_horiz(widp);
tree_rootp wid_get_children(widp);
widp wid_find(widp, const char *name);

/*
 * Update
 */
void wid_update(widp);
void wid_update_mouse(void);
void wid_tick_all(void);
void wid_display_all(void);
boolean wid_ignore_for_events(widp);
boolean wid_ignore_for_focus(widp);
boolean wid_ignore_being_destroyed(widp);

/*
 * Visibility
 */
void wid_raise(widp);
void wid_lower(widp);
void wid_hide(widp, uint32_t delay);
void wid_visible(widp, uint32_t delay);
void wid_toggle_hidden(widp, uint32_t delay);

/*
 * Location
 */
void wid_get_abs_coords(widp w,
                        int32_t *tlx,
                        int32_t *tly,
                        int32_t *brx,
                        int32_t *bry);

/*
 * Text input.
 */
void wid_scroll_text(widp);
void wid_scroll_with_input(widp, const char *str);
boolean wid_receive_input(widp, const SDL_KEYSYM *key);

/*
 * Events
 */
void wid_mouse_motion(int32_t x, int32_t y,
                      int32_t relx, int32_t rely,
                      int32_t wheelx, int32_t wheely);
void wid_mouse_down(uint32_t button, int32_t x, int32_t y);
void wid_mouse_up(uint32_t button, int32_t x, int32_t y);
void wid_key_down(const struct SDL_KEYSYM *, int32_t x, int32_t y);
void wid_key_up(const struct SDL_KEYSYM *, int32_t x, int32_t y);

typedef void(*on_mouse_focus_begin_t)(widp);
typedef void(*on_mouse_focus_end_t)(widp);
typedef void(*on_mouse_over_begin_t)(widp);
typedef void(*on_mouse_over_end_t)(widp);
typedef boolean(*on_mouse_down_t)(widp, int32_t x, int32_t y, uint32_t button);
typedef boolean(*on_mouse_up_t)(widp, int32_t x, int32_t y, uint32_t button);
typedef boolean(*on_mouse_motion_t)(widp,
                                    int32_t x, int32_t y,
                                    int32_t relx, int32_t rely,
                                    int32_t wheelx, int32_t wheely);
typedef boolean(*on_key_down_t)(widp, const struct SDL_KEYSYM *);
typedef boolean(*on_key_up_t)(widp, const struct SDL_KEYSYM *);
typedef void(*on_destroy_t)(widp);
typedef void(*on_tick_t)(widp);

typedef widp (*grid_wid_replace_t)(widp,
                                   int32_t x,
                                   int32_t y,
                                   thing_templatep thing_template);

/*
 * Client.
 */
void wid_set_focus(widp);
void wid_focus_lock(widp);
boolean wid_get_bevelled(widp);
boolean wid_get_do_not_lower(widp);
boolean wid_get_do_not_raise(widp);
boolean wid_is_movable(widp);
boolean wid_is_movable_bounded(widp);
boolean wid_is_movable_horiz(widp);
boolean wid_is_movable_vert(widp);
boolean wid_get_mode_is_movable(widp);
boolean wid_get_mode_is_movable_bounded(widp);
boolean wid_get_mode_is_movable_horiz(widp);
boolean wid_get_mode_is_movable_vert(widp);
boolean wid_get_mode_movable(widp);
boolean wid_get_mode_movable_bounded(widp);
boolean wid_get_mode_movable_horiz(widp);
boolean wid_get_mode_movable_vert(widp);
boolean wid_get_mode_top(widp);
boolean wid_get_movable(widp);
boolean wid_get_movable_bounded(widp);
boolean wid_get_movable_horiz(widp);
boolean wid_get_movable_vert(widp);
boolean wid_get_received_input(widp);
boolean wid_get_rounded(widp);
boolean wid_get_show_cursor(widp);
boolean wid_get_square(widp);
boolean wid_get_square_outline(widp);
boolean wid_get_text_bot(widp);
boolean wid_get_text_centerx(widp);
boolean wid_get_text_centery(widp);
boolean wid_get_text_fixed_width(widp);
boolean wid_get_text_lhs(widp);
boolean wid_get_text_outline(widp);
boolean wid_get_blit_outline(widp);
boolean wid_get_text_pos(widp, double *x, double *y);
boolean wid_get_text_rhs(widp);
boolean wid_get_text_top(widp);
color wid_get_color(widp, wid_color which);
color wid_get_mode_color(widp, wid_color which);
const char *wid_name(widp);
const char *wid_logname(widp);
const char *wid_get_text(widp);
const char *wid_get_tooltip(widp);
const char *wid_get_text_with_cursor(widp);
double wid_get_bevel(widp);
double wid_get_br_x(widp);
double wid_get_br_y(widp);
double wid_get_height(widp);
void wid_get_mxy(widp w, double *x, double *y);
double wid_get_mode_bevel(widp);
double wid_get_scaling_w(widp);
double wid_get_scaling_h(widp);
double wid_get_rotate(widp);
double wid_get_text_advance(widp);
double wid_get_text_scaling(widp);
double wid_get_tl_x(widp);
double wid_get_tl_y(widp);
double wid_get_width(widp);
fontp wid_get_font(widp);
fontp wid_get_mode_font(widp);
fsize wid_get_mode_radius(widp);
fsize wid_get_radius(widp);
fsize wid_get_tex_br(widp);
fsize wid_get_tex_tl(widp);
int32_t wid_get_mode_sides(widp);
int32_t wid_get_sides(widp);
texp wid_get_tex(widp, fsize *size);
tilep wid_get_tile(widp);
tilep wid_get_tile2(widp);
thingp wid_get_thing(widp);
thing_templatep wid_get_thing_template(widp);
thingp wid_get_thing(widp);
uint32_t wid_get_cursor(widp);
uint32_t wid_get_gllist(widp);
uint32_t wid_get_mode_gllist(widp);
uint8_t wid_get_focusable(widp);
void *wid_get_client_context(widp);
void wid_fade_in(widp, uint32_t delay);
void wid_fade_out(widp, uint32_t delay);
void wid_fade_in_out(widp w, uint32_t delay, uint32_t repeat, 
                     boolean fade_in_firs);
void wid_get_children_size(widp, double *width, double *height);
void wid_get_offset(widp, fpoint *offset);
void wid_get_tl_br(widp, fpoint *tl, fpoint *br);
void wid_move_end(widp);
void wid_move_delta(widp, double dx, double dy);
void wid_move_to_abs(widp, double dx, double dy);
void wid_move_to_abs_centered(widp, double dx, double dy);
void wid_move_to_abs_centered_in(widp, double dx, double dy, uint32_t delay);
void wid_move_delta_pct_in(widp, double dx, double dy, uint32_t delay);
void wid_move_to_abs_in(widp, double dx, double dy, uint32_t delay);
void wid_move_to_bottom(widp);
void wid_move_to_left(widp);
void wid_move_to_pct(widp, double dx, double dy);
void wid_move_to_pct_centered(widp, double dx, double dy);
void wid_move_to_pct_centered_in(widp, double dx, double dy, uint32_t delay);
void wid_move_to_pct_in(widp, double dx, double dy, uint32_t delay);
void wid_move_to_right(widp);
void wid_move_to_top(widp);
void wid_scale_immediate(widp, double val);
void wid_scale_w_immediate(widp, double val);
void wid_scale_h_immediate(widp, double val);
void wid_scaling_to_pct_in(widp, double start, double end, uint32_t ms, uint32_t bounce_count);
void wid_rotate_to_pct_in(widp, double start, double end, uint32_t ms, uint32_t bounce_count);
void wid_rotate_immediate(widp, double val);
void wid_flip_horiz(widp, boolean);
void wid_flip_vert(widp, boolean);
boolean wid_get_flip_horiz(widp);
boolean wid_get_flip_vert(widp);
void wid_set_bevel(widp, double val);
void wid_set_bevelled(widp, boolean);
void wid_set_client_context(widp w, void *client_context);
void wid_set_color(widp, wid_color col, color val);
void wid_set_cursor(widp, uint32_t val);
void wid_set_do_not_lower(widp, boolean val);
void wid_set_do_not_raise(widp, boolean val);
void wid_set_focusable(widp, uint8_t val);
void wid_set_font(widp, fontp val);
void wid_set_gllist(widp, uint32_t val);
void wid_set_mode(widp, wid_mode mode);
void wid_set_movable(widp, boolean val);
void wid_set_movable(widp, boolean val);
void wid_set_movable_bounded(widp, boolean val);
void wid_set_movable_bounded(widp, boolean val);
void wid_set_movable_horiz(widp, boolean val);
void wid_set_movable_horiz(widp, boolean val);
void wid_set_movable_vert(widp, boolean val);
void wid_set_movable_vert(widp, boolean val);
void wid_set_name(widp, const char *string);
void wid_set_offset(widp, fpoint offset);
void wid_set_on_key_down(widp, on_key_down_t fn);
void wid_set_on_key_up(widp, on_key_up_t fn);
void wid_set_on_mouse_down(widp, on_mouse_down_t fn);
void wid_set_on_mouse_motion(widp, on_mouse_motion_t fn);
void wid_set_on_mouse_focus_begin(widp, on_mouse_focus_begin_t fn);
void wid_set_on_mouse_focus_end(widp, on_mouse_focus_end_t fn);
void wid_set_on_mouse_over_begin(widp, on_mouse_over_begin_t fn);
void wid_set_on_mouse_over_end(widp, on_mouse_over_end_t fn);
void wid_set_on_mouse_up(widp, on_mouse_up_t fn);
void wid_set_on_destroy(widp, on_destroy_t fn);
void wid_set_on_tick(widp, on_tick_t fn);
void wid_set_prev(widp w, widp);
void wid_set_radius(widp, fsize val);
void wid_set_received_input(widp, boolean val);
void wid_set_rounded(widp);
void wid_set_rounded_large(widp);
void wid_set_rounded_small(widp);
void wid_set_show_cursor(widp, boolean val);
void wid_set_sides(widp, int32_t val);
void wid_set_square(widp);
void wid_set_square_outline(widp);
void wid_set_no_shape(widp);
void wid_set_tex(widp, const char *file, const char *name);
void wid_set_tex_br(widp, fsize val);
void wid_set_tex_sz(widp, fsize uv);
void wid_set_tex_tl(widp, fsize val);
void wid_set_text(widp, const char *string);
void wid_set_tooltip(widp, const char *string);
void wid_set_text_advance(widp, double val);
void wid_set_text_bot(widp, boolean val);
void wid_set_text_centerx(widp, boolean val);
void wid_set_text_centery(widp, boolean val);
void wid_set_text_fixed_width(widp, boolean val);
void wid_set_text_lhs(widp, boolean val);
void wid_set_text_outline(widp, boolean val);
void wid_set_blit_outline(widp, boolean val);
void wid_set_text_pos(widp, boolean val, double x, double y);
void wid_set_text_rhs(widp, boolean val);
void wid_set_text_scaling(widp, double val);
void wid_set_text_top(widp, boolean val);
void wid_set_tilename(widp, const char *name);
void wid_set_tile2name(widp, const char *name);
void wid_set_tile(widp, tilep);
void wid_set_tile2(widp, tilep);
void wid_set_z_depth(widp, uint8_t);
uint8_t wid_get_z_depth(widp);
void wid_set_z_order(widp, uint8_t);
uint8_t wid_get_z_order(widp);
void wid_set_thing(widp, thingp);
void wid_set_thing_template(widp, thing_templatep);
void wid_set_tl_br(widp, fpoint tl, fpoint br);
void wid_set_tl_br_pct(widp, fpoint tl, fpoint br);
void wid_set_top(widp, boolean val);
void wid_setx_tl_br_pct(widp, fpoint tl, fpoint br);
void wid_sety_tl_br_pct(widp, fpoint tl, fpoint br);
wid_mode wid_get_mode(widp);
widp wid_get_focus(widp);
widp wid_get_next(widp);
widp wid_get_prev(widp);
widp wid_get_current_focus(void);
void wid_effect_pulses(widp);
void wid_effect_sways(widp);
widp wid_find_matching(widp, fpoint tl, fpoint br, uint8_t z_depth,
                       uint8_t z_order);
widp wid_find_matching_top(widp, fpoint tl, fpoint br);
widp wid_grid_find(widp, fpoint tl, fpoint br, uint8_t z_depth,
                   uint8_t z_order);
widp wid_grid_find_first(widp parent, uint32_t x, uint32_t y);
widp wid_grid_find_next(widp parent, widp w, uint32_t x, uint32_t y);
widp wid_grid_find_top(widp, fpoint tl, fpoint br);
widp wid_grid_find_thing_template(widp parent,
                                  uint32_t x,
                                  uint32_t y,
                                  thing_template_is_callback func);
widp wid_grid_find_thing_template_is(widp parent,
                                  uint32_t x,
                                  uint32_t y,
                                  thing_templatep);
boolean wid_remove_from_grid(widp);
void marshal_wid_grid(marshal_p ctx, widp);
boolean demarshal_wid_grid(demarshal_p ctx, widp, grid_wid_replace_t);
boolean wid_is_moving(widp w);
void wid_move_stop(widp w);
void wid_move_resume(widp w);
boolean wid_is_hidden(widp w);
boolean wid_is_scaling(widp w);
boolean wid_is_fading(widp w);
void wid_get_grid_coord(widp w, int32_t *x, int32_t *y,
                        boolean *aligned_x,
                        boolean *aligned_y);
void wid_get_grid_dim(widp w, uint32_t *x, uint32_t *y);
boolean wids_overlap(widp A, widp B);

extern const int32_t wid_fade_delay;
extern const int32_t wid_hide_delay;
extern const int32_t wid_visible_delay;
extern const int32_t wid_swipe_delay;

/*
 * History for all text widgets.
 */
#define HISTORY_MAX 16
extern char history[HISTORY_MAX][MAXSTR];
extern uint32_t history_at;
extern uint32_t history_walk;

extern tree_rootp wid_timers;
