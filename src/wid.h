/*
 * Copyright(C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license.
 */

#pragma once

#include <SDL.h>

#include "sdl.h"
#include "map.h"

typedef enum {
    WID_COLOR_BG,
    WID_COLOR_TL,
    WID_COLOR_BR,
    WID_COLOR_TEXT,
    WID_COLOR_BLIT_OUTLINE,
    WID_COLOR_BLIT,
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
uint8_t wid_init(void);
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
uint8_t wid_has_grid(widp);
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
void wid_destroy_ptr_in(widp *w, uint32_t ms);
void wid_set_ignore_events(widp, uint8_t);
void wid_set_disable_scissors(widp w, uint8_t val);
void wid_set_debug(widp, uint8_t);
void wid_set_animate(widp, uint8_t);

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
void wid_move_all(void);
void wid_gc_all(void);
void wid_display_all(void);
uint8_t wid_ignore_for_events(widp);
uint8_t wid_ignore_for_focus(widp);
uint8_t wid_ignore_being_destroyed(widp);

/*
 * Visibility
 */
void wid_raise(widp);
void wid_lower(widp);
void wid_hide(widp, uint32_t delay);
void wid_visible(widp, uint32_t delay);
void wid_this_hide(widp, uint32_t delay);
void wid_this_visible(widp, uint32_t delay);
void wid_toggle_hidden(widp, uint32_t delay);
void wid_always_hidden(widp, uint8_t value);

/*
 * Mousy
 */
void wid_mouse_warp(widp w);
void wid_mouse_move(widp w);
void wid_mouse_hide(int);
extern int wid_mouse_visible;

/*
 * Location
 */
void wid_get_abs_coords(widp w,
                        int32_t *tlx,
                        int32_t *tly,
                        int32_t *brx,
                        int32_t *bry);

void wid_get_abs(widp w, int32_t *x, int32_t *y);
void wid_get_pct(widp w, double *x, double *y);

/*
 * Text input.
 */
void wid_scroll_text(widp);
void wid_scroll_with_input(widp, const char *str);
uint8_t wid_receive_input(widp, const SDL_KEYSYM *key);

/*
 * Events
 */
void wid_mouse_motion(int32_t x, int32_t y,
                      int32_t relx, int32_t rely,
                      int32_t wheelx, int32_t wheely);
void wid_joy_button(int32_t x, int32_t y);
void wid_fake_joy_button(int32_t x, int32_t y);
void wid_mouse_down(uint32_t button, int32_t x, int32_t y);
void wid_mouse_up(uint32_t button, int32_t x, int32_t y);
void wid_key_down(const struct SDL_KEYSYM *, int32_t x, int32_t y);
void wid_key_up(const struct SDL_KEYSYM *, int32_t x, int32_t y);

typedef void(*on_tooltip_t)(widp, widp);
typedef void(*on_mouse_focus_begin_t)(widp);
typedef void(*on_mouse_focus_end_t)(widp);
typedef void(*on_mouse_over_begin_t)(widp,
                                     int32_t relx, int32_t rely,
                                     int32_t wheelx, int32_t wheely);
typedef void(*on_mouse_over_end_t)(widp);
typedef uint8_t(*on_joy_button_t)(widp, int32_t x, int32_t y);
typedef uint8_t(*on_mouse_down_t)(widp, int32_t x, int32_t y, uint32_t button);
typedef uint8_t(*on_mouse_up_t)(widp, int32_t x, int32_t y, uint32_t button);
typedef uint8_t(*on_mouse_motion_t)(widp,
                                    int32_t x, int32_t y,
                                    int32_t relx, int32_t rely,
                                    int32_t wheelx, int32_t wheely);
typedef uint8_t(*on_key_down_t)(widp, const struct SDL_KEYSYM *);
typedef uint8_t(*on_key_up_t)(widp, const struct SDL_KEYSYM *);
typedef void(*on_destroy_t)(widp);
typedef void(*on_tick_t)(widp);
typedef void(*on_display_top_level_t)(widp);
typedef void(*on_display_t)(widp, fpoint tl, fpoint br);

typedef widp (*grid_wid_replace_t)(widp,
                                   double x,
                                   double y,
                                   thingp, 
                                   tpp, 
                                   tpp_data,
                                   itemp item,
                                   thing_statsp stats);

/*
 * Client.
 */
void wid_set_focus(widp);
void wid_focus_lock(widp);
uint8_t wid_get_bevelled(widp);
uint8_t wid_get_do_not_lower(widp);
uint8_t wid_get_do_not_raise(widp);
uint8_t wid_is_movable(widp);
uint8_t wid_is_movable_bounded(widp);
uint8_t wid_is_movable_horiz(widp);
uint8_t wid_is_movable_vert(widp);
uint8_t wid_get_mode_is_movable(widp);
uint8_t wid_get_mode_is_movable_bounded(widp);
uint8_t wid_get_mode_is_movable_horiz(widp);
uint8_t wid_get_mode_is_movable_vert(widp);
uint8_t wid_get_mode_movable(widp);
uint8_t wid_get_mode_movable_bounded(widp);
uint8_t wid_get_mode_movable_horiz(widp);
uint8_t wid_get_mode_movable_vert(widp);
uint8_t wid_get_mode_top(widp);
uint8_t wid_get_movable(widp);
uint8_t wid_get_movable_bounded(widp);
uint8_t wid_get_movable_horiz(widp);
uint8_t wid_get_movable_vert(widp);
uint8_t wid_get_received_input(widp);
uint8_t wid_get_rounded(widp);
uint8_t wid_get_show_cursor(widp);
uint8_t wid_get_square(widp);
uint8_t wid_get_square_outline(widp);
uint8_t wid_get_text_bot(widp);
uint8_t wid_get_text_centerx(widp);
uint8_t wid_get_text_centery(widp);
uint8_t wid_get_text_fixed_width(widp);
uint8_t wid_get_text_lhs(widp);
uint8_t wid_get_text_outline(widp);
uint8_t wid_get_blit_outline(widp);
uint8_t wid_get_blit_center(widp);
uint8_t wid_get_text_pos(widp, double *x, double *y);
uint8_t wid_get_text_rhs(widp);
uint8_t wid_get_text_top(widp);
color wid_get_color(widp, wid_color which);
color wid_get_mode_color(widp, wid_color which);
const char *wid_name(widp);
const char *wid_logname(widp);
const char *wid_get_text(widp);
const char *wid_get_name(widp);
const char *wid_get_tooltip(widp);
const char *wid_get_text_with_cursor(widp);
double wid_get_bevel(widp);
double wid_get_br_x(widp);
double wid_get_br_y(widp);
double wid_get_height(widp);
void wid_get_mxy(widp w, double *x, double *y);
double wid_get_mode_bevel(widp);
double wid_get_text_advance(widp);
double wid_get_text_scaling(widp);
double wid_get_tl_x(widp);
double wid_get_tl_y(widp);
double wid_get_cx(widp);
double wid_get_cy(widp);
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
tpp wid_get_thing_template(widp);
thingp wid_get_thing(widp);
uint32_t wid_get_cursor(widp);
uint32_t wid_get_gllist(widp);
uint32_t wid_get_mode_gllist(widp);
uint8_t wid_get_focusable(widp);
void *wid_get_client_context(widp);
void *wid_get_client_context2(widp);
void *wid_get_client_context3(widp);
void wid_set_client_context(widp w, void *client_context);
void wid_set_client_context2(widp w, void *client_context);
void wid_set_client_context3(widp w, void *client_context);
void wid_fade_in(widp, uint32_t delay);
void wid_fade_out(widp, uint32_t delay);
void wid_fade_in_out(widp w, uint32_t delay, uint32_t repeat, 
                     uint8_t fade_in_firs);
void wid_get_children_size(widp, double *width, double *height);
void wid_get_offset(widp, fpoint *offset);
void wid_get_tl_br(widp, fpoint *tl, fpoint *br);
void wid_set_tl_br_no_relative_offset(widp w, fpoint tl, fpoint br);
void wid_move_end(widp);
void wid_move_delta(widp, double dx, double dy);
void wid_move_to_abs(widp, double dx, double dy);
void wid_move_to_abs_centered(widp, double dx, double dy);
void wid_move_to_abs_centered_in(widp, double dx, double dy, uint32_t delay);
void wid_move_to_centered_in(widp, double dx, double dy, uint32_t delay);
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
void wid_move_to_vert_pct(widp w, double pct);
void wid_move_to_horiz_pct(widp w, double pct);
void wid_move_to_vert_pct_in(widp w, double pct, double in);
void wid_move_to_horiz_pct_in(widp w, double pct, double in);
void wid_get_move_interpolated_progress(widp w, double *dx, double *dy);

double wid_get_scaling_w(widp);
double wid_get_scaling_h(widp);
void wid_scale_immediate(widp, double val);
void wid_scale_w_immediate(widp, double val);
void wid_scale_h_immediate(widp, double val);
void wid_scaling_to_pct_in(widp, double start, double end, uint32_t ms, uint32_t repeat_count);
void wid_effect_pulses(widp);
void wid_effect_pulse_forever(widp);
void wid_effect_pulse_stop(widp);

double wid_get_blit_scaling_w(widp);
double wid_get_blit_scaling_h(widp);
void wid_blit_scale_immediate(widp, double val);
void wid_blit_scale_w_immediate(widp, double val);
void wid_blit_scale_h_immediate(widp, double val);
void wid_scaling_blit_to_pct_in(widp, double start, double end, uint32_t ms, uint32_t repeat_count);
void wid_blit_effect_pulses(widp);

double wid_get_rotate(widp);
void wid_rotate_to_pct_in(widp, double start, double end, uint32_t ms, uint32_t repeat_count);
void wid_rotate_immediate(widp, double val);

void wid_bounce_to_pct_in(widp, double height, double fade, uint32_t ms, uint32_t repeat_count);
double wid_get_bounce(widp);

void wid_shake_to_pct_in(widp, double height, double fade, uint32_t ms, uint32_t repeat_count);
void wid_get_shake(widp, double *x, double *y);

void wid_flip_horiz(widp, uint8_t);
void wid_flip_vert(widp, uint8_t);
uint8_t wid_get_flip_horiz(widp);
uint8_t wid_get_flip_vert(widp);
void wid_set_bevel(widp, double val);
void wid_set_bevelled(widp, uint8_t);
void wid_set_color(widp, wid_color col, color val);
void wid_set_cursor(widp, uint32_t val);
void wid_set_do_not_lower(widp, uint8_t val);
void wid_set_do_not_raise(widp, uint8_t val);
void wid_set_focusable(widp, uint8_t val);
void wid_set_font(widp, fontp val);
void wid_set_gllist(widp, uint32_t val);
void wid_set_mode(widp, wid_mode mode);
void wid_set_movable(widp, uint8_t val);
void wid_set_movable(widp, uint8_t val);
void wid_set_movable_bounded(widp, uint8_t val);
void wid_set_movable_bounded(widp, uint8_t val);
void wid_set_movable_horiz(widp, uint8_t val);
void wid_set_movable_horiz(widp, uint8_t val);
void wid_set_movable_vert(widp, uint8_t val);
void wid_set_movable_vert(widp, uint8_t val);
void wid_set_name(widp, const char *string);
void wid_set_offset(widp, fpoint offset);
void wid_set_on_tooltip(widp, on_tooltip_t fn);
void wid_set_on_key_down(widp, on_key_down_t fn);
void wid_set_on_key_up(widp, on_key_up_t fn);
void wid_set_on_joy_down(widp, on_joy_button_t fn);
void wid_set_on_mouse_down(widp, on_mouse_down_t fn);
void wid_set_on_mouse_motion(widp, on_mouse_motion_t fn);
void wid_set_on_mouse_focus_begin(widp, on_mouse_focus_begin_t fn);
void wid_set_on_mouse_focus_end(widp, on_mouse_focus_end_t fn);
void wid_set_on_mouse_over_begin(widp, on_mouse_over_begin_t fn);
void wid_set_on_mouse_over_end(widp, on_mouse_over_end_t fn);
void wid_set_on_mouse_up(widp, on_mouse_up_t fn);
void wid_set_on_destroy(widp, on_destroy_t fn);
void wid_set_on_destroy_begin(widp, on_destroy_t fn);
void wid_set_on_tick(widp, on_tick_t fn);
void wid_set_on_display(widp, on_display_t fn);
void wid_set_on_display_top_level(widp, on_display_top_level_t fn);
void wid_set_prev(widp w, widp);
void wid_set_radius(widp, fsize val);
void wid_set_received_input(widp, uint8_t val);
void wid_set_rounded(widp);
void wid_set_rounded_large(widp);
void wid_set_rounded_small(widp);
void wid_set_show_cursor(widp, uint8_t val);
void wid_set_sides(widp, int32_t val);
void wid_set_square(widp);
void wid_set_square_outline(widp);
void wid_set_no_shape(widp);
void wid_set_tex(widp, const char *file, const char *name);
void wid_set_tex_br(widp, fsize val);
void wid_set_tex_sz(widp, fsize uv);
void wid_set_tex_tl(widp, fsize val);
void wid_set_text(widp, const char *string);
void wid_set_tooltip(widp, const char *string, fontp);
void wid_set_text_advance(widp, double val);
void wid_set_text_bot(widp, uint8_t val);
void wid_set_text_centerx(widp, uint8_t val);
void wid_set_text_centery(widp, uint8_t val);
void wid_set_text_fixed_width(widp, uint8_t val);
void wid_set_text_lhs(widp, uint8_t val);
void wid_set_text_outline(widp, uint8_t val);
void wid_set_blit_outline(widp, double val);
void wid_set_blit_center(widp, uint8_t val);
void wid_set_text_pos(widp, uint8_t val, double x, double y);
void wid_set_text_rhs(widp, uint8_t val);
void wid_set_text_scaling(widp, double val);
void wid_set_text_top(widp, uint8_t val);
void wid_set_tilename(widp, const char *name);
void wid_set_tile2name(widp, const char *name);
void wid_set_tile(widp, tilep);
void wid_set_tile2(widp, tilep);
void wid_set_z_depth(widp, uint8_t);
uint8_t wid_get_z_depth(widp);
void wid_set_thing(widp, thingp);
void wid_set_thing_template(widp, tpp);
void wid_set_tl_br(widp, fpoint tl, fpoint br);
void wid_set_tl_br_pct(widp, fpoint tl, fpoint br);
void wid_set_top(widp, uint8_t val);
void wid_setx_tl_br_pct(widp, fpoint tl, fpoint br);
void wid_sety_tl_br_pct(widp, fpoint tl, fpoint br);
wid_mode wid_get_mode(widp);
widp wid_get_focus(widp);
widp wid_get_next(widp);
widp wid_get_prev(widp);
widp wid_get_current_focus(void);
void wid_effect_pulses(widp);
void wid_effect_sways(widp);
widp wid_find_matching(widp, fpoint tl, fpoint br, uint8_t z_depth);
widp wid_grid_find(widp, fpoint tl, fpoint br, uint8_t z_depth);
widp wid_grid_find_first(widp parent, uint32_t x, uint32_t y, uint8_t depth);
widp wid_grid_find_next(widp parent, widp w, uint32_t x, uint32_t y, uint8_t depth);
widp wid_grid_find_top(widp, fpoint tl, fpoint br);
widp wid_grid_find_thing_template(widp parent,
                                  uint32_t x,
                                  uint32_t y,
                                  tp_is_callback func);
widp wid_grid_find_tp_is(widp parent,
                                  uint32_t x,
                                  uint32_t y,
                                  tpp);
void marshal_wid_grid(marshal_p ctx, widp);
uint8_t demarshal_wid_grid(demarshal_p ctx, widp, grid_wid_replace_t);
uint8_t wid_is_hidden(widp w);
uint8_t wid_this_is_hidden(widp w);
uint8_t wid_is_scaling(widp w);
uint8_t wid_is_fading(widp w);
uint8_t wid_is_always_hidden(widp w);
void wid_get_grid_coord(widp w, int32_t *x, int32_t *y,
                        uint8_t *aligned_x,
                        uint8_t *aligned_y);
void wid_get_grid_dim(widp w, uint32_t *x, uint32_t *y);
uint8_t wids_overlap(widp A, widp B);
void wid_animate(widp);

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

typedef struct {
    /*
     * Widget display settings
     */
    fontp font;

    /*
     * Colors
     */
    color colors[WID_COLOR_MAX];
    uint8_t color_set [WID_COLOR_MAX];
} wid_cfg;

typedef struct tree_wid_key_ {
    tree_node node;

    /*
     * Higher number == most raised/frontmost wid
     *
     * Higher numbers are first in the event tree.
     * Higher numbers are drawn last and hence in the foreground.
     *
     * Lower numbers are last in the event tree.
     * Lower numbers are drawn first and hence in the background.
     */
    int32_t priority;
    uint8_t z_depth;

    /*
     * The real position on the screen initially.
     */
    fpoint tl;
    fpoint br;

    /*
     * Unique wid ID.
     */
    uint64_t key;
} tree_wid_key;

typedef struct widgridnode_ {
    tree_wid_key tree;
 
    widp wid;
    uint32_t x;
    uint32_t y;
    uint8_t aligned_x:1;
    uint8_t aligned_y:1;
} widgridnode;

typedef struct widgrid_ {
    tree_root **grid_of_trees[MAP_DEPTH_MAX];
    uint32_t nelems;
    uint32_t width;
    uint32_t height;
    uint32_t pixwidth;
    uint32_t pixheight;
    double tl_x;
    double tl_y;
    double br_x;
    double br_y;
    uint8_t bounds_valid:1;
    uint8_t bounds_locked:1;
} widgrid;

typedef struct wid_move_ {
    fpoint moving_end;
    uint32_t timestamp_moving_end;
} wid_move_t;

typedef struct wid_ {
    /*
     * Sorted for display order.
     */
    tree_wid_key tree;

    /*
     * A second tree, unsorted.
     */
    tree_key_int tree2_unsorted;

    /*
     * A tree for moving things
     */
    tree_key_int tree3_moving_wids;
    
    /*
     * A tree for things being destroyed.
     */
    tree_key_int tree4_wids_being_destroyed;

    /*
     * A tree for ticking things
     */
    tree_key_int tree5_ticking_wids;

    /*
     * Fast lookup for grid widgets. The parent has a grid pointer.
     */
    widgrid *grid;

    /*
     * The children have node pointers.
     */
    widgridnode *gridnode;

    /*
     * And a pointer back to the tree they are on.
     */
    tree_rootp gridtree;

    /*
     * Sorted for display onto the screen.
     */
    tree_root *children_display_sorted;

    /*
     * No particular sort order.
     */
    tree_root *tree2_children_unsorted;

    /*
     * Used a lot, so keep at the head of the struct for speed.
     */
    uint8_t hidden:1;
    uint8_t scaling_w:1;
    uint8_t scaling_h:1;

    /*
     * Flags.
     */
    uint8_t disable_scissors:1;
    uint8_t debug:1;
    uint8_t bevelled:1;
    uint8_t rounded:1;
    uint8_t square:1;
    uint8_t square_outline:1;
    uint8_t radius_set:1;
    uint8_t tex_tl_set:1;
    uint8_t tex_br_set:1;
    uint8_t always_hidden:1;
    uint8_t visible:1;
    uint8_t received_input:1;
    uint8_t movable:1;
    uint8_t movable_set:1;
    uint8_t movable_horiz:1;
    uint8_t movable_horiz_set:1;
    uint8_t movable_vert:1;
    uint8_t movable_vert_set:1;
    uint8_t movable_bounded:1;
    uint8_t movable_bounded_set:1;
    uint8_t fade_in:1;
    uint8_t fade_out:1;
    uint8_t ignore_for_events:1;
    uint8_t scaled_w:1;
    uint8_t scaled_h:1;
    uint8_t blit_scaled_w:1;
    uint8_t blit_scaled_h:1;
    uint8_t blit_scaling_w:1;
    uint8_t blit_scaling_h:1;
    uint8_t rotated:1;
    uint8_t rotating:1;
    uint8_t bouncing:1;
    uint8_t shaking:1;
    uint8_t shaking_set:1;
    uint8_t flip_vert:1;
    uint8_t flip_horiz:1;
    uint8_t first_update:1;
    uint8_t show_cursor:1;
    uint8_t text_pos_set:1;
    uint8_t text_fixed_width;
    uint8_t text_lhs:1;
    uint8_t text_rhs:1;
    uint8_t text_centerx:1;
    uint8_t text_top:1;
    uint8_t text_bot:1;
    uint8_t text_centery:1;
    uint8_t text_outline:1;
    uint8_t blit_outline:1;
    uint8_t blit_center:1;
    uint8_t being_destroyed:1;
    uint8_t do_not_raise:1;
    uint8_t do_not_lower:1;
    uint8_t can_be_atteched_now:1;
    uint8_t animate:1;

    /*
     * Optionally set to the previous wid in a list
     */
    widp prev;
    widp next;
    widp scrollbar_horiz;
    widp scrollbar_vert;
    widp scrollbar_owner;
    tree_rootp in_tree;
    tree_rootp in_tree2_unsorted;
    tree_rootp in_tree3_moving_wids;
    tree_rootp in_tree4_wids_being_destroyed;
    tree_rootp in_tree5_ticking_wids;

    /*
     * Thing related.
     */
    thingp thing;
    tilep tile_eyes;
    tpp tp;

    /*
     * The real position after scrollbar adjustments.
     */
    fpoint abs_tl;
    fpoint abs_br;

    /*
     * The wid tex and tex scaling
     */
    texp tex;
    tilep tile;

    /*
     * First tile found in an animation. Used as the centering tile when
     * scaling.
     */
    tilep first_tile;

    /*
     * For animation.
     */
    thing_tilep current_tile;

    fsize texuv;
    /*
     * The wid shape
     */
    double bevel;
    fsize radius;
    uint8_t sides;

    /*
     * Cache of text size as this is expensive to work out if there are colors
     * in the strings.
     */
    double ttf_width;
    double ttf_height;
    enum_fmt fmt;

    fsize tex_tl;
    fsize tex_br;

    /*
     * WID_MODE_NORMAL ...
     */
    wid_mode mode;

    /*
     * Offset of child widgets in the parent window.
     */
    fpoint offset;

    /*
     * Config layers:
     */
    wid_cfg cfg[WID_MODE_LAST];

    /*
     * Client context
     */
    void *client_context;
    void *client_context2;
    void *client_context3;

    /*
     * Text placement.
     */
    double text_scaling;
    double text_advance;
    fpoint text_pos;

    /*
     * Fade in/out/... effects.
     */
    uint32_t timestamp_fading_begin;
    uint32_t timestamp_fading_end;

    uint32_t timestamp_scaling_w_begin;
    uint32_t timestamp_scaling_w_end;
    uint32_t timestamp_scaling_h_begin;
    uint32_t timestamp_scaling_h_end;

    uint32_t timestamp_blit_scaling_w_begin;
    uint32_t timestamp_blit_scaling_w_end;
    uint32_t timestamp_blit_scaling_h_begin;
    uint32_t timestamp_blit_scaling_h_end;

    uint32_t timestamp_rotate_begin;
    uint32_t timestamp_rotate_end;

    uint32_t timestamp_bounce_begin;
    uint32_t timestamp_bounce_end;

    uint32_t timestamp_shake_begin;
    uint32_t timestamp_shake_end;

    uint32_t destroy_when;
    uint32_t timestamp_created;

    uint32_t timestamp_last_mode_change;

    /*
     * When to change frame for animation.
     */
    uint32_t timestamp_change_to_next_frame;

    widp *destroy_ptr;

    /*
     * Queue of wid move requests.
     */
#define WID_MAX_MOVE_QUEUE 4
    wid_move_t move[WID_MAX_MOVE_QUEUE];
    fpoint moving_start;
    fpoint moving_end;
    uint32_t timestamp_moving_begin;
    uint32_t timestamp_moving_end;
    uint8_t moving;

    double scale_w_base;
    double scaling_w_start;
    double scaling_w_end;
    double scale_h_base;
    double scaling_h_start;
    double scaling_h_end;
    uint16_t scaling_w_repeat_count;
    uint16_t scaling_h_repeat_count;

    double blit_outline_val;
    double blit_scale_w_base;
    double blit_scaling_w_start;
    double blit_scaling_w_end;
    double blit_scale_h_base;
    double blit_scaling_h_start;
    double blit_scaling_h_end;
    uint16_t blit_scaling_w_repeat_count;
    uint16_t blit_scaling_h_repeat_count;

    double rotate_base;
    double rotate_start;
    double rotate_end;

    /*
     * Percentage of wid height. 1.0; bounce own height.
     */
    double bounce_height;

    /*
     * Effect of gravity. 0.1; rapid decrease, 0.9 slow
     */
    double bounce_fade;

    /*
     * Percentage of wid size to shake. 1.0; shake own width/height.
     */
    double shake_amount;

    /*
     * How much teh shake fades each shake.
     */
    double shake_fade;

    /*
     * The amount we are currently shaking.
     */
    double shake_x;
    double shake_y;

    uint16_t rotate_sways_count;
    uint16_t bounce_count;
    uint16_t shake_count;
    uint16_t fade_count;
    uint16_t fade_delay;

    /*
     * Text input
     */
    uint16_t cursor;

    /*
     * Order of this wid amongst other focusable widgets.
     */
    uint8_t focus_order;

    /*
     * Who had it last ? Used when raising this wid again.
     */
    uint8_t focus_last;

    /*
     * The wids children
     */
    widp parent;

    /*
     * Widget internal name.
     */
    char *name;

    /*
     * For debugging.
     */
    char *logname;

    /*
     * Text that appears on the wid.
     */
    char text[MAXSTR];

    /*
     * Text that appears as a tooltip.
     */
    char *tooltip;
    fontp tooltip_font;

    /*
     * Action handlers
     */
    on_key_down_t on_key_down;
    on_tooltip_t on_tooltip;
    on_key_up_t on_key_up;
    on_joy_button_t on_joy_button;
    on_mouse_down_t on_mouse_down;
    on_mouse_up_t on_mouse_up;
    on_mouse_motion_t on_mouse_motion;
    on_mouse_focus_begin_t on_mouse_focus_begin;
    on_mouse_focus_end_t on_mouse_focus_end;
    on_mouse_over_begin_t on_mouse_over_begin;
    on_mouse_over_end_t on_mouse_over_end;
    on_destroy_t on_destroy;
    on_destroy_t on_destroy_begin;
    on_tick_t on_tick;
    on_display_t on_display;
    on_display_top_level_t on_display_top_level;

} wid;

static inline uint8_t wid_is_moving (widp w)
{
    fast_verify(w);

    if (w->moving) {
        return (true);
    }

    return (false);
}

static inline int8_t 
tree_wid_compare_func (const tree_node *a, const tree_node *b)
{
    widp A = (typeof(A))a;
    widp B = (typeof(B))b;

    if (A->tree.priority > B->tree.priority) {
        return (-1);
    }

    if (A->tree.priority < B->tree.priority) {
        return (1);
    }

    if (A->tree.z_depth > B->tree.z_depth) {
        return (-1);
    }

    if (A->tree.z_depth < B->tree.z_depth) {
        return (1);
    }

    if (A->tree.br.y > B->tree.br.y) {
        return (-1);
    }

    if (A->tree.br.y < B->tree.br.y) {
        return (1);
    }

    if (A->tree.key < B->tree.key) {
        return (-1);
    }

    if (A->tree.key > B->tree.key) {
        return (1);
    }

    return (0);
}

TREE_PREV_INLINE(tree_wid_compare_func) 

extern widp wid_mouse_template;

extern const int32_t wid_fade_delay;
extern const int32_t wid_destroy_delay_ms;
extern const int32_t wid_visible_delay;
extern const int32_t wid_hide_delay;
extern const int32_t wid_swipe_delay;
extern const int32_t wid_pulse_delay;
extern const int32_t wid_scaling_forever_delay;

