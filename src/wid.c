/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <SDL.h>
#include "slre.h"

#include "main.h"
#include "gl.h"
#include "wid.h"
#include "color.h"
#include "ttf.h"
#include "tex.h"
#include "tile.h"
#include "thing.h"
#include "thing_tile.h"
#include "command.h"
#include "time.h"
#include "wid_console.h"
#include "wid_tooltip.h"
#include "marshal.h"
#include "string.h"
#include "sound.h"
#include "timer.h"

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
    uint8_t z_order;

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
    tree_root **trees;
    uint32_t nelems;
    uint32_t width;
    uint32_t height;
    uint32_t pixwidth;
    uint32_t pixheight;
} widgrid;

typedef struct wid_ {
    /*
     * Sorted for display order.
     */
    tree_wid_key tree;

    /*
     * A second tree, unsorted.
     */
    tree_key_int tree2;

    /*
     * A tree for moving things
     */
    tree_key_int tree3;
    
    /*
     * A tree for things being destroyed.
     */
    tree_key_int tree4;

    /*
     * A tree for ticking things
     */
    tree_key_int tree5;

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
    int16_t ttf_width;
    int16_t ttf_height;
    enum_fmt fmt;

    /*
     * The wid tex and tex scaling
     */
    texp tex;
    tilep tile;
    tilep tile2;
    thingp thing;
    thing_templatep thing_template;
    fsize texuv;
    fsize tex_tl;
    fsize tex_br;

    /*
     * WID_MODE_NORMAL ...
     */
    wid_mode mode;
    int32_t timestamp_last_mode_change;

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
     * The real position after scrollbar adjustments.
     */
    fpoint abs_tl;
    fpoint abs_br;

    /*
     * Offset of child widgets in the parent window.
     */
    fpoint offset;

    /*
     * Config layers:
     */
    wid_cfg cfg[WID_MODE_LAST];

    /*
     * The wids children
     */
    widp parent;

    /*
     * Sorted for display onto the screen.
     */
    tree_root *children_display_sorted;

    /*
     * No particular sort order.
     */
    tree_root *children_unsorted;

    /*
     * Optionally set to the previous wid in a list
     */
    widp prev;
    widp next;
    widp scrollbar_horiz;
    widp scrollbar_vert;
    widp scrollbar_owner;

    /*
     * Client context
     */
    void *client_context;

    /*
     * Text input
     */
    uint32_t cursor;

    /*
     * Text placement.
     */
    double text_scaling;
    double text_advance;
    fpoint text_pos;

    /*
     * Action handlers
     */
    on_key_down_t on_key_down;
    on_key_up_t on_key_up;
    on_mouse_down_t on_mouse_down;
    on_mouse_up_t on_mouse_up;
    on_mouse_motion_t on_mouse_motion;
    on_mouse_focus_begin_t on_mouse_focus_begin;
    on_mouse_focus_end_t on_mouse_focus_end;
    on_mouse_over_begin_t on_mouse_over_begin;
    on_mouse_over_end_t on_mouse_over_end;
    on_destroy_t on_destroy;
    on_tick_t on_tick;

    /*
     * Fade in/out/... effects.
     */
    uint32_t timestamp_fading_begin;
    uint32_t timestamp_fading_end;
    uint32_t timestamp_moving_begin;
    uint32_t timestamp_moving_end;
    uint32_t timestamp_scaling_w_begin;
    uint32_t timestamp_scaling_w_end;
    uint32_t timestamp_scaling_h_begin;
    uint32_t timestamp_scaling_h_end;
    uint32_t timestamp_rotate_begin;
    uint32_t timestamp_rotate_end;
    uint32_t destroy_when;
    fpoint moving_start;
    fpoint moving_end;
    double scale_w_base;
    double scaling_w_start;
    double scaling_w_end;
    double scale_h_base;
    double scaling_h_start;
    double scaling_h_end;
    double rotate_base;
    double rotate_start;
    double rotate_end;
    uint32_t scaling_w_bounce_count;
    uint32_t scaling_h_bounce_count;
    uint32_t rotate_sways_count;
    uint32_t fade_count;
    uint32_t fade_delay;

    /*
     * Order of this wid amongst other focusable widgets.
     */
    uint8_t focus_order;

    /*
     * Who had it last ? Used when raising this wid again.
     */
    uint8_t focus_last;

    /*
     * Flags.
     */
    uint8_t bevelled:1;
    uint8_t rounded:1;
    uint8_t square:1;
    uint8_t square_outline:1;
    uint8_t radius_set:1;
    uint8_t tex_tl_set:1;
    uint8_t tex_br_set:1;
    uint8_t hidden:1;
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
    uint8_t moving:1;
    uint8_t ignore_for_events:1;
    uint8_t paused:1;
    uint8_t scaled_w:1;
    uint8_t scaled_h:1;
    uint8_t scaling_w:1;
    uint8_t scaling_h:1;
    uint8_t rotated:1;
    uint8_t rotating:1;
    uint8_t flip_vert:1;
    uint8_t flip_horiz:1;
    uint8_t first_update:1;
    uint8_t show_cursor:1;
    uint8_t text_size_cached:1;
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
    uint8_t being_destroyed:1;
    uint8_t do_not_raise:1;
    uint8_t do_not_lower:1;
    uint8_t in_tree3:1;
    uint8_t in_tree4:1;
    uint8_t in_tree5:1;
} wid;

/*
 * Display sorted.
 */
static tree_root *wid_top_level;

/*
 * Creation sorted.
 */
static tree_root *wid_top_level2;

/*
 * For moving things.
 */
static tree_root *wid_top_level3;

/*
 * For garbage collection.
 */
static tree_root *wid_top_level4;

/*
 * For ticking things.
 */
static tree_root *wid_top_level5;

/*
 * Mouse movement
 */
static widp wid_popup_tooltip;

/*
 * Scope the focus to children of this widget and do not change it.
 * Good for popups.
 */
static widp wid_focus_locked;

static widp wid_focus;
static widp wid_over;

/*
 * Widget moving
 */
static widp wid_moving;
static int32_t wid_moving_last_x;
static int32_t wid_moving_last_y;

/*
 * Widget effects
 */
const int32_t wid_fade_delay = 200;
const int32_t wid_destroy_delay_ms = 300;
const int32_t wid_visible_delay = 100;
const int32_t wid_hide_delay = 200;
const int32_t wid_swipe_delay = 200;
const int32_t wid_pulse_delay = 300;

/*
 * Prototypes.
 */
static boolean wid_scroll_trough_mouse_down(widp w, int32_t x, int32_t y,
                                            uint32_t button);
static boolean wid_scroll_trough_mouse_motion(widp w,
                                              int32_t x, int32_t y,
                                              int32_t relx, int32_t rely,
                                              int32_t wheelx, int32_t wheely);
static double wid_get_fade_amount(widp w);
static void wid_find_first_focus(void);
static void wid_find_top_focus(void);
static void wid_destroy_immediate(widp w);
static void wid_destroy_immediate_internal(widp w);
static void wid_grid_tree_attach(widp w);
static boolean wid_grid_tree_detach(widp w);
static void wid_update_internal(widp w);
static void wid_tree_detach(widp w);
static void wid_tree_attach(widp w);
static void wid_tree_remove(widp w);
static void wid_tree2_remove(widp w);
static void wid_tree3_remove(widp w);
static void wid_tree3_insert(widp w);
static void wid_tree4_remove(widp w);
static void wid_tree4_insert(widp w);
static void wid_tree4_remove(widp w);
static void wid_tree4_insert(widp w);
static void wid_tree5_remove(widp w);
static void wid_tree5_insert(widp w);

/*
 * Child sort priority
 */
static int32_t wid_highest_priority = 1;
static int32_t wid_lowest_priority = -1;

/*
 * History for all text widgets.
 */
#define HISTORY_MAX 16
char history[HISTORY_MAX][MAXSTR];
uint32_t history_at;
uint32_t history_walk;

static boolean wid_init_done;
static boolean wid_exiting;

tree_rootp wid_timers;

boolean wid_init (void)
{
    wid_init_done = true;

    return (true);
}

void wid_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (wid_init_done) {
        wid_init_done = false;
        wid_exiting = true;

        tree_destroy(&wid_top_level5, (tree_destroy_func)0);
        tree_destroy(&wid_top_level4, (tree_destroy_func)0);
        tree_destroy(&wid_top_level3, (tree_destroy_func)0);
        tree_destroy(&wid_top_level2, (tree_destroy_func)0);
        tree_destroy(&wid_top_level,
                     (tree_destroy_func)wid_destroy_immediate_internal);

        action_timers_destroy(&wid_timers);
    }
}

static void wid_grid_tree_attach (widp w)
{
    widgrid *grid;

    if (!w->parent) {
        return;
    }

    grid = w->parent->grid;
    if (!grid) {
        return;
    }

    fast_verify(w);

    int32_t tlx;
    int32_t tly;
    int32_t brx;
    int32_t bry;

    tlx = w->tree.tl.x - w->parent->tree.tl.x;
    tly = w->tree.tl.y - w->parent->tree.tl.y;
    brx = w->tree.br.x - w->parent->tree.tl.x;
    bry = w->tree.br.y - w->parent->tree.tl.y;

    /*
     * Get the midpoint of the tile and find out which grid cell it
     * resides on.
     */
    uint32_t mx = ((tlx + brx) / 2);
    uint32_t my = ((tly + bry) / 2);
    uint32_t x = mx / grid->pixwidth;
    uint32_t y = my / grid->pixheight;

    if (x >= grid->width) {
        return;
    }

    if (y >= grid->height) {
        return;
    }

    tree_root **newtree = grid->trees + (y * grid->width) + x;

    /*
     * Still on the same tree? i.e. no real move?
     */
    if (w->gridtree == *newtree) {
        return;
    }

    /*
     * If not on the grid yet, allocate a node.
     */
    if (!w->gridnode) {
        w->gridnode = (widgridnode*) myzalloc(sizeof(widgridnode),
                                              "TREE NODE: gridnode");
        /*
         * Use the same key as the wid.
         */
        w->gridnode->tree.priority = w->tree.priority;
        w->gridnode->tree.br = w->tree.br;
        w->gridnode->tree.z_depth = w->tree.z_depth;
        w->gridnode->tree.z_order = w->tree.z_order;
        w->gridnode->tree.key = w->tree.key;

        /*
         * Back pointer to the widget for lookups.
         */
        w->gridnode->wid = w;
    } else {
        /*
         * Take us out of the old tree in preparation for the new tree.
         */
        if (!tree_remove(w->gridtree, &w->gridnode->tree.node)) {
            DIE("wid remove from grid");
        }
    }

    w->gridtree = *newtree;

    /*
     * Now add to the new tree.
     */
    if (!tree_insert(w->gridtree, &w->gridnode->tree.node)) {
        DIE("wid insert to grid");
    }

    w->gridnode->x = x;
    w->gridnode->y = y;
    w->gridnode->aligned_x = !((mx + (grid->pixwidth/2)) % grid->pixwidth);
    w->gridnode->aligned_y = !((my + (grid->pixheight/2)) % grid->pixheight);
}

static boolean wid_grid_tree_detach (widp w)
{
    fast_verify(w);

    /*
     * On the grid ?
     */
    if (!w->gridnode) {
        return (false);
    }

    if (!tree_remove(w->gridtree, &w->gridnode->tree.node)) {
        DIE("wid remove from grid");
    }

    myfree(w->gridnode);

    w->gridnode = 0;
    w->gridtree = 0;

    return (true);
}

void wid_get_grid_coord (widp w, int32_t *x, int32_t *y,
                         boolean *aligned_x,
                         boolean *aligned_y)
{
    if (!w->gridnode) {
        if (x) {
            *x = -1;
        }

        if (y) {
            *y = -1;
        }

        if (aligned_x) {
            *aligned_x = false;
        }

        if (aligned_y) {
            *aligned_y = false;
        }

        return;
    }

    if (x) {
        *x = w->gridnode->x;
    }

    if (y) {
        *y = w->gridnode->y;
    }

    if (aligned_x) {
        *aligned_x = w->gridnode->aligned_x;
    }

    if (aligned_y) {
        *aligned_y = w->gridnode->aligned_y;
    }
}

void wid_get_grid_dim (widp w, uint32_t *x, uint32_t *y)
{
    if (!w->grid) {
        DIE("no parent grid");
        return;
    }

    *x = w->grid->pixwidth * w->grid->width;
    *y = w->grid->pixheight * w->grid->height;
}

double wid_get_cx (widp w)
{
    double cx = (w->tree.tl.x + w->tree.br.x) / 2.0;

    return (cx);
}

double wid_get_cy (widp w)
{
    double cy = (w->tree.tl.y + w->tree.br.y) / 2.0;

    return (cy);
}

double wid_get_tl_x (widp w)
{
    double cx = (w->tree.tl.x + w->tree.br.x) / 2.0;

    return (cx - ((cx - w->tree.tl.x) * wid_get_scaling_w(w)));
}

double wid_get_tl_y (widp w)
{
    double cy = (w->tree.tl.y + w->tree.br.y) / 2.0;

    return (cy - ((cy - w->tree.tl.y) * wid_get_scaling_h(w)));
}

double wid_get_br_x (widp w)
{
    double cx = (w->tree.tl.x + w->tree.br.x) / 2.0;

    return (cx + ((w->tree.br.x - cx) * wid_get_scaling_w(w)));
}

double wid_get_br_y (widp w)
{
    double cy = (w->tree.tl.y + w->tree.br.y) / 2.0;

    return (cy + ((w->tree.br.y - cy) * wid_get_scaling_h(w)));
}

static inline void wid_get_tl_x_tl_y_br_x_br_y (widp w, 
                                                double *tl_x,
                                                double *tl_y,
                                                double *br_x,
                                                double *br_y)
{
    const double cx = (w->tree.tl.x + w->tree.br.x) / 2.0;
    const double cy = (w->tree.tl.y + w->tree.br.y) / 2.0;
    const double scaling = wid_get_scaling_w(w);

    *tl_x = cx - ((cx - w->tree.tl.x) * scaling);
    *tl_y = cy - ((cy - w->tree.tl.y) * scaling);
    *br_x = cx + ((w->tree.br.x - cx) * scaling);
    *br_y = cy + ((w->tree.br.y - cy) * scaling);
}

/*
 * Set the wid new co-ords. Returns true if there is a change.
 */
void wid_set_tl_br (widp w, fpoint tl, fpoint br)
{
    fast_verify(w);

    widp p;

    wid_tree_detach(w);

    w->tree.tl = tl;
    w->tree.br = br;

    /*
     * Child postion is relative from the parent.
     */
    p = w->parent;
    if (p) {
        w->tree.tl.x += wid_get_tl_x(p);
        w->tree.tl.y += wid_get_tl_y(p);
        w->tree.br.x += wid_get_tl_x(p);
        w->tree.br.y += wid_get_tl_y(p);
    }

    wid_tree_attach(w);
}

/*
 * Set the wid new co-ords. Returns true if there is a change.
 */
void wid_set_tl_br_pct (widp w, fpoint tl, fpoint br)
{
    fast_verify(w);

    widp p;

    wid_tree_detach(w);

    if (!w->parent) {
        tl.x *= (double)global_config.video_gl_width;
        tl.y *= (double)global_config.video_gl_height;
        br.x *= (double)global_config.video_gl_width;
        br.y *= (double)global_config.video_gl_height;
    } else {
        tl.x *= wid_get_width(w->parent);
        tl.y *= wid_get_height(w->parent);
        br.x *= wid_get_width(w->parent);
        br.y *= wid_get_height(w->parent);
    }

    w->tree.tl = tl;
    w->tree.br = br;

    /*
     * Child postion is relative from the parent.
     */
    p = w->parent;
    if (p) {
        w->tree.tl.x += wid_get_tl_x(p);
        w->tree.tl.y += wid_get_tl_y(p);
        w->tree.br.x += wid_get_tl_x(p);
        w->tree.br.y += wid_get_tl_y(p);
    }

    wid_tree_attach(w);
}

/*
 * Set the wid new co-ords. Returns true if there is a change.
 */
void wid_setx_tl_br_pct (widp w, fpoint tl, fpoint br)
{
    fast_verify(w);

    widp p;

    wid_tree_detach(w);

    if (!w->parent) {
        tl.x *= (double)global_config.video_gl_width;
        br.x *= (double)global_config.video_gl_width;
    } else {
        tl.x *= wid_get_width(w->parent);
        br.x *= wid_get_width(w->parent);
    }

    w->tree.tl.x = tl.x;
    w->tree.br.x = br.x;

    /*
     * Child postion is relative from the parent.
     */
    p = w->parent;
    if (p) {
        w->tree.tl.x += wid_get_tl_x(p);
        w->tree.br.x += wid_get_tl_x(p);
    }

    wid_tree_attach(w);
}

/*
 * Set the wid new co-ords. Returns true if there is a change.
 */
void wid_sety_tl_br_pct (widp w, fpoint tl, fpoint br)
{
    fast_verify(w);

    widp p;

    wid_tree_detach(w);

    if (!w->parent) {
        tl.y *= (double)global_config.video_gl_height;
        br.y *= (double)global_config.video_gl_height;
    } else {
        tl.y *= wid_get_height(w->parent);
        br.y *= wid_get_height(w->parent);
    }

    w->tree.tl.y = tl.y;
    w->tree.br.y = br.y;

    /*
     * Child postion is relative from the parent.
     */
    p = w->parent;
    if (p) {
        w->tree.tl.y += wid_get_tl_y(p);
        w->tree.br.y += wid_get_tl_y(p);
    }

    wid_tree_attach(w);
}

void wid_get_tl_br (widp w, fpoint *tl, fpoint *br)
{
    fast_verify(w);

    *tl = w->tree.tl;
    *br = w->tree.br;
}

void wid_set_client_context (widp w, void *client_context)
{
    fast_verify(w);

    w->client_context = client_context;
}

void *wid_get_client_context (widp w)
{
    return (w->client_context);
}

void wid_set_prev (widp w, widp prev)
{
    fast_verify(w);

    if (w == prev) {
        DIE("wid list loop");
    }

    w->prev = prev;

    if (prev) {
        prev->next = w;
    }
}

widp wid_get_prev (widp w)
{
    if (w->prev == w) {
        DIE("wid list get prev loop");
    }

    return (w->prev);
}

widp wid_get_next (widp w)
{
    if (w->next == w) {
        DIE("wid list get next loop");
    }

    return (w->next);
}

static widp wid_get_head (widp w)
{
    widp prev;

    while (w) {

        prev = wid_get_prev(w);
        if (!prev) {
            break;
        }

        w = prev;
    }

    return (w);
}

static widp wid_get_tail (widp w)
{
    widp next;

    while (w) {

        next = wid_get_next(w);
        if (!next) {
            break;
        }

        w = next;
    }

    return (w);
}

widp wid_get_top_parent (widp w)
{
    if (!w) {
        return (0);
    }

    if (!w->parent) {
        return (w);
    }

    while (w->parent) {
        w = w->parent;
    }

    return (w);
}

widp wid_get_parent (widp w)
{
    if (!w) {
        return (0);
    }

    return (w->parent);
}

widp wid_get_scrollbar_vert (widp w)
{
    if (!w) {
        return (0);
    }

    return (w->scrollbar_vert);
}

widp wid_get_scrollbar_horiz (widp w)
{
    if (!w) {
        return (0);
    }

    return (w->scrollbar_horiz);
}

tree_root *wid_get_children (widp w)
{
    if (!w) {
        return (0);
    }

    return (w->children_display_sorted);
}

static void wid_mouse_motion_end (void)
{
    wid_moving = 0;
}

void wid_set_ignore_events (widp w, boolean val)
{
    w->ignore_for_events = val;
}

/*
 * Should this widget be ignored for events?
 */
boolean wid_ignore_for_events (widp w)
{
    widp top;

    if (!w) {
        return (true);
    }

    verify(w);

    if (w->ignore_for_events || w->moving ||
        /*
         * For buttons that pulse.
         */
        (!w->fade_count && (w->fade_out || w->fade_in)) ||
        w->hidden ||
        w->being_destroyed) {
        return (true);
    }

    if (w->parent) {
        top = wid_get_top_parent(w);

        if (top->moving ||
            (!w->fade_count && (w->fade_in)) ||
            top->hidden ||
            top->being_destroyed) {
            return (true);
        }
    }

    return (false);
}

boolean wid_ignore_for_focus (widp w)
{
    widp top;

    if (w->hidden ||
        w->being_destroyed) {
        return (true);
    }

    if (w->parent) {
        top = wid_get_top_parent(w);

        if (top->hidden ||
            top->being_destroyed) {
            return (true);
        }
    }

    return (false);
}

/*
 * Should this widget be ignored for events?
 */
boolean wid_ignore_being_destroyed (widp w)
{
    widp top;

    if (w->being_destroyed) {
        return (true);
    }

    if (w->parent) {
        top = wid_get_top_parent(w);

        if (top->being_destroyed) {
            return (true);
        }
    }

    return (false);
}

static void wid_mouse_motion_begin (widp w, int32_t x, int32_t y)
{
    wid_mouse_motion_end();

    if (wid_ignore_being_destroyed(w)) {
        return;
    }

    wid_moving = w;
    wid_moving_last_x = x;
    wid_moving_last_y = y;
}

static void wid_mouse_focus_end (void)
{
    widp w;

    w = wid_focus;
    wid_focus = 0;
    wid_focus_locked = 0;

    if (!w) {
        return;
    }

    if (w->on_mouse_focus_end) {
        w->on_mouse_focus_end(w);
    }
}

widp wid_get_current_focus (void)
{
    return (wid_focus);
}

static void wid_mouse_focus_begin (widp w)
{
    widp top;

    if (!w) {
        wid_mouse_focus_end();
        wid_focus = 0;

        wid_find_top_focus();
        return;
    }

    if (wid_focus == w) {
        return;
    }

    wid_mouse_focus_end();

    if (wid_ignore_for_focus(w)) {
        return;
    }

    top = wid_get_top_parent(w);

    wid_focus = w;
    top->focus_last = w->focus_order;

    if (w->on_mouse_focus_begin) {
        w->on_mouse_focus_begin(w);
    }
}

static void wid_mouse_over_end (void)
{
    widp w;

    w = wid_over;
    wid_over = 0;

    if (!w) {
        return;
    }

    wid_set_mode(w, WID_MODE_NORMAL);

    if (w->on_mouse_over_end) {
        w->on_mouse_over_end(w);
    }

    if (wid_popup_tooltip) {
        wid_destroy(&wid_popup_tooltip);
    }
}

static boolean wid_mouse_over_begin (widp w, uint32_t x, uint32_t y)
{
    if (wid_over == w) {
        return (true);
    }

    if (!w->on_mouse_over_begin) {
        if (w->cfg[WID_MODE_OVER].font ||
            w->cfg[WID_MODE_OVER].color_set[WID_COLOR_TL] ||
            w->cfg[WID_MODE_OVER].color_set[WID_COLOR_BR] ||
            w->cfg[WID_MODE_OVER].color_set[WID_COLOR_BG] ||
            w->cfg[WID_MODE_OVER].color_set[WID_COLOR_TEXT]) {
            /*
             * Changes appearance on mouse over, so choose this wid even
             * if it has no over callback.
             */
        } else {
            /*
             * Can ignore. It doesn't really do anything when the mouse
             * is over.
             */
            return (false);
        }
    }

    if (wid_ignore_being_destroyed(w)) {
        return (false);
    }

    wid_mouse_over_end();

    wid_over = w;

    wid_set_mode(w, WID_MODE_OVER);

    if (w->on_mouse_over_begin) {
        w->on_mouse_over_begin(w);
    }

    if (w->tooltip) {
        wid_popup_tooltip = wid_tooltip(w->tooltip, 0.5, 0.5, small_font);
    }

    return (true);
}

/*
 * Map an SDL key event to the char the user typed
 */
static char wid_event_to_char (const struct SDL_KEYSYM *evt)
{
    switch (evt->mod) {
        case KMOD_LSHIFT:
        case KMOD_RSHIFT:
            switch (evt->sym) {
                case SDLK_a: return ('A');
                case SDLK_b: return ('B');
                case SDLK_c: return ('C');
                case SDLK_d: return ('D');
                case SDLK_e: return ('E');
                case SDLK_f: return ('F');
                case SDLK_g: return ('G');
                case SDLK_h: return ('H');
                case SDLK_i: return ('I');
                case SDLK_j: return ('J');
                case SDLK_k: return ('K');
                case SDLK_l: return ('L');
                case SDLK_m: return ('M');
                case SDLK_n: return ('N');
                case SDLK_o: return ('O');
                case SDLK_p: return ('P');
                case SDLK_q: return ('Q');
                case SDLK_r: return ('R');
                case SDLK_s: return ('S');
                case SDLK_t: return ('T');
                case SDLK_u: return ('U');
                case SDLK_v: return ('V');
                case SDLK_w: return ('W');
                case SDLK_x: return ('X');
                case SDLK_y: return ('Y');
                case SDLK_z: return ('Z');
                case SDLK_QUOTE: return ('¬');
                case SDLK_COMMA: return ('<');
                case SDLK_MINUS: return ('_');
                case SDLK_PERIOD: return ('>');
                case SDLK_SLASH: return ('?');
                case SDLK_EQUALS: return ('+');
                case SDLK_0: return (')');
                case SDLK_1: return ('!');
                case SDLK_2: return ('"');
                case SDLK_3: return ('£');
                case SDLK_4: return ('$');
                case SDLK_5: return ('%');
                case SDLK_6: return ('^');
                case SDLK_7: return ('&');
                case SDLK_8: return ('*');
                case SDLK_9: return ('(');
                case SDLK_SEMICOLON: return (':');
                case SDLK_LEFTBRACKET: return ('{');
                case SDLK_BACKSLASH: return ('|');
                case SDLK_RIGHTBRACKET: return ('}');
                case SDLK_HASH: return ('~');

            default:
                break;
            }

        case KMOD_LALT:
        case KMOD_RALT:
            switch (evt->sym) {
                default:
                break;
            }

        case KMOD_LCTRL:
        case KMOD_RCTRL:
            switch (evt->sym) {
                default:
                break;
            }

        default:
            break;
    }

    switch (evt->sym) {
        case SDLK_BACKSPACE: return ('');
        case SDLK_TAB: return ('\t');
        case SDLK_RETURN: return ('\n');
        case SDLK_ESCAPE: return ('');
        case SDLK_SPACE: return (' ');
        case SDLK_EXCLAIM: return ('!');
        case SDLK_QUOTEDBL: return ('"');
        case SDLK_HASH: return ('#');
        case SDLK_DOLLAR: return ('$');
        case SDLK_AMPERSAND: return ('%');
        case SDLK_QUOTE: return ('\'');
        case SDLK_LEFTPAREN: return ('(');
        case SDLK_RIGHTPAREN: return (')');
        case SDLK_ASTERISK: return ('*');
        case SDLK_PLUS: return ('+');
        case SDLK_COMMA: return (',');
        case SDLK_MINUS: return ('-');
        case SDLK_PERIOD: return ('.');
        case SDLK_SLASH: return ('/');
        case SDLK_0: return ('0');
        case SDLK_1: return ('1');
        case SDLK_2: return ('2');
        case SDLK_3: return ('3');
        case SDLK_4: return ('4');
        case SDLK_5: return ('5');
        case SDLK_6: return ('6');
        case SDLK_7: return ('7');
        case SDLK_8: return ('8');
        case SDLK_9: return ('9');
        case SDLK_COLON: return (':');
        case SDLK_SEMICOLON: return (';');
        case SDLK_LESS: return ('<');
        case SDLK_EQUALS: return ('=');
        case SDLK_GREATER: return ('>');
        case SDLK_QUESTION: return ('?');
        case SDLK_AT: return ('@');
        case SDLK_LEFTBRACKET: return ('[');
        case SDLK_BACKSLASH: return ('\\');
        case SDLK_RIGHTBRACKET: return (']');
        case SDLK_CARET: return ('^');
        case SDLK_UNDERSCORE: return ('_');
        case SDLK_BACKQUOTE: return ('`');
        case SDLK_a: return ('a');
        case SDLK_b: return ('b');
        case SDLK_c: return ('c');
        case SDLK_d: return ('d');
        case SDLK_e: return ('e');
        case SDLK_f: return ('f');
        case SDLK_g: return ('g');
        case SDLK_h: return ('h');
        case SDLK_i: return ('i');
        case SDLK_j: return ('j');
        case SDLK_k: return ('k');
        case SDLK_l: return ('l');
        case SDLK_m: return ('m');
        case SDLK_n: return ('n');
        case SDLK_o: return ('o');
        case SDLK_p: return ('p');
        case SDLK_q: return ('q');
        case SDLK_r: return ('r');
        case SDLK_s: return ('s');
        case SDLK_t: return ('t');
        case SDLK_u: return ('u');
        case SDLK_v: return ('v');
        case SDLK_w: return ('w');
        case SDLK_x: return ('x');
        case SDLK_y: return ('y');
        case SDLK_z: return ('z');
        case SDLK_DELETE: return ('');
#if SDL_MAJOR_VERSION == 1 /* { */
        case SDLK_KP0: return ('0');
        case SDLK_KP1: return ('1');
        case SDLK_KP2: return ('2');
        case SDLK_KP3: return ('3');
        case SDLK_KP4: return ('4');
        case SDLK_KP5: return ('5');
        case SDLK_KP6: return ('6');
        case SDLK_KP7: return ('7');
        case SDLK_KP8: return ('8');
        case SDLK_KP9: return ('9');
#else
        case SDLK_KP_0: return ('0');
        case SDLK_KP_1: return ('1');
        case SDLK_KP_2: return ('2');
        case SDLK_KP_3: return ('3');
        case SDLK_KP_4: return ('4');
        case SDLK_KP_5: return ('5');
        case SDLK_KP_6: return ('6');
        case SDLK_KP_7: return ('7');
        case SDLK_KP_8: return ('8');
        case SDLK_KP_9: return ('9');
#endif /* } */
        case SDLK_KP_PERIOD: return ('.');
        case SDLK_KP_DIVIDE: return ('/');
        case SDLK_KP_MULTIPLY: return ('*');
        case SDLK_KP_MINUS: return ('-');
        case SDLK_KP_PLUS: return ('+');
        case SDLK_KP_ENTER: return ('\0');
        case SDLK_KP_EQUALS: return ('=');
        case SDLK_UP: return ('\0');
        case SDLK_DOWN: return ('\0');
        case SDLK_RIGHT: return ('\0');
        case SDLK_LEFT: return ('\0');
        case SDLK_INSERT: return ('\0');
        case SDLK_HOME: return ('\0');
        case SDLK_END: return ('\0');
        case SDLK_PAGEUP: return ('\0');
        case SDLK_PAGEDOWN: return ('\0');
        case SDLK_F1: return ('\0');
        case SDLK_F2: return ('\0');
        case SDLK_F3: return ('\0');
        case SDLK_F4: return ('\0');
        case SDLK_F5: return ('\0');
        case SDLK_F6: return ('\0');
        case SDLK_F7: return ('\0');
        case SDLK_F8: return ('\0');
        case SDLK_F9: return ('\0');
        case SDLK_F10: return ('\0');
        case SDLK_F11: return ('\0');
        case SDLK_F12: return ('\0');
        case SDLK_F13: return ('\0');
        case SDLK_F14: return ('\0');
        case SDLK_F15: return ('\0');
        case SDLK_CAPSLOCK: return ('\0');
        case SDLK_RSHIFT: return ('\0');
        case SDLK_LSHIFT: return ('\0');
        case SDLK_RCTRL: return ('\0');
        case SDLK_LCTRL: return ('\0');
        case SDLK_RALT: return ('\0');
        case SDLK_LALT: return ('\0');
        case SDLK_MODE: return ('\0');
        case SDLK_HELP: return ('!');
        case SDLK_SYSREQ: return ('\0');
        case SDLK_MENU: return ('\0');
        case SDLK_POWER: return ('\0');
        case SDLK_UNDO: return ('\0');
        default: return ('\0');
    }
    return ('\0');
}

/*
 * Widget mode, whether it is active, inactive etc...
 */
void wid_set_mode (widp w, wid_mode mode)
{
    fast_verify(w);

    w->timestamp_last_mode_change = time_get_time_cached();
    w->mode = mode;
}

/*
 * Widget mode, whether it is active, inactive etc...
 */
wid_mode wid_get_mode (widp w)
{
    /*
     * Allow focus to override less important modes.
     */
    if (w == wid_focus) {
        if ((w->mode == WID_MODE_NORMAL) || (w->mode == WID_MODE_OVER)) {
            return (WID_MODE_FOCUS);
        }
    }

    return (w->mode);
}

const char *wid_logname (widp w)
{
    fast_verify(w);

    if (!w->logname) {
        return ("");
    }

    return (w->logname);
}

const char *wid_name (widp w)
{
    fast_verify(w);

    if (!w->name) {
        return ("");
    }

    return (w->name);
}

const char *wid_get_text (widp w)
{
    fast_verify(w);

    return (w->text);
}

const char *wid_get_tooltip (widp w)
{
    fast_verify(w);

    if (!w->tooltip) {
        return ("");
    }

    return (w->tooltip);
}

const char *wid_get_text_with_cursor (widp w)
{
    static char tmp[MAXSTR];
    static char cursor_char[2] = { TTF_CURSOR_CHAR, '\0' };

    if (!w->received_input) {
        w->cursor = (uint32_t)strlen(w->text);
    }

    strlcpy(tmp, w->text, w->cursor + 1);
    strlcat(tmp, cursor_char, sizeof(tmp));
    strlcat(tmp, w->text + w->cursor, sizeof(tmp));

    return (tmp);
}

void wid_set_name (widp w, const char *string)
{
    fast_verify(w);

    if (w->name) {
        myfree(w->name);
        w->name = 0;
    }

    if (!string) {
        return;
    }

    w->name = dupstr(string, "wid name");
}

void wid_set_text (widp w, const char *string)
{
    fast_verify(w);

    if (!string) {
        w->text[0] = '\0';
    } else {
        if (!strcmp(string, w->text)) {
            return;
        }

        strlcpy(w->text, string, MAXSTR);
    }

    if (!w->name && string) {
        wid_set_name(w, string);
    }

    w->text_size_cached = false;

    uint32_t len;

    if (!string) {
        len = 0;
    } else {
        len = (uint32_t)strlen(string);
    }

    if (w->cursor > len) {
        w->cursor = len;
    }
}

void wid_set_tooltip (widp w, const char *string)
{
    fast_verify(w);

    if (w->tooltip) {
        myfree(w->tooltip);
        w->tooltip = 0;
    }

    if (!string) {
        return;
    }

    w->tooltip = dupstr(string, "wid tooltip");
}

boolean wid_get_received_input (widp w)
{
    return (w->received_input);
}

void wid_set_received_input (widp w, boolean val)
{
    fast_verify(w);

    w->received_input = val;
}

uint32_t wid_get_cursor (widp w)
{
    return (w->cursor);
}

void wid_set_cursor (widp w, uint32_t val)
{
    fast_verify(w);

    w->cursor = val;
}

double wid_get_width (widp w)
{
    return (wid_get_br_x(w) - wid_get_tl_x(w));
}

double wid_get_height (widp w)
{
    return (wid_get_br_y(w) - wid_get_tl_y(w));
}

void wid_get_mxy (widp w, double *x, double *y)
{
    *x = (wid_get_br_x(w) + wid_get_tl_x(w)) / 2.0;
    *y = (wid_get_br_y(w) + wid_get_tl_y(w)) / 2.0;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
fontp wid_get_font (widp w)
{
    uint32_t mode = (typeof(mode)) wid_get_mode(w); // for c++, no enum walk
    wid_cfg *cfg = &w->cfg[mode];

    if (cfg->font) {
        return (cfg->font);
    }

    if ((wid_focus == w) && (wid_over == w)) {
        mode = WID_MODE_OVER;
        cfg = &w->cfg[mode];
        if (cfg->font) {
            return (cfg->font);
        }
    }

    mode = WID_MODE_NORMAL;
    cfg = &w->cfg[mode];
    if (cfg->font) {
        return (cfg->font);
    }

    return (0);
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
fontp wid_get_mode_font (widp w)
{
    uint32_t mode = (typeof(mode)) wid_get_mode(w); // for c++, no enum walk
    wid_cfg *cfg = &w->cfg[mode];

    return (cfg->font);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_font (widp w, fontp val)
{
    fast_verify(w);

    w->cfg[wid_get_mode(w)].font = val;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
uint8_t wid_get_focusable (widp w)
{
    return (w->focus_order);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_focusable (widp w, uint8_t val)
{
    fast_verify(w);

    w->focus_order = val;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
boolean wid_get_show_cursor (widp w)
{
    return (w->show_cursor);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_show_cursor (widp w, boolean val)
{
    fast_verify(w);

    w->show_cursor = val;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
boolean wid_get_do_not_raise (widp w)
{
    return (w->do_not_raise);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_do_not_raise (widp w, boolean val)
{
    fast_verify(w);

    w->do_not_raise = val;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
boolean wid_get_do_not_lower (widp w)
{
    return (w->do_not_lower);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_do_not_lower (widp w, boolean val)
{
    fast_verify(w);

    w->do_not_lower = val;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
boolean wid_get_movable (widp w)
{
    if (w->movable_set) {
        return (w->movable);
    }

    return (false);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_movable (widp w, boolean val)
{
    fast_verify(w);

    w->movable = val;
    w->movable_set = true;

    wid_set_movable_horiz(w, val);
    wid_set_movable_vert(w, val);
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
boolean wid_get_movable_horiz (widp w)
{
    if (w->movable_horiz_set) {
        return (w->movable_horiz);
    }

    return (false);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_movable_horiz (widp w, boolean val)
{
    fast_verify(w);

    w->movable_horiz = val;
    w->movable_horiz_set = true;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
boolean wid_get_movable_vert (widp w)
{
    if (w->movable_vert_set) {
        return (w->movable_vert);
    }

    return (false);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_movable_vert (widp w, boolean val)
{
    fast_verify(w);

    w->movable_vert = val;
    w->movable_vert_set = true;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
boolean wid_get_movable_bounded (widp w)
{
    if (w->movable_bounded_set) {
        return (w->movable_bounded);
    }

    return (false);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_movable_bounded (widp w, boolean val)
{
    fast_verify(w);

    w->movable_bounded = val;
    w->movable_bounded_set = true;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
boolean wid_get_text_lhs (widp w)
{
    return (w->text_lhs);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_text_lhs (widp w, boolean val)
{
    fast_verify(w);

    w->text_lhs = val;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
boolean wid_get_text_rhs (widp w)
{
    return (w->text_rhs);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_text_rhs (widp w, boolean val)
{
    fast_verify(w);

    w->text_rhs = true;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
boolean wid_get_text_centerx (widp w)
{
    return (w->text_centerx);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_text_centerx (widp w, boolean val)
{
    fast_verify(w);

    w->text_centerx = val;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
boolean wid_get_text_top (widp w)
{
    return (w->text_top);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_text_top (widp w, boolean val)
{
    fast_verify(w);

    w->text_top = val;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
boolean wid_get_text_bot (widp w)
{
    return (w->text_bot);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_text_bot (widp w, boolean val)
{
    fast_verify(w);

    w->text_bot = val;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
boolean wid_get_text_centery (widp w)
{
    return (w->text_centery);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_text_centery (widp w, boolean val)
{
    fast_verify(w);

    w->text_centery = val;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
boolean wid_get_text_pos (widp w, double *x, double *y)
{
    if (w->text_pos_set) {
        *x = w->text_pos.x;
        *y = w->text_pos.y;

        return (true);
    }

    return (false);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_text_pos (widp w, boolean val, double x, double y)
{
    fast_verify(w);

    w->text_pos.x = x;
    w->text_pos.y = y;
    w->text_pos_set = val;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
boolean wid_get_text_outline (widp w)
{
    return (w->text_outline);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_text_outline (widp w, boolean val)
{
    fast_verify(w);

    w->text_outline = val;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
boolean wid_get_blit_outline (widp w)
{
    return (w->blit_outline);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_blit_outline (widp w, boolean val)
{
    fast_verify(w);

    w->blit_outline = val;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
double wid_get_bevel (widp w)
{
    return (w->bevel);
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
double wid_get_mode_bevel (widp w)
{
    return (w->bevel);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_bevel (widp w, double val)
{
    fast_verify(w);

    w->bevel = val;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
double wid_get_text_scaling (widp w)
{
    return (w->text_scaling + wid_get_scaling_w(w));
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_text_scaling (widp w, double val)
{
    fast_verify(w);

    w->text_scaling = val;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
double wid_get_text_advance (widp w)
{
    if (w->text_advance != 0) {
        return (w->text_advance);
    }

    return (1.0f);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_text_advance (widp w, double val)
{
    fast_verify(w);

    w->text_advance = val;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
boolean wid_get_text_fixed_width (widp w)
{
    return (w->text_fixed_width);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_text_fixed_width (widp w, boolean val)
{
    fast_verify(w);

    w->text_fixed_width = val;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
int32_t wid_get_sides (widp w)
{
    return (w->sides);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_sides (widp w, int32_t val)
{
    fast_verify(w);

    w->sides = val;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
texp wid_get_tex (widp w, fsize *size)
{
    if (size) {
        *size = w->texuv;
    }

    return (w->tex);
}

void wid_set_tex (widp w, const char *tex, const char *name)
{
    fast_verify(w);

    if (!tex && !name) {
        w->tex = 0;
        return;
    }

    texp t = tex_load(tex, name);
    if (!t) {
        DIE("failed to set wid tex %s", tex);
    }

    w->tex = t;

    fsize sz = {
        (double) tex_get_width(t),
        (double) tex_get_height(t)
    };

    wid_set_tex_sz(w, sz);
}

void wid_set_tex_sz (widp w, fsize uv)
{
    fast_verify(w);

    w->texuv = uv;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
tilep wid_get_tile (widp w)
{
    return (w->tile);
}

tilep wid_get_tile2 (widp w)
{
    return (w->tile2);
}

thingp wid_get_thing (widp w)
{
    return (w->thing);
}

thing_templatep wid_get_thing_template (widp w)
{
    return (w->thing_template);
}

void wid_set_tilename (widp w, const char *name)
{
    fast_verify(w);

    tilep t = tile_find(name);
    if (!t) {
        DIE("failed to set wid tile %s", name);
    }

    w->tile = t;
}

void wid_set_tile2name (widp w, const char *name)
{
    fast_verify(w);

    tilep t = tile_find(name);
    if (!t) {
        DIE("failed to set wid tile %s", name);
    }

    w->tile2 = t;
}

void wid_set_tile (widp w, tilep tile)
{
    fast_verify(w);

    w->tile = tile;
}

void wid_set_tile2 (widp w, tilep tile)
{
    fast_verify(w);

    w->tile2 = tile;
}

void wid_set_z_depth (widp w, uint8_t z_depth)
{
    fast_verify(w);

    wid_tree_detach(w);
    w->tree.z_depth = z_depth;
    wid_tree_attach(w);
}

uint8_t wid_get_z_depth (widp w)
{
    fast_verify(w);

    return (w->tree.z_depth);
}

void wid_set_z_order (widp w, uint8_t z_order)
{
    fast_verify(w);

    wid_tree_detach(w);
    w->tree.z_order = z_order;
    wid_tree_attach(w);
}

uint8_t wid_get_z_order (widp w)
{
    fast_verify(w);

    return (w->tree.z_order);
}

void wid_set_thing (widp w, thingp t)
{
    tree_rootp tiles;
    tree_rootp tiles2;
    thing_tilep tile;

    fast_verify(w);

    w->thing = t;
    if (!t) {
        return;
    }

    wid_set_z_depth(w, thing_z_depth(t));
    wid_set_z_order(w, thing_z_order(t));

    tiles = thing_tile_tiles(t);
    if (!tiles) {
        return;
    }

    tile = (typeof(tile)) tree_root_first(tiles);
    if (!tile) {
        return;
    }

    wid_set_tilename(w, thing_tile_name(tile));
    wid_set_name(w, thing_name(t));

    tiles2 = thing_tile_tiles2(t);
    if (tiles2) {
        tile = (typeof(tile)) tree_root_first(tiles2);
        if (!tile) {
            return;
        }

        wid_set_tile2name(w, thing_tile_name(tile));
    }

    thing_set_wid(t, w);
}

void wid_set_thing_template (widp w, thing_templatep t)
{
    tree_rootp tiles;
    tree_rootp tiles2;
    thing_tilep tile;

    fast_verify(w);

    w->thing_template = t;

    if (!t) {
        wid_set_z_depth(w, 0);
        wid_set_z_order(w, 0);
        wid_set_tile(w, 0);
        return;
    }

    wid_set_z_depth(w, thing_template_get_z_depth(t));
    wid_set_z_order(w, thing_template_get_z_order(t));

    tiles = thing_template_get_tiles(t);
    if (!tiles) {
        return;
    }

    tile = (typeof(tile)) tree_root_get_random(tiles);
    if (!tile) {
        return;
    }

    wid_set_tilename(w, thing_tile_name(tile));

    tiles2 = thing_template_get_tiles2(t);
    if (tiles2) {
        tile = (typeof(tile)) tree_root_get_random(tiles2);
        if (!tile) {
            return;
        }

        wid_set_tile2name(w, thing_tile_name(tile));
    }

    wid_set_name(w, thing_template_name(t));
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
fsize wid_get_tex_tl (widp w)
{
    if (w->tex_tl_set) {
        return (w->tex_tl);
    }

    fsize nosize = {0, 0};
    return (nosize);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_tex_tl (widp w, fsize val)
{
    fast_verify(w);

    w->tex_tl_set = true;
    w->tex_tl = val;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
fsize wid_get_tex_br (widp w)
{
    if (w->tex_br_set) {
        return (w->tex_br);
    }

    fsize size = {1, 1};
    return (size);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_tex_br (widp w, fsize val)
{
    fast_verify(w);

    w->tex_br_set = true;
    w->tex_br = val;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
fsize wid_get_radius (widp w)
{
    if (w->radius_set) {
        return (w->radius);
    }

    fsize nosize = {0, 0};
    return (nosize);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_radius (widp w, fsize val)
{
    fast_verify(w);

    w->radius_set = true;
    w->radius = val;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
color wid_get_color (widp w, wid_color which)
{
    uint32_t mode = (typeof(mode)) wid_get_mode(w); // for c++, no enum walk
    wid_cfg *cfg = &w->cfg[mode];

    if (cfg->color_set[which]) {
        return (cfg->colors[which]);
    }

    if ((wid_focus == w) && (wid_over == w)) {
        mode = WID_MODE_OVER;
        cfg = &w->cfg[mode];
        if (cfg->color_set[which]) {
            return (cfg->colors[which]);
        }
    }

    mode = WID_MODE_NORMAL;
    cfg = &w->cfg[mode];
    if (cfg->color_set[which]) {
        return (cfg->colors[which]);
    }

    return (WHITE);
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
color wid_get_mode_color (widp w, wid_color which)
{
    uint32_t mode = (typeof(mode)) wid_get_mode(w); // for c++, no enum walk
    wid_cfg *cfg = &w->cfg[mode];

    return (cfg->colors[which]);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_color (widp w, wid_color col, color val)
{
    fast_verify(w);

    w->cfg[wid_get_mode(w)].colors[col] = val;
    w->cfg[wid_get_mode(w)].color_set[col] = true;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
boolean wid_get_bevelled (widp w)
{
    return (w->bevelled);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_bevelled (widp w, boolean val)
{
    fast_verify(w);

    w->bevelled = val;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
boolean wid_get_rounded (widp w)
{
    return (w->rounded);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_rounded_large (widp w)
{
    fast_verify(w);

    w->square = false;
    w->rounded = true;
    w->square_outline = false;

    fsize radius = {40.0f, 40.0f};
    wid_set_radius(w, radius);
    wid_set_sides(w, 40);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_rounded_small (widp w)
{
    fast_verify(w);

    w->square = false;
    w->rounded = true;
    w->square_outline = false;

    fsize radius = {5.0f, 5.0f};
    wid_set_radius(w, radius);
    wid_set_sides(w, 8);
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
boolean wid_get_square (widp w)
{
    return (w->square);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_square (widp w)
{
    fast_verify(w);

    w->rounded = false;
    w->square = true;
    w->square_outline = false;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
boolean wid_get_square_outline (widp w)
{
    return (w->square_outline);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_square_outline (widp w)
{
    fast_verify(w);

    w->rounded = false;
    w->square = false;
    w->square_outline = true;
}

void wid_set_no_shape (widp w)
{
    fast_verify(w);

    w->rounded = false;
    w->square = false;
    w->square_outline = false;
}

void wid_set_offset (widp w, fpoint offset)
{
    fast_verify(w);

    w->offset = offset;
}

void wid_get_offset (widp w, fpoint *offset)
{
    fast_verify(w);

    *offset = w->offset;
}

void wid_set_focus (widp w)
{
    fast_verify(w);

    /*
     * Don't allow focus override if hard focus is set.
     */
    if (wid_focus_locked) {
        if (wid_get_top_parent(w) != wid_focus_locked) {
            return;
        }
    }

    if (!w->focus_order) {
        return;
    }

    wid_mouse_focus_begin(w);
}

void wid_focus_lock (widp w)
{
    WID_DBG(w, "focus locked");

    wid_focus_locked = w;
}

void wid_set_on_key_down (widp w, on_key_down_t fn)
{
    fast_verify(w);

    w->on_key_down = fn;
}

void wid_set_on_key_up (widp w, on_key_up_t fn)
{
    fast_verify(w);

    w->on_key_up = fn;
}

void wid_set_on_mouse_down (widp w, on_mouse_down_t fn)
{
    fast_verify(w);

    w->on_mouse_down = fn;
}

void wid_set_on_mouse_up (widp w, on_mouse_up_t fn)
{
    fast_verify(w);

    w->on_mouse_up = fn;
}

void wid_set_on_mouse_motion (widp w, on_mouse_motion_t fn)
{
    fast_verify(w);

    w->on_mouse_motion = fn;
}

void wid_set_on_mouse_focus_begin (widp w, on_mouse_focus_begin_t fn)
{
    fast_verify(w);

    w->on_mouse_focus_begin = fn;
}

void wid_set_on_mouse_focus_end (widp w, on_mouse_focus_end_t fn)
{
    fast_verify(w);

    w->on_mouse_focus_end = fn;
}

void wid_set_on_mouse_over_begin (widp w, on_mouse_over_begin_t fn)
{
    fast_verify(w);

    w->on_mouse_over_begin = fn;
}

void wid_set_on_mouse_over_end (widp w, on_mouse_over_end_t fn)
{
    fast_verify(w);

    w->on_mouse_over_end = fn;
}

void wid_set_on_destroy (widp w, on_destroy_t fn)
{
    fast_verify(w);

    w->on_destroy = fn;
}

void wid_set_on_tick (widp w, on_tick_t fn)
{
    fast_verify(w);

    if (!fn) {
        DIE("no ticker function set");
    }

    w->on_tick = fn;

    wid_tree5_insert(w);
}

static int8_t tree_wid_compare_func (const tree_node *a, const tree_node *b)
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

#if 0
    int32_t Ax = (A->tree.tl.x + A->tree.br.x) / 2;
    int32_t Ay = (A->tree.tl.y + A->tree.br.y) / 2;

    int32_t Bx = (B->tree.tl.x + B->tree.br.x) / 2;
    int32_t By = (B->tree.tl.y + B->tree.br.y) / 2;

    int32_t Aval = Ax + Ay;
    int32_t Bval = Bx + By;

    if (Aval > Bval) {
        return (-1);
    }

    if (Aval < Bval) {
        return (1);
    }
#endif

    if (A->tree.z_order > B->tree.z_order) {
        return (-1);
    }

    if (A->tree.z_order < B->tree.z_order) {
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

/*
 * Remove this wid from any trees it is in.
 */
static void wid_tree_detach (widp w)
{
    wid_tree_remove(w);

    wid_grid_tree_detach(w);
}

/*
 * Add back to all trees.
 */
static void wid_tree_attach (widp w)
{
    fast_verify(w);

    tree_root *root;

    if (!w->parent) {
        root = wid_top_level;
    } else {
        root = w->parent->children_display_sorted;
    }

    if (!tree_insert(root, &w->tree.node)) {
        DIE("wid set z depth tree insert");
    }

    wid_grid_tree_attach(w);
}

static void wid_tree_insert (widp w)
{
    fast_verify(w);

    static int32_t key;

    tree_root *root;

    if (!w->parent) {
        root = wid_top_level;
    } else {
        root = w->parent->children_display_sorted;
    }

    if (!root) {
        root = tree_alloc_custom(tree_wid_compare_func, "TREE ROOT: wid");
        if (!w->parent) {
            wid_top_level = root;
        } else {
            w->parent->children_display_sorted = root;
        }
    }

    /*
     * Get a wid sort ID.
     */
    w->tree.key = ++key;

    if (!tree_insert(root, &w->tree.node)) {
        DIE("widget tree insert");
    }
}

static void wid_tree2_insert (widp w)
{
    fast_verify(w);

    static int32_t key;

    tree_root *root;

    if (!w->parent) {
        root = wid_top_level2;
    } else {
        root = w->parent->children_unsorted;
    }

    if (!root) {
        root = tree_alloc(TREE_KEY_INTEGER, "TREE ROOT2: wid");
        if (!w->parent) {
            wid_top_level2 = root;
        } else {
            w->parent->children_unsorted = root;
        }

        root->offset = STRUCT_OFFSET(struct wid_, tree2);
    }

    /*
     * Get a wid sort ID.
     */
    w->tree2.key = ++key;

    if (!tree_insert(root, &w->tree2.node)) {
        DIE("widget tree2 insert");
    }

    /*
     * The other tree will do the actual node free.
     */
    w->tree2.node.is_static_mem = true;
}

static void wid_tree3_insert (widp w)
{
    fast_verify(w);

    if (w->in_tree3) {
        return;
    }

    if (wid_exiting) {
        return;
    }

    static int32_t key;

    tree_root *root;

    root = wid_top_level3;

    if (!root) {
        root = tree_alloc(TREE_KEY_INTEGER, "TREE ROOT3: wid");
        wid_top_level3 = root;

        root->offset = STRUCT_OFFSET(struct wid_, tree3);
    }

    /*
     * Get a wid sort ID.
     */
    w->tree3.key = ++key;

    if (!tree_insert(root, &w->tree3.node)) {
        DIE("widget tree3 insert");
    }

    /*
     * The other tree will do the actual node free.
     */
    w->tree3.node.is_static_mem = true;
    w->in_tree3 = true;
}

static void wid_tree4_insert (widp w)
{
    fast_verify(w);

    if (w->in_tree4) {
        return;
    }

    if (wid_exiting) {
        return;
    }

    static int32_t key;

    tree_root *root;

    root = wid_top_level4;

    if (!root) {
        root = tree_alloc(TREE_KEY_INTEGER, "TREE ROOT4: wid");
        wid_top_level4 = root;

        root->offset = STRUCT_OFFSET(struct wid_, tree4);
    }

    /*
     * Get a wid sort ID.
     */
    w->tree4.key = ++key;

    if (!tree_insert(root, &w->tree4.node)) {
        DIE("widget tree4 insert");
    }

    /*
     * The other tree will do the actual node free.
     */
    w->tree4.node.is_static_mem = true;
    w->in_tree4 = true;
}

static void wid_tree5_insert (widp w)
{
    fast_verify(w);

    if (w->in_tree5) {
        return;
    }

    if (wid_exiting) {
        return;
    }

    static int32_t key;

    tree_root *root;

    root = wid_top_level5;

    if (!root) {
        root = tree_alloc(TREE_KEY_INTEGER, "TREE ROOT5: wid");
        wid_top_level5 = root;

        root->offset = STRUCT_OFFSET(struct wid_, tree5);
    }

    /*
     * Get a wid sort ID.
     */
    w->tree5.key = ++key;

    if (!tree_insert(root, &w->tree5.node)) {
        DIE("widget tree5 insert");
    }

    /*
     * The other tree will do the actual node free.
     */
    w->tree5.node.is_static_mem = true;
    w->in_tree5 = true;
}

static void wid_tree_remove (widp w)
{
    fast_verify(w);

    tree_root *root;

    if (!w->parent) {
        root = wid_top_level;
    } else {
        root = w->parent->children_display_sorted;
    }

    if (!tree_remove(root, &w->tree.node)) {
        DIE("remove from display sorted tree");
    }
}

static void wid_tree2_remove (widp w)
{
    fast_verify(w);

    tree_root *root;

    if (!w->parent) {
        root = wid_top_level2;
    } else {
        root = w->parent->children_unsorted;
    }

    if (!tree_remove(root, &w->tree2.node)) {
        DIE("remove from unsorted tree");
    }
}

static void wid_tree3_remove (widp w)
{
    fast_verify(w);

    if (!w->in_tree3) {
        return;
    }

    tree_root *root;

    root = wid_top_level3;

    if (!root) {
        return;
    }

    if (!tree_remove(root, &w->tree3.node)) {
        DIE("remove from move tree");
    }

    w->in_tree3 = false;
}

static void wid_tree4_remove (widp w)
{
    fast_verify(w);

    if (!w->in_tree4) {
        return;
    }

    tree_root *root;

    root = wid_top_level4;

    if (!root) {
        return;
    }

    if (!tree_remove(root, &w->tree4.node)) {
        DIE("remove from gc tree");
    }

    w->in_tree4 = false;
}

static inline void wid_tree5_remove (widp w)
{
    fast_verify(w);

    if (!w->in_tree5) {
        return;
    }

    tree_root *root;

    root = wid_top_level5;

    if (!root) {
        return;
    }

    if (!tree_remove(root, &w->tree5.node)) {
        DIE("remove from tick tree");
    }

    w->in_tree5 = false;
}

/*
 * Initialize a wid with basic settings
 */
static widp wid_new (widp parent)
{
    widp w;

    w = (typeof(w)) myzalloc(sizeof(*w), "widget");
    w->parent = parent;

    wid_tree_insert(w);
    wid_tree2_insert(w);

    /*
     * Give some lame 3d to the wid
     */
    wid_set_mode(w, WID_MODE_NORMAL);
    wid_set_square(w);

    fsize sz = {0.0f, 0.0f};
    wid_set_tex_tl(w, sz);

    fsize sz2 = {1.0f, 1.0f};
    wid_set_tex_br(w, sz2);

    w->visible = true;

    return (w);
}

static void wid_destroy_immediate_internal (widp w)
{
    fast_verify(w);

    wid_tree3_remove(w);
    wid_tree4_remove(w);
    wid_tree5_remove(w);

    if (w->on_destroy) {
        (w->on_destroy)(w);
    }

    if (wid_focus == w) {
        wid_mouse_focus_end();
    }

    if (wid_focus_locked == w) {
        wid_focus_locked = 0;
    }

    if (wid_over == w) {
        wid_mouse_over_end();
    }

    if (wid_moving == w) {
        wid_mouse_motion_end();
    }

    if (w->scrollbar_vert) {
        w->scrollbar_vert->scrollbar_owner = 0;
    }

    if (w->scrollbar_horiz) {
        w->scrollbar_horiz->scrollbar_owner = 0;
    }

    if (w->scrollbar_owner) {
        if (w->scrollbar_owner->scrollbar_vert == w) {
            w->scrollbar_owner->scrollbar_vert = 0;
        }

        if (w->scrollbar_owner->scrollbar_horiz == w) {
            w->scrollbar_owner->scrollbar_horiz = 0;
        }
    }

    tree_destroy(&w->children_unsorted, (tree_destroy_func)0);
    tree_destroy(&w->children_display_sorted,
                 (tree_destroy_func)wid_destroy_immediate_internal);

    /*
     * If on the grid, take us off.
     */
    wid_grid_tree_detach(w);

    /*
     * If a grid owner, destroy all nodes.
     */
    wid_destroy_grid(w);

    if (w->name) {
        myfree(w->name);
        w->name = 0;
    }

    if (w->tooltip) {
        myfree(w->tooltip);
        w->tooltip = 0;
    }

    if (w->logname) {
        myfree(w->logname);
        w->logname = 0;
    }
}

static void wid_destroy_immediate (widp w)
{
    fast_verify(w);

    /*
     * If removing a top level widget, choose a new focus.
     */
    if (!w->parent) {
        wid_find_top_focus();
    }

    wid_tree_detach(w);

    wid_tree2_remove(w);

    wid_destroy_immediate_internal(w);

    myfree(w);
}

static void wid_destroy_delay (widp *wp, int32_t delay)
{
    widp w;
    widp child;
    int32_t tlx;
    int32_t tly;
    int32_t brx;
    int32_t bry;

    if (!wp) {
        return;
    }

    w = *wp;

    if (!w) {
        return;
    }

    fast_verify(w);

    WID_DBG(w, "destroy");

    *wp = 0;

    if (w->being_destroyed) {
        if (delay) {
            return;
        }
    }

    w->being_destroyed = true;
    wid_tree4_insert(w);

    if (wid_focus == w) {
        wid_mouse_focus_end();
    }

    if (wid_over == w) {
        wid_mouse_over_end();
    }

    if (wid_moving == w) {
        wid_mouse_motion_end();
    }

    TREE_OFFSET_WALK(w->children_unsorted, child, tree2) {
        widp c;

        fast_verify(child);
        c = child;

        wid_destroy(&c);
    }

    if (!w->parent) {
        wid_fade_out(w, delay);

        wid_get_abs_coords(w, &tlx, &tly, &brx, &bry);
    }

    wid_grid_tree_detach(w);
}

void wid_destroy (widp *wp)
{
    wid_destroy_delay(wp, wid_destroy_delay_ms);
}

void wid_destroy_nodelay (widp *wp)
{
    wid_destroy_delay(wp, 0);
}

void wid_destroy_in (widp w, uint32_t ms)
{
    fast_verify(w);

    w->destroy_when = time_get_time_cached() + ms;

    wid_tree4_insert(w);
}

/*
 * Initialize a top level wid with basic settings
 */
widp wid_new_window (const char *name)
{
    widp w = wid_new(0);

    w->logname = dynprintf("%s[%p]", name, w);

    WID_DBG(w, "%s", __FUNCTION__);

    wid_set_square(w);
    wid_set_font(w, small_font);
    wid_set_name(w, name);

    color col = BLACK;
    col.a = 0;
    glcolor(col);

    wid_set_mode(w, WID_MODE_NORMAL);
    wid_set_color(w, WID_COLOR_TL, col);
    wid_set_color(w, WID_COLOR_BR, col);
    wid_set_color(w, WID_COLOR_BG, col);
    wid_set_color(w, WID_COLOR_TEXT, WHITE);

    return (w);
}

/*
 * Initialize a top level wid with basic settings
 */
widp wid_new_container (widp parent, const char *name)
{
    widp w = wid_new(parent);

    w->logname = dynprintf("%s[%p] (parent %s[%p])", name, w,
                           parent->logname, parent);

    WID_DBG(w, "%s", __FUNCTION__);

    wid_set_square(w);
    wid_set_font(w, small_font);
    wid_set_name(w, name);

    color col = BLACK;
    col.a = 0;
    glcolor(col);

    wid_set_mode(w, WID_MODE_NORMAL);
    wid_set_color(w, WID_COLOR_TL, col);
    wid_set_color(w, WID_COLOR_BR, col);
    wid_set_color(w, WID_COLOR_BG, col);
    wid_set_color(w, WID_COLOR_TEXT, WHITE);

    return (w);
}

/*
 * Initialize a top level wid with basic settings
 */
widp wid_new_plain (widp parent, const char *name)
{
    widp w = wid_new(parent);

    w->logname = dynprintf("%s[%p] (parent %s[%p])", name, w,
                           parent->logname, parent);
    
    WID_DBG(w, "%s", __FUNCTION__);

    wid_set_no_shape(w);
    wid_set_name(w, name);

    return (w);
}

/*
 * Initialize a top level wid with basic settings
 */
widp wid_new_rounded_window (const char *name)
{
    widp w = wid_new(0);

    w->logname = dynprintf("%s[%p]", name, w);
    WID_DBG(w, "%s", __FUNCTION__);

    wid_set_rounded_large(w);
    wid_set_bevelled(w, true);
    wid_set_bevel(w, 20);
    wid_set_text_outline(w, false);
    wid_set_blit_outline(w, false);
    wid_set_font(w, large_font);
    wid_set_name(w, name);

    color c;
    color tl;
    color br;

    c = WHITE;
    tl = GRAY80;
    br = GRAY90;

    c.a = 255;
    tl.a = 200;
    br.a = 200;

    wid_set_mode(w, WID_MODE_NORMAL);
    wid_set_color(w, WID_COLOR_TL, tl);
    wid_set_color(w, WID_COLOR_BR, br);
    wid_set_color(w, WID_COLOR_BG, c);
    wid_set_color(w, WID_COLOR_TEXT, STEELBLUE);
    wid_set_movable(w, true);

    wid_raise(w);

    return (w);
}

/*
 * Initialize a top level wid with basic settings
 */
widp wid_new_square_window (const char *name)
{
    widp w = wid_new(0);

    w->logname = dynprintf("%s[%p]", name, w);
    WID_DBG(w, "%s", __FUNCTION__);

    wid_set_font(w, large_font);

    color c;
    color tl;
    color br;

    c = WHITE;
    tl = GRAY80;
    br = GRAY90;

    c.a = 255;
    tl.a = 200;
    br.a = 200;

    wid_set_mode(w, WID_MODE_NORMAL);
    wid_set_color(w, WID_COLOR_TL, tl);
    wid_set_color(w, WID_COLOR_BR, br);
    wid_set_color(w, WID_COLOR_BG, c);
    wid_set_color(w, WID_COLOR_TEXT, STEELBLUE);
    wid_set_movable(w, true);
    wid_set_name(w, name);

    wid_raise(w);

    return (w);
}

widp wid_new_square_button (widp parent, const char *name)
{
    widp w = wid_new(parent);

    w->logname = dynprintf("%s[%p] (parent %s[%p])", name, w,
                           parent->logname, parent);

    WID_DBG(w, "%s", __FUNCTION__);

    wid_set_square(w);
    wid_set_bevelled(w, true);
    wid_set_bevel(w, 2);
    wid_set_text_outline(w, false);
    wid_set_blit_outline(w, false);
    wid_set_font(w, small_font);
    wid_set_name(w, name);

    wid_set_mode(w, WID_MODE_NORMAL); {
        color c;

        c = GRAY80;
        c.a = 150;
        wid_set_color(w, WID_COLOR_TL, c);

        c = GRAY40;
        c.a = 200;
        wid_set_color(w, WID_COLOR_BR, c);

        wid_set_color(w, WID_COLOR_TEXT, WHITE);
    }

    wid_set_mode(w, WID_MODE_OVER); {
        wid_set_color(w, WID_COLOR_BG, GRAY80);
    }

    wid_set_mode(w, WID_MODE_FOCUS); {
        color c;

        c = STEELBLUE2;
        c.a = 200;
        wid_set_color(w, WID_COLOR_TL, c);

        c = STEELBLUE3;
        c.a = 200;
        wid_set_color(w, WID_COLOR_BR, c);
    }

    wid_set_mode(w, WID_MODE_ACTIVE); {
        wid_set_color(w, WID_COLOR_BG, STEELBLUE4);
    }

    wid_set_mode(w, WID_MODE_NORMAL); {
        wid_set_color(w, WID_COLOR_BG, WHITE);
    }

    return (w);
}

widp wid_new_rounded_button (widp parent, const char *name)
{
    widp child;

    child = wid_new_square_button(parent, name);

    wid_set_rounded_large(child);
    wid_set_bevel(child, 5);
    wid_set_text_outline(child, true);
    wid_set_blit_outline(child, false);

    return (child);
}

widp wid_new_rounded_small_button (widp parent, const char *name)
{
    widp child;

    child = wid_new_square_button(parent, name);

    wid_set_rounded_large(child);
    wid_set_bevel(child, 3);
    fsize radius = {10.0f, 10.0f};
    wid_set_radius(child, radius);
    wid_set_text_outline(child, true);
    wid_set_blit_outline(child, false);

    return (child);
}

/*
 * Initialize a wid with basic settings
 */
static widp wid_new_scroll_trough (widp parent)
{
    widp w = wid_new(parent);

    w->logname = dynprintf("%s[%p]", "scroll trough", w);
    WID_DBG(w, "%s", __FUNCTION__);

    wid_set_rounded_small(w);
    wid_set_text_outline(w, false);
    wid_set_blit_outline(w, false);

    wid_set_mode(w, WID_MODE_NORMAL); {
        color c;
        color tl;
        color br;

        c = GRAY20;
        c.a = 20;
        tl = GRAY60;
        br = GRAY90;

        wid_set_color(w, WID_COLOR_TL, tl);
        wid_set_color(w, WID_COLOR_BR, br);
        wid_set_color(w, WID_COLOR_BG, c);
    }

    wid_set_on_mouse_down(w, wid_scroll_trough_mouse_down);
    wid_set_on_mouse_motion(w, wid_scroll_trough_mouse_motion);

    return (w);
}

/*
 * Initialize a wid with basic settings
 */
static widp wid_new_scroll_bar (widp parent, widp scrollbar_owner,
                                boolean vertical)
{
    color c;
    color tl;
    color br;

    widp w = wid_new(parent);

    w->logname = dynprintf("%s[%p]", "scroll bar", w);
    WID_DBG(w, "%s", __FUNCTION__);

    wid_set_rounded_small(w);
    wid_set_bevelled(w, true);
    wid_set_bevel(w, 2);
    wid_set_text_outline(w, false);
    wid_set_blit_outline(w, false);

    wid_set_mode(w, WID_MODE_ACTIVE); {
        c = STEELBLUE;
        c.a = 200;
        wid_set_color(w, WID_COLOR_BG, c);

        tl = GRAY70;
        tl.a = 100;
        wid_set_color(w, WID_COLOR_TL, tl);

        br = GRAY10;
        br.a = 100;
        wid_set_color(w, WID_COLOR_BR, br);
    }

    wid_set_mode(w, WID_MODE_NORMAL); {
        c = STEELBLUE;
        c.a = 100;
        wid_set_color(w, WID_COLOR_BG, c);

        tl = GRAY50;
        tl.a = 100;
        wid_set_color(w, WID_COLOR_TL, tl);

        br = GRAY10;
        br.a = 100;
        wid_set_color(w, WID_COLOR_BR, br);
    }

    wid_set_movable(w, true);
    wid_set_movable_bounded(w, true);

    if (vertical) {
        wid_set_movable_vert(w, true);
        wid_set_movable_horiz(w, false);
        scrollbar_owner->scrollbar_vert = w;
    } else {
        wid_set_movable_horiz(w, true);
        wid_set_movable_vert(w, false);
        scrollbar_owner->scrollbar_horiz = w;
    }

    w->scrollbar_owner = scrollbar_owner;

    return (w);
}

widp wid_new_vert_scroll_bar (widp parent, widp scrollbar_owner)
{
    fpoint tl;
    fpoint br;

    int32_t tlx;
    int32_t tly;
    int32_t brx;
    int32_t bry;
    int32_t ptlx;
    int32_t ptly;
    int32_t pbrx;
    int32_t pbry;

    /*
     * Make the trough line up with the scrolling window.
     */
    wid_get_abs_coords(parent, &ptlx, &ptly, &pbrx, &pbry);
    wid_get_abs_coords(scrollbar_owner, &tlx, &tly, &brx, &bry);

    tl.x = tlx - ptlx;
    tl.y = tly - ptly;
    br.x = tl.x + 20;
    br.y = tl.y + wid_get_br_y(scrollbar_owner) - wid_get_tl_y(scrollbar_owner);

    widp trough = wid_new_scroll_trough(parent);
    wid_set_tl_br(trough, tl, br);

    tl.x = 0.0f;
    tl.y = 0.0f;
    br.x = 1.0f;
    br.y = 1.0f;

    widp scrollbar = wid_new_scroll_bar(trough, scrollbar_owner, true);
    wid_set_tl_br_pct(scrollbar, tl, br);

    wid_update_internal(scrollbar);
    wid_hide(scrollbar->parent, 0);

    return (scrollbar);
}

widp wid_new_horiz_scroll_bar (widp parent, widp scrollbar_owner)
{
    fpoint tl;
    fpoint br;

    int32_t tlx;
    int32_t tly;
    int32_t brx;
    int32_t bry;
    int32_t ptlx;
    int32_t ptly;
    int32_t pbrx;
    int32_t pbry;

    /*
     * Make the trough line up with the scrolling window.
     */
    wid_get_abs_coords(parent, &ptlx, &ptly, &pbrx, &pbry);
    wid_get_abs_coords(scrollbar_owner, &tlx, &tly, &brx, &bry);

    tl.x = tlx - ptlx;
    tl.y = tly - ptly;
    br.x = tl.x + wid_get_br_x(scrollbar_owner) - wid_get_tl_x(scrollbar_owner);
    br.y = tl.y + 20;

    widp trough = wid_new_scroll_trough(parent);
    wid_set_tl_br(trough, tl, br);

    tl.x = 0.0f;
    tl.y = 0.0f;
    br.x = 1.0f;
    br.y = 1.0f;

    widp scrollbar = wid_new_scroll_bar(trough, scrollbar_owner, false);
    wid_set_tl_br_pct(scrollbar, tl, br);

    wid_update_internal(scrollbar);
    wid_hide(scrollbar->parent, 0);

    return (scrollbar);
}

boolean wid_has_grid (widp w)
{
    verify(w);

    return (w->grid != 0);
}

void wid_new_grid (widp w, uint32_t width, uint32_t height,
                   uint32_t pixwidth, uint32_t pixheight)
{
    widgrid *grid;
    uint32_t i;

    verify(w);

    if (w->grid) {
        DIE("grid exists");
    }

    grid = w->grid = (widgrid *) myzalloc(sizeof(widgrid), "widgrid ptr");
    grid->width = width;
    grid->height = height;
    grid->pixwidth = pixwidth;
    grid->pixheight = pixheight;
    grid->nelems = width * height;

    grid->trees = (tree_root **)
        myzalloc(sizeof(tree_root *) * grid->nelems, "widgrid tree ptrs");

    for (i = 0; i < width * height; i++) {
        grid->trees[i] =
            tree_alloc_custom(tree_wid_compare_func, "TREE ROOT: widgrid");
    }
}

void wid_destroy_grid (widp w)
{
    widgrid *grid;
    uint32_t i;

    grid = w->grid;
    if (!grid) {
        return;
    }

    w->grid = 0;

    for (i = 0; i < grid->nelems; i++) {
        tree_destroy(&grid->trees[i], 0);
    }

    myfree(grid->trees);
    myfree(grid);
}

void wid_empty_grid (widp w)
{
    widgridnode *node;
    widgrid *grid;
    widp child;
    uint32_t i;

    grid = w->grid;
    if (!grid) {
        return;
    }

    for (i = 0; i < grid->nelems; i++) {
        TREE_WALK(grid->trees[i], node) {
            child = node->wid;

            wid_destroy_nodelay(&child);
        }
    }

    for (i = 0; i < grid->nelems; i++) {
        TREE_WALK(grid->trees[i], node) {
            child = node->wid;

            ERR("%s exists after empty grid",child->name);
        }
    }
}

void wid_detach_from_grid (widp w)
{
    widgridnode *node;
    widgrid *grid;
    widp child;
    uint32_t i;

    grid = w->grid;
    if (!grid) {
        DIE("no grid");
        return;
    }

    for (i = 0; i < grid->nelems; i++) {
        TREE_WALK(grid->trees[i], node) {
            child = node->wid;

            wid_move_stop(child);

            wid_grid_tree_detach(child);
        }
    }

    for (i = 0; i < grid->nelems; i++) {
        TREE_WALK(grid->trees[i], node) {
            child = node->wid;

            DIE("%s exists after clear grid",child->name);
        }
    }
}

static void wid_attach_to_grid_internal (widp w)
{
    widp child;

    { TREE_OFFSET_WALK(w->children_unsorted, child, tree2) {
        wid_attach_to_grid_internal(child);
    } }

    wid_grid_tree_attach(w);

    wid_move_resume(w);
}

void wid_attach_to_grid (widp w)
{
    wid_attach_to_grid_internal(w);
}

void marshal_wid_grid (marshal_p ctx, widp w)
{
    widgridnode *node;
    uint32_t count;
    widgrid *grid;
    widp child;
    uint32_t i;
    uint32_t x;
    uint32_t y;

    grid = w->grid;
    if (!grid) {
        return;
    }

    PUT_BRA(ctx);

    PUT_NAMED_UINT32(ctx, "width", grid->width);
    PUT_NAMED_UINT32(ctx, "height", grid->height);

    for (i = 0; i < grid->nelems; i++) {
        TREE_WALK(grid->trees[i], node) {
            child = node->wid;

            x = i % grid->width;
            y = i / grid->height;

            if (!child->thing_template) {
                continue;
            }

            if (thing_template_is_hidden_from_editor(child->thing_template)) {
                continue;
            }

            PUT_BRA(ctx);

            PUT_NAMED_UINT32(ctx, "x", x);
            PUT_NAMED_UINT32(ctx, "y", y);
            PUT_NAMED_STRING(ctx, "t",
                             thing_template_name(child->thing_template));

            count = (uintptr_t) wid_get_client_context(child);

            if (count > 0) {
                PUT_NAMED_UINT32(ctx, "count", count);
            }

            PUT_KET(ctx);
        }
    }

    PUT_KET(ctx);
}

boolean demarshal_wid_grid (demarshal_p ctx, widp w,
                            grid_wid_replace_t callback)
{
    thing_templatep thing_template;
    uint32_t width;
    uint32_t height;
    widgrid *grid;
    uint32_t x;
    uint32_t y;
    widp child;
    boolean rc;

    rc = true;

    grid = w->grid;
    if (!grid) {
        return (false);
    }

    GET_BRA(ctx);

    GET_NAMED_UINT32(ctx, "width", width);
    GET_NAMED_UINT32(ctx, "height", height);

    for (;;) {
        if (!GET_PEEK_BRA(ctx)) {
            break;
        }

        rc = rc && GET_BRA(ctx);

        char *name;

        rc = rc && GET_NAMED_UINT32(ctx, "x", x);
        rc = rc && GET_NAMED_UINT32(ctx, "y", y);
        rc = rc && GET_NAMED_STRING(ctx, "t", name);

        thing_template = thing_template_find(name);
        if (!thing_template) {
            ERR("thing %s not found", name);
            rc = false;
            continue;
        }

        child = (*callback)(w, x, y, thing_template);

        if (!child) {
            ERR("Loading thing %s failed to replace at (%u,%u)", name, x, y);
            myfree(name);

            rc = false;
            continue;
        }

        myfree(name);

        rc = rc && GET_KET(ctx);

        if (!rc) {
            break;
        }
    }

    GET_KET(ctx);

    return (rc);
}

/*
 * Fast find in a 2d array of trees.
 */
widp wid_grid_find (widp parent, fpoint tl, fpoint br,
                    uint8_t z_depth,
                    uint8_t z_order)
{
    widgridnode *node;
    widgrid *grid;
    widp w;

    grid = parent->grid;
    if (!grid) {
        DIE("no grid wid");
    }

    /*
     * Get the midpoint of the tile and find out which grid cell it
     * resides on.
     */
    uint32_t x = ((tl.x + br.x) / 2) / grid->pixwidth;
    uint32_t y = ((tl.y + br.y) / 2) / grid->pixheight;

    /*
     * This should never happen as wid_grid_tree_attach should have been
     * called.
     */
    if (x >= grid->width) {
        return (0);
    }

    if (y >= grid->height) {
        return (0);
    }

    /*
     * Now find the node in the (hopefully small) tree.
     */
    tree_root **gridtree = grid->trees + (y * grid->width) + x;

    /*
     * Trees should be already allocated.
     */
    if (!*gridtree) {
        DIE("no gridtree");
    }

    TREE_WALK(*gridtree, node) {
        w = node->wid;

        if (w->tree.tl.x != tl.x) {
            continue;
        }

        if (w->tree.tl.y != tl.y) {
            continue;
        }

        if (w->tree.br.x != br.x) {
            continue;
        }

        if (w->tree.br.y != br.y) {
            continue;
        }

        if (w->tree.z_depth != z_depth) {
            continue;
        }

        if (w->tree.z_order != z_order) {
            continue;
        }

        fast_verify(w);
        return (w);
    }

    return (0);
}

/*
 * Fast find in a 2d array of trees.
 */
widp wid_grid_find_thing_template (widp parent,
                                   uint32_t x,
                                   uint32_t y,
                                   thing_template_is_callback func)
{
    widgridnode *node;
    widgrid *grid;
    widp w;

    grid = parent->grid;
    if (!grid) {
        DIE("no grid wid");
    }

    /*
     * This should never happen as wid_grid_tree_attach should have been
     * called.
     */
    if (x >= grid->width) {
        return (0);
    }

    if (y >= grid->height) {
        return (0);
    }

    /*
     * Now find the node in the (hopefully small) tree.
     */
    tree_root **gridtree = grid->trees + (y * grid->width) + x;

    /*
     * Trees should be already allocated.
     */
    if (!*gridtree) {
        DIE("no gridtree");
    }

    TREE_WALK(*gridtree, node) {
        w = node->wid;

        if (!w->thing_template) {
            continue;
        }

        if ((*func)(w->thing_template)) {
            fast_verify(w);

            return (w);
        }
    }

    return (0);
}

/*
 * Fast find in a 2d array of trees.
 */
widp wid_grid_find_thing_template_is (widp parent,
                                      uint32_t x,
                                      uint32_t y,
                                      thing_templatep thing_template)
{
    widgridnode *node;
    widgrid *grid;
    widp w;

    grid = parent->grid;
    if (!grid) {
        DIE("no grid wid");
    }

    /*
     * This should never happen as wid_grid_tree_attach should have been
     * called.
     */
    if (x >= grid->width) {
        return (0);
    }

    if (y >= grid->height) {
        return (0);
    }

    /*
     * Now find the node in the (hopefully small) tree.
     */
    tree_root **gridtree = grid->trees + (y * grid->width) + x;

    /*
     * Trees should be already allocated.
     */
    if (!*gridtree) {
        DIE("no gridtree");
    }

    TREE_WALK(*gridtree, node) {
        w = node->wid;

        if (w->thing_template == thing_template) {
            return (w);
        }
    }

    return (0);
}

/*
 * Find the first widget in the grid at this tile co-ordinate.
 */
widp wid_grid_find_first (widp parent, uint32_t x, uint32_t y)
{
    widgridnode *node;
    widgrid *grid;
    widp w;

    grid = parent->grid;
    if (!grid) {
        DIE("no grid wid");
    }

    if (x >= grid->width) {
        return (0);
    }

    if (y >= grid->height) {
        return (0);
    }

    /*
     * Now find the node in the (hopefully small) tree.
     */
    tree_root **gridtree = grid->trees + (y * grid->width) + x;

    /*
     * Trees should be already allocated.
     */
    if (!*gridtree) {
        DIE("no gridtree");
    }

    node = (typeof(node)) tree_first((*gridtree)->node);
    if (!node) {
        return (0);
    }

    fast_verify(node);

    w = node->wid;
    if (!w) {
        return (0);
    }

    fast_verify(w);
    return (w);
}

/*
 * Find the next widget in the grid at this tile co-ordinate.
 */
widp wid_grid_find_next (widp parent, widp w, uint32_t x, uint32_t y)
{
    widgridnode *node;
    widgrid *grid;

    grid = parent->grid;
    if (!grid) {
        DIE("no grid wid");
    }

    if (x >= grid->width) {
        return (0);
    }

    if (y >= grid->height) {
        return (0);
    }

    for (;;) {
        /*
        * Now find the node in the (hopefully small) tree.
        */
        tree_root **gridtree = grid->trees + (y * grid->width) + x;

        /*
         * Trees should be already allocated.
         */
        if (!*gridtree) {
            DIE("no gridtree");
        }

        node = (typeof(node)) tree_get_next(*gridtree, (*gridtree)->node,
                                            &w->gridnode->tree.node);
        if (!node) {
            return (0);
        }

        fast_verify(node);

        w = node->wid;
        if (!w) {
            return (0);
        }

        if (!w->being_destroyed) {
            break;
        }
    }

    fast_verify(w);

    return (w);
}

/*
 * Fast find in a 2d array of trees of highest z value wid.
 */
widp wid_grid_find_top (widp parent, fpoint tl, fpoint br)
{
    widgridnode *node;
    int32_t z_depth;
    int32_t z_order;
    widgrid *grid;
    widp best;
    widp w;

    grid = parent->grid;
    best = 0;
    z_depth = 0;
    z_order = 0;

    /*
     * Get the midpoint of the tile and find out which grid cell it
     * resides on.
     */
    uint32_t x = ((tl.x + br.x) / 2) / grid->pixwidth;
    uint32_t y = ((tl.y + br.y) / 2) / grid->pixheight;

    /*
     * This should never happen as wid_grid_tree_attach should have been
     * called.
     */
    if (x >= grid->width) {
        return (0);
    }

    if (y >= grid->height) {
        return (0);
    }

    /*
     * Now find the node in the (hopefully small) tree.
     */
    tree_root **gridtree = grid->trees + (y * grid->width) + x;

    /*
     * Trees should be already allocated.
     */
    if (!*gridtree) {
        DIE("no gridtree");
    }

    TREE_WALK(*gridtree, node) {
        w = node->wid;

        thing_templatep t = wid_get_thing_template(w);
        if (!t) {
            continue;
        }

        if (w->tree.z_depth < z_depth) {
            continue;
        }

        if (w->tree.z_order < z_order) {
            continue;
        }

        z_depth = w->tree.z_depth;
        z_order = w->tree.z_depth;
        best = w;
    }

    return (best);
}

widp wid_find_matching (widp parent, fpoint tl, fpoint br,
                        uint8_t z_depth,
                        uint8_t z_order)
{
    widp child;

    fast_verify(parent);

    if (parent->grid) {
        return (wid_grid_find(parent, tl, br, z_depth, z_order));
    }

    TREE_WALK(parent->children_display_sorted, child) {

        if (child->tree.tl.x != tl.x) {
            continue;
        }

        if (child->tree.tl.y != tl.y) {
            continue;
        }

        if (child->tree.br.x != br.x) {
            continue;
        }

        if (child->tree.br.y != br.y) {
            continue;
        }

        if (child->tree.z_depth != z_depth) {
            continue;
        }

        if (child->tree.z_order != z_order) {
            continue;
        }

        return (child);
    }

    return (0);
}

widp wid_find_matching_top (widp parent, fpoint tl, fpoint br)
{
    int32_t z_depth;
    int32_t z_order;
    widp child;
    widp best;

    z_depth = 0;
    z_order = 0;
    best = 0;

    fast_verify(parent);

    if (parent->grid) {
        return (wid_grid_find_top(parent, tl, br));
    }

    TREE_WALK(parent->children_display_sorted, child) {

        thing_templatep t = wid_get_thing_template(child);
        if (!t) {
            continue;
        }

        if (child->tree.tl.x != tl.x) {
            continue;
        }

        if (child->tree.tl.y != tl.y) {
            continue;
        }

        if (child->tree.br.x != br.x) {
            continue;
        }

        if (child->tree.br.y != br.y) {
            continue;
        }

        if (child->tree.z_depth < z_depth) {
            continue;
        }

        if (child->tree.z_order < z_order) {
            continue;
        }

        z_depth = child->tree.z_depth;
        z_order = child->tree.z_order;

        best = child;
    }

    return (best);
}

static void wid_raise_internal (widp w)
{
    if (w->do_not_raise) {
        return;
    }

    if (wid_moving != w) {
        wid_mouse_motion_end();
    }

    if (wid_get_top_parent(wid_moving) != w) {
        wid_mouse_motion_end();
    }

    wid_tree_detach(w);
    w->tree.priority = ++wid_highest_priority;
    wid_tree_attach(w);
}

static void wid_raise_override (widp parent)
{
    /*
     * If some widget wants to be on top, let it.
     */
    widp w;

    if (parent->do_not_lower) {
        wid_raise_internal(parent);
    }

    TREE_WALK(parent->children_display_sorted, w) {
        if (w->do_not_lower) {
            wid_raise_internal(w);
            break;
        }

        wid_raise_override(w);
    }
}

void wid_raise (widp w)
{
    if (!w) {
        return;
    }

    wid_raise_internal(w);

    /*
     * If some widget wants to be on top, let it.
     */
    TREE_WALK(wid_top_level, w) {
        wid_raise_override(w);
    }

    wid_find_top_focus();
}

static void wid_lower_internal (widp w)
{
    if (w->do_not_lower) {
        return;
    }

    if (wid_moving == w) {
        wid_mouse_motion_end();
    }

    if (wid_get_top_parent(wid_moving) == w) {
        wid_mouse_motion_end();
    }

    wid_tree_detach(w);
    w->tree.priority = --wid_lowest_priority;
    wid_tree_attach(w);
}

void wid_lower (widp w)
{
    if (!w) {
        return;
    }

    wid_lower_internal(w);

    /*
     * If some widget wants to be on top, let it.
     */
    TREE_WALK(wid_top_level, w) {
        if (w->do_not_raise) {
            wid_lower_internal(w);
            break;
        }
    }

    wid_find_top_focus();
}

void wid_toggle_hidden (widp w, uint32_t delay)
{
    if (w->hidden) {
        wid_visible(w, delay);
    } else {
        wid_hide(w, delay);
    }
}

static void wid_find_first_child_focus (widp w, widp *best)
{
    if (w->focus_order) {
        if (!*best) {
            *best = w;
        } else if (w->focus_order < (*best)->focus_order) {
            *best = w;
        }
    }

    widp child;

    TREE_WALK(w->children_display_sorted, child) {
        fast_verify(child);

        wid_find_first_child_focus(child, best);
    }
}

static void wid_find_first_focus (void)
{
    widp w;
    widp best;

    TREE_WALK(wid_top_level, w) {
        fast_verify(w);

        if (wid_ignore_for_focus(w)) {
            continue;
        }

        best = 0;
        wid_find_first_child_focus(w, &best);
        if (best) {
            wid_set_focus(best);
            return;
        }
    }
}

static void wid_find_specific_child_focus (widp w, widp *best,
                                           uint8_t focus_order)
{
    if (w->focus_order) {
        if (w->focus_order == focus_order) {
            *best = w;
            return;
        }
    }

    widp child;

    TREE_WALK(w->children_display_sorted, child) {
        fast_verify(child);

        wid_find_specific_child_focus(child, best, focus_order);
    }
}

static widp wid_find_top_wid_focus (widp w)
{
    widp best;

    best = 0;

    if (wid_ignore_for_focus(w)) {
        return (best);
    }

    /*
     * First time we've looked at this widget, hunt for the first focus.
     */
    if (!w->focus_last) {
        wid_find_first_child_focus(w, &best);
        if (best) {
            return (best);
        }
    }

    wid_find_specific_child_focus(w, &best, w->focus_last);
    if (best) {
        return (best);
    }

    return (best);
}

static void wid_find_top_focus (void)
{
    widp best;
    widp w;

    TREE_WALK(wid_top_level, w) {
        fast_verify(w);

        if (wid_ignore_for_focus(w)) {
            continue;
        }

        best = 0;

        /*
         * First time we've looked at this widget, hunt for the first focus.
         */
        if (!w->focus_last) {
            wid_find_first_child_focus(w, &best);
            if (best) {
                wid_set_focus(best);
                return;
            }
        }

        wid_find_specific_child_focus(w, &best, w->focus_last);
        if (best) {
            wid_set_focus(best);
            return;
        }
    }

    wid_find_first_focus();
}

static void wid_find_last_child_focus (widp w, widp *best)
{
    if (w->focus_order) {
        if (!*best) {
            *best = w;
        } else if (w->focus_order > (*best)->focus_order) {
            *best = w;
        }
    }

    widp child;

    TREE_WALK(w->children_display_sorted, child) {
        fast_verify(w);

        wid_find_last_child_focus(child, best);
    }
}

widp wid_get_focus (widp w)
{
    widp best;

    best = 0;

    if (wid_focus) {
        if (wid_get_top_parent(wid_focus) == wid_get_top_parent(w)) {
            return (wid_focus);
        }
    }

    if (!w->focus_last) {
        best = wid_find_top_wid_focus(wid_get_top_parent(w));
        if (best) {
            return (best);
        }
    }

    wid_find_specific_child_focus(w, &best, w->focus_last);

    return (best);
}

static void wid_find_last_focus (void)
{
    widp w;
    widp best;

    TREE_WALK(wid_top_level, w) {
        fast_verify(w);

        if (wid_ignore_for_focus(w)) {
            continue;
        }

        best = 0;
        wid_find_last_child_focus(w, &best);
        if (best) {
            wid_set_focus(best);
            return;
        }
    }
}

static void wid_find_next_child_focus (widp w, widp *best)
{
    if (w->focus_order) {
        if (*best) {
            if ((w->focus_order < (*best)->focus_order) &&
                (w->focus_order > wid_focus->focus_order)) {
                *best = w;
            }
        } else if (w->focus_order > wid_focus->focus_order) {
            *best = w;
        }
    }

    widp child;

    TREE_WALK(w->children_display_sorted, child) {
        fast_verify(child);

        wid_find_next_child_focus(child, best);
    }
}

static void wid_find_next_focus (void)
{
    widp w;
    widp best;

    if (!wid_focus) {
        wid_find_first_focus();
        return;
    }

    best = 0;
    TREE_WALK(wid_top_level, w) {
        fast_verify(w);

        if (wid_ignore_for_focus(w)) {
            continue;
        }

        if (!w->focus_last) {
            continue;
        }

        wid_find_next_child_focus(w, &best);
        if (best) {
            wid_set_focus(best);
            return;
        }

        wid_find_first_focus();
        break;
    }
}

static void wid_find_prev_child_focus (widp w, widp *best)
{
    if (w->focus_order) {
        if (*best) {
            if ((w->focus_order > (*best)->focus_order) &&
                (w->focus_order < wid_focus->focus_order)) {
                *best = w;
            }
        } else if (w->focus_order < wid_focus->focus_order) {
            *best = w;
        }
    }

    widp child;

    TREE_WALK(w->children_display_sorted, child) {
        fast_verify(child);

        wid_find_prev_child_focus(child, best);
    }
}

static void wid_find_prev_focus (void)
{
    widp w;
    widp best;

    if (!wid_focus) {
        wid_find_first_focus();
        return;
    }

    best = 0;
    TREE_WALK(wid_top_level, w) {
        fast_verify(w);

        if (wid_ignore_for_focus(w)) {
            continue;
        }

        if (!w->focus_last) {
            continue;
        }

        wid_find_prev_child_focus(w, &best);
        if (best) {
            wid_set_focus(best);
            return;
        }

        wid_find_last_focus();
        break;
    }
}

widp wid_find (widp w, const char *name)
{
    fast_verify(w);

    if (!w->name) {
        return (0);
    }

    if (!strcasecmp(w->name, name)) {
        return (w);
    }

    widp child;

    TREE_WALK(w->children_display_sorted, child) {
        fast_verify(child);

        widp ret;

        ret = wid_find(child, name);
        if (ret) {
            return (ret);
        }
    }

    return (0);
}

void wid_visible (widp w, uint32_t delay)
{
    fast_verify(w);

    if (!w) {
        return;
    }

    if (!delay) {
        w->visible = true;
    }

    wid_fade_in(w, delay);

    w->hidden = false;

    wid_find_top_focus();
}

void wid_hide (widp w, uint32_t delay)
{
    fast_verify(w);

    if (!w) {
        return;
    }

    if (!delay) {
        w->hidden = true;
    }

    wid_fade_out(w, delay);

    w->visible = false;

    if (wid_over == w) {
        wid_mouse_over_end();
    }

    if (wid_moving == w) {
        wid_mouse_motion_end();
    }

    if (wid_get_top_parent(wid_over) == w) {
        wid_mouse_over_end();
    }

    if (wid_get_top_parent(wid_moving) == w) {
        wid_mouse_motion_end();
    }

    wid_find_top_focus();
}

static boolean wid_scroll_trough_mouse_down (widp w,
                                             int32_t x,
                                             int32_t y,
                                             uint32_t button)
{
    int32_t dx;
    int32_t dy;
    widp child;

    TREE_WALK(w->children_display_sorted, child) {
        fast_verify(child);

        dx = 0;
        dy = 0;

        if (x < wid_get_tl_x(child)) {
            dx = -10;
        }

        if (x > wid_get_tl_x(child)) {
            dx = 10;
        }

        if (y < wid_get_tl_y(child)) {
            dy = -10;
        }

        if (y > wid_get_tl_y(child)) {
            dy = 10;
        }

        if (dx || dy) {
            wid_set_mode(child, WID_MODE_ACTIVE);
        }

        if (!wid_get_movable_horiz(child)) {
            dx = 0;
        }

        if (!wid_get_movable_vert(child)) {
            dy = 0;
        }

        wid_move_delta(child, dx, dy);
    }

    return (true);
}

static boolean wid_scroll_trough_mouse_motion (widp w,
                                               int32_t x, int32_t y,
                                               int32_t relx, int32_t rely,
                                               int32_t wheelx, int32_t wheely)
{
    int32_t dx;
    int32_t dy;
    widp child;

    if ((SDL_BUTTON(SDL_BUTTON_LEFT) & SDL_GetMouseState(0, 0)) ||
        wheely || wheelx) {

        dy = rely ? rely : -wheely;

        dx = relx ? relx : -wheelx;
    } else {
        return (false);
    }

    TREE_WALK(w->children_display_sorted, child) {
        fast_verify(child);

        if (dx || dy) {
            wid_set_mode(child, WID_MODE_ACTIVE);
        }

        if (!wid_get_movable_horiz(child)) {
            dx = 0;
        }

        if (!wid_get_movable_vert(child)) {
            dy = 0;
        }

        wid_move_delta(child, dx, dy);
    }

    return (true);
}

static void wid_adjust_scrollbar (widp scrollbar, widp owner)
{
    double height = wid_get_height(owner);
    double width = wid_get_width(owner);
    double child_height = 0;
    double child_width = 0;
    double scrollbar_width;
    double scrollbar_height;
    double trough_height;
    double trough_width;
    double miny = 0;
    double maxy = 0;
    double minx = 0;
    double maxx = 0;
    double pct;
    boolean first = true;
    widp child;

    /*
     * Find out the space that the children take up then use this to
     * adjust the scrollbar dimensions.
     */
    TREE_OFFSET_WALK_UNSAFE(owner->children_unsorted, child) {

        double tl_x, tl_y, br_x, br_y;
        wid_get_tl_x_tl_y_br_x_br_y(child, &tl_x, &tl_y, &br_x, &br_y);

        double ptl_x, ptl_y, pbr_x, pbr_y;
        wid_get_tl_x_tl_y_br_x_br_y(child->parent, 
                                    &ptl_x, &ptl_y, &pbr_x, &pbr_y);

        double tminx = tl_x - ptl_x;
        double tminy = tl_y - ptl_y;
        double tmaxx = br_x - ptl_x;
        double tmaxy = br_y - ptl_y;

        if (first) {
            minx = tminx;
            miny = tminy;
            maxx = tmaxx;
            maxy = tmaxy;
            first = false;
            continue;
        }

        if (tminx < minx) {
            minx = tminx;
        }

        if (tminy < miny) {
            miny = tminy;
        }

        if (tmaxx > maxx) {
            maxx = tmaxx;
        }

        if (tmaxy > maxy) {
            maxy = tmaxy;
        }
    }

    child_width = maxx - minx;
    child_height = maxy - miny;

    if (child_width < width) {
        maxx = minx + width;
        child_width = maxx - minx;
    }

    if (child_height < height) {
        maxy = miny + height;
        child_height = maxy - miny;
    }

    if (owner->scrollbar_vert) {
        if (wid_get_movable_vert(scrollbar)) {
            trough_height = wid_get_height(owner->scrollbar_vert->parent);
            scrollbar_height = trough_height * (height / child_height);

            if (trough_height - scrollbar_height == 0.0f) {
                pct = 0.0f;
            } else {
                pct = (wid_get_tl_y(scrollbar) - wid_get_tl_y(scrollbar->parent)) /
                        (trough_height - scrollbar_height);
            }

            owner->offset.y = -miny;
            owner->offset.y -= (pct * (child_height - height));

            scrollbar->tree.tl.y =
                wid_get_tl_y(scrollbar->parent) +
                pct * (trough_height - scrollbar_height);

            wid_tree_detach(scrollbar);
            scrollbar->tree.br.y = wid_get_tl_y(scrollbar) + scrollbar_height;
            wid_tree_attach(scrollbar);

            wid_set_mode(scrollbar, WID_MODE_ACTIVE);
        }
    }

    if (owner->scrollbar_horiz) {
        if (wid_get_movable_horiz(scrollbar)) {
            trough_width = wid_get_width(owner->scrollbar_horiz->parent);
            scrollbar_width = trough_width * (width / child_width);

            if (trough_width - scrollbar_width == 0.0f) {
                pct = 0.0f;
            } else {
                pct = (wid_get_tl_x(scrollbar) - wid_get_tl_x(scrollbar->parent)) /
                        (trough_width - scrollbar_width);
            }

            owner->offset.x = -minx;
            owner->offset.x -= (pct * (child_width - width));

            scrollbar->tree.tl.x =
                wid_get_tl_x(scrollbar->parent) +
                pct * (trough_width - scrollbar_width);

            wid_tree_detach(scrollbar);
            scrollbar->tree.br.x = wid_get_tl_x(scrollbar) + scrollbar_width;
            wid_tree_attach(scrollbar);

            wid_set_mode(scrollbar, WID_MODE_ACTIVE);
        }
    }
}

void wid_get_children_size (widp owner, double *w, double *h)
{
    double height = wid_get_height(owner);
    double width = wid_get_width(owner);
    double child_height = 0;
    double child_width = 0;
    double miny = 0;
    double maxy = 0;
    double minx = 0;
    double maxx = 0;
    boolean first = true;
    widp child;

    /*
     * Find out the space that the children take up then use this to
     * adjust the scrollbar dimensions.
     */
    TREE_WALK(owner->children_display_sorted, child) {
        fast_verify(child);

        double tminx = wid_get_tl_x(child) - wid_get_tl_x(child->parent);
        double tminy = wid_get_tl_y(child) - wid_get_tl_y(child->parent);
        double tmaxx = wid_get_br_x(child) - wid_get_tl_x(child->parent);
        double tmaxy = wid_get_br_y(child) - wid_get_tl_y(child->parent);

        if (first) {
            minx = tminx;
            miny = tminy;
            maxx = tmaxx;
            maxy = tmaxy;
            first = false;
            continue;
        }

        if (tminx < minx) {
            minx = tminx;
        }

        if (tminy < miny) {
            miny = tminy;
        }

        if (tmaxx > maxx) {
            maxx = tmaxx;
        }

        if (tmaxy > maxy) {
            maxy = tmaxy;
        }
    }

    child_width = maxx - minx;
    child_height = maxy - miny;

    if (child_width < width) {
        maxx = minx + width;
        child_width = maxx - minx;
    }

    if (child_height < height) {
        maxy = miny + height;
        child_height = maxy - miny;
    }

    if (w) {
        *w = child_width;
    }

    if (h) {
        *h = child_height;
    }
}

static void wid_update_internal (widp w)
{
    fast_verify(w);

    int32_t tlx;
    int32_t tly;
    int32_t brx;
    int32_t bry;

    wid_get_abs_coords(w, &tlx, &tly, &brx, &bry);

    /*
     * First time around, initialize the wid.
     */
    if (!w->first_update) {
        w->first_update = true;

        if (!w->parent) {
            /*
             * Find the focus.
             */
            wid_find_top_focus();
        }

        /*
         * Set back to normal to undo any settings when creating.
         */
        wid_set_mode(w, WID_MODE_NORMAL);
    }

    widp child;

    /*
     * Clip all the children
     */
    TREE_OFFSET_WALK_UNSAFE(w->children_unsorted, child) {
        wid_update_internal(child);
    }

    /*
     * If the source of the event is the scrollbars themselves...
     */
    if (w->scrollbar_owner) {
        fast_verify(w);
        wid_adjust_scrollbar(w, w->scrollbar_owner);

        wid_update_internal(w->scrollbar_owner);
    } else {
        /*
         * If the source of the event is the owner of the scrollbars...
         */
        if (w->scrollbar_vert) {
            fast_verify(w);
            wid_adjust_scrollbar(w->scrollbar_vert, w);
        }

        if (w->scrollbar_horiz) {
            fast_verify(w);
            wid_adjust_scrollbar(w->scrollbar_horiz, w);
        }
    }

    wid_grid_tree_attach(w);
}

void wid_update (widp w)
{
    fast_verify(w);

    wid_update_internal(w);
}

void wid_update_mouse (void)
{
    /*
     * So if we are now over a new widget that was created on top of the 
     * mouse, we activate it.
     */
    int32_t x;
    int32_t y;

    SDL_GetMouseState(&x, &y);

    x *= global_config.xscale;
    y *= global_config.yscale;

    wid_mouse_motion(x, y, 0, 0, 0, 0);
}

void wid_scroll_text (widp w)
{
    const char *s;
    widp prev;
    widp tmp;

    /*
     * Get the wid on the top of the list/screen.
     */
    tmp = wid_get_tail(w);

    /*
     * Now copy the text up to the parent widgets.
     */
    while (tmp) {
        prev = wid_get_prev(tmp);

        if (prev) {
            s = wid_get_text(prev);

            wid_set_text(tmp, s);
        }

        tmp = prev;
    }
}

/*
 * Replace the 2nd last line of text and scroll. The assumption is the last
 * line is the input line.
 */
void wid_scroll_with_input (widp w, const char *str)
{
    widp tmp;

    wid_scroll_text(w);

    /*
     * Get the wid on the bottom of the list/screen.
     */
    tmp = wid_get_head(w);

    /*
     * Now get the 2nd last line. The last line is the input. The 2nd last
     * line is where new output goes.
     */
    if (tmp) {
        tmp = wid_get_next(tmp);
        if (tmp) {
            wid_set_text(tmp, str);
        }
    }
}

boolean wid_receive_input (widp w, const SDL_KEYSYM *key)
{
    char beforecursor[MAXSTR];
    char updatedtext[MAXSTR];
    char aftercursor[MAXSTR];
    char entered[MAXSTR];
    char tmp[MAXSTR];
    const char *origtext;
    char newchar[2];
    uint32_t origlen;
    uint32_t cnt;

    newchar[0] = '\0';
    newchar[0] = wid_event_to_char(key);

    origtext = wid_get_text(w);
    origlen = (uint32_t)strlen(origtext);

    if (!w->received_input) {
        wid_set_received_input(w, true);
        w->cursor = (uint32_t)strlen(origtext);
    }

    strlcpy(beforecursor, origtext, w->cursor + 1);
    strlcpy(aftercursor, origtext + w->cursor, sizeof(aftercursor));

    switch (key->mod) {
        case KMOD_LCTRL:
        case KMOD_RCTRL:
            switch (wid_event_to_char(key)) {
            case 'p':
                if (!history_walk) {
                    history_walk = HISTORY_MAX - 1;
                } else {
                    history_walk--;
                }

                wid_set_text(w, history[history_walk]);
                w->cursor = (uint32_t)strlen(wid_get_text(w));;
                break;

            case 'n':
                history_walk++;
                if (history_walk >= HISTORY_MAX) {
                    history_walk = 0;
                }

                wid_set_text(w, history[history_walk]);
                w->cursor = (uint32_t)strlen(wid_get_text(w));;
                break;

            case 'a':
                w->cursor = 0;
                break;

            case 'e':
                w->cursor = origlen;
                break;
            }
            break;

        default:

        switch (key->sym) {
            case SDLK_BACKSPACE:
                if (w->cursor > 0) {
                    strlcpy(updatedtext, beforecursor, w->cursor);
                    strlcat(updatedtext, aftercursor, sizeof(updatedtext));

                    w->cursor--;

                    wid_set_text(w, updatedtext);
                }
                break;

            case SDLK_TAB:
                if (w != wid_console_input_line) {
                    return (true);
                }

                updatedtext[0] = '\0';
                command_handle(wid_get_text(w), updatedtext,
                               false /* show ambiguous */,
                               true /* show complete */,
                               false /* execute command */,
                               0 /* context */);

                if (updatedtext[0]) {
                    wid_set_text(w, updatedtext);
                    w->cursor = (uint32_t)strlen(updatedtext);;
                }
                return (true);

            case SDLK_RETURN:
                if (w != wid_console_input_line) {
                    return (false);
                }

                if (origlen) {
                    strlcpy(entered, wid_get_text(w), sizeof(entered));
                    wid_scroll_text(w);
                    snprintf(tmp, sizeof(tmp), "> %s", entered);
                    wid_set_text(w->next, tmp);

                    if (!command_handle(entered, updatedtext,
                                    true /* show ambiguous */,
                                    false /* show complete */,
                                    true /* execute command */,
                                    0 /* context */)) {
                        return (true);
                    }

                    if (updatedtext[0]) {
                        wid_set_text(w, updatedtext);
                        w->cursor = (uint32_t)strlen(updatedtext);;
                    }

                    strlcpy(history[history_at], updatedtext,
                            sizeof(history[history_at]));

                    history_at++;
                    if (history_at >= HISTORY_MAX) {
                        history_at = 0;
                    }
                    history_walk = history_at;

                    wid_set_text(w, "");
                    w->cursor = 0;
                } else {
                    wid_scroll_text(w);
                }
                return (true);

            case SDLK_LEFT:
                if (w->cursor > 0) {
                    w->cursor--;
                }
                break;

            case SDLK_RIGHT:
                if (w->cursor < origlen) {
                    w->cursor++;
                }
                break;

            case SDLK_UP:
                cnt = 0;
                while (cnt < HISTORY_MAX) {
                    cnt++;
                    if (!history_walk) {
                        history_walk = HISTORY_MAX - 1;
                    } else {
                        history_walk--;
                    }

                    wid_set_text(w, history[history_walk]);
                    if (!history[history_walk][0]) {
                        continue;
                    }

                    w->cursor = (uint32_t)strlen(wid_get_text(w));;
                    break;
                }
                break;

            case SDLK_DOWN:
                cnt = 0;
                while (cnt < HISTORY_MAX) {
                    cnt++;

                    history_walk++;
                    if (history_walk >= HISTORY_MAX) {
                        history_walk = 0;
                    }

                    wid_set_text(w, history[history_walk]);
                    if (!history[history_walk][0]) {
                        continue;
                    }

                    w->cursor = (uint32_t)strlen(wid_get_text(w));;
                    break;
                }
                break;

            case SDLK_HOME:
                w->cursor = 0;
                break;

            case SDLK_END:
                w->cursor = origlen;
                break;

            default: {
                char c;

                if (origlen >= sizeof(updatedtext) - 1) {
                    break;
                }

                c = wid_event_to_char(key);

                switch (c) {
                case SDLK_ESCAPE:
                    if (w != wid_console_input_line) {
                        break;
                    }

                case '`':
                case '~':

                    /*
                     * Magic keys we use to toggle the console.
                     */
                    return (false);

                case '?':
                    if (w != wid_console_input_line) {
                        break;
                    }

                    updatedtext[0] = '\0';
                    command_handle(wid_get_text(w), updatedtext,
                                   true /* show ambiguous */,
                                   false /* show complete */,
                                   false /* execute command */,
                                   0 /* context */);

                    if (updatedtext[0]) {
                        wid_set_text(w, updatedtext);
                        w->cursor = (uint32_t)strlen(updatedtext);;
                    }
                    return (true);
                }

                newchar[1] = '\0';
                newchar[0] = c;
                if (!newchar[0]) {
                    break;
                }

                strlcpy(updatedtext, beforecursor, w->cursor + 1);
                strlcat(updatedtext, newchar, sizeof(updatedtext));
                strlcat(updatedtext, aftercursor, sizeof(updatedtext));

                w->cursor++;

                wid_set_text(w, updatedtext);
            }
        }
    }

    return (true);
}

/*
 * Handle keys no one grabbed.
 */
static boolean wid_receive_unhandled_input (const SDL_KEYSYM *key)
{
    widp w;

    w = wid_get_top_parent(wid_console_input_line);

    switch (key->mod) {
        default:

        switch ((int32_t)key->sym) {
            case '`':
            case '~':
                wid_toggle_hidden(w, 0);
                wid_raise(w);
                break;

            case SDLK_ESCAPE:
                if (w->visible) {
                    wid_hide(w, wid_hide_delay);
                }
                break;

            case SDLK_TAB:
            case SDLK_RETURN:
            case SDLK_DOWN:
            case SDLK_RIGHT:
                wid_find_next_focus();
                break;

            case SDLK_UP:
            case SDLK_LEFT:
                wid_find_prev_focus();
                break;

            default: {
                break;
            }
        }
    }

    return (true);
}

static widp wid_find_at (widp w, int32_t x, int32_t y)
{
    widp child;

    if (wid_ignore_being_destroyed(w)) {
        return (0);
    }

    if ((x < w->abs_tl.x) ||
        (y < w->abs_tl.y) ||
        (x > w->abs_br.x) ||
        (y > w->abs_br.y)) {
        return (0);
    }

    TREE_WALK(w->children_display_sorted, child) {
        fast_verify(child);

        widp closer_match = wid_find_at(child, x, y);
        if (closer_match) {
            return (closer_match);
        }
    }

    return (w);
}

static widp wid_key_down_handler_at (widp w, int32_t x, int32_t y,
                                     boolean strict)
{
    widp child;

    if (!w) {
        return (0);
    }

    if (wid_ignore_for_events(w)) {
        return (0);
    }

    if (strict) {
        if ((x < w->abs_tl.x) ||
            (y < w->abs_tl.y) ||
            (x > w->abs_br.x) ||
            (y > w->abs_br.y)) {
            return (0);
        }
    }

    TREE_WALK(w->children_display_sorted, child) {
        fast_verify(child);

        if (wid_focus_locked &&
            (wid_get_top_parent(child) != wid_get_top_parent(wid_focus_locked))) {
            continue;
        }

        widp closer_match = wid_key_down_handler_at(child, x, y,
                                                    true /* strict */);
        if (closer_match) {
            return (closer_match);
        }
    }

    { TREE_WALK(w->children_display_sorted, child) {
        fast_verify(child);

        if (wid_focus_locked &&
            (wid_get_top_parent(child) != wid_get_top_parent(wid_focus_locked))) {
            continue;
        }

        widp closer_match = wid_key_down_handler_at(child, x, y,
                                                    false /* strict */);
        if (closer_match) {
            return (closer_match);
        }
    } }

    if (w->on_key_down) {

        if (wid_focus_locked &&
            (wid_get_top_parent(w) != wid_get_top_parent(wid_focus_locked))) {
            return (0);
        }

        return (w);
    }

    return (0);
}

static widp wid_key_up_handler_at (widp w, int32_t x, int32_t y,
                                     boolean strict)
{
    widp child;

    if (!w) {
        return (0);
    }

    if (wid_ignore_for_events(w)) {
        return (0);
    }

    if (strict) {
        if ((x < w->abs_tl.x) ||
            (y < w->abs_tl.y) ||
            (x > w->abs_br.x) ||
            (y > w->abs_br.y)) {
            return (0);
        }
    }

    TREE_WALK(w->children_display_sorted, child) {
        fast_verify(child);

        if (wid_focus_locked &&
            (wid_get_top_parent(child) != wid_get_top_parent(wid_focus_locked))) {
            continue;
        }

        widp closer_match = wid_key_up_handler_at(child, x, y,
                                                    true /* strict */);
        if (closer_match) {
            return (closer_match);
        }
    }

    { TREE_WALK(w->children_display_sorted, child) {
        fast_verify(child);

        if (wid_focus_locked &&
            (wid_get_top_parent(child) != wid_get_top_parent(wid_focus_locked))) {
            continue;
        }

        widp closer_match = wid_key_up_handler_at(child, x, y,
                                                    false /* strict */);
        if (closer_match) {
            return (closer_match);
        }
    } }

    if (w->on_key_up) {
        if (wid_focus_locked &&
            (wid_get_top_parent(w) != wid_get_top_parent(wid_focus_locked))) {
            return (0);
        }

        return (w);
    }

    return (0);
}

static widp wid_mouse_down_handler_at (widp w, int32_t x, int32_t y,
                                       boolean strict)
{
    widp child;

    if (!w) {
        return (0);
    }

    if (wid_ignore_for_events(w)) {
        return (0);
    }

    if (strict) {
        if ((x < w->abs_tl.x) ||
            (y < w->abs_tl.y) ||
            (x > w->abs_br.x) ||
            (y > w->abs_br.y)) {
            return (0);
        }
    }

    TREE_WALK(w->children_display_sorted, child) {
        fast_verify(child);

        if (wid_focus_locked &&
            (wid_get_top_parent(child) != wid_get_top_parent(wid_focus_locked))) {
            continue;
        }

        widp closer_match = wid_mouse_down_handler_at(child, x, y,
                                                      true /* strict */);
        if (closer_match) {
            return (closer_match);
        }
    }

    if (w->on_mouse_down) {
        if (wid_focus_locked &&
            (wid_get_top_parent(w) != wid_get_top_parent(wid_focus_locked))) {
            return (0);
        }

        return (w);
    }

    if (wid_get_movable(w)) {
        if (wid_focus_locked &&
            (wid_get_top_parent(w) != wid_get_top_parent(wid_focus_locked))) {
            return (0);
        }

        return (w);
    }

    /*
     * Prevent mouse events that occur in the bounds of one window, leaking
     * into lower levels.
     */
    if (!w->parent) {
        if (wid_focus_locked &&
            (wid_get_top_parent(w) != wid_get_top_parent(wid_focus_locked))) {
            return (0);
        }

        return (w);
    }

    return (0);
}

static widp wid_mouse_up_handler_at (widp w, int32_t x, int32_t y, boolean strict)
{
    widp child;

    if (!w) {
        return (0);
    }

    if (wid_ignore_for_events(w)) {
        return (0);
    }

    if (strict) {
        if ((x < w->abs_tl.x) ||
            (y < w->abs_tl.y) ||
            (x > w->abs_br.x) ||
            (y > w->abs_br.y)) {
            return (0);
        }
    }

    TREE_WALK(w->children_display_sorted, child) {
        fast_verify(child);

        if (wid_focus_locked &&
            (wid_get_top_parent(child) != wid_get_top_parent(wid_focus_locked))) {
            continue;
        }

        widp closer_match = wid_mouse_up_handler_at(child, x, y,
                                                      true /* strict */);
        if (closer_match) {
            return (closer_match);
        }
    }

    if (w->on_mouse_up) {
        if (wid_focus_locked &&
            (wid_get_top_parent(w) != wid_get_top_parent(wid_focus_locked))) {
            return (0);
        }

        return (w);
    }

    if (wid_get_movable(w)) {
        if (wid_focus_locked &&
            (wid_get_top_parent(w) != wid_get_top_parent(wid_focus_locked))) {
            return (0);
        }

        return (w);
    }

    /*
     * Prevent mouse events that occur in the bounds of one window, leaking
     * into lower levels.
     */
    if (!w->parent) {
        if (wid_focus_locked &&
            (wid_get_top_parent(w) != wid_get_top_parent(wid_focus_locked))) {
            return (0);
        }

        return (w);
    }

    return (0);
}

static widp wid_mouse_motion_handler_at (widp w, int32_t x, int32_t y,
                                         int32_t relx, int32_t rely,
                                         int32_t wheelx, int32_t wheely,
                                         boolean strict)
{
    widp child;

    if (!w) {
        return (0);
    }

    if (wid_ignore_for_events(w)) {
        return (0);
    }

    if (strict) {
        if ((x < w->abs_tl.x) ||
            (y < w->abs_tl.y) ||
            (x > w->abs_br.x) ||
            (y > w->abs_br.y)) {
            return (0);
        }
    }

    TREE_WALK(w->children_display_sorted, child) {
        fast_verify(child);

        if (wid_focus_locked &&
            (wid_get_top_parent(child) != wid_get_top_parent(wid_focus_locked))) {
            continue;
        }

        widp closer_match =
                wid_mouse_motion_handler_at(child, x, y,
                                            relx, rely,
                                            wheelx, wheely,
                                            true /* strict */);
        if (closer_match) {
            return (closer_match);
        }
    }

    /*
     * If there is a mouse handler, or a scrollbar attached, they will be
     * interested in the mouse move.
     */
    if (!w->on_mouse_over_begin) {
        if (w->cfg[WID_MODE_OVER].font ||
            w->cfg[WID_MODE_OVER].color_set[WID_COLOR_TL] ||
            w->cfg[WID_MODE_OVER].color_set[WID_COLOR_BR] ||
            w->cfg[WID_MODE_OVER].color_set[WID_COLOR_BG] ||
            w->cfg[WID_MODE_OVER].color_set[WID_COLOR_TEXT]) {
            /*
             * Changes appearance on mouse over, so choose this wid even
             * if it has no over callback.
             */
            return (w);
        }
    }

    if (w->on_mouse_motion || w->on_mouse_over_begin) {
        if (wid_focus_locked) {
            if (wid_get_top_parent(w) == wid_get_top_parent(wid_focus_locked)) {
                return (w);
            }
        } else {
            return (w);
        }
    }

    if (w->tooltip) {
        return (w);
    }

    if (wheely) {
        if (w->scrollbar_vert) {
            return (w);
        }
    }

    if (wheelx) {
        if (w->scrollbar_horiz) {
            return (w);
        }
    }

    if (!strict) {
        if (wid_focus_locked) {
            if (wid_get_top_parent(w) == wid_get_top_parent(wid_focus_locked)) {
                return (w);
            }
        } else {
            return (w);
        }
    }

    /*
     * Prevent mouse events that occur in the bounds of one window, leaking
     * into lower levels.
     */
    if (!w->parent) {
        if (wid_focus_locked) {
            if (wid_get_top_parent(w) == wid_get_top_parent(wid_focus_locked)) {
                return (w);
            }
        } else {
            return (w);
        }
    }

    return (0);
}

static void wid_children_move_delta_internal (widp w, double dx, double dy)
{
    /*
     * Make sure you can't move a wid outside the parents box.
     */
    widp p = w->parent;
    if (p) {
        if (wid_get_movable_bounded(w)) {
            if (wid_get_tl_x(w) + dx < wid_get_tl_x(p)) {
                dx = wid_get_tl_x(p) - wid_get_tl_x(w);
            }

            if (wid_get_tl_y(w) + dy < wid_get_tl_y(p)) {
                dy = wid_get_tl_y(p) - wid_get_tl_y(w);
            }

            if (wid_get_br_x(w) + dx > wid_get_br_x(p)) {
                dx = wid_get_br_x(p) - wid_get_br_x(w);
            }

            if (wid_get_br_y(w) + dy > wid_get_br_y(p)) {
                dy = wid_get_br_y(p) - wid_get_br_y(w);
            }
        }
    }

    w->tree.tl.x += dx;
    w->tree.tl.y += dy;
    w->tree.br.x += dx;
    w->tree.br.y += dy;

    widp child;

    { TREE_OFFSET_WALK_UNSAFE(w->children_unsorted, child) {
        wid_children_move_delta_internal(child, dx, dy);
    } }
}

static void wid_move_delta_internal (widp w, double dx, double dy)
{
    wid_tree_detach(w);

    /*
     * Make sure you can't move a wid outside the parents box.
     */
    widp p = w->parent;
    if (p) {
        if (wid_get_movable_bounded(w)) {
            if (wid_get_tl_x(w) + dx < wid_get_tl_x(p)) {
                dx = wid_get_tl_x(p) - wid_get_tl_x(w);
            }

            if (wid_get_tl_y(w) + dy < wid_get_tl_y(p)) {
                dy = wid_get_tl_y(p) - wid_get_tl_y(w);
            }

            if (wid_get_br_x(w) + dx > wid_get_br_x(p)) {
                dx = wid_get_br_x(p) - wid_get_br_x(w);
            }

            if (wid_get_br_y(w) + dy > wid_get_br_y(p)) {
                dy = wid_get_br_y(p) - wid_get_br_y(w);
            }
        }
    }

    w->tree.tl.x += dx;
    w->tree.tl.y += dy;
    w->tree.br.x += dx;
    w->tree.br.y += dy;

    widp child;

    { TREE_OFFSET_WALK_UNSAFE(w->children_unsorted, child) {
        wid_children_move_delta_internal(child, dx, dy);
    } }

    wid_tree_attach(w);
}

void wid_move_delta (widp w, double dx, double dy)
{
    if ((dx == 0.0) && (dy == 0.0)) {
        return;
    }

    wid_move_delta_internal(w, dx, dy);

    wid_update_internal(w);
}

void wid_move_to_bottom (widp w)
{
    wid_move_delta(w, 0, wid_get_br_y(w->parent) - wid_get_br_y(w));
}

void wid_move_to_left (widp w)
{
    wid_move_delta(w, wid_get_tl_x(w->parent) - wid_get_tl_x(w), 0);
}

void wid_move_to_right (widp w)
{
    wid_move_delta(w, wid_get_br_x(w->parent) - wid_get_br_x(w), 0);
}

void wid_move_to_vert_pct (widp w, double pct)
{
    double pheight = wid_get_br_y(w->parent) - wid_get_tl_y(w->parent);
    double at = (wid_get_tl_y(w) - wid_get_tl_y(w->parent)) / pheight;
    double delta = (pct - at) * pheight;
    
    wid_move_delta(w, 0, delta);
}

void wid_move_to_horiz_pct (widp w, double pct)
{
    double pwidth = wid_get_br_x(w->parent) - wid_get_tl_x(w->parent);
    double at = (wid_get_tl_x(w) - wid_get_tl_x(w->parent)) / pwidth;
    double delta = (pct - at) * pwidth;
    
    wid_move_delta(w, delta, 0);
}

void wid_move_to_vert_pct_in (widp w, double pct, double in)
{
    if (pct < 0.0) {
        pct = 0.0;
    }

    if (pct > 1.0) {
        pct = 1.0;
    }

    double pheight = wid_get_br_y(w->parent) - wid_get_tl_y(w->parent);
    double at = (wid_get_tl_y(w) - wid_get_tl_y(w->parent)) / pheight;
    double delta = (pct - at) * pheight;
    
    wid_move_to_abs_in(w, wid_get_tl_x(w), wid_get_tl_y(w) + delta, in);
}

void wid_move_to_horiz_pct_in (widp w, double pct, double in)
{
    if (pct < 0.0) {
        pct = 0.0;
    }

    if (pct > 1.0) {
        pct = 1.0;
    }

    double pwidth = wid_get_br_x(w->parent) - wid_get_tl_x(w->parent);
    double at = (wid_get_tl_x(w) - wid_get_tl_x(w->parent)) / pwidth;
    double delta = (pct - at) * pwidth;
    
    wid_move_to_abs_in(w, wid_get_tl_x(w) + delta, wid_get_tl_y(w), in);
}

void wid_move_to_top (widp w)
{
    wid_move_delta(w, 0, wid_get_tl_y(w->parent) - wid_get_tl_y(w));
}

static widp wid_mouse_down_handler (int32_t x, int32_t y)
{
    widp w;

    w = wid_mouse_down_handler_at(wid_focus, x, y, true /* strict */);
    if (w) {
        return (w);
    }

    w = wid_mouse_down_handler_at(wid_over, x, y, true /* strict */);
    if (w) {
        return (w);
    }

    { TREE_WALK(wid_top_level, w) {
        fast_verify(w);

        if (wid_focus_locked &&
            (wid_get_top_parent(w) != wid_get_top_parent(wid_focus_locked))) {
            continue;
        }

        w = wid_mouse_down_handler_at(w, x, y, true /* strict */);
        if (!w) {
            continue;
        }

        return (w);
    } }

    { TREE_WALK(wid_top_level, w) {
        fast_verify(w);

        if (wid_focus_locked &&
            (wid_get_top_parent(w) != wid_get_top_parent(wid_focus_locked))) {
            continue;
        }

        w = wid_mouse_down_handler_at(w, x, y, false /* strict */);
        if (!w) {
            continue;
        }

        return (w);
    } }

    return (0);
}

static widp wid_mouse_up_handler (int32_t x, int32_t y)
{
    widp w;

    w = wid_mouse_up_handler_at(wid_focus, x, y, true /* strict */);
    if (w) {
        return (w);
    }

    w = wid_mouse_up_handler_at(wid_over, x, y, true /* strict */);
    if (w) {
        return (w);
    }

    TREE_WALK(wid_top_level, w) {
        fast_verify(w);

        if (wid_focus_locked &&
            (wid_get_top_parent(w) != wid_get_top_parent(wid_focus_locked))) {
            continue;
        }

        w = wid_mouse_up_handler_at(w, x, y, true /* strict */);
        if (!w) {
            continue;
        }

        return (w);
    }

    { TREE_WALK(wid_top_level, w) {
        fast_verify(w);

        if (wid_focus_locked &&
            (wid_get_top_parent(w) != wid_get_top_parent(wid_focus_locked))) {
            continue;
        }

        w = wid_mouse_up_handler_at(w, x, y, false /* strict */);
        if (!w) {
            continue;
        }

        return (w);
    } }

    return (0);
}

static widp wid_mouse_motion_handler (int32_t x, int32_t y,
                                      int32_t relx, int32_t rely,
                                      int32_t wheelx, int32_t wheely)
{
    widp w;

    w = wid_mouse_motion_handler_at(wid_focus, x, y,
                                    relx, rely,
                                    wheelx, wheely,
                                    true /* strict */);
    if (w) {
        return (w);
    }

    w = wid_mouse_motion_handler_at(wid_over, x, y,
                                    relx, rely,
                                    wheelx, wheely,
                                    true /* strict */);
    if (w) {
        return (w);
    }

    TREE_WALK(wid_top_level, w) {
        fast_verify(w);

        if (wid_focus_locked &&
            (wid_get_top_parent(w) != wid_get_top_parent(wid_focus_locked))) {
            continue;
        }

        w = wid_mouse_motion_handler_at(w, x, y,
                                        relx, rely,
                                        wheelx, wheely,
                                        true /* strict */);
        if (!w) {
            continue;
        }

        return (w);
    }

    { TREE_WALK(wid_top_level, w) {
        fast_verify(w);

        if (wid_focus_locked &&
            (wid_get_top_parent(w) != wid_get_top_parent(wid_focus_locked))) {
            continue;
        }

        w = wid_mouse_motion_handler_at(w, x, y,
                                        relx, rely,
                                        wheelx, wheely,
                                        false /* strict */);
        if (!w) {
            continue;
        }

        return (w);
    } }

    return (0);
}

void wid_mouse_motion (int32_t x, int32_t y,
                       int32_t relx, int32_t rely,
                       int32_t wheelx, int32_t wheely)
{
    widp w;

    if (wid_moving) {
        int32_t dx = x - wid_moving_last_x;
        int32_t dy = y - wid_moving_last_y;

        if (!wid_get_movable_horiz(wid_moving)) {
            dx = 0;
        }

        if (!wid_get_movable_vert(wid_moving)) {
            dy = 0;
        }

        wid_moving_last_x = x;
        wid_moving_last_y = y;

        /*
         * If beneath the widgets bound and moving up then ignore.
         */
        if ((y > wid_get_br_y(wid_moving)) && (dy < 0)) {
            dy = 0;
        }

        /*
         * If above the widgets bound and moving down then ignore.
         */
        if ((y < wid_get_tl_y(wid_moving)) && (dy > 0)) {
            dy = 0;
        }

        /*
         * Same for left/right.
         */
        if ((x > wid_get_br_x(wid_moving)) && (dx < 0)) {
            dx = 0;
        }

        if ((x < wid_get_tl_x(wid_moving)) && (dx > 0)) {
            dx = 0;
        }

        wid_move_delta(wid_moving, dx, dy);
        return;
    }

    boolean over = false;

    TREE_WALK(wid_top_level, w) {
        fast_verify(w);

        if (wid_focus_locked &&
            (wid_get_top_parent(w) != wid_get_top_parent(wid_focus_locked))) {
            continue;
        }

        w = wid_find_at(w, x, y);
        if (!w) {
            continue;
        }

        if (wid_ignore_for_events(w)) {
            /*
             * This wid is ignoring events, but what about the parent?
             */
            w = w->parent;
            while (w) {
                if (!wid_ignore_for_events(w)) {
                    break;
                }
                w = w->parent;
            }

            if (!w) {
                continue;
            }
        }

        /*
         * Over a new wid.
         */
        widp orig_wid = w;

        while (w && !wid_mouse_over_begin(w, x, y)) {
            w = w->parent;
        }

        boolean done = false;

        if (!w) {
            /*
             * Allow scrollbar to grab.
             */
            w = orig_wid;
        } else {
            /*
             * This widget reacted somehow when we went over it. i.e. popup ot 
             * function.
             */
            over = true;
        }

        w = wid_mouse_motion_handler(x, y, relx, rely, wheelx, wheely);
        if (w) {
            if (wid_mouse_over_begin(w, x, y)) {
                over = true;
            }

            /*
             * If the mouse event is fully processed then do not pass onto
             * scrollbars.
             */
            if (w->on_mouse_motion) {
                fast_verify(w);

                if ((*w->on_mouse_motion)(w, x, y,
                                          relx, rely, wheelx, wheely)) {
                    break;
                }
            }

            /*
             * If there is a tooltip then allow us to 'begin' on this wid so 
             * that we pop up the tooltip. But if wheel scrolling then pass
             * it to the scrollbar instead.
             */
            if (w->tooltip) {
                if (!wheelx && !wheely) {
                    break;
                }
            }

            if (wid_over == w) {
                if (!wheelx && !wheely) {
                    break;
                }
            }

            while (w) {
                /*
                 * If there are scrollbars and the wid did not grab the event
                 * then scroll for it.
                 */
                if (wheely) {
                    if (w->scrollbar_vert) {
                        wid_move_delta(w->scrollbar_vert, 0, -wheely);
                        done = true;
                    }
                }

                if (wheelx) {
                    if (w->scrollbar_horiz) {
                        wid_move_delta(w->scrollbar_horiz, -wheelx, 0);
                        done = true;
                    }
                }

                if (done) {
                    break;
                }

                /*
                 * Maybe the container has a scrollbar. Try it.
                 */
                w = w->parent;
            }
        } 

        if (done) {
            break;
        }
    }

    if (!over) {
        wid_mouse_over_end();
    }
}

void wid_mouse_down (uint32_t button, int32_t x, int32_t y)
{
    widp w;

    sound_play_click();

    w = wid_mouse_down_handler(x, y);
    if (!w) {
        return;
    }

    /*
     * Raise on mouse.
     */
    if ((w->on_mouse_down && (w->on_mouse_down)(w, x, y, button)) ||
        wid_get_movable(w)) {

        fast_verify(w);

        wid_set_focus(w);
        wid_set_mode(w, WID_MODE_ACTIVE);
        wid_raise(w);

        /*
         * Move on mouse.
         */
        if (wid_get_movable(w)) {
            wid_mouse_motion_begin(w, x, y);
            return;
        }

        return;
    }

    if (wid_get_movable(w)) {
        wid_set_mode(w, WID_MODE_ACTIVE);
        wid_raise(w);
        wid_mouse_motion_begin(w, x, y);
        return;
    }
}

void wid_mouse_up (uint32_t button, int32_t x, int32_t y)
{
    widp w;

    wid_mouse_motion_end();

    w = wid_mouse_up_handler(x, y);
    if (!w) {
        return;
    }

    if ((w->on_mouse_up && (w->on_mouse_up)(w, x, y, button)) ||
        wid_get_movable(w)) {

        fast_verify(w);

        wid_set_mode(w, WID_MODE_ACTIVE);
        wid_raise(w);
    }
}

static widp wid_key_down_handler (int32_t x, int32_t y)
{
    widp w;

    w = wid_key_down_handler_at(wid_focus, x, y, true /* strict */);
    if (w) {
        return (w);
    }

    w = wid_key_down_handler_at(
                wid_get_top_parent(wid_focus), x, y, false /* strict */);
    if (w) {
        return (w);
    }

    w = wid_key_down_handler_at(wid_over, x, y, true /* strict */);
    if (w) {
        return (w);
    }

    w = wid_key_down_handler_at(
                wid_get_top_parent(wid_over), x, y, false /* strict */);
    if (w) {
        return (w);
    }

    TREE_WALK(wid_top_level, w) {
        fast_verify(w);

        if (wid_focus_locked &&
            (wid_get_top_parent(w) != wid_get_top_parent(wid_focus_locked))) {
            continue;
        }

        w = wid_key_down_handler_at(w, x, y, true /* strict */);
        if (!w) {
            continue;
        }

        return (w);
    }

    { TREE_WALK(wid_top_level, w) {
        fast_verify(w);

        if (wid_focus_locked &&
            (wid_get_top_parent(w) != wid_get_top_parent(wid_focus_locked))) {
            continue;
        }

        w = wid_key_down_handler_at(w, x, y, false /* strict */);
        if (!w) {
            continue;
        }

        return (w);
    } }

    return (0);
}

static widp wid_key_up_handler (int32_t x, int32_t y)
{
    widp w;

    w = wid_key_up_handler_at(wid_focus, x, y, true /* strict */);
    if (w) {
        return (w);
    }

    w = wid_key_up_handler_at(
                wid_get_top_parent(wid_focus), x, y, false /* strict */);
    if (w) {
        return (w);
    }

    w = wid_key_up_handler_at(wid_over, x, y, true /* strict */);
    if (w) {
        return (w);
    }

    w = wid_key_up_handler_at(
                wid_get_top_parent(wid_over), x, y, false /* strict */);
    if (w) {
        return (w);
    }

    TREE_WALK(wid_top_level, w) {
        fast_verify(w);

        if (wid_focus_locked &&
            (wid_get_top_parent(w) != wid_get_top_parent(wid_focus_locked))) {
            continue;
        }

        w = wid_key_up_handler_at(w, x, y, true /* strict */);
        if (!w) {
            continue;
        }

        return (w);
    }

    { TREE_WALK(wid_top_level, w) {
        fast_verify(w);

        if (wid_focus_locked &&
            (wid_get_top_parent(w) != wid_get_top_parent(wid_focus_locked))) {
            continue;
        }

        w = wid_key_up_handler_at(w, x, y, false /* strict */);
        if (!w) {
            continue;
        }

        return (w);
    } }

    return (0);
}

void wid_key_down (const struct SDL_KEYSYM *key, int32_t x, int32_t y)
{
    widp w;

    if (wid_focus &&
        !wid_is_hidden(wid_focus) &&
        (wid_focus->on_key_down)) {

        fast_verify(wid_focus);

        if ((wid_focus->on_key_down)(wid_focus, key)) {

            if (wid_focus) {
                fast_verify(wid_focus);

                wid_set_mode(wid_focus, WID_MODE_ACTIVE);
            }

            /*
             * Do not raise, gets in the way of popups the callback creates.
             */
            return;
        }

        w = wid_focus;

        goto try_parent;
    }

    w = wid_key_down_handler(x, y);
    if (!w) {
        /*
         * If no-one handles it, feed it to the default handler, the console.
         */
        wid_receive_unhandled_input(key);
        return;
    }

    if ((w->on_key_down)(w, key)) {
        fast_verify(w);

        wid_set_mode(w, WID_MODE_ACTIVE);

        /*
         * Do not raise, gets in the way of popups the callback creates.
         */
        return;
    }

try_parent:
    w = w->parent;

    /*
     * Ripple the key event to the parent so global things like pressing
     * escape can do things.
     */
    while (w) {
        if (w->on_key_down) {
            if ((w->on_key_down)(w, key)) {
                fast_verify(w);

                wid_set_mode(w, WID_MODE_ACTIVE);

                /*
                 * Do not raise, gets in the way of popups the callback
                 * creates.
                 */
                return;
            }
        }

        w = w->parent;
    }

    /*
     * If no-one handles it, feed it to the default handler, the console.
     */
    wid_receive_unhandled_input(key);
}

void wid_key_up (const struct SDL_KEYSYM *key, int32_t x, int32_t y)
{
    widp w;

    if (wid_focus &&
        !wid_is_hidden(wid_focus) &&
        (wid_focus->on_key_up)) {

        fast_verify(wid_focus);

        if ((wid_focus->on_key_up)(wid_focus, key)) {
            wid_set_mode(wid_focus, WID_MODE_ACTIVE);

            /*
             * Do not raise, gets in the way of popups the callback creates.
             */
            return;
        }

        w = wid_focus;

        goto try_parent;
    }

    w = wid_key_up_handler(x, y);
    if (!w) {
        /*
         * If no-one handles it, drop it. We only hand key down to the
         * console.
         */
        return;
    }

    if ((w->on_key_up)(w, key)) {
        fast_verify(w);

        wid_set_mode(w, WID_MODE_ACTIVE);

        /*
         * Do not raise, gets in the way of popups the callback creates.
         */
        return;
    }

try_parent:
    w = w->parent;

    /*
     * Ripple the key event to the parent so global things like pressing
     * escape can do things.
     */
    while (w) {
        if (w->on_key_up) {
            if ((w->on_key_up)(w, key)) {
                fast_verify(w);

                wid_set_mode(w, WID_MODE_ACTIVE);

                /*
                 * Do not raise, gets in the way of popups the callback
                 * creates.
                 */
                return;
            }
        }

        w = w->parent;
    }
}

/*
 * Get the onscreen co-ords of the widget, clipped to the parent.
 */
void wid_get_abs_coords (widp w,
                         int32_t *tlx,
                         int32_t *tly,
                         int32_t *brx,
                         int32_t *bry)
{
    widp p;

    *tlx = wid_get_tl_x(w);
    *tly = wid_get_tl_y(w);
    *brx = wid_get_br_x(w);
    *bry = wid_get_br_y(w);

    p = w->parent;
    if (p) {
        *tlx += p->offset.x;
        *tly += p->offset.y;
        *brx += p->offset.x;
        *bry += p->offset.y;
    }

    while (p) {
        int32_t ptlx = wid_get_tl_x(p);
        int32_t ptly = wid_get_tl_y(p);
        int32_t pbrx = wid_get_br_x(p);
        int32_t pbry = wid_get_br_y(p);

        if (p->parent) {
            ptlx += p->parent->offset.x;
            ptly += p->parent->offset.y;
            pbrx += p->parent->offset.x;
            pbry += p->parent->offset.y;
        }

        if (ptlx > *tlx) {
            *tlx = ptlx;
        }

        if (ptly > *tly) {
            *tly = ptly;
        }

        if (pbrx < *brx) {
            *brx = pbrx;
        }

        if (pbry < *bry) {
            *bry = pbry;
        }

        p = p->parent;
    }

    w->abs_tl.x = *tlx;
    w->abs_tl.y = *tly;
    w->abs_br.x = *brx;
    w->abs_br.y = *bry;
}

/*
 * Finish off a widgets move.
 */
void wid_move_end (widp w)
{
    if (!w->moving) {
        return;
    }

    wid_move_to_abs(w, w->moving_end.x, w->moving_end.y);

    w->moving = false;

    wid_tree3_remove(w);
}

/*
 * Do stuff for widgets once per frame.
 */
static void wid_gc (widp w)
{
    if (w->being_destroyed) {
        /*
         * If being destroyed, is it done fading ? We only do this for the top
         * level widgets. The childen inherit the fading from the parent.
         */
        if (wid_is_fading(w)) {
            double fade = wid_get_fade_amount(w);

            if (fade == 0.0) {
                if (!w->parent) {
                    wid_destroy_immediate(w);
                    return;
                }
            }
        } else {
            /*
             * If this is a sub widget being destoyed without a parent being
             * destroyed then remove it now.
             */
            wid_destroy_immediate(w);
            return;
        }
    }

    /*
     * Delayed destroy?
     */
    if (w->destroy_when && (time_get_time_cached() >= w->destroy_when)) {
        wid_destroy(&w);
    }
}

/*
 * Display one wid and its children
 */
static void wid_display (widp w,
                         boolean disable_scissor,
                         boolean *updated_scissors)
{
    boolean did_push_matrix;
    int32_t clip_height;
    int32_t clip_width;
    boolean fading;
    boolean hidden;
    int32_t owidth;
    boolean scaling;
    int32_t oheight;
    int32_t otlx;
    int32_t otly;
    int32_t obrx;
    int32_t obry;
    int32_t tlx;
    int32_t tly;
    int32_t brx;
    int32_t bry;
    widp p;
    widp child;

    /*
     * Bounding box for drawing the wid. Co-ords are negative as we
     * flipped the screen
     */
    tlx = w->abs_tl.x;
    tly = w->abs_tl.y;
    brx = w->abs_br.x;
    bry = w->abs_br.y;

    /*
     * If we're clipped out of existence! then nothing to draw. This can
     * be outside the bounds of a widget or if at the top level, off screeen.
     */
    clip_width = brx - tlx;
    if (clip_width < 0) {
        return;
    }

    clip_height = bry - tly;
    if (clip_height < 0) {
        return;
    }

    fading = wid_is_fading(w);
    hidden = wid_is_hidden(w);
    scaling = wid_is_scaling(w);

    if (fading) {
        /*
         * Always render. Not hidden yet.
         */
    } else if (scaling) {
        /*
         * Always render. Not hidden yet.
         */
    } else if (hidden) {
        /*
         * Hidden or parent is hidden.
         */
        return;
    }

    /*
     * Record the original pre clip sizes for text centering.
     */
    otlx = wid_get_tl_x(w);
    otly = wid_get_tl_y(w);
    obrx = wid_get_br_x(w);
    obry = wid_get_br_y(w);

    p = w->parent;
    if (p) {
        otlx += p->offset.x;
        otly += p->offset.y;
        obrx += p->offset.x;
        obry += p->offset.y;
    }

    owidth = obrx - otlx;
    oheight = obry - otly;

    /*
     * If this widget was active and the time has elapsed, make it normal.
     */
    if (wid_get_mode(w) == WID_MODE_ACTIVE) {
        if ((time_get_time_cached() - w->timestamp_last_mode_change) > 250) {
            wid_set_mode(w, WID_MODE_NORMAL);
        }
    }

    /*
     * Draw the wid frame
     */
    color col_text = wid_get_color(w, WID_COLOR_TEXT);
    color col_text_outline = BLACK;

    color col_tl = wid_get_color(w, WID_COLOR_TL);
    color col_br = wid_get_color(w, WID_COLOR_BR);
    color col_blit_outline = wid_get_color(w, WID_COLOR_BLIT_OUTLINE);
    color col = wid_get_color(w, WID_COLOR_BG);
    color col_tile = wid_get_color(w, WID_COLOR_BLIT);

    if (wid_focus_locked) {
        if (wid_get_top_parent(w) != wid_focus_locked) {
            double fade = 0.6;

            if (col.r && col.g && col.b) {
                col_tl.a *= fade;
                col.a *= fade;
                col_br.a *= fade;
            }

            col_text.a *= fade;
            col_text_outline.a *= fade;
            col_blit_outline.a *= fade;
        }
    }

    /*
     * Apply fade in/out effects.
     */
    if (fading) {
        double fade = wid_get_fade_amount(w);

        /*
         * So hidden scrolbars stay hidden in a fading parent. I'm drunk.
         */
        if (!w->fade_out && !w->fade_in && w->hidden) {
            fade = 0.0;
        }

        col_tl.a *= fade;
        col.a *= fade;
        col_br.a *= fade;
        col_text.a *= fade;
        col_text_outline.a *= fade;
        col_tile.a *= fade;
    }

    did_push_matrix = false;

    /*
     * Do rotation and flipping.
     */
    if (w->rotating || w->rotated || w->flip_horiz || w->flip_vert) {
        did_push_matrix = true;

        glPushMatrix();
        glTranslatef(((tlx + brx)/2), ((tly + bry)/2), 0);

        /*
         * If rotating the widget, turn off the scissors.
         */
        if (w->rotating || w->rotated) {
            disable_scissor = true;

            glRotatef(wid_get_rotate(w), 0, 0, 1);
        }

        if (w->flip_vert) {
            glScalef(-1, 1, 1);
        }

        if (w->flip_horiz) {
            glScalef(1, -1, 1);
        }

        if (did_push_matrix) {
            glTranslatef(-((tlx + brx)/2), -((tly + bry)/2), 0);
        }
    }

    /*
     * If inputting text, show a cursor.
     */
    const char *text;

    if (wid_get_show_cursor(w)) {
        text = wid_get_text_with_cursor(w);
    } else {
        text = wid_get_text(w);
    }

    /*
     * Should be no need for scissors if you do not have any children
     * or are not the top level wid.
     */
    if (!disable_scissor) {
        /*
         * Text box needs clipping when the text gets too wide.
         */
       if (w->children_display_sorted || !w->parent || w->show_cursor) {
            /*
             * Tell the parent we are doing scissors so they can re-do
             * their own scissors.
             */
            if (updated_scissors) {
                *updated_scissors = true;
            }

            glScissor(
                tlx / global_config.xscale,
                global_config.video_pix_height -
                    ((tly + clip_height) / global_config.yscale),
                clip_width,
                clip_height);
        }
    }

    fsize bevel = {wid_get_bevel(w), wid_get_bevel(w)};
    int32_t sides = wid_get_sides(w);
    fsize radius = wid_get_radius(w);

    /*
     * Widget tiles and textures.
     */
    tilep tile = wid_get_tile(w);
    tilep tile2 = wid_get_tile2(w);

    texp tex;

    if (!tile) {
        tex = wid_get_tex(w, 0);
    } else {
        tex = 0;
    }

    /*
     * Only care about tex scaling here.
     */
    fsize texuv;
    (void) wid_get_tex(w, &texuv);

    fsize tex_tl = wid_get_tex_tl(w);
    fsize tex_br = wid_get_tex_br(w);

    fpoint tl;
    fpoint br;

    tl.x = otlx;
    tl.y = otly;
    br.x = obrx;
    br.y = obry;

    if (tex) {
        /*
         * Fit texture to the window size.
         */
        texuv.width *= ((double)(br.x - tl.x)) / ((double)tex_get_width(tex));
        texuv.height *= ((double)(br.y - tl.y)) / ((double)tex_get_height(tex));

        glBindTexture(GL_TEXTURE_2D, tex_get_gl_binding(tex));
    } else {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    if (wid_get_rounded(w)) {
        if (wid_get_bevelled(w)) {
            gl_list_rounded_square_bevelled(
                tl, br, tex, tex_tl, tex_br,
                texuv, col_tl, col, col_br, bevel, radius, sides);
        } else {
            gl_list_rounded_square(
                tl, br, tex, tex_tl, tex_br,
                texuv, col_tl, col, col_br, radius, sides);
        }
    } else if (wid_get_square(w)) {
        if (wid_get_bevelled(w)) {
            gl_list_square_bevelled_plain(
                tl, br, tex, tex_tl, tex_br,
                texuv, col_tl, col, col_br, bevel);
        } else {
            gl_list_square(tl, br, tex, tex_tl, tex_br,
                texuv, col_tl, col, col_br);
        }
    } else if (wid_get_square_outline(w)) {
        gl_list_square_outline_bevelled_plain(
            tl, br, tex, tex_tl, tex_br,
            texuv, col_tl, col, col_br, bevel);
    }

    if (tile) {
        /*
         * Blit a border around the tile.
         */
        if (wid_get_blit_outline(w)) {
            glcolor(col_blit_outline);

            fpoint otl;
            fpoint obr;
            int32_t dx;
            int32_t dy;

            dx = 2;
            dy = 2;
            otl.x = tl.x - dx;
            otl.y = tl.y - dx;
            obr.x = br.x + dy;
            obr.y = br.y + dy;

            tile_blit_fat(tile, 0, otl, obr);

            glcolor(WHITE);
        }

        glcolor(col_tile);
        tile_blit_fat(tile, 0, tl, br);
        glBindTexture(GL_TEXTURE_2D, 0);

        if (tile2) {
            glcolor(col_tile);

            tile_blit_fat(tile2, 0, tl, br);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        /*
         * If we are just blitting the occasional tile and this is not part of 
         * the main map grid, then just flush the blit pipe.
         */
        if (!w->gridtree) {
            blit_flush();
        }
    }

    if (text && text[0]) {
        fontp font = wid_get_font(w);
        double scaling = wid_get_text_scaling(w);
        double advance = wid_get_text_advance(w);
        boolean fixed_width = wid_get_text_fixed_width(w);

        int32_t x, y;
        double xpc, ypc;
        uint32_t width, height;
        enum_fmt fmt;

        if (!w->text_size_cached) {
            /*
             * Manually specified text position.
             */
            fmt = ENUM_FMT_NONE;
            ttf_text_size(font, text, &width, &height, &fmt, scaling, advance,
                          fixed_width);

            w->text_size_cached = true;
            w->ttf_width = width;
            w->ttf_height = height;
            w->fmt = fmt;
        } else {
            width = w->ttf_width;
            height = w->ttf_height;
            fmt = w->fmt;
        }

        if (wid_get_text_pos(w, &xpc, &ypc)) {
            x = (owidth * xpc) - ((int32_t)width / 2) + otlx;
            y = (oheight * ypc) - ((int32_t)height / 2) + otly;
        } else {
            /*
             * Position the text
             */
            if ((width > owidth) && w->show_cursor) {
                /*
                 * If the text is too big, center it on the cursor.
                 */
                x = ((owidth - (int32_t)width) / 2) + otlx;

                uint32_t c_width = (width / (double)strlen(text));

                x -= (w->cursor - (strlen(text) / 2)) * c_width;
            } else if ((fmt == ENUM_FMT_LEFT) || wid_get_text_lhs(w)) {
                x = otlx;
            } else if ((fmt == ENUM_FMT_CENTERX) || wid_get_text_centerx(w)) {
                x = ((owidth - (int32_t)width) / 2) + otlx;
            } else if ((fmt == ENUM_FMT_RIGHT) || wid_get_text_rhs(w)) {
                x = obrx - (int32_t)width;
            } else {
                x = ((owidth - (int32_t)width) / 2) + otlx;
            }

            if ((fmt == ENUM_FMT_TOP) || wid_get_text_top(w)) {
                y = otly;
            } else if ((fmt == ENUM_FMT_CENTERY) || wid_get_text_centery(w)) {
                y = ((oheight - (int32_t)height) / 2) + otly;
            } else if ((fmt == ENUM_FMT_BOTTOM) || wid_get_text_bot(w)) {
                y = obry - (int32_t)height;
            } else {
                y = ((oheight - (int32_t)height) / 2) + otly;
            }
        }

        /*
         * Outline text in black
         */
        if (wid_get_text_outline(w)) {
            glcolor(col_text_outline);

            ttf_puts_no_fmt(font, text,
                            x - 2.0f * scaling,
                            y + 2.0f * scaling, scaling, advance,
                            fixed_width);
        }

        /*
         * Widget text
         */
        glcolor(col_text);
        ttf_puts(font, text, x, y, scaling, advance, fixed_width);

        blit_flush();
    }

    /*
     * If this is a grid wid, draw the elements in y sorted order.
     */
    TREE_WALK_REVERSE_UNSAFE(w->children_display_sorted, child) {
        boolean child_updated_scissors = false;

        wid_display(child, disable_scissor, &child_updated_scissors);

        /*
         * Need to re-enforce the parent's scissors if the child did
         * their own bit of scissoring?
         */
        if (!disable_scissor && child_updated_scissors) {
            glScissor(
                tlx / global_config.xscale,
                global_config.video_pix_height -
                    ((tly + clip_height) / global_config.yscale),
                clip_width,
                clip_height);
        }
    }

    if (w->children_display_sorted) {
        blit_flush();
    }

    /*
     * Undo any push we did earlier.
     */
    if (did_push_matrix) {
        glPopMatrix();
    }
}

/*
 * Do stuff for all widgets.
 */
void wid_move_all (void)
{
    if (!wid_top_level3) {
        return;
    }

    uint32_t N = tree_root_size(wid_top_level3);
    widp w;
    widp wids[N];
    uint32_t n = 0;

    { TREE_OFFSET_WALK_UNSAFE(wid_top_level3, w) {
        wids[n] = w;
        n++;
    } }

    while (n--) {
        w = wids[n];

        int32_t x;
        int32_t y;

        if (time_get_time_cached() >= w->timestamp_moving_end) {
            x = w->moving_end.x;
            y = w->moving_end.y;

            w->moving = false;

            wid_tree3_remove(w);
        } else {
            double time_step =
                (double)(time_get_time_cached() - w->timestamp_moving_begin) /
                (double)(w->timestamp_moving_end - w->timestamp_moving_begin);

            x = (time_step * (double)(w->moving_end.x - w->moving_start.x)) +
                w->moving_start.x;
            y = (time_step * (double)(w->moving_end.y - w->moving_start.y)) +
                w->moving_start.y;
        }

        wid_move_to_abs(w, x, y);
    }
}

/*
 * Do stuff for all widgets.
 */
void wid_gc_all (void)
{
    widp w;

    { TREE_OFFSET_WALK(wid_top_level4, w, tree4) {
        wid_gc(w);
    } }
}

/*
 * Do stuff for all widgets.
 */
void wid_tick_all (void)
{
    widp w;

    { TREE_OFFSET_WALK_UNSAFE(wid_top_level5, w) {
        verify(w);

        if (!w->on_tick) {
            DIE("wid on ticker tree, but no callback set");
        }

        (w->on_tick)(w);
    } }

    action_timers_tick(wid_timers);
}

/*
 * Display all widgets
 */
void wid_display_all (void)
{
    widp w;

    if (HEADLESS) {
        return;
    }

    glEnable(GL_SCISSOR_TEST);

    glBindTexture(GL_TEXTURE_2D, 0);

    { TREE_WALK_REVERSE_UNSAFE(wid_top_level, w) {
        wid_display(w,
                    false /* disable_scissors */,
                    0 /* updated_scissors */);
    } }

    glDisable(GL_SCISSOR_TEST);
}

void wid_fade_in (widp w, uint32_t delay)
{
    fast_verify(w);

    w->timestamp_fading_begin = time_get_time_cached();
    w->timestamp_fading_end = w->timestamp_fading_begin + delay;
    w->fade_out = false;
    w->fade_in = true;
}

void wid_fade_out (widp w, uint32_t delay)
{
    fast_verify(w);

    w->timestamp_fading_begin = time_get_time_cached();
    w->timestamp_fading_end = w->timestamp_fading_begin + delay;
    w->fade_out = true;
    w->fade_in = false;
}

void wid_fade_in_out (widp w, uint32_t delay, uint32_t repeat, boolean in)
{
    fast_verify(w);

    w->timestamp_fading_begin = time_get_time_cached();
    w->timestamp_fading_end = w->timestamp_fading_begin + delay;

    if (in) {
        w->fade_out = false;
        w->fade_in = true;
    } else {
        w->fade_out = true;
        w->fade_in = false;
    }

    w->fade_count = repeat;
    w->fade_delay = delay;
}

boolean wid_is_hidden (widp w)
{
    fast_verify(w);

    if (!w) {
        return (false);
    }

    if (w->hidden) {
        return (true);
    }

    while (w->parent) {
        w = w->parent;

        if (w->hidden) {
            return (true);
        }
    }

    return (false);
}

boolean wid_is_fading (widp w)
{
    fast_verify(w);

    if (!w) {
        return (false);
    }

    if (w->fade_out || w->fade_in) {
        return (true);
    }

    while (w->parent) {
        w = w->parent;

        if (w->fade_out || w->fade_in) {
            return (true);
        }
    }

    return (false);
}

boolean wid_is_scaling (widp w)
{
    fast_verify(w);

    if (!w) {
        return (false);
    }

    if (w->scaling_h || w->scaling_w) {
        return (true);
    }

    while (w->parent) {
        w = w->parent;

        if (w->scaling_h || w->scaling_w) {
            return (true);
        }
    }

    return (false);
}

/*
 * How much are we fading from 0.0 to 1.0 (solid)
 */
static double wid_get_fade_amount (widp w)
{
    double fade;

    fast_verify(w);

    if (!w) {
        return (1.0f);
    }

    /*
     * Find which wid is doing the fading.
     */
    if (!w->fade_out && !w->fade_in) {
        /*
         * Must be a parent.
         */
        while (w->parent) {
            if (w->fade_out || w->fade_in) {
                break;
            }

            w = w->parent;
        }

        if (!w) {
            return (1.0f);
        }
    }

    if (!w->fade_out && !w->fade_in) {
        return (1.0f);
    }

    if (time_get_time_cached() >= w->timestamp_fading_end) {
        if (w->fade_in) {
            w->fade_in = false;
            w->hidden = false;
            w->visible = true;

            if (w == wid_focus) {
                wid_find_top_focus();
            }

            if (w->fade_count) {
                wid_fade_in_out(w, w->fade_delay, w->fade_count - 1, false);
            }

            return (1.0f);
        }

        if (w->fade_out) {
            w->fade_out = false;
            w->hidden = true;
            w->visible = false;

            if (w == wid_focus) {
                wid_find_top_focus();
            }

            if (w->fade_count) {
                wid_fade_in_out(w, w->fade_delay, w->fade_count - 1, true);
            }

            return (0.0f);
        }

        return (1.0f);
    }

    int32_t effect_duration = w->timestamp_fading_end -
                    w->timestamp_fading_begin;

    if (w->fade_in) {
        fade = (double)(time_get_time_cached() - w->timestamp_fading_begin) /
               (double)effect_duration;

        return (fade);
    }

    if (w->fade_out) {
        fade = 1.0f -
                ((double)(time_get_time_cached() - w->timestamp_fading_begin) /
                 (double)effect_duration);

        return (fade);
    }

    return (1.0f);
}

boolean wid_is_moving (widp w)
{
    fast_verify(w);

    if (w->moving) {
        return (true);
    }

    return (false);
}

void wid_move_to_pct (widp w, double x, double y)
{
    fast_verify(w);

    x *= (double)global_config.video_gl_width;
    y *= (double)global_config.video_gl_height;

    int32_t dx = x - wid_get_tl_x(w);
    int32_t dy = y - wid_get_tl_y(w);

    wid_move_delta(w, dx, dy);
}

void wid_move_to_abs (widp w, double x, double y)
{
    fast_verify(w);

    int32_t dx = x - wid_get_tl_x(w);
    int32_t dy = y - wid_get_tl_y(w);

    wid_move_delta(w, dx, dy);
}

void wid_move_to_pct_centered (widp w, double x, double y)
{
    fast_verify(w);

    x *= (double)global_config.video_gl_width;
    y *= (double)global_config.video_gl_height;

    int32_t dx = x - wid_get_tl_x(w);
    int32_t dy = y - wid_get_tl_y(w);

    dx -= (wid_get_br_x(w) - wid_get_tl_x(w))/2;
    dy -= (wid_get_br_y(w) - wid_get_tl_y(w))/2;

    wid_move_delta(w, dx, dy);
}

void wid_move_to_abs_centered (widp w, double x, double y)
{
    fast_verify(w);

    int32_t dx = x - wid_get_tl_x(w);
    int32_t dy = y - wid_get_tl_y(w);

    dx -= (wid_get_br_x(w) - wid_get_tl_x(w))/2;
    dy -= (wid_get_br_y(w) - wid_get_tl_y(w))/2;

    wid_move_delta(w, dx, dy);
}

void wid_move_to_pct_in (widp w, double x, double y, uint32_t ms)
{
    fast_verify(w);

    w->timestamp_moving_begin = time_get_time_cached();
    w->timestamp_moving_end = w->timestamp_moving_begin + ms;

    x *= (double)global_config.video_gl_width;
    y *= (double)global_config.video_gl_height;

    w->moving_start.x = wid_get_tl_x(w);
    w->moving_start.y = wid_get_tl_y(w);
    w->moving_end.x = x;
    w->moving_end.y = y;
    w->moving = true;

    wid_tree3_insert(w);
}

/*
 * Return numbers in the 0 to 1 range indicating how far the move has 
 * progressed from start to end.
 */
void wid_get_move_interpolated_progress (widp w, double *dx, double *dy)
{
    fast_verify(w);

    if (!wid_is_moving(w)) {
        *dx = 0.0;
        *dy = 0.0;
        return;
    }

    double x = wid_get_tl_x(w);
    double y = wid_get_tl_y(w);
      
    x -= w->moving_start.x;
    y -= w->moving_start.y;

    double wx = (double)(w->moving_end.x - w->moving_start.x);
    double wy = (double)(w->moving_end.y - w->moving_start.y);

    if (wx == 0.0) {
        *dx = x;
    } else {
        *dx = x / wx;
    }

    if (wy == 0.0) {
        *dy = y;
    } else {
        *dy = y / wy;
    }
}

void wid_move_stop (widp w)
{
    fast_verify(w);

    if (!w->moving) {
        return;
    }

    w->moving = false;
    w->paused = true;

    wid_tree3_remove(w);
}

void wid_move_resume (widp w)
{
    fast_verify(w);

    if (!w->paused) {
        return;
    }

    w->moving = true;
    w->paused = false;

    wid_tree3_insert(w);
}

void wid_move_to_abs_in (widp w, double x, double y, uint32_t ms)
{
    fast_verify(w);

    w->timestamp_moving_begin = time_get_time_cached();
    w->timestamp_moving_end = w->timestamp_moving_begin + ms;

    w->moving_start.x = wid_get_tl_x(w);
    w->moving_start.y = wid_get_tl_y(w);
    w->moving_end.x = x;
    w->moving_end.y = y;
    w->moving = true;

    wid_tree3_insert(w);
}

void wid_move_to_pct_centered_in (widp w, double x, double y, uint32_t ms)
{
    fast_verify(w);

    w->timestamp_moving_begin = time_get_time_cached();
    w->timestamp_moving_end = w->timestamp_moving_begin + ms;

    x *= (double)global_config.video_gl_width;
    y *= (double)global_config.video_gl_height;

    x -= (wid_get_br_x(w) - wid_get_tl_x(w))/2;
    y -= (wid_get_br_y(w) - wid_get_tl_y(w))/2;

    w->moving_start.x = wid_get_tl_x(w);
    w->moving_start.y = wid_get_tl_y(w);
    w->moving_end.x = x;
    w->moving_end.y = y;
    w->moving = true;

    wid_tree3_insert(w);
}

void wid_move_delta_pct_in (widp w, double x, double y, uint32_t ms)
{
    fast_verify(w);

    w->timestamp_moving_begin = time_get_time_cached();
    w->timestamp_moving_end = w->timestamp_moving_begin + ms;

    x *= (double)global_config.video_gl_width;
    y *= (double)global_config.video_gl_height;

    w->moving_start.x = wid_get_tl_x(w);
    w->moving_start.y = wid_get_tl_y(w);
    w->moving_end.x = w->moving_start.x + x;
    w->moving_end.y = w->moving_start.y + y;
    w->moving = true;

    wid_tree3_insert(w);
}

void wid_move_to_abs_centered_in (widp w, double x, double y, uint32_t ms)
{
    fast_verify(w);

    w->timestamp_moving_begin = time_get_time_cached();
    w->timestamp_moving_end = w->timestamp_moving_begin + ms;

    x -= (wid_get_br_x(w) - wid_get_tl_x(w))/2;
    y -= (wid_get_br_y(w) - wid_get_tl_y(w))/2;

    w->moving_start.x = wid_get_tl_x(w);
    w->moving_start.y = wid_get_tl_y(w);
    w->moving_end.x = x;
    w->moving_end.y = y;
    w->moving = true;

    wid_tree3_insert(w);
}

void wid_scaling_to_pct_in (widp w,
                            double scaling_start,
                            double scaling_end,
                            uint32_t ms,
                            uint32_t scaling_bounce_count)
{
    fast_verify(w);

    w->timestamp_scaling_w_begin = time_get_time_cached();
    w->timestamp_scaling_h_begin = time_get_time_cached();

    w->timestamp_scaling_w_end = w->timestamp_scaling_w_begin + ms;
    w->timestamp_scaling_h_end = w->timestamp_scaling_h_begin + ms;

    w->scaling_w_start = scaling_start;
    w->scaling_h_start = scaling_start;

    w->scaling_w_end = scaling_end;
    w->scaling_h_end = scaling_end;

    w->scaling_w_bounce_count = scaling_bounce_count;
    w->scaling_h_bounce_count = scaling_bounce_count;

    w->scaling_w = true;
    w->scaling_h = true;
}

void wid_scale_immediate (widp w, double val)
{
    fast_verify(w);

    w->scale_w_base = val;
    w->scale_h_base = val;
    w->scaled_w = true;
    w->scaled_h = true;

    if (val == 0.0) {
        w->scaled_w = false;
        w->scaled_h = false;
    }
}

void wid_scale_w_immediate (widp w, double val)
{
    fast_verify(w);

    w->scale_w_base = val;
    w->scaled_w = true;

    if (val == 0.0) {
        w->scaled_w = false;
    }
}

void wid_scale_h_immediate (widp w, double val)
{
    fast_verify(w);

    w->scale_h_base = val;
    w->scaled_h = true;

    if (val == 0.0) {
        w->scaled_h = false;
    }
}

void wid_effect_pulses (widp w)
{
    fast_verify(w);

    if (wid_get_height(w) > 100) {
        wid_scaling_to_pct_in(w, 1.0, 1.01, wid_pulse_delay, 1);
    } else {
        wid_scaling_to_pct_in(w, 1.0, 1.1, wid_pulse_delay, 1);
    }
}

double wid_get_scaling_w (widp w)
{
    double scaling;

    if (!w->scaling_w && !w->scaled_w) {
        return (1.0);
    }

    w->text_size_cached = false;

    if (time_get_time_cached() >= w->timestamp_scaling_w_end) {

        scaling = w->scaling_w_end;
        w->scaling_w = false;

        if (w->scaling_w_bounce_count) {
            wid_scaling_to_pct_in(w,
                                  w->scaling_w_end,
                                  w->scaling_w_start,
                                  w->timestamp_scaling_w_end -
                                  w->timestamp_scaling_w_begin,
                                  w->scaling_w_bounce_count - 1);
        }

        return (scaling + w->scale_w_base);
    }

    double time_step =
        (double)(time_get_time_cached() - w->timestamp_scaling_w_begin) /
        (double)(w->timestamp_scaling_w_end - w->timestamp_scaling_w_begin);

    scaling = (time_step * (double)(w->scaling_w_end - w->scaling_w_start)) +
                    w->scaling_w_start;

    return (scaling + w->scale_w_base);
}

double wid_get_scaling_h (widp w)
{
    double scaling;

    if (!w->scaling_h && !w->scaled_h) {
        return (1.0);
    }

    w->text_size_cached = false;

    if (time_get_time_cached() >= w->timestamp_scaling_h_end) {

        scaling = w->scaling_h_end;
        w->scaling_h = false;

        if (w->scaling_h_bounce_count) {
            wid_scaling_to_pct_in(w,
                                  w->scaling_h_end,
                                  w->scaling_h_start,
                                  w->timestamp_scaling_h_end -
                                  w->timestamp_scaling_h_begin,
                                  w->scaling_h_bounce_count - 1);
        }

        return (scaling + w->scale_h_base);
    }

    double time_step =
        (double)(time_get_time_cached() - w->timestamp_scaling_h_begin) /
        (double)(w->timestamp_scaling_h_end - w->timestamp_scaling_h_begin);

    scaling = (time_step * (double)(w->scaling_h_end - w->scaling_h_start)) +
                    w->scaling_h_start;

    return (scaling + w->scale_h_base);
}

void wid_rotate_to_pct_in (widp w,
                            double rotate_start,
                            double rotate_end,
                            uint32_t ms,
                            uint32_t rotate_sways_count)
{
    fast_verify(w);

    w->timestamp_rotate_begin = time_get_time_cached();
    w->timestamp_rotate_end = w->timestamp_rotate_begin + ms;

    w->rotate_start = rotate_start;
    w->rotate_end = rotate_end;
    w->rotate_sways_count = rotate_sways_count;
    w->rotating = true;
}

void wid_rotate_immediate (widp w, double rotate_base)
{
    fast_verify(w);

    w->rotated = true;

    if (rotate_base == 0.0) {
        w->rotated = false;
    }

    w->rotate_base = rotate_base;
}

void wid_flip_horiz (widp w, boolean val)
{
    fast_verify(w);

    w->flip_vert = val;
}

void wid_flip_vert (widp w, boolean val)
{
    fast_verify(w);

    w->flip_horiz = val;
}

boolean wid_get_flip_horiz (widp w)
{
    fast_verify(w);

    return (w->flip_vert);
}

boolean wid_get_flip_vert (widp w)
{
    fast_verify(w);

    return (w->flip_horiz);
}

void wid_effect_sways (widp w)
{
    fast_verify(w);

    if (w->rotating) {
        return;
    }

    wid_rotate_to_pct_in(w, -5, 5, ONESEC, 5);
}

double wid_get_rotate (widp w)
{
    double rotating;

    if (!w->rotating && !w->rotated) {
        return (0.0);
    }

    w->text_size_cached = false;

    if (time_get_time_cached() >= w->timestamp_rotate_end) {

        rotating = w->rotate_end;
        w->rotating = false;

        if (w->rotate_sways_count) {
            wid_rotate_to_pct_in(w,
                                 w->rotate_end,
                                 w->rotate_start,
                                 w->timestamp_rotate_end -
                                 w->timestamp_rotate_begin,
                                 w->rotate_sways_count - 1);
        }

        return (w->rotate_base + rotating);
    }

    double time_step =
        (double)(time_get_time_cached() - w->timestamp_rotate_begin) /
        (double)(w->timestamp_rotate_end - w->timestamp_rotate_begin);

    rotating = (time_step * (double)(w->rotate_end - w->rotate_start)) +
                    w->rotate_start;

    return (w->rotate_base + rotating);
}

boolean wids_overlap (widp A, widp B)
{
    /*
     * The rectangles don't overlap if one rectangle's minimum in some 
     * dimension is greater than the other's maximum in that dimension.
     */
    boolean no_overlap = (A->tree.tl.x > B->tree.br.x) ||
                      (B->tree.tl.x > A->tree.br.x) ||
                      (A->tree.tl.y > B->tree.br.y) ||
                      (B->tree.tl.y > A->tree.br.y);

    return (!no_overlap);
}
