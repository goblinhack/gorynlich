/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#include "slre.h"

#include "wid.h"
#include "ttf.h"
#include "tex.h"
#include "tile.h"
#include "thing.h"
#include "thing_tile.h"
#include "command.h"
#include "time_util.h"
#include "wid_console.h"
#include "wid_chat.h"
#include "wid_tooltip.h"
#include "marshal.h"
#include "string_util.h"
#include "sound.h"
#include "timer.h"
#include "client.h"
#include "wid_game_map_client.h"
#include "math_util.h"
#include "level.h"
#include "map.h"

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
static char *wid_tooltip_string;

/*
 * Scope the focus to children of this widget and do not change it.
 * Good for popups.
 */
static widp wid_focus_locked;
static widp wid_focus;
static widp wid_over;

/*
 * Mouse
 */
int wid_mouse_visible = 1;

/*
 * Widget moving
 */
static widp wid_moving;
static int32_t wid_moving_last_x;
static int32_t wid_moving_last_y;

static uint32_t wid_time;

/*
 * Widget effects
 */
const int32_t wid_fade_delay = 200;
int32_t wid_destroy_delay_ms = 200;
const int32_t wid_visible_delay = 100;
const int32_t wid_hide_delay = 200;
const int32_t wid_swipe_delay = 200;
const int32_t wid_pulse_delay = 300;
const int32_t wid_scaling_forever_delay = 500;

/*
 * Prototypes.
 */
static void wid_destroy_delay(widp *wp, int32_t delay);
static uint8_t wid_scroll_trough_mouse_down(widp w, int32_t x, int32_t y,
                                            uint32_t button);
static uint8_t wid_scroll_trough_mouse_motion(widp w,
                                              int32_t x, int32_t y,
                                              int32_t relx, int32_t rely,
                                              int32_t wheelx, int32_t wheely);
static double wid_get_fade_amount(widp w);
static void wid_find_first_focus(void);
static void wid_find_top_focus(void);
static void wid_destroy_immediate(widp w);
static void wid_destroy_immediate_internal(widp w);
static void wid_grid_tree_attach(widp w);
static uint8_t wid_grid_tree_detach(widp w);
static void wid_update_internal(widp w);
static void wid_tree_detach(widp w);
static void wid_tree_attach(widp w);
static void wid_tree_remove(widp w);
static void wid_tree2_unsorted_remove(widp w);
static void wid_tree3_moving_wids_remove(widp w);
static void wid_tree3_moving_wids_insert(widp w);
static void wid_tree4_wids_being_destroyed_remove(widp w);
static void wid_tree4_wids_being_destroyed_insert(widp w);
static void wid_tree5_ticking_wids_remove(widp w);
static void wid_tree5_ticking_wids_insert(widp w);
static void wid_move_dequeue(widp w);

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

/*
 * A tile over the mouse pointer
 */
widp wid_mouse_template;

static uint8_t wid_init_done;
static uint8_t wid_exiting;

tree_rootp wid_timers;

uint8_t wid_init (void)
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

        wid_gc_all();

        {
            widp child;
            TREE_WALK(wid_top_level, child) {
                wid_destroy_immediate(child);
            }
        }

        tree_destroy(&wid_top_level, (tree_destroy_func)0);
        tree_destroy(&wid_top_level3, (tree_destroy_func)0);
        tree_destroy(&wid_top_level2, (tree_destroy_func)0);
        tree_destroy(&wid_top_level5, (tree_destroy_func)0);
        tree_destroy(&wid_top_level4, (tree_destroy_func)0);

        if (wid_tooltip_string) {
            myfree(wid_tooltip_string);
            wid_tooltip_string = 0;
        }

        action_timers_destroy(&wid_timers);
    }
}

static void wid_grid_tree_attach (widp w)
{
    widgrid *grid;

    if (!w->can_be_atteched_now) {
        return;
    }

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

    uint8_t depth = w->tree.z_depth;

    tree_root **newtree = grid->grid_of_trees[depth] + (y * grid->width) + x;

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
            ERR("wid remove from grid");
        }
    }

    w->gridtree = *newtree;

    /*
     * Now add to the new tree.
     */
    if (!tree_insert(w->gridtree, &w->gridnode->tree.node)) {
        ERR("wid insert %s to grid", w->logname);
    }

    grid->bounds_valid = false;
    w->gridnode->x = x;
    w->gridnode->y = y;
    w->gridnode->aligned_x = !((mx + (grid->pixwidth/2)) % grid->pixwidth);
    w->gridnode->aligned_y = !((my + (grid->pixheight/2)) % grid->pixheight);

    if (w->thing) {
        thing_map_add(w->thing, x, y);
    }

    /*
     * Optimization do not keep grid wids which are numerous on the big
     * parent tree where it is slow to add/remove.
     */
    wid_tree_remove(w);
}

static uint8_t wid_grid_tree_detach (widp w)
{
    fast_verify(w);

    /*
     * On the grid ?
     */
    if (!w->gridnode) {
        return (false);
    }

    if (!tree_remove(w->gridtree, &w->gridnode->tree.node)) {
        ERR("wid remove from grid for wid %s", w->logname);
    }

    myfree(w->gridnode);

    w->gridnode = 0;
    w->gridtree = 0;

    if (w->thing) {
        thing_map_remove(w->thing);
    }

    return (true);
}

void wid_get_grid_coord (widp w, int32_t *x, int32_t *y,
                         uint8_t *aligned_x,
                         uint8_t *aligned_y)
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
        ERR("no parent grid");
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

static void wid_get_tl_x_tl_y_br_x_br_y (widp w, 
                                         double *tl_x,
                                         double *tl_y,
                                         double *br_x,
                                         double *br_y)
{
    const double tlx = w->tree.tl.x;
    const double tly = w->tree.tl.y;
    const double brx = w->tree.br.x;
    const double bry = w->tree.br.y;

    const double cx = (tlx + brx) / 2.0;
    const double cy = (tly + bry) / 2.0;
    const double scaling = wid_get_scaling_w(w);

    *tl_x = cx - ((cx - tlx) * scaling);
    *tl_y = cy - ((cy - tly) * scaling);
    *br_x = cx + ((brx - cx) * scaling);
    *br_y = cy + ((bry - cy) * scaling);
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

    w->can_be_atteched_now = true;
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

    w->can_be_atteched_now = true;
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

    w->can_be_atteched_now = true;
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

    w->can_be_atteched_now = true;
    wid_tree_attach(w);
}

void wid_get_tl_br (widp w, fpoint *tl, fpoint *br)
{
    fast_verify(w);

    *tl = w->tree.tl;
    *br = w->tree.br;
}

void wid_set_tl_br_no_relative_offset (widp w, fpoint tl, fpoint br)
{
    wid_tree_detach(w);

    w->tree.tl = tl;
    w->tree.br = br;

    w->can_be_atteched_now = true;
    wid_tree_attach(w);
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

void wid_set_client_context2 (widp w, void *client_context2)
{
    verify(w);

    w->client_context2 = client_context2;
}

void *wid_get_client_context2 (widp w)
{
    verify(w);

    return (w->client_context2);
}

void wid_set_client_context3 (widp w, void *client_context3)
{
    verify(w);

    w->client_context3 = client_context3;
}

void *wid_get_client_context3 (widp w)
{
    verify(w);

    return (w->client_context3);
}

void wid_set_prev (widp w, widp prev)
{
    fast_verify(w);

    if (w == prev) {
        ERR("wid list loop");
    }

    w->prev = prev;

    if (prev) {
        prev->next = w;
    }
}

widp wid_get_prev (widp w)
{
    if (w->prev == w) {
        ERR("wid list get prev loop");
    }

    return (w->prev);
}

widp wid_get_next (widp w)
{
    if (w->next == w) {
        ERR("wid list get next loop");
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

void wid_set_ignore_events (widp w, uint8_t val)
{
    w->ignore_for_events = val;
}

void wid_set_disable_scissors (widp w, uint8_t val)
{
    w->disable_scissors = val;
}

/*
 * Should this widget be ignored for events?
 */
uint8_t wid_ignore_for_events (widp w)
{
    widp top;

    if (!w) {
        return (true);
    }

    verify(w);

    if (w->ignore_for_events || w->moving ||
        /*
         * Need this disabled for fading buttons to work
         */
        (!w->fade_count && (w->fade_out || w->fade_in)) ||
        w->hidden ||
        w->being_destroyed) {
        return (true);
    }

    if (w->parent) {
        top = wid_get_top_parent(w);

        if (top->moving ||
            /*
             * Need this disabled for fading buttons to work
             */
            (!w->fade_count && (w->fade_in)) ||
            top->hidden ||
            top->being_destroyed) {
            return (true);
        }
    }

    return (false);
}

uint8_t wid_ignore_for_focus (widp w)
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
uint8_t wid_ignore_being_destroyed (widp w)
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

    if (!wid_mouse_visible) {
        return;
    }

    w = wid_over;
    wid_over = 0;

    if (!w) {
        return;
    }

    wid_set_mode(w, WID_MODE_NORMAL);

    if (w->on_mouse_over_end) {
        w->on_mouse_over_end(w);
    }
}

static uint8_t wid_mouse_over_begin (widp w, uint32_t x, uint32_t y,
                                     int32_t relx, int32_t rely,
                                     int32_t wheelx, int32_t wheely)
{
    if (!wid_mouse_visible) {
        return (false);
    }

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
        (w->on_mouse_over_begin)(w, relx, rely, wheelx, wheely);
    }

    if (w->tooltip) {
        if (wid_tooltip_string) {
            if (!strcmp(wid_tooltip_string, w->tooltip)) {
                return (true);
            }

            wid_destroy(&wid_popup_tooltip);
        }

        if (wid_tooltip_string) {
            myfree(wid_tooltip_string);
            wid_tooltip_string = 0;
        }

        wid_tooltip_string = dupstr(w->tooltip, "tooltop str");

        wid_popup_tooltip = wid_tooltip(w->tooltip, 0.5, 0.0, 
                                        w->tooltip_font ? 
                                        w->tooltip_font : small_font);

        /*
         * Move just above and to the left of the widget.
         */
#ifdef ENABLE_TOOLTIP_OVER_MOUSE
        double px = x - wid_get_width(wid_popup_tooltip) - wid_get_width(w);
        double py = y - wid_get_height(wid_popup_tooltip) - wid_get_height(w);

        if (py < 0) {
            py = y + wid_get_height(w);
        }

        if (px < 0) {
            px = x + wid_get_width(w);
        }

        wid_move_to_abs(wid_popup_tooltip, px, py);
#else
        /*
         * If the mouse is over the pop up window, move the window a bit
         */
        int32_t mx = global_config.video_gl_width / 2;
        int32_t wx = wid_get_width(wid_popup_tooltip) / 2;
        int32_t maxy = (global_config.video_gl_height * 0.2) +
                        wid_get_height(wid_popup_tooltip);

        double minx = mx - wx;
        double maxx = mx + wx;

        double atx = 0.5;

        if ((mouse_x >= mx) && (mouse_x < maxx) && (mouse_y < maxy)) {
            atx = 0.25;
        }

        if ((mouse_x <= mx) && (mouse_x > minx) && (mouse_y < maxy)) {
            atx = 0.75;
        }

        wid_move_to_pct_centered(wid_popup_tooltip, atx, -0.5);
        wid_move_to_pct_centered_in(wid_popup_tooltip, atx, 0.2, 10);

        wid_destroy_ptr_in(&wid_popup_tooltip, 1500);
#endif

        if (w->on_tooltip) {
            (w->on_tooltip)(w, wid_popup_tooltip);
        }
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
                case SDLK_QUOTE: return ('\'');
                case SDLK_COMMA: return ('<');
                case SDLK_MINUS: return ('_');
                case SDLK_PERIOD: return ('>');
                case SDLK_SLASH: return ('?');
                case SDLK_EQUALS: return ('+');
                case SDLK_0: return (')');
                case SDLK_1: return ('!');
                case SDLK_2: return ('"');
                case SDLK_3: return ('#');
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

    w->timestamp_last_mode_change = wid_time;;
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

const char *wid_get_name (widp w)
{
    fast_verify(w);

    return (w->name);
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

void wid_set_debug (widp w, uint8_t val)
{
    w->debug = val;
}

void wid_set_animate (widp w, uint8_t val)
{
    w->animate = val;
}

void wid_set_text (widp w, const char *string)
{
    fast_verify(w);

    if (!string) {
        w->text[0] = '\0';
    } else {
        if (!strcasecmp(string, w->text)) {
            return;
        }

        strlcpy(w->text, string, MAXSTR);
    }

    if (!w->name && string) {
        wid_set_name(w, string);
    }

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

void wid_set_tooltip (widp w, const char *string,
                      fontp font)
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
    w->tooltip_font = font;
}

uint8_t wid_get_received_input (widp w)
{
    return (w->received_input);
}

void wid_set_received_input (widp w, uint8_t val)
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
uint8_t wid_get_show_cursor (widp w)
{
    return (w->show_cursor);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_show_cursor (widp w, uint8_t val)
{
    fast_verify(w);

    w->show_cursor = val;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
uint8_t wid_get_do_not_raise (widp w)
{
    return (w->do_not_raise);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_do_not_raise (widp w, uint8_t val)
{
    fast_verify(w);

    w->do_not_raise = val;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
uint8_t wid_get_do_not_lower (widp w)
{
    return (w->do_not_lower);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_do_not_lower (widp w, uint8_t val)
{
    fast_verify(w);

    w->do_not_lower = val;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
uint8_t wid_get_movable (widp w)
{
    if (w->movable_set) {
        return (w->movable);
    }

    return (false);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_movable (widp w, uint8_t val)
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
uint8_t wid_get_movable_horiz (widp w)
{
    if (w->movable_horiz_set) {
        return (w->movable_horiz);
    }

    return (false);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_movable_horiz (widp w, uint8_t val)
{
    fast_verify(w);

    w->movable_horiz = val;
    w->movable_horiz_set = true;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
uint8_t wid_get_movable_vert (widp w)
{
    if (w->movable_vert_set) {
        return (w->movable_vert);
    }

    return (false);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_movable_vert (widp w, uint8_t val)
{
    fast_verify(w);

    w->movable_vert = val;
    w->movable_vert_set = true;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
uint8_t wid_get_movable_bounded (widp w)
{
    if (w->movable_bounded_set) {
        return (w->movable_bounded);
    }

    return (false);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_movable_bounded (widp w, uint8_t val)
{
    fast_verify(w);

    w->movable_bounded = val;
    w->movable_bounded_set = true;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
uint8_t wid_get_text_lhs (widp w)
{
    return (w->text_lhs);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_text_lhs (widp w, uint8_t val)
{
    fast_verify(w);

    w->text_lhs = val;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
uint8_t wid_get_text_rhs (widp w)
{
    return (w->text_rhs);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_text_rhs (widp w, uint8_t val)
{
    fast_verify(w);

    w->text_rhs = true;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
uint8_t wid_get_text_centerx (widp w)
{
    return (w->text_centerx);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_text_centerx (widp w, uint8_t val)
{
    fast_verify(w);

    w->text_centerx = val;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
uint8_t wid_get_text_top (widp w)
{
    return (w->text_top);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_text_top (widp w, uint8_t val)
{
    fast_verify(w);

    w->text_top = val;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
uint8_t wid_get_text_bot (widp w)
{
    return (w->text_bot);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_text_bot (widp w, uint8_t val)
{
    fast_verify(w);

    w->text_bot = val;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
uint8_t wid_get_text_centery (widp w)
{
    return (w->text_centery);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_text_centery (widp w, uint8_t val)
{
    fast_verify(w);

    w->text_centery = val;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
uint8_t wid_get_text_pos (widp w, double *x, double *y)
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
void wid_set_text_pos (widp w, uint8_t val, double x, double y)
{
    fast_verify(w);

    w->text_pos.x = x;
    w->text_pos.y = y;
    w->text_pos_set = val;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
uint8_t wid_get_text_outline (widp w)
{
    return (w->text_outline);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_text_outline (widp w, uint8_t val)
{
    fast_verify(w);

    w->text_outline = val;
}

uint8_t wid_get_blit_outline (widp w)
{
    return (w->blit_outline);
}

void wid_set_blit_outline (widp w, double val)
{
    w->blit_outline = val ? 1 : 0;
    w->blit_outline_val = val;
}

uint8_t wid_get_blit_center (widp w)
{
    return (w->blit_center);
}

void wid_set_blit_center (widp w, uint8_t val)
{
    w->blit_center = val ? 1 : 0;
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

void wid_set_blit_y_offset (widp w, double val)
{
    fast_verify(w);

    w->blit_y_offset = val;
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
uint8_t wid_get_text_fixed_width (widp w)
{
    return (w->text_fixed_width);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_text_fixed_width (widp w, uint8_t val)
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
        ERR("failed to set wid tex %s", tex);
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

/*
 * Look at all the wid modes and return the most relevent setting
 */
static tilep wid_get_tile_eyes (widp w)
{
    return (w->tile_eyes);
}

thingp wid_get_thing (widp w)
{
    fast_verify(w);

    return (w->thing);
}

tpp wid_get_thing_template (widp w)
{
    fast_verify(w);

    return (w->tp);
}

void wid_set_tilename (widp w, const char *name)
{
    fast_verify(w);

    tilep tile = tile_find(name);
    if (!tile) {
        ERR("failed to find wid tile %s", name);
    }

    w->tile = tile;
    if (!w->first_tile) {
        w->first_tile = tile;
    }

    thingp t = wid_get_thing(w);

    if (t && thing_is_cats_eyes(t)) {
        char tmp[SMALL_STRING_LEN_MAX];

        snprintf(tmp, sizeof(tmp), "%s-eyes", name);

        tilep tile = tile_find(tmp);
        if (!tile) {
            /*
             * knock off the last char off of the name so things like 
             * warrior-right3 becomes warrior-right-eyes which might exist if 
             * we do not need eyes for every tile.
             */
            snprintf(tmp + strlen(name) - 1, sizeof(tmp), "-eyes");
            tile = tile_find(tmp);
            if (!tile) {
                /*
                 * Not all tiles, like if the player is dying, need to have 
                 * cats eyes.
                 *
                ERR("failed to set wid tile %s for eyes", tmp);
                 */
            }
        }

        w->tile_eyes = tile;
    }
}

void wid_set_tile (widp w, tilep tile)
{
    fast_verify(w);

    w->tile = tile;
    if (!w->first_tile) {
        w->first_tile = tile;
    }

    thingp t = wid_get_thing(w);

    if (t && thing_is_cats_eyes(t)) {
        char tmp[SMALL_STRING_LEN_MAX];
        const char *name = tile_name(tile);

        snprintf(tmp, sizeof(tmp), "%s-eyes", name);

        tilep tile = tile_find(tmp);
        if (!tile) {
            snprintf(tmp + strlen(name) - 1, sizeof(tmp), "-eyes");
            tile = tile_find(tmp);
            if (!tile) {
                /*
                 * Not all tiles, like if the player is dying, need to have 
                 * cats eyes.
                 *
                ERR("failed to set wid tile %s for eyes", tmp);
                 */
            }
        }

        w->tile_eyes = tile;
    }
}

/*
 * Scale a tile defined by its corners and recenter it based on the graphics 
 * in that tile.
 */
static void wid_tile_scale (widp w, 
                            fpoint tl,
                            fpoint br,
                            fpoint *new_tl,
                            fpoint *new_br,
                            double scale) 
{
#ifdef TEST_SCALING
    static double dd = 0.5;
    static double del = 0.001;

    scale += dd;
    dd += del;

    if (dd > 1.0) {
        del = -del;
    }
    if (dd < 0.01) {
        del = -del;
    }

    scale = dd;
#endif

    double ow = br.x - tl.x;
    double oh = br.y - tl.y;
    double nw = ow * scale;
    double nh = oh * scale;

    /*
     * Scale the tile.
     */
    double mx = (tl.x + br.x) / 2.0;
    double my = (tl.y + br.y) / 2.0;

    new_tl->x = mx - (nw / 2.0);
    new_tl->y = my - (nh / 2.0);
    new_br->x = mx + (nw / 2.0);
    new_br->y = my + (nh / 2.0);
}

void wid_set_z_depth (widp w, uint8_t z_depth)
{
    fast_verify(w);

    if (z_depth >= MAP_DEPTH_MAX) {
        ERR("setting depth for %s to %u", w->logname, z_depth);
        return;
    }

    wid_tree_detach(w);
    w->tree.z_depth = z_depth;
    wid_tree_attach(w);
}

uint8_t wid_get_z_depth (widp w)
{
    fast_verify(w);

    return (w->tree.z_depth);
}

void wid_set_thing (widp w, thingp t)
{
    tree_rootp tiles;
    thing_tilep tile;

    fast_verify(w);

    w->thing = t;
    if (!t) {
        return;
    }

    wid_set_z_depth(w, thing_z_depth(t));

    tiles = thing_tile_tiles(t);
    if (!tiles) {
        return;
    }

    /*
     * Get the first tile and not random on purpose as animations will
     * use the first tile as a center. If the widget is remade again and
     * again then the tile will appear to wobble as a new center is chosen.
     * i.e. the inventory screen with a torch.
     */
    tile = (typeof(tile)) tree_root_first(tiles);
    if (!tile) {
        return;
    }

    wid_set_tilename(w, thing_tile_name(tile));
    wid_set_name(w, thing_name(t));

    thing_set_wid(t, w);
}

void wid_set_thing_template (widp w, tpp t)
{
    tree_rootp tiles;
    thing_tilep tile;

    fast_verify(w);

    w->tp = t;

    if (!t) {
        wid_set_z_depth(w, 0);
        wid_set_tile(w, 0);
        return;
    }

    wid_set_z_depth(w, tp_get_z_depth(t));

    tiles = tp_get_tiles(t);
    if (!tiles) {
        return;
    }

    tile = (typeof(tile)) tree_root_first(tiles);
    if (!tile) {
        return;
    }

    wid_set_tilename(w, thing_tile_name(tile));

    wid_set_name(w, tp_name(t));
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
uint8_t wid_get_bevelled (widp w)
{
    return (w->bevelled);
}

/*
 * Look at all the widset modes and return the most relevent setting
 */
void wid_set_bevelled (widp w, uint8_t val)
{
    fast_verify(w);

    w->bevelled = val;
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
uint8_t wid_get_rounded (widp w)
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
    wid_set_sides(w, 8);
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
    wid_set_sides(w, 4);
}

/*
 * Look at all the wid modes and return the most relevent setting
 */
uint8_t wid_get_square (widp w)
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
uint8_t wid_get_square_outline (widp w)
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
    /*
     * Don't allow focus override if hard focus is set.
     */
    if (w) {
        if (wid_focus_locked) {
            if (wid_get_top_parent(w) != wid_focus_locked) {
                return;
            }
        }

        if (!w->focus_order) {
            return;
        }
    }

    wid_mouse_focus_begin(w);
}

void wid_focus_lock (widp w)
{
    WID_DBG(w, "focus locked");

    wid_focus_locked = w;
}

void wid_set_on_tooltip (widp w, on_tooltip_t fn)
{
    fast_verify(w);

    w->on_tooltip = fn;
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

void wid_set_on_joy_down (widp w, on_joy_button_t fn)
{
    fast_verify(w);

    w->on_joy_button = fn;
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

void wid_set_on_destroy_begin (widp w, on_destroy_t fn)
{
    fast_verify(w);

    w->on_destroy_begin = fn;
}

void wid_set_on_tick (widp w, on_tick_t fn)
{
    fast_verify(w);

    if (!fn) {
        ERR("no ticker function set");
    }

    w->on_tick = fn;

    wid_tree5_ticking_wids_insert(w);
}

void wid_set_on_display (widp w, on_display_t fn)
{
    fast_verify(w);

    if (!fn) {
        ERR("no ticker function set");
    }

    w->on_display = fn;
}

void wid_set_on_display_top_level (widp w, on_display_top_level_t fn)
{
    fast_verify(w);

    if (!fn) {
        ERR("no ticker function set");
    }

    w->on_display_top_level = fn;
}

/*
 * Remove this wid from any trees it is in.
 */
static void wid_tree_detach (widp w)
{
    verify(w);

    wid_grid_tree_detach(w);

    wid_tree_remove(w);
}

/*
 * Add back to all trees.
 */
static void wid_tree_attach (widp w)
{
    fast_verify(w);

    if (w->parent && w->parent->grid) {
        wid_grid_tree_attach(w);
        return;
    }

    tree_root *root;

    if (!w->parent) {
        root = wid_top_level;
    } else {
        root = w->parent->children_display_sorted;
    }

    if (!root) {
        return;
    }

    if (!tree_insert(root, &w->tree.node)) {
        ERR("wid set z depth tree insert");
    }

    w->in_tree = root;
}

static void wid_tree_insert (widp w)
{
    fast_verify(w);

    static int32_t key;

    tree_root *root;

    /*
     * Get a wid sort ID.
     */
    w->tree.key = ++key;

    /*
     * If we're going to put it on the grid don't put it on the big tree.
     */
    if (w->parent && w->parent->grid) {
        return;
    }

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

    if (!tree_insert(root, &w->tree.node)) {
        ERR("widget tree insert");
    }

    w->in_tree = root;
}

static void wid_tree2_unsorted_insert (widp w)
{
    fast_verify(w);

    static int32_t key;

    tree_root *root;

    if (!w->parent) {
        root = wid_top_level2;
    } else {
        root = w->parent->tree2_children_unsorted;
    }

    if (!root) {
        root = tree_alloc(TREE_KEY_INTEGER, "TREE ROOT2: wid");
        if (!w->parent) {
            wid_top_level2 = root;
        } else {
            w->parent->tree2_children_unsorted = root;
        }

        root->offset = STRUCT_OFFSET(struct wid_, tree2_unsorted);
    }

    /*
     * Get a wid sort ID.
     */
    w->tree2_unsorted.key = ++key;

    if (!tree_insert(root, &w->tree2_unsorted.node)) {
        ERR("widget tree2_unsorted insert");
    }

    /*
     * The other tree will do the actual node free.
     */
    w->tree2_unsorted.node.is_static_mem = true;
    w->in_tree2_unsorted = root;
}

static void wid_tree3_moving_wids_insert (widp w)
{
    fast_verify(w);

    if (w->in_tree3_moving_wids) {
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

        root->offset = STRUCT_OFFSET(struct wid_, tree3_moving_wids);
    }

    /*
     * Get a wid sort ID.
     */
    w->tree3_moving_wids.key = ++key;

    if (!tree_insert(root, &w->tree3_moving_wids.node)) {
        ERR("widget tree3_moving_wids insert");
    }

    /*
     * The other tree will do the actual node free.
     */
    w->tree3_moving_wids.node.is_static_mem = true;
    w->in_tree3_moving_wids = root;
}

static void wid_tree4_wids_being_destroyed_insert (widp w)
{
    fast_verify(w);

    if (w->in_tree4_wids_being_destroyed) {
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

        root->offset = STRUCT_OFFSET(struct wid_, tree4_wids_being_destroyed);
    }

    /*
     * Get a wid sort ID.
     */
    w->tree4_wids_being_destroyed.key = ++key;

    if (!tree_insert(root, &w->tree4_wids_being_destroyed.node)) {
        ERR("widget tree4_wids_being_destroyed insert");
    }

    /*
     * The other tree will do the actual node free.
     */
    w->tree4_wids_being_destroyed.node.is_static_mem = true;
    w->in_tree4_wids_being_destroyed = root;
}

static void wid_tree5_ticking_wids_insert (widp w)
{
    fast_verify(w);

    if (w->in_tree5_ticking_wids) {
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

        root->offset = STRUCT_OFFSET(struct wid_, tree5_ticking_wids);
    }

    /*
     * Get a wid sort ID.
     */
    w->tree5_ticking_wids.key = ++key;

    if (!tree_insert(root, &w->tree5_ticking_wids.node)) {
        ERR("widget tree5_ticking_wids insert");
    }

    /*
     * The other tree will do the actual node free.
     */
    w->tree5_ticking_wids.node.is_static_mem = true;
    w->in_tree5_ticking_wids = root;
}

static void wid_tree_remove (widp w)
{
    fast_verify(w);

    tree_root *root = w->in_tree;
    if (!root) {
        return;
    }
    verify(root);

    if (!tree_remove(root, &w->tree.node)) {
        ERR("remove %s from display sorted tree", w->logname);
    }

    w->in_tree = 0;
}

static void wid_tree2_unsorted_remove (widp w)
{
    fast_verify(w);

    tree_root *root = w->in_tree2_unsorted;
    if (!root) {
        return;
    }
    verify(root);

    if (!tree_remove(root, &w->tree2_unsorted.node)) {
        ERR("remove %s from unsorted tree", w->logname);
    }

    w->in_tree2_unsorted = false;
}

static void wid_tree3_moving_wids_remove (widp w)
{
    fast_verify(w);

    tree_root *root = w->in_tree3_moving_wids;
    if (!root) {
        return;
    }
    verify(root);

    if (!tree_remove(root, &w->tree3_moving_wids.node)) {
        ERR("remove from move tree");
    }

    w->in_tree3_moving_wids = false;
}

static void wid_tree4_wids_being_destroyed_remove (widp w)
{
    fast_verify(w);

    tree_root *root = w->in_tree4_wids_being_destroyed;
    if (!root) {
        return;
    }
    verify(root);

    if (!tree_remove(root, &w->tree4_wids_being_destroyed.node)) {
        ERR("remove from gc tree");
    }

    w->in_tree4_wids_being_destroyed = 0;
}

static void wid_tree5_ticking_wids_remove (widp w)
{
    fast_verify(w);

    tree_root *root = w->in_tree5_ticking_wids;
    if (!root) {
        return;
    }
    verify(root);

    if (!tree_remove(root, &w->tree5_ticking_wids.node)) {
        ERR("remove from tick tree");
    }

    w->in_tree5_ticking_wids = 0;
    w->on_tick = 0;
}

/*
 * Initialize a wid with basic settings
 */
static widp wid_new (widp parent)
{
    widp w;

    w = (typeof(w)) myzalloc(sizeof(*w), "widget");
    w->parent = parent;
    w->timestamp_created = wid_time;

    wid_tree_insert(w);
    wid_tree2_unsorted_insert(w);

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
    w->animate = true;

    return (w);
}

static void wid_destroy_immediate_internal (widp w)
{
    fast_verify(w);

    wid_tree3_moving_wids_remove(w);
    wid_tree4_wids_being_destroyed_remove(w);
    wid_tree5_ticking_wids_remove(w);

    if (w->thing) {
        ERR("thing still set");
        verify(w->thing);
        ERR("thing %s still set", thing_logname(w->thing));
    }

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

    /*
     * If on the grid, take us off.
     */
    wid_grid_tree_detach(w);

    /*
     * If a grid owner, destroy all nodes.
     */
    wid_destroy_grid(w);

    {
        widp child;
        TREE_WALK(w->children_display_sorted, child) {
            wid_destroy_immediate(child);
        }
    }

    {
        widp child;
        TREE_OFFSET_WALK(w->tree2_children_unsorted, child, tree2_unsorted) {
            wid_destroy_immediate(child);
        }
    }

    w->parent = 0;
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

    wid_tree2_unsorted_remove(w);

    wid_destroy_immediate_internal(w);

    tree_destroy(&w->children_display_sorted, (tree_destroy_func)0);
    tree_destroy(&w->tree2_children_unsorted, (tree_destroy_func)0);

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

    if (w == wid_popup_tooltip) {
        wid_popup_tooltip = 0;
        if (wid_tooltip_string) {
            myfree(wid_tooltip_string);
            wid_tooltip_string = 0;
        }
    }

    if (w == wid_focus_locked) {
        wid_focus_locked = 0;
    }

    if (w == wid_focus) {
        wid_focus = 0;
    }

    if (w == wid_over) {
        wid_over = 0;
    }

    if (w == wid_moving) {
        wid_moving = 0;
    }

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
    wid_tree4_wids_being_destroyed_insert(w);

    if (wid_focus == w) {
        wid_mouse_focus_end();
    }

    if (wid_over == w) {
        wid_mouse_over_end();
    }

    if (wid_moving == w) {
        wid_mouse_motion_end();
    }

    TREE_OFFSET_WALK(w->tree2_children_unsorted, child, tree2_unsorted) {
        widp c;

        fast_verify(child);
        c = child;

        wid_destroy(&c);
    }

    if (!w->parent) {
        wid_fade_out(w, delay);

        wid_get_abs_coords(w, &tlx, &tly, &brx, &bry);
    }

    if (w->on_destroy_begin) {
        (w->on_destroy_begin)(w);
    }

    /*
     * Make sure it stops ticking right now as client pointers this widget 
     * might use in the ticker may no longer be valid.
     */
    wid_tree5_ticking_wids_remove(w);
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

    w->destroy_when = wid_time + ms;

    wid_tree4_wids_being_destroyed_insert(w);
}

void wid_destroy_ptr_in (widp *w, uint32_t ms)
{
    fast_verify(*w);

    (*w)->destroy_when = wid_time + ms;
    (*w)->destroy_ptr = w;

    wid_tree4_wids_being_destroyed_insert(*w);
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

#ifdef WID_FULL_LOGNAME
    w->logname = dynprintf("%s[%p] (parent %s[%p])", name, w,
                           parent->logname, parent);
#else
    w->logname = dynprintf("%s[%p]", name, w);
#endif

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
    if (!parent) {
        ERR("no parent");
    }

    widp w = wid_new(parent);

#ifdef WID_FULL_LOGNAME
    w->logname = dynprintf("%s[%p] (parent %s[%p])", name, w,
                           parent->logname, parent);
#else
    w->logname = dynprintf("%s[%p]", name, w);
#endif
    
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
    wid_set_bevel(w, 8);
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
    wid_set_color(w, WID_COLOR_TEXT, WHITE);
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
    wid_set_color(w, WID_COLOR_TEXT, WHITE);
    wid_set_movable(w, true);
    wid_set_name(w, name);

    wid_raise(w);

    return (w);
}

widp wid_new_square_button (widp parent, const char *name)
{
    if (!parent) {
        ERR("no parent");
    }

    widp w = wid_new(parent);

#ifdef WID_FULL_LOGNAME
    w->logname = dynprintf("%s[%p] (parent %s[%p])", name, w,
                           parent->logname, parent);
#else
    w->logname = dynprintf("%s[%p]", name, w);
#endif

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
        wid_set_color(w, WID_COLOR_BG, RED);
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
        wid_set_color(w, WID_COLOR_BG, RED);
    }

    wid_set_mode(w, WID_MODE_NORMAL); {
        wid_set_color(w, WID_COLOR_BG, WHITE);
    }

    return (w);
}

widp wid_new_rounded_button (widp parent, const char *name)
{
    if (!parent) {
        ERR("no parent");
    }

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
    if (!parent) {
        ERR("no parent");
    }

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
    if (!parent) {
        ERR("no parent");
    }

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
                                uint8_t vertical)
{
    if (!parent) {
        ERR("no parent");
    }

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
        c = RED;
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
    if (!parent) {
        ERR("no parent");
    }

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
    if (!parent) {
        ERR("no parent");
    }

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

uint8_t wid_has_grid (widp w)
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
        ERR("grid exists");
    }

    grid = w->grid = (widgrid *) myzalloc(sizeof(widgrid), "widgrid ptr");
    grid->width = width;
    grid->height = height;
    grid->pixwidth = pixwidth;
    grid->pixheight = pixheight;
    grid->nelems = width * height;

    uint8_t z;

    for (z = 0; z < MAP_DEPTH_MAX; z++) {
        grid->grid_of_trees[z] = (tree_root **)
            myzalloc(sizeof(tree_root *) * grid->nelems, "widgrid tree ptrs");

        for (i = 0; i < width * height; i++) {
            grid->grid_of_trees[z][i] =
                tree_alloc_custom(tree_wid_compare_func_fast, "TREE ROOT: widgrid");
        }
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

    int32_t x, y, z;

    for (z = 0; z < MAP_DEPTH_MAX; z++) {
        for (x = 0; x < MAP_WIDTH; x++) {
            for (y = 0; y < MAP_HEIGHT; y++) {

                tree_root **tree;
retry:
                tree = grid->grid_of_trees[z] + (y * grid->width) + x;
                widgridnode *node;

                TREE_WALK_REVERSE_UNSAFE_INLINE(*tree, node,
                                                tree_prev_tree_wid_compare_func_fast) {

                    wid_destroy_immediate(node->wid);
                    goto retry;
                }
            }
        }
    }

    w->grid = 0;

    for (z = 0; z < MAP_DEPTH_MAX; z++) {
        for (i = 0; i < grid->nelems; i++) {
            tree_destroy(&grid->grid_of_trees[z][i], (tree_destroy_func)0);
        }

        myfree(grid->grid_of_trees[z]);
    }

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

    uint8_t z;

    for (z = 0; z < MAP_DEPTH_MAX; z++) {
        for (i = 0; i < grid->nelems; i++) {
            TREE_WALK(grid->grid_of_trees[z][i], node) {
                child = node->wid;

                wid_destroy_nodelay(&child);
            }
        }

        for (i = 0; i < grid->nelems; i++) {
            TREE_WALK(grid->grid_of_trees[z][i], node) {
                child = node->wid;

                ERR("%s exists after empty grid",child->name);
            }
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
        ERR("no grid");
        return;
    }

    uint8_t z;

    for (z = 0; z < MAP_DEPTH_MAX; z++) {
        for (i = 0; i < grid->nelems; i++) {
retry:
            {
                TREE_WALK(grid->grid_of_trees[z][i], node) {
                    child = node->wid;

                    wid_tree_remove(child);
                    wid_tree2_unsorted_remove(child);
                    wid_tree3_moving_wids_remove(child);
                    wid_tree4_wids_being_destroyed_remove(child);
                    wid_tree5_ticking_wids_remove(child);

                    wid_grid_tree_detach(child);

                    wid_destroy_nodelay(&child);
                    goto retry;
                }
            }
        }

        for (i = 0; i < grid->nelems; i++) {
            TREE_WALK(grid->grid_of_trees[z][i], node) {
                child = node->wid;

                ERR("%s exists after clear grid",child->name);
            }
        }
    }
}

static void wid_attach_to_grid_internal (widp w)
{
    widp child;

    { TREE_OFFSET_WALK(w->tree2_children_unsorted, child, tree2_unsorted) {
        wid_attach_to_grid_internal(child);
    } }

    wid_grid_tree_attach(w);
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

    uint8_t z;

    for (z = 0; z < MAP_DEPTH_MAX; z++) {
        for (i = 0; i < grid->nelems; i++) {
            TREE_WALK(grid->grid_of_trees[z][i], node) {
                child = node->wid;

                x = i % grid->width;
                y = i / grid->height;

                if (!child->tp) {
                    continue;
                }

                if (tp_is_hidden_from_editor(child->tp)) {
                    continue;
                }

                PUT_BRA(ctx);

                PUT_NAMED_UINT32(ctx, "x", x);
                PUT_NAMED_UINT32(ctx, "y", y);
                PUT_NAMED_STRING(ctx, "t",
                                tp_name(child->tp));

                count = (uint32_t) (uintptr_t) wid_get_client_context(child);

                if (count > 0) {
                    PUT_NAMED_UINT32(ctx, "count", count);
                }

                PUT_KET(ctx);
            }
        }
    }

    PUT_KET(ctx);
}

uint8_t demarshal_wid_grid (demarshal_p ctx, widp w,
                            grid_wid_replace_t callback)
{
    tpp tp;
    uint32_t width;
    uint32_t height;
    uint32_t x;
    uint32_t y;
    uint8_t rc;

    rc = true;

    GET_BRA(ctx);

    GET_NAMED_UINT32(ctx, "width", width);
    GET_NAMED_UINT32(ctx, "height", height);

    for (;;) {
        if (!GET_PEEK_BRA(ctx)) {
            break;
        }

        rc = rc && GET_BRA(ctx);

        char *name = 0;

        x = y = 0;
        
        rc = rc && GET_NAMED_UINT32(ctx, "x", x);
        rc = rc && GET_NAMED_UINT32(ctx, "y", y);
        rc = rc && GET_NAMED_STRING(ctx, "t", name);

        tp = tp_find(name);
        if (!tp) {
            ERR("thing %s not found", name);
            rc = false;
            continue;
        }

        thing_template_data data;
        int gotdata = false;

        memset(&data, 0, sizeof(data));

        if (GET_PEEK_NAME(ctx, "exit")) {
            GET_NAME(ctx, "exit");
            GET_BRA(ctx);
            GET_NAMED_INT8(ctx, "x", data.exit.x);
            GET_NAMED_INT8(ctx, "y", data.exit.y);
            GET_KET(ctx);
            data.exit_set = true;
            gotdata = true;
        }

        if (GET_PEEK_NAME(ctx, "text")) {
            GET_NAME(ctx, "text");
            GET_BRA(ctx);
            char *text = 0;
            GET_NAMED_STRING(ctx, "text", text);
            strlcpy(data.text, text, sizeof(data.text) - 1);
            GET_KET(ctx);
            myfree(text);
            gotdata = true;
        }

        if (GET_PEEK_NAME(ctx, "color")) {
            GET_NAME(ctx, "color");
            GET_BRA(ctx);

            char *name;
            GET_NAMED_STRING(ctx, "color", name);

            data.col = color_find(name);

            /*
             * Convert the dynamic name read in, into a constant string.
             */
            const char *const_name = color_find_name(name);
            myfree(name);
            data.col_name = const_name;

            GET_KET(ctx);
            gotdata = true;
        } else {
            static const char *white = "white";
            data.col_name = (char*)white;
            data.col = WHITE;
            gotdata = true;
        }

        (void) (*callback)(w, x, y,
                           0, /* thing */
                           tp,
                           gotdata ? & data : 0,
                           0 /* item */,
                           0 /* stats */);

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
                    uint8_t z_depth)
{
    widgridnode *node;
    widgrid *grid;
    widp w;

    grid = parent->grid;
    if (!grid) {
        ERR("no grid wid in wid_grid_find");
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
    tree_root **gridtree = grid->grid_of_trees[z_depth] + 
                    (y * grid->width) + x;

    /*
     * Trees should be already allocated.
     */
    if (!*gridtree) {
        ERR("no gridtree");
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
                                   tp_is_callback func)
{
    widgridnode *node;
    widgrid *grid;
    widp w;

    grid = parent->grid;
    if (!grid) {
        ERR("no grid wid in wid_grid_find_thing_template");
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
    uint8_t z;

    for (z = 0; z < MAP_DEPTH_MAX; z++) {
        tree_root **gridtree = grid->grid_of_trees[z] + (y * grid->width) + x;

        /*
         * Trees should be already allocated.
         */
        if (!*gridtree) {
            ERR("no gridtree");
        }

        TREE_WALK(*gridtree, node) {
            w = node->wid;

            if (!w->tp) {
                continue;
            }

            if ((*func)(w->tp)) {
                fast_verify(w);

                return (w);
            }
        }
    }

    return (0);
}

/*
 * Fast find in a 2d array of trees.
 */
widp wid_grid_find_tp_is (widp parent,
                                      uint32_t x,
                                      uint32_t y,
                                      tpp tp)
{
    widgridnode *node;
    widgrid *grid;
    widp w;

    grid = parent->grid;
    if (!grid) {
        ERR("no grid wid in wid_grid_find_tp_is");
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
    uint8_t z;

    for (z = 0; z < MAP_DEPTH_MAX; z++) {
        tree_root **gridtree = grid->grid_of_trees[z] + (y * grid->width) + x;

        /*
         * Trees should be already allocated.
         */
        if (!*gridtree) {
            ERR("no gridtree");
        }

        TREE_WALK(*gridtree, node) {
            w = node->wid;

            if (w->tp == tp) {
                return (w);
            }
        }
    }

    return (0);
}

/*
 * Find the first widget in the grid at this tile co-ordinate.
 */
widp wid_grid_find_first (widp parent, uint32_t x, uint32_t y,
                          uint8_t depth)
{
    widgridnode *node;
    widgrid *grid;
    widp w;

    grid = parent->grid;
    if (!grid) {
        ERR("no grid wid in wid_grid_find_first");
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
    tree_root **gridtree = grid->grid_of_trees[depth] + (y * grid->width) + x;

    /*
     * Trees should be already allocated.
     */
    if (!*gridtree) {
        ERR("no gridtree");
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
widp wid_grid_find_next (widp parent, widp w, uint32_t x, uint32_t y,
                         uint8_t depth)
{
    widgridnode *node;
    widgrid *grid;

    grid = parent->grid;
    if (!grid) {
        ERR("no grid wid in wid_grid_find_next");
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
        tree_root **gridtree = 
                        grid->grid_of_trees[depth] + (y * grid->width) + x;

        /*
         * Trees should be already allocated.
         */
        if (!*gridtree) {
            ERR("no gridtree");
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
    widgrid *grid;
    widp w;

    grid = parent->grid;

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
    int8_t z;

    for (z = MAP_DEPTH_MAX - 1; z >= 0; z--) {
        tree_root **gridtree = grid->grid_of_trees[z] + (y * grid->width) + x;

        /*
         * Trees should be already allocated.
         */
        if (!*gridtree) {
            ERR("no gridtree");
        }

        TREE_WALK(*gridtree, node) {
            w = node->wid;

            return (w);
        }
    }

    return (0);
}

widp wid_find_matching (widp parent, fpoint tl, fpoint br,
                        uint8_t z_depth)
{
    widp child;

    fast_verify(parent);

    if (parent->grid) {
        return (wid_grid_find(parent, tl, br, z_depth));
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

        return (child);
    }

    return (0);
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

void wid_raise (widp w_in)
{
    if (!w_in) {
        return;
    }

    wid_raise_internal(w_in);

    /*
     * If some widget wants to be on top, let it.
     */
    widp w;
    TREE_WALK(wid_top_level, w) {
        wid_raise_override(w);
    }

    wid_find_top_focus();

    /*
     * If we were hovering over a window and it was replaced, we need to fake 
     * a mouse movement so we know we are still over it.
     */
    if (!w_in->parent && w_in->children_display_sorted) {
        wid_update_mouse();
    }
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

void wid_lower (widp w_in)
{
    if (!w_in) {
        return;
    }

    wid_lower_internal(w_in);

    /*
     * If some widget wants to be on top, let it.
     */
    widp w;
    TREE_WALK(wid_top_level, w) {
        if (w->do_not_raise) {
            wid_lower_internal(w);
            break;
        }
    }

    wid_find_top_focus();

    /*
     * If we were hovering over a window and it was replaced, we need to fake 
     * a mouse movement so we know we are still over it.
     */
    if (!w_in->parent && w_in->children_display_sorted) {
        wid_update_mouse();
    }
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

    if (strcasestr_(w->name, name)) {
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

void wid_always_hidden (widp w, uint8_t value)
{
    fast_verify(w);

    w->always_hidden = value;
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

void wid_this_visible (widp w, uint32_t delay)
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

    if (w == wid_focus) {
        wid_find_top_focus();
    }
}

void wid_this_hide (widp w, uint32_t delay)
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
}

static uint8_t wid_scroll_trough_mouse_down (widp w,
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

static uint8_t wid_scroll_trough_mouse_motion (widp w,
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
    uint8_t first = true;
    widp child;

    /*
     * Find out the space that the children take up then use this to
     * adjust the scrollbar dimensions.
     */
    if (owner->grid && (owner->grid->bounds_valid || owner->grid->bounds_locked)) {
        minx = owner->grid->tl_x;
        miny = owner->grid->tl_y;
        maxx = owner->grid->br_x;
        maxy = owner->grid->br_y;
    } else {
        TREE_OFFSET_WALK_UNSAFE(owner->tree2_children_unsorted, child) {
            double tl_x, tl_y, br_x, br_y;

            wid_get_tl_x_tl_y_br_x_br_y(child, &tl_x, &tl_y, &br_x, &br_y);

            if (first) {
                minx = tl_x;
                miny = tl_y;
                maxx = br_x;
                maxy = br_y;
                first = false;
                continue;
            }

            if (tl_x < minx) {
                minx = tl_x;
            }

            if (tl_y < miny) {
                miny = tl_y;
            }

            if (br_x > maxx) {
                maxx = br_x;
            }

            if (br_y > maxy) {
                maxy = br_y;
            }
        }

        if (owner->grid) {
            owner->grid->tl_x = minx;
            owner->grid->tl_y = miny;
            owner->grid->br_x = maxx;
            owner->grid->br_y = maxy;
            owner->grid->bounds_valid = true;
        }
    }

    double ptl_x, ptl_y, pbr_x, pbr_y;
    wid_get_tl_x_tl_y_br_x_br_y(owner, &ptl_x, &ptl_y, &pbr_x, &pbr_y);

    minx -= ptl_x;
    miny -= ptl_y;
    maxx -= ptl_x;
    maxy -= ptl_y;

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
                pct = (wid_get_tl_y(scrollbar) - 
                       wid_get_tl_y(scrollbar->parent)) /
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
                pct = (wid_get_tl_x(scrollbar) - 
                       wid_get_tl_x(scrollbar->parent)) /
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
    uint8_t first = true;
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
         *
         * No, make the clients fix their code.
         */
//        wid_set_mode(w, WID_MODE_NORMAL);
    }

    widp child;

    /*
     * Clip all the children. Avoid this for speed for the main game window.
     */
    if (w != wid_game_map_client_grid_container) {
        TREE_OFFSET_WALK_UNSAFE(w->tree2_children_unsorted, child) {
            wid_update_internal(child);
        }

        if (w->grid) {
            widgrid *grid = w->grid;
            uint32_t gx;
            uint32_t gy;
            uint8_t z;

            for (gx = 0; gx < grid->width; gx++) {
                for (gy = 0; gy < grid->height; gy++) {
                    for (z = 0; z < MAP_DEPTH_MAX; z++) {
                        tree_root **gridtree = 
                                grid->grid_of_trees[z] + (gy * grid->width) + gx;

                        /*
                         * Trees should be already allocated.
                         */
                        if (!*gridtree) {
                            ERR("no gridtree");
                        }

                        widgridnode *node;

                        TREE_WALK(*gridtree, node) {
                            widp w = node->wid;

                            wid_update_internal(w);
                        }
                    }
                }
            }
        }
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

    /*
     * If we were hovering over a window and it was replaced, we need to fake 
     * a mouse movement so we know we are still over it.
     */
    if (!w->parent && w->children_display_sorted) {
        wid_update_mouse();
    }
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

uint8_t wid_receive_input (widp w, const SDL_KEYSYM *key)
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
                if ((w != wid_console_input_line) &&
                    (w != wid_chat_input_line)) {
                    return (false);
                }

                if (origlen && (w == wid_console_input_line)) {
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
                } else if (origlen && (w == wid_chat_input_line)) {
                    strlcpy(entered, wid_get_text(w), sizeof(entered));

                    MESG(CHAT, "You say, %s", entered);

                    client_socket_shout(entered);

                    wid_set_text(w, "");
                    w->cursor = 0;
                } else if (w == wid_console_input_line) {
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
static uint8_t wid_receive_unhandled_input (const SDL_KEYSYM *key)
{
    widp w;

    w = wid_get_top_parent(wid_console_input_line);

    switch (key->mod) {
        default:

        switch ((int32_t)key->sym) {
            case '\\':
                sdl_screenshot();
                break;

            case '`':
            case '~':
                wid_toggle_hidden(w, 0);
                wid_raise(w);

                /*
                 * Need this so the console gets focus over the menu.
                 */
                if (w->visible) {
                    wid_set_focus(w);
                    wid_focus_lock(w);
                } else {
                    wid_set_focus(0);
                    wid_focus_lock(0);
                }
                break;

            case SDLK_ESCAPE:
                if (w->visible) {
                    wid_hide(w, wid_visible_delay);
                }

                /*
                 * Need this so the console gets focus over the menu.
                 */
                if (w->visible) {
                    wid_set_focus(w);
                    wid_focus_lock(w);
                } else {
                    wid_set_focus(0);
                    wid_focus_lock(0);
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
                                     uint8_t strict)
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
                                     uint8_t strict)
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

static widp wid_joy_down_handler_at (widp w, int32_t x, int32_t y,
                                     uint8_t strict)
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

        widp closer_match = wid_joy_down_handler_at(child, x, y,
                                                    true /* strict */);
        if (closer_match) {
            return (closer_match);
        }
    }

    if (w->on_joy_button) {
        if (wid_focus_locked &&
            (wid_get_top_parent(w) != wid_get_top_parent(wid_focus_locked))) {
            return (0);
        }

        return (w);
    }

    return (0);
}

static widp wid_mouse_down_handler_at (widp w, int32_t x, int32_t y,
                                       uint8_t strict)
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

static widp wid_mouse_up_handler_at (widp w, int32_t x, int32_t y, uint8_t strict)
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
                                         uint8_t strict,
                                         int depth,
                                         int debug)
{
    widp child;

    if (!w) {
        return (0);
    }

    if (wid_ignore_for_events(w)) {
        /*
         * Fading in/out ?
         */
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

    if (w->grid) {
        widgrid *grid = w->grid;
        uint32_t gx;
        uint32_t gy;
        uint8_t z;

        for (gx = 0; gx < grid->width; gx++) {
            for (gy = 0; gy < grid->height; gy++) {
                /*
                 * For the editor widget we only want to move the mouse over 
                 * the grid on the bottom level.
                 */
                z = 0; {
                    tree_root **gridtree = 
                            grid->grid_of_trees[z] + (gy * grid->width) + gx;

                    /*
                     * Trees should be already allocated.
                     */
                    if (!*gridtree) {
                        ERR("no gridtree");
                    }

                    widgridnode *node;

                    TREE_WALK(*gridtree, node) {
                        widp w = node->wid;

                        widp closer_match =
                            wid_mouse_motion_handler_at(w, x, y,
                                                        relx, rely,
                                                        wheelx, wheely,
                                                        true /* strict */,
                                                        depth + 2,
                                                        debug);
                        if (closer_match) {
                            return (closer_match);
                        }
                    }
                }
            }
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
                                            true /* strict */,
                                            depth + 2,
                                            debug);
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

    { TREE_OFFSET_WALK_UNSAFE(w->tree2_children_unsorted, child) {
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

    if (w->grid) {
        uint8_t z;
        uint16_t x, y;

        for (z = 0; z < MAP_DEPTH_MAX; z++) {
            for (x = 0; x < MAP_WIDTH; x++) {
                for (y = 0; y < MAP_HEIGHT; y++) {
                    tree_root **tree = w->grid->grid_of_trees[z] + 
                                    (y * w->grid->width) + x;
                    widgridnode *node;

                    TREE_WALK_REVERSE_UNSAFE_INLINE(*tree, node,
                                                    tree_prev_tree_wid_compare_func_fast) {
                        w->tree.tl.x += dx;
                        w->tree.tl.y += dy;
                        w->tree.br.x += dx;
                        w->tree.br.y += dy;
                    }
                }
            }
        }

        if (w->grid->bounds_valid) {
            w->grid->tl_x += dx;
            w->grid->tl_y += dy;
            w->grid->br_x += dx;
            w->grid->br_y += dy;
        }
    } else {
        { TREE_OFFSET_WALK_UNSAFE(w->tree2_children_unsorted, child) {
            wid_children_move_delta_internal(child, dx, dy);
        } }
    }

    w->can_be_atteched_now = true;
    wid_tree_attach(w);
}

void wid_move_delta (widp w, double dx, double dy)
{
    verify(w);

    wid_move_delta_internal(w, dx, dy);

    if (!w->grid) {
        wid_update_internal(w);
    }
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

static widp wid_joy_down_handler (int32_t x, int32_t y)
{
    widp w;

    { TREE_WALK(wid_top_level, w) {
        fast_verify(w);

        if (wid_focus_locked &&
            (wid_get_top_parent(w) != wid_get_top_parent(wid_focus_locked))) {
            continue;
        }

        w = wid_joy_down_handler_at(w, x, y, false /* strict */);
        if (!w) {
            continue;
        }

        return (w);
    } }

    return (0);
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
                                    true /* strict */,
                                    0 /* depth */,
                                    0 /* debug */);
    if (w) {
        return (w);
    }

    w = wid_mouse_motion_handler_at(wid_over, x, y,
                                    relx, rely,
                                    wheelx, wheely,
                                    true /* strict */,
                                    0 /* depth */,
                                    0 /* debug */);
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
                                        true /* strict */,
                                        0 /* depth */,
                                        0 /* debug */);
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
                                        false /* strict */,
                                        0 /* depth */,
                                        0 /* debug */);
        if (!w) {
            continue;
        }

        return (w);
    } }

    return (0);
}

/*
 * Catch recursive cases like this:
 *
 * #42 0x00000001180a1233 in wid_update (w=0x11f0c0c00) at wid.c:5353
 *
 * #43 0x00000001180bf45d in wid_editor_map_thing_replace_template 
 * 
 * #44 0x00000001180c1509 in wid_editor_map_thing_replace (w=0x11f1dca00, x=5, 
 * 
 * #45 0x00000001180c0275 in wid_editor_map_tile_mouse_motion (w=0x11f1dca00, 
 * 
 * #46 0x00000001180a170f in wid_mouse_motion (x=338, y=357, relx=0, rely=0, 
 * 
 * #47 0x000000011809ff26 in wid_update_mouse () at wid.c:5371
 *
 * #48 0x00000001180a1233 in wid_update (w=0x11f0c0c00) at wid.c:5353
 *
 * #49 0x00000001180bf45d in wid_editor_map_thing_replace_template 
 * 
 * #50 0x00000001180c1509 in wid_editor_map_thing_replace (w=0x11f1dca00, x=5, 
 * 
 * #51 0x00000001180c0275 in wid_editor_map_tile_mouse_motion (w=0x11f1dca00, 
 * 
 * #52 0x00000001180a170f in wid_mouse_motion (x=338, y=357, relx=0, rely=0, 
 * 
 */
static int wid_mouse_motion_recursion;

void wid_mouse_motion (int32_t x, int32_t y,
                       int32_t relx, int32_t rely,
                       int32_t wheelx, int32_t wheely)
{
    if (!wid_mouse_visible) {
        return;
    }

    if (wid_mouse_motion_recursion) {
        return;
    }

    wid_mouse_motion_recursion = 1;

    widp w;

    if (wid_mouse_template) {
        wid_move_to_abs_centered_in(wid_mouse_template, mouse_x, mouse_y, 10);
        wid_raise(wid_mouse_template);
    }

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
        wid_mouse_motion_recursion = 0;
        return;
    }

    uint8_t over = false;

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
        while (w && 
               !wid_mouse_over_begin(w, x, y, relx, rely, wheelx, wheely)) {
            w = w->parent;
        }

        uint8_t done = false;

        if (!w) {
            /*
             * Allow scrollbar to grab.
             */
        } else {
            /*
             * This widget reacted somehow when we went over it. i.e. popup ot 
             * function.
             */
            over = true;
        }

        w = wid_mouse_motion_handler(x, y, relx, rely, wheelx, wheely);
        if (w) {
            if (wid_mouse_over_begin(w, x, y, relx, rely, wheelx, wheely)) {
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

    wid_mouse_motion_recursion = 0;
}

/*
 * If no handler for this button, fake a mouse event.
 */
void wid_fake_joy_button (int32_t x, int32_t y)
{
    if (sdl_joy_buttons[SDL_JOY_BUTTON_A]) {
        wid_mouse_down(SDL_BUTTON_LEFT, x, y);
        return;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_B]) {
        wid_mouse_down(SDL_BUTTON_RIGHT, x, y);
        return;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_X]) {
        wid_mouse_down(SDL_BUTTON_RIGHT, x, y);
        return;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_Y]) {
        wid_mouse_down(2, x, y);
        return;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_TOP_LEFT]) {
        wid_mouse_down(SDL_BUTTON_LEFT, x, y);
        return;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_TOP_RIGHT]) {
        wid_mouse_down(SDL_BUTTON_RIGHT, x, y);
        return;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_LEFT_STICK_DOWN]) {
        wid_mouse_down(SDL_BUTTON_LEFT, x, y);
        return;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_RIGHT_STICK_DOWN]) {
        wid_mouse_down(SDL_BUTTON_RIGHT, x, y);
        return;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_START]) {
        wid_mouse_down(SDL_BUTTON_LEFT, x, y);
        return;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_XBOX]) {
        wid_mouse_down(SDL_BUTTON_LEFT, x, y);
        return;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_BACK]) {
        wid_mouse_down(SDL_BUTTON_RIGHT, x, y);
        return;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_UP]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_DOWN]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_LEFT]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_RIGHT]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_LEFT_FIRE]) {
        wid_mouse_down(SDL_BUTTON_LEFT, x, y);
        return;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_RIGHT_FIRE]) {
        wid_mouse_down(SDL_BUTTON_RIGHT, x, y);
        return;
    }
}

void wid_joy_button (int32_t x, int32_t y)
{
    /*
     * Only if there is a change in status, send an event.
     */
    static uint32_t ts[SDL_MAX_BUTTONS];
    int changed = false;
    int b;

    for (b = 0; b < SDL_MAX_BUTTONS; b++) {
        if (sdl_joy_buttons[b]) {
            if (time_have_x_tenths_passed_since(2, ts[b])) {
                changed = true;
                ts[b] = time_get_time_ms();
            }
        }
    }

    if (!changed) {
        return;
    }

    widp w;

    sound_play_click();

    w = wid_joy_down_handler(x, y);
    if (!w) {
        wid_fake_joy_button(x, y);
        return;
    }

    /*
     * Raise on mouse.
     */
    if (w->on_joy_button) {
        /*
         * If the button doesn't eat the event, try the parent.
         */
        while (!(w->on_joy_button)(w, x, y)) {
            w = w->parent;

            while (w && !w->on_joy_button) {
                w = w->parent;
            }

            if (!w) {
                wid_fake_joy_button(x, y);
                return;
            }
        }

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
    } else {
        wid_fake_joy_button(x, y);
    }

    if (wid_get_movable(w)) {
        wid_set_mode(w, WID_MODE_ACTIVE);
        wid_raise(w);
        wid_mouse_motion_begin(w, x, y);
        return;
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
 * Get the onscreen co-ords of the widget, clipped to the parent.
 */
void wid_get_abs (widp w,
                  int32_t *x,
                  int32_t *y)
{
    int32_t tlx;
    int32_t tly;
    int32_t brx;
    int32_t bry;

    wid_get_abs_coords(w, &tlx, &tly, &brx, &bry);

    *x = (tlx + brx) / 2;
    *y = (tly + bry) / 2;
}

void wid_get_pct (widp w,
                  double *px,
                  double *py)
{
    int32_t x;
    int32_t y;

    wid_get_abs(w, &x, &y);

    *px = (double)x / (double)global_config.video_gl_width;
    *py = (double)y / (double)global_config.video_gl_height;
}

/*
 * Finish off a widgets move.
 */
void wid_move_end (widp w)
{
    while (w->moving) {
        wid_move_dequeue(w);
    }
}

/*
 * Do stuff for widgets once per frame.
 */
static void wid_gc (widp w)
{
    verify(w);

    if (w->being_destroyed) {
        /*
         * If being destroyed, is it done fading ? We only do this for the top
         * level widgets. The childen inherit the fading from the parent.
         *
         * Only do this for single shot fade count widgets, not those that 
         * pulse in and out.
         */
        if (!wid_exiting && wid_is_fading(w) && !w->fade_count) {
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
    if (w->destroy_when && (wid_time >= w->destroy_when)) {
        fast_verify(w);

        if (w->destroy_ptr) {
            *(w->destroy_ptr) = 0;
            w->destroy_ptr = 0;
        }

        wid_destroy(&w);
    }
}

static void wid_gc_force (widp w)
{
    verify(w);

    /*
     * Delayed destroy?
     */
    if (w->destroy_when) {
        fast_verify(w);

        if (w->destroy_ptr) {
            *(w->destroy_ptr) = 0;
            w->destroy_ptr = 0;
        }

        wid_destroy(&w);
    }
}

#define LIGHT_LEVELS 2
static const double MAX_LIGHT_STRENGTH = 1000.0;
static double ray_depth[MAX_LIGHT_RAYS][LIGHT_LEVELS];

static int wid_light_count;

typedef struct wid_light_ {
    thingp t;
    fpoint at;
    double ostrength;
    double strength;
    double fuzz;
    widp w;
    uint16_t max_light_rays;
    color col;
} wid_light;
        
static wid_light wid_lights[MAX_LIGHTS];

static void wid_light_init (void)
{
    wid_light_count = 0;
}

static void wid_light_add (widp w, fpoint at, double strength, color c)
{
    thingp t = wid_get_thing(w);

    c.a = 255;

    /*
     * Do a quick dmap check so that lights that are enclosed in a room do not 
     * shine
     */
    if (thing_is_ethereal(t)) {
        /*
         * Death is always lit...
         */
    } else if (player && !thing_is_player_or_owned_by_player(t)) {
        int sx, sy;

        thing_real_to_map(t, &sx, &sy);

        int distance = dmap_distance_to_player(sx, sy);
        if (distance == -1) {
            return;
        }

        /*
         * Cheap effect, make the light fade away with distance.
         */
        double scale = (256.0 - (((double)distance) * 8.0)) / 256.0;
        if (scale <= 0.1) {
            return;
        }

        c.r = (uint8_t) (((double)c.r) * scale);
        c.g = (uint8_t) (((double)c.g) * scale);
        c.b = (uint8_t) (((double)c.b) * scale);
        c.a = (uint8_t) (((double)c.a) * scale);
    }

    if (wid_light_count >= MAX_LIGHTS) {
        return;
    }

    wid_lights[wid_light_count].at = at;
    wid_lights[wid_light_count].strength = strength;
    wid_lights[wid_light_count].ostrength = strength;
    wid_lights[wid_light_count].w = w;
    wid_lights[wid_light_count].col = c;
    wid_lights[wid_light_count].t = t;

    uint16_t max_light_rays;

    if (thing_is_player_or_owned_by_player(t)) {
        max_light_rays = MAX_LIGHT_RAYS;
    } else {
        if (strength >= 10) {
            max_light_rays = MAX_LIGHT_RAYS;
        } else if (strength > 5) {
            max_light_rays = MAX_LIGHT_RAYS / 2;
        } else if (strength > 2) {
            max_light_rays = MAX_LIGHT_RAYS / 8;
        } else {
            max_light_rays = MAX_LIGHT_RAYS / 16;
        }
    }

    wid_lights[wid_light_count].max_light_rays = max_light_rays;

    wid_light_count++;
}

/*
 * Display one wid and its children
 */
static void wid_display_fast (widp w, 
                              double shake_x,
                              double shake_y,
                              uint8_t pass, 
                              uint8_t black_and_white)
{
    double otlx;
    double otly;
    double obrx;
    double obry;
    widp p;

    /*
     * Record the original pre clip sizes for text centering.
     */
    wid_get_tl_x_tl_y_br_x_br_y(w, &otlx, &otly, &obrx, &obry);

    p = w->parent;
    otlx += p->offset.x;
    otly += p->offset.y;
    obrx += p->offset.x;
    obry += p->offset.y;

    /*
     * If this widget was active and the time has elapsed, make it normal.
     */
    if (unlikely(wid_get_mode(w) == WID_MODE_ACTIVE)) {
        if ((wid_time - w->timestamp_last_mode_change) > 250) {
            wid_set_mode(w, WID_MODE_NORMAL);
        }
    }

    /*
     * Widget tiles and textures.
     */
    tilep tile = wid_get_tile(w);

    /*
     * Does this thing light up its environment?
     */
    thingp t = wid_get_thing(w);
    tpp tp = 0;

    if (likely(t != 0)) {
        if (unlikely(debug)) {
            /*
             * Always show in debug mode
             */
        } else if (thing_is_visible_on_debug_only_noverify(t)) {
            /*
             * Show only in debug mode
             */
            return;
        }

        tp = thing_tp(t);

        /*
         * Reset lighting for this thing before it is lit.
         */
        if (pass == 0) {
            t->lit = 0;
        } else {
            /*
             * If the thing is not lit and it has cats eyes, let them shine.
             *
             * Another case is if you are a light source yet emit no light,
             * like a player. Let the eyes shine in that case. But don't let 
             * them shine when the torch light is on else you have glowing 
             * eyes over the player and they look like a zombie! But don't let 
             * them shine when the torch light is on else you have glowing 
             * eyes over the player and they look like a zombie!
             */
            if ((t->lit == 0) && 
                (t->torch_light_radius == 0) &&
                thing_is_cats_eyes_noverify(t)) {

                tile = wid_get_tile_eyes(w);
                if (!tile) {
                    ERR("thing %s has no eyes for the darkness",
                        thing_logname(t));
                }
            }
        }
    }

    if (pass == 0) {
        int light_source = false;

        if (t) {
            /*
             * Only light lava at edges for speed
             */
            if (thing_is_lava(t) || thing_is_acid(t)) {
                if (t->join_index != IS_JOIN_BLOCK) {
                    light_source = true;
                }
            } else {
                if (thing_is_light_source_noverify(t)) {
                    light_source = true;
                }
            }
        }

        if (light_source) {
            fpoint light_pos;

            /*
             * Widget tiles and textures.
             */
            tilep tile = wid_get_tile(w);
            if (tile) {
                light_pos.x = otlx + (((tile->px1 + tile->px2) / 2.0) * wid_get_width(w));
                light_pos.y = otly + (((tile->py1 + tile->py2) / 2.0) * wid_get_height(w));
            } else {
                light_pos.x = otlx + wid_get_width(w) / 2;
                light_pos.y = otly + wid_get_height(w) / 2;
            }

            double light_radius = tp_get_light_radius(tp);

            if (thing_is_player_or_owned_by_player(t)) {
                /*
                 * Player light is limited by the number of torches.
                 */
                if (t == player) {
                    light_radius = t->torch_light_radius;
                } else {
                    /*
                     * If another player is on screen we will not know how
                     * much they are lighting up the screen; so just light
                     * up the default.
                     */
                    light_radius = tp_get_light_radius(tp);
                }
            } else if (thing_is_cloud_effect(t) && !t->is_epicenter) {
                /*
                 * No light source for explosion edges. Too high a cpu drain.
                 */
                light_radius = 0.0;
            }

            if (light_radius > 0.0) {
                /*
                 * Give the player a more glowing light.
                 */
                if (thing_is_player_or_owned_by_player(t)) {
                    wid_light_add(w, light_pos, light_radius, WHITE);

                    wid_light_add(w, light_pos, light_radius * 0.55,
                                  tp_light_color(tp));
                } else {
                    wid_light_add(w, light_pos, light_radius,
                                  tp_light_color(tp));

                }
            }
        }
    }

    /*
     * For light embers
     */
    if (tp) {
        if (tp_is_hidden(tp)) {
            return;
        }
    }

    /*
     * Only care about tex scaling here.
     */
    fsize texuv;
    (void) wid_get_tex(w, &texuv);

    fpoint tl;
    fpoint br;

    tl.x = otlx;
    tl.y = otly;
    br.x = otlx + wid_get_width(w);
    br.y = otly + wid_get_height(w);

    /*
     * Do rotation and flipping.
     */
    if (unlikely(w->bouncing)) {
        double height = wid_get_bounce(w);

        tl.y -= height;
        br.y -= height;
    }

    if (unlikely(w->blit_scaled_w || w->blit_scaled_h ||
                 w->blit_scaling_w || w->blit_scaling_h)) {

        double scale = wid_get_blit_scaling_w(w);
        fpoint new_tl;
        fpoint new_br;

        wid_tile_scale(w, 
                       tl,
                       br,
                       &new_tl,
                       &new_br,
                       scale);
        tl = new_tl;
        br = new_br;
    }

    /*
     * Blit a border around the tile.
     */
    if (unlikely(wid_get_blit_outline(w))) {
        color col_blit_outline = wid_get_color(w, WID_COLOR_BLIT_OUTLINE);

        glcolor(col_blit_outline);

        double scale = w->blit_outline_val;
        fpoint new_tl;
        fpoint new_br;

        wid_tile_scale(w, 
                       tl,
                       br,
                       &new_tl,
                       &new_br,
                       scale);

        glcolor(WHITE);

        tile_blit_fat(tile, 0, new_tl, new_br);
    }

    {
        double dx = 0;
        double dy = 0;

        if (unlikely(w->shaking)) {
            wid_get_shake(w, &dx, &dy);

            shake_x += dx;
            shake_y += dy;
        }
    }

    tl.x += shake_x;
    tl.y += shake_y;
    br.x += shake_x;
    br.y += shake_y;

    tl.y += w->blit_y_offset;
    br.y += w->blit_y_offset;

    /*
     * Else we get little lines of white sometimes.
     */
    tl.x = (int)tl.x;
    tl.y = (int)tl.y;
    br.x = (int)br.x;
    br.y = (int)br.y;

    /*
     * Allow slime molds to look either way without bothering to create the 
     * animation for it! Lazy...
     */
    if (unlikely(w->flip_horiz)) {
        swap(tl.x, br.x);
    }

    if (unlikely(w->flip_vert)) {
        swap(tl.y, br.y);
    }

    /*
     * Apply fade in/out effects.
     */
    color col_tile = wid_get_color(w, WID_COLOR_BLIT);
    if (unlikely(w->fade_out || w->fade_in)) {
        double fade = wid_get_fade_amount(w);

        col_tile.a *= fade;
    }

    glcolor(col_tile);

    if (unlikely(black_and_white)) {
        tile_blit_fat_black_and_white(tile, 0, tl, br);
    } else {
        tile_blit_fat(tile, 0, tl, br);
    }

    if (debug && t) {
        int mx, my;

        thing_real_to_map(t, &mx, &my);

        int distance = dmap_distance_to_player(mx, my);

        char tmp[80];

        if (thing_is_player(t)) {
            glcolor(YELLOW);

            sprintf(tmp, "%f,%f",t->x, t->y);
            ttf_puts_no_fmt(small_font, tmp, (otlx + obrx) / 2, (otly + obry) / 2, 1.0, 1.0, true);

            sprintf(tmp, "%d,%d (rounded)",mx, my);
            ttf_puts_no_fmt(small_font, tmp, (otlx + obrx) / 2, obry, 1.0, 1.0, true);
        } else {
            if (distance == -1) {
                glcolor(RED);
            } else {
                glcolor(GREEN);
            }

            sprintf(tmp, "%d",distance);
            ttf_puts_no_fmt(vsmall_font, tmp,
                            (otlx + obrx) / 2, (otly + obry) / 2, 1.0, 1.0, true);

            if (thing_is_dungeon_floor(t)) {
                glcolor(WHITE);
                sprintf(tmp, "%d,%d",(int)(t->x), (int)(t->y));
                ttf_puts_no_fmt(vsmall_font, tmp, otlx, otly, 1.0, 1.0, true);

                glcolor(RED);
                gl_blitline(otlx, otly, obrx, otly);
                gl_blitline(obrx, otly, obrx, obry);
            }

        }
    }
}

/*
 * Display one wid and its children
 */
static void wid_light_calculate_for_single_obstacle (widp w, 
                                                     int z, 
                                                     int light_index,
                                                     int pass)
{
    const wid_light *light = &wid_lights[light_index];
    int32_t owidth;
    int32_t oheight;
    int32_t otlx;
    int32_t otly;
    int32_t obrx;
    int32_t obry;
    widp p;

    thingp t = wid_get_thing(w);
    if (!t) {
        return;
    }

    /*
     * No blocking our own light.
     */
    if (t == light->t) {
        return;
    }

    /*
     * Things that are populous but don't cast shadows.
     */
    if (wid_this_is_hidden(w)) {
        return;
    }

    uint8_t soft_shadow = 0;

    if (pass == 0) {
        if (!thing_is_shadow_caster_noverify(t)) {
            return;
        }

        if (thing_is_shadow_caster_soft_noverify(t)) {
            soft_shadow = 1;
        }
    }

    double light_radius = light->strength;

    /*
     * For small light sources, just skip doing collision detection.
     */
    if (light->ostrength < 1.0) {
        int dr = MAX_LIGHT_RAYS / light->max_light_rays;
        int i;
        int r = 0;

        for (i = 0; i < light->max_light_rays; i++, r += dr) {
            ray_depth[i][0] = light_radius;
        }

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
     * Widget tiles and textures.
     */
    tilep tile = wid_get_tile(w);
    if (!tile) {
        return;
    }

    double etlx;
    double etly;
    double ebrx;
    double ebry;

    if (thing_is_shadow_caster_soft_noverify(t)) {
        /*
         * Make soft shadow things block less light.
         */
        double fudge = owidth / 4.0;
        double mx = (otlx + obrx) / 2.0;
        double my = (otly + obry) / 2.0;

        etlx = mx - fudge;
        etly = my - fudge;
        ebrx = mx + fudge;
        ebry = my + fudge;
    } else {
        /*
         * So no little breaks between walls allow light through.
         */
        double fudge = 0.05;
        etlx = (double)otlx + ((tile->px1-fudge) * (double)owidth);
        etly = (double)otly + ((tile->py1-fudge) * (double)oheight);
        ebrx = (double)otlx + ((tile->px2+fudge) * (double)owidth);
        ebry = (double)otly + ((tile->py2+fudge) * (double)oheight);
    }

    fpoint P[4];
    P[0].x = etlx;
    P[0].y = etly;
    P[1].x = ebrx;
    P[1].y = etly;
    P[2].x = ebrx;
    P[2].y = ebry;
    P[3].x = etlx;
    P[3].y = ebry;

    fpoint edge[4];
    edge[0] = fsub(P[1], P[0]);
    edge[1] = fsub(P[2], P[1]);
    edge[2] = fsub(P[3], P[2]);
    edge[3] = fsub(P[0], P[3]);

    fpoint normal[4];
    normal[0].x = edge[0].y;
    normal[0].y = -edge[0].x;
    normal[1].x = edge[1].y;
    normal[1].y = -edge[1].x;
    normal[2].x = edge[2].y;
    normal[2].y = -edge[2].x;
    normal[3].x = edge[3].y;
    normal[3].y = -edge[3].x;

    /*
     * For each clockwise side of the tile.
     */
    fpoint light_pos = light->at;

    /*
     * For each clockwise quadrant.
     */
    uint8_t k;
    for (k = 0; k<4; k++) {

        fpoint light_dir = fsub(light_pos, P[k]);

        float dot = normal[k].x * light_dir.x + normal[k].y * light_dir.y;   
                
        if (dot <= 0.0f) {
            continue;
        }

        /*
         * Facing the light source. Blocks light.
         */
        int l = (k + 1) % 4;

        fpoint p1 = fsub(P[k], light_pos);
        fpoint p2 = fsub(P[l], light_pos);

        /*
         * Start and end points of the face blocking the light.
         */
        double p1_rad = anglerot(p1);
        double p2_rad = anglerot(p2);
        if (p1_rad == RAD_360) {
            p1_rad = 0;
        }
        if (p2_rad == RAD_360) {
            p2_rad = 0;
        }

        uint16_t max_light_rays = light->max_light_rays;
        int32_t p1_deg = p1_rad * ((double)max_light_rays / RAD_360);
        int32_t p2_deg = p2_rad * ((double)max_light_rays / RAD_360);

        /*
         * How many radians does this obstacle block?
         */
        double tot_rad = p1_rad - p2_rad;
        int32_t tot_deg = p1_deg - p2_deg;
        if (tot_deg < 0) {
            tot_deg += max_light_rays;
            tot_rad += RAD_360;
        }

        double dr = tot_rad / tot_deg;
        int32_t deg = p2_deg;
        double rad = p2_rad;

        /*
         * For each blocking radian, look at the distance to the light.
         * If closer than what is blocking that radian curretly, then use 
         * this value.
         *
         * In essence, this is a depth buffer.
         */
        while (tot_deg-- > 0) {

            fpoint light_end;
            light_end.x = light_pos.x + fcos(rad) * MAX_LIGHT_STRENGTH;
            light_end.y = light_pos.y + fsin(rad) * MAX_LIGHT_STRENGTH;

            fpoint intersect;

            /*
             * Check the light ray really does hit this obstacle and where
             * so we can work out distance.
             */
            if (get_line_known_intersection(P[k], P[l], 
                                            light_pos, light_end, 
                                            &intersect)) {

                double len = DISTANCE(light_pos.x, light_pos.y, 
                                      intersect.x, intersect.y);

                if ((deg < 0) || (deg >= max_light_rays)) {
                    ERR("%d",deg);
                }

                if (len > light_radius) {
                    len = light_radius;
                }

                if (pass == 0) {
                    if (soft_shadow) {
                        if (!ray_depth[deg][0]) {
                            ray_depth[deg][1] = len;
                        } if (len < ray_depth[deg][1]) {
                            ray_depth[deg][1] = len;
                        }
                    } else {
                        if (!ray_depth[deg][0]) {
                            ray_depth[deg][0] = len;
                            ray_depth[deg][1] = len;
                        } if (len < ray_depth[deg][0]) {
                            ray_depth[deg][0] = len;
                            ray_depth[deg][1] = len;
                        }
                    }
                } else {
                    if (len <= ray_depth[deg][0]) {
                        t->lit++;
                    }

                    if (len <= ray_depth[deg][1]) {
                        t->lit++;
                    }
                }
            }

            rad += dr;
            if (rad >= RAD_360) {
                rad = 0;
            }

            deg++;
            if (deg >= max_light_rays) {
                deg = 0;
            }
        }
    }
}

/*
 * Smooth out the ray lenghts to avoid jagged jumps in distance.
 */
static void wid_lighting_smooth (wid_light *light)
{
    double ray_depth_tmp[MAX_LIGHT_RAYS][LIGHT_LEVELS];
    uint8_t pass;

    for (pass = 0; pass <= 1; pass++) {
        uint16_t i;
        uint16_t max_light_rays = light->max_light_rays;
        uint16_t before;
        uint16_t after;

        for (i = 0; i < max_light_rays; i++) {

            if (i == 0) {
                before = max_light_rays - 1;
                after = i + 1;
            } else if (i == max_light_rays - 1) {
                before = i - 1;
                after = 0;
            } else {
                before = i - 1;
                after = i + 1;
            }

            double a = ray_depth[before][pass];
            double b = ray_depth[i][pass];
            double c = ray_depth[after][pass];

            if (a == 0) {
                a = light->strength;
            }
            if (b == 0) {
                b = light->strength;
            }
            if (c == 0) {
                c = light->strength;
            }
            ray_depth_tmp[i][pass] = (a + b + c) / 3.0;
        }
    }

    memcpy(ray_depth, ray_depth_tmp, sizeof(ray_depth));
}

/*
 * Walk all widgets next to this light source and find light intersections.
 */
static void wid_lighting_calculate (widp w, 
                                    const int light_index)
{
    wid_light *light = &wid_lights[light_index];
    double light_radius = light->strength;

    int16_t maxx;
    int16_t minx;
    int16_t maxy;
    int16_t miny;

    widp light_wid = light->w;
    thingp t = wid_get_thing(light_wid);
    if (!t) {
        return;
    }

    double visible_width = light_radius + 3;
    double visible_height = light_radius + 3;

    light_radius *= wid_get_width(light_wid);
    light->strength *= wid_get_width(light_wid);

    if (light_wid->blit_scaled_w || light_wid->blit_scaled_h ||
        light_wid->blit_scaling_w || light_wid->blit_scaling_h) {
    
        light->strength *= wid_get_blit_scaling_w(light_wid);
    }

    maxx = t->x + visible_width;
    minx = t->x - visible_width;
    maxy = t->y + visible_height;
    miny = t->y - visible_height;

    if (minx < 0) {
        minx = 0;
    }
    if (maxx > MAP_WIDTH) {
        maxx = MAP_WIDTH;
    }

    if (miny < 0) {
        miny = 0;
    }
    if (maxy > MAP_HEIGHT) {
        maxy = MAP_HEIGHT;
    }

    int32_t x, y;
    uint8_t z, pass;

    /*
     * Blit the light map to a FBO. First generate the right ray lengths.
     */
    memset(ray_depth, 0, sizeof(ray_depth));

    for (pass = 0; pass <= 1; pass++) {
        for (z = MAP_DEPTH_WALL; z <= MAP_DEPTH_OBJ; z++) {
            for (x = maxx - 1; x >= minx; x--) {
                for (y = miny; y < maxy; y++) {

                    tree_root **tree = 
                        w->grid->grid_of_trees[z] + (y * w->grid->width) + x;

                    widgridnode *node;

                    TREE_WALK_REVERSE_UNSAFE_INLINE(
                                *tree, node,
                                tree_prev_tree_wid_compare_func_fast) {

                        wid_light_calculate_for_single_obstacle(
                                    node->wid, z, light_index, pass);
                    }
                }
            }
        }
    }

    /*
     * Seems to add nothing.
     */
    if (/* DISABLES CODE */ (0)) {
        wid_lighting_smooth(light);
    }
}

/*
 * Walk all widgets next to this light source and find light intersections.
 */
static void wid_lighting_render (widp w, 
                                 const int light_index,
                                 const int light_level)
{
    wid_light *light = &wid_lights[light_index];
    fpoint light_pos = light->at;
    double light_radius = light->strength;

    /*
     * No need for soft shadows in small lights
     */
    if (light_level == 1) {
        if (light->ostrength < 1.0) {
            return;
        }
    }

    int16_t maxx;
    int16_t minx;
    int16_t maxy;
    int16_t miny;

    widp light_wid = light->w;
    thingp t = wid_get_thing(light_wid);
    if (!t) {
        return;
    }

    double visible_width = light_radius + 3;
    double visible_height = light_radius + 3;

    uint16_t max_light_rays = light->max_light_rays;

    maxx = t->x + visible_width;
    minx = t->x - visible_width;
    maxy = t->y + visible_height;
    miny = t->y - visible_height;

    if (minx < 0) {
        minx = 0;
    }
    if (maxx > MAP_WIDTH) {
        maxx = MAP_WIDTH;
    }

    if (miny < 0) {
        miny = 0;
    }
    if (maxy > MAP_HEIGHT) {
        maxy = MAP_HEIGHT;
    }

    color c = light->col;

    float red   = ((float)c.r) / 255.0;
    float green = ((float)c.g) / 255.0;
    float blue  = ((float)c.b) / 255.0;
    float alpha = ((float)c.a) / 255.0;

    double len_bright = 0.7;


    /*
     * Draw the light in three stages. 
     *
     * 1. Central core of light as a fan
     * 2. Strip of dimmer light at the edges
     * 3. Strip of flickering even dimmer light
     */


    /*
     * Now blit to the FBO, drawing the central core of the light rays
     */
    blit_init();

    {
        double r = 0;
        double dr = RAD_360 / (double)max_light_rays;
        int i;

        /*
         * Walk the light rays in a circle.
         */
        push_point(light_pos.x, light_pos.y, red, green, blue, alpha);

        for (i = 0; i < max_light_rays; i++, r += dr) {
            double p1_len = ray_depth[i][light_level];
            if (p1_len == 0) {
                p1_len = light_radius;
            }

            p1_len *= len_bright;

            double cosr = fcos(r);
            double sinr = fsin(r);
            double p1x = light_pos.x + cosr * p1_len;
            double p1y = light_pos.y + sinr * p1_len;

            push_point(p1x, p1y, red, green, blue, len_bright * alpha);
        }

        /*
         * Complete the circle with the first point again.
         */
        r = 0;
        i = 0; {
            double p1_len = ray_depth[i][light_level];
            if (p1_len == 0) {
                p1_len = light_radius;
            }

            p1_len *= len_bright;

            double cosr = fcos(r);
            double sinr = fsin(r);
            double p1x = light_pos.x + cosr * p1_len;
            double p1y = light_pos.y + sinr * p1_len;

            push_point(p1x, p1y, red, green, blue, len_bright * alpha);
        }
    }

    blit_flush_triangle_fan();




    /*
     * Draw dimmer light
     */
    blit_init();

    {
        double r = 0;
        double dr = RAD_360 / (double)max_light_rays;
        int i;

        /*
         * Walk the light rays in a circle.
         */
        for (i = 0; i < max_light_rays; i++, r += dr) {
            double p1_len = ray_depth[i][light_level];
            double p3_len;
            if (p1_len == 0) {
                p1_len = light_radius;
            }

            p3_len = p1_len;
            p1_len *= len_bright;

            double cosr = fcos(r);
            double sinr = fsin(r);

            double p1x = light_pos.x + cosr * p1_len;
            double p1y = light_pos.y + sinr * p1_len;

            double p3x = light_pos.x + cosr * p3_len;
            double p3y = light_pos.y + sinr * p3_len;

            push_point(p1x, p1y, red, green, blue, len_bright * alpha);
            push_point(p3x, p3y, red, green, blue, len_bright * alpha * 0.35);
        }

        /*
         * Complete the strip with 1.5 triangles.
         */
        r = 0;
        i = 0; {
            double p1_len = ray_depth[i][light_level];
            double p3_len;
            if (p1_len == 0) {
                p1_len = light_radius;
            }

            p3_len = p1_len;
            p1_len *= len_bright;

            double cosr = fcos(r);
            double sinr = fsin(r);

            double p1x = light_pos.x + cosr * p1_len;
            double p1y = light_pos.y + sinr * p1_len;

            double p3x = light_pos.x + cosr * p3_len;
            double p3y = light_pos.y + sinr * p3_len;

            push_point(p1x, p1y, red, green, blue, len_bright * alpha);
            push_point(p3x, p3y, red, green, blue, len_bright * alpha * 0.35);
        }
    }

    /*
     * Flush non shaded triangles.
     */
    blit_flush_triangle_strip();






    /*
     * Now draw the fuzzy edge of the light as a trigangle strip.
     */
    blit_init();

    {
        double r = 0;
        double dr = RAD_360 / (double)max_light_rays;
        int i;

        if (!light->fuzz || !(myrand() % MAP_LIGHT_CANDLELIGHT_FLICKER_RATE)) {
            if (thing_is_candle_light_noverify(t)) {
                light->fuzz = MAP_LIGHT_CANDLELIGHT_RADIUS + 
                                myrand() % MAP_LIGHT_CANDLELIGHT_FLICKER_RADIUS;
            } else {
                light->fuzz = MAP_LIGHT_CANDLELIGHT_RADIUS;
            }
        }

        double fuzz = light->fuzz;

        /*
         * Walk the light rays in a circle.
         */
        for (i = 0; i < max_light_rays; i++, r += dr) {
            double p1_len = ray_depth[i][light_level];

            if (p1_len == 0) {
                p1_len = light_radius;
            }

            double p3_len = p1_len + fuzz;

            double cosr = fcos(r);
            double sinr = fsin(r);

            double p1x = light_pos.x + cosr * p1_len;
            double p1y = light_pos.y + sinr * p1_len;

            double p3x = light_pos.x + cosr * p3_len;
            double p3y = light_pos.y + sinr * p3_len;

            push_point(p1x, p1y, red, green, blue, len_bright * alpha * 0.35);
            push_point(p3x, p3y, red, green, blue, 0);
        }

        /*
         * Complete the strip with 1.5 triangles.
         */
        r = 0;
        i = 0; {
            double p1_len = ray_depth[i][light_level];

            if (p1_len == 0) {
                p1_len = light_radius;
            }

            double p3_len = p1_len + fuzz;

            double cosr = fcos(r);
            double sinr = fsin(r);

            double p1x = light_pos.x + cosr * p1_len;
            double p1y = light_pos.y + sinr * p1_len;

            double p3x = light_pos.x + cosr * p3_len;
            double p3y = light_pos.y + sinr * p3_len;

            push_point(p1x, p1y, red, green, blue, len_bright * alpha * 0.35);
            push_point(p3x, p3y, red, green, blue, 0);
        }
    }

    /*
     * Flush non shaded triangles.
     */
    blit_flush_triangle_strip();
}

/*
 * Walk all widgets next to this light source and find light intersections.
 */
static void wid_lighting_debug (widp w, 
                                 const int light_index,
                                 const int light_level)
{
    wid_light *light = &wid_lights[light_index];
    fpoint light_pos = light->at;
    double light_radius = light->strength;

    int16_t maxx;
    int16_t minx;
    int16_t maxy;
    int16_t miny;

    widp light_wid = light->w;
    thingp t = wid_get_thing(light_wid);
    if (!t) {
        return;
    }

    if (thing_is_player_or_owned_by_player(t)) {
        return;
    }

    double visible_width = light_radius + 3;
    double visible_height = light_radius + 3;

    uint16_t max_light_rays = light->max_light_rays;

    maxx = t->x + visible_width;
    minx = t->x - visible_width;
    maxy = t->y + visible_height;
    miny = t->y - visible_height;

    if (minx < 0) {
        minx = 0;
    }
    if (maxx > MAP_WIDTH) {
        maxx = MAP_WIDTH;
    }

    if (miny < 0) {
        miny = 0;
    }
    if (maxy > MAP_HEIGHT) {
        maxy = MAP_HEIGHT;
    }

    {
        double r = 0;
        double dr = RAD_360 / (double)max_light_rays;
        int i;

        /*
         * Walk the light rays in a circle.
         */
        for (i = 0; i < max_light_rays; i++, r += dr) {
            double p1_len = ray_depth[i][light_level];
            if (p1_len == 0) {
                p1_len = light_radius;
            }

            double cosr = fcos(r);
            double sinr = fsin(r);
            double p1x = light_pos.x + cosr * p1_len;
            double p1y = light_pos.y + sinr * p1_len;

            gl_blitline(light_pos.x, light_pos.y, p1x, p1y);
        }
    }
}

/*
 * Display one wid and its children
 */
static void wid_display (widp w,
                         uint8_t disable_scissor,
                         uint8_t *updated_scissors)
{
    uint8_t did_push_matrix;
    int32_t clip_height;
    int32_t clip_width;
    uint8_t fading;
    uint8_t hidden;
    uint8_t always_hidden;
    int32_t owidth;
    uint8_t scaling;
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
    always_hidden = wid_is_always_hidden(w);
    scaling = wid_is_scaling(w);

    if (always_hidden) {
        /*
         * Always render. Not hidden yet.
         */
        return;
    } else if (fading) {
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
        if ((wid_time - w->timestamp_last_mode_change) > 250) {
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
    color col = wid_get_color(w, WID_COLOR_BG);
    color col_tile = wid_get_color(w, WID_COLOR_BLIT);

#ifdef ENABLE_FADE_OUT_OTHER_WIDGETS_WHEN_FOCUS_LOCKED
    /*
     * This fades out other widgets in the background when a popup appears.
     */
    if (wid_focus_locked) {
        if (wid_get_top_parent(w) != wid_focus_locked) {
            double fade = 0.5;

            /*
             * Darken the colours.
             */
            col_tl.r *= fade;
            col_tl.g *= fade;
            col_tl.b *= fade;

            col_br.r *= fade;
            col_br.g *= fade;
            col_br.b *= fade;

            col.r *= fade;
            col.g *= fade;
            col.b *= fade;

            col_text.a *= fade;
            col_text_outline.a *= fade;
            col_blit_outline.a *= fade;
        }
    }
#endif

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
    if (w->rotating || w->rotated || w->bouncing || 
        w->flip_horiz || w->flip_vert) {

        did_push_matrix = true;

        glPushMatrix();
        glTranslatef(((tlx + brx)/2), ((tly + bry)/2), 0);

        if (w->bouncing) {
            glTranslatef(0, - wid_get_bounce(w), 0);
        }

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

        glTranslatef(-((tlx + brx)/2), -((tly + bry)/2), 0);
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

    if (w->disable_scissors) {
        disable_scissor = true;
    }

    /*
     * Should be no need for scissors if you do not have any children
     * or are not the top level wid.
     */
    if (!disable_scissor) {
        /*
         * Text box needs clipping when the text gets too wide.
         */
       if (w->grid || w->children_display_sorted || 
           !w->parent || w->show_cursor) {
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

    /*
     * Widget tiles and textures.
     */
    wid_animate(w);

    tilep tile = wid_get_tile(w);

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
    br.x = otlx + wid_get_width(w);
    br.y = otly + wid_get_height(w);

    if (w->on_display) {
        (w->on_display)(w, tl, br);
    }

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
#if 0
        /*
         * A bit faster, but ugly.
         */
        gl_list_square_bevelled_plain(
            tl, br, tex, tex_tl, tex_br,
            texuv, col_tl, col, col_br, bevel);
#else
        /*
         * Always quite slow.
         */
        fsize radius = wid_get_radius(w);
        int32_t sides = wid_get_sides(w);

        if (wid_get_bevelled(w)) {
            gl_list_rounded_square_bevelled(
                tl, br, tex, tex_tl, tex_br,
                texuv, col_tl, col, col_br, bevel, radius, sides);
        } else {
            gl_list_rounded_square(
                tl, br, tex, tex_tl, tex_br,
                texuv, col_tl, col, col_br, radius, sides);
        }
#endif
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
            color col_blit_outline = wid_get_color(w, WID_COLOR_BLIT_OUTLINE);

            glcolor(col_blit_outline);

            double scale = w->blit_outline_val;
            fpoint new_tl;
            fpoint new_br;

            wid_tile_scale(w, 
                           tl,
                           br,
                           &new_tl,
                           &new_br,
                           scale);

            tile_blit_fat(tile, 0, new_tl, new_br);

            glcolor(WHITE);
        }

        glcolor(col_tile);

        if (w->blit_scaled_w || w->blit_scaled_h ||
            w->blit_scaling_w || w->blit_scaling_h) {
        
            double scale = wid_get_blit_scaling_w(w);
            fpoint new_tl;
            fpoint new_br;

            wid_tile_scale(w, 
                           tl,
                           br,
                           &new_tl,
                           &new_br,
                           scale);

            tl = new_tl;
            br = new_br;
        }

        /*
         * Center the texture, on the position we want, to tiles that have 
         * padding on one side are put in the middle. i.e. small block of 
         * cheese drawn in the bottom left of a tile for the game is now being 
         * drawn in an item slot is not center.
         */

        if (w->blit_center) {
            double twidth = br.x - tl.x;
            double theight = br.y - tl.y;

            /*
            * Use the first tile else things like torches jump around.
            */
            tilep ftile = w->first_tile ? w->first_tile : w->tile;

            /*
            * Work out the center of the tile and how far from center it is.
            */
            double mpx = (ftile->px2 + ftile->px1) / 2.0;
            double mpy = (ftile->py2 + ftile->py1) / 2.0;
            double dx = (0.5 - mpx) * twidth;
            double dy = (0.5 - mpy) * theight;

            fpoint a, b;
            a.x = tl.x+ dx;
            b.x = br.x+ dx;
            a.y = tl.y + dy;
            b.y = br.y + dy;

            tile_blit_fat(tile, 0, a, b);
        } else {
            tile_blit_fat(tile, 0, tl, br);
        }

        glBindTexture(GL_TEXTURE_2D, 0);

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
        uint8_t fixed_width = wid_get_text_fixed_width(w);

        int32_t x, y;
        double xpc, ypc;
        double width, height;
        enum_fmt fmt;

        /*
         * Manually specified text position.
         */
        fmt = ENUM_FMT_NONE;
        ttf_text_size(&font, text, &width, &height, &fmt, scaling, advance,
                        fixed_width);

        w->ttf_width = width;
        w->ttf_height = height;
        w->fmt = fmt;

        if (wid_get_text_pos(w, &xpc, &ypc)) {
            x = (owidth * xpc) - ((int32_t)width / 2) + otlx;
            y = (oheight * ypc) - ((int32_t)height / 2) + otly;
        } else {
            /*
             * Position the text
             */
            if (((int)width > owidth) && w->show_cursor) {
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

#ifdef ENABLE_LARGE_TEXT_OUTLINE
            double outline = 0;

            if (font == fixed_font) {
                outline = 1.0;
            } else if (font == vsmall_font) {
                outline = 1.0;
            } else if (font == small_font) {
                outline = 1.0;
            } else if (font == med_font) {
                outline = 1.5;
            } else if (font == large_font) {
                outline = 3.0;
            } else if (font == vlarge_font) {
                outline = 4.0;
            } else if (font == vvlarge_font) {
                outline = 5.0;
            } else {
                ERR("unhandled text outline case");
            }

            double dx;

            col_text_outline.a = (int)(wid_get_fade_amount(w) * 255.0);

            for (dx = 0.5; dx < outline; dx += 0.5) {
                glcolor(col_text_outline);
                col_text_outline.a = (col_text_outline.a * 2) / 3;

                ttf_puts_no_fmt(font, text,
                                x - dx * scaling,
                                y + dx * scaling, scaling, advance,
                                fixed_width);
                ttf_puts_no_fmt(font, text,
                                x + dx * scaling,
                                y + dx * scaling, scaling, advance,
                                fixed_width);
                ttf_puts_no_fmt(font, text,
                                x - dx * scaling,
                                y - dx * scaling, scaling, advance,
                                fixed_width);
                ttf_puts_no_fmt(font, text,
                                x + dx * scaling,
                                y - dx * scaling, scaling, advance,
                                fixed_width);
            }
#else
            ttf_puts_no_fmt(font, text,
                            x - 2.0f * scaling,
                            y + 2.0f * scaling, scaling, advance,
                            fixed_width);
#endif
        }

        /*
         * Widget text
         */
        glcolor(col_text);
        ttf_puts(font, text, x, y, scaling, advance, fixed_width);

        blit_flush();
    }

    if (w == wid_game_map_client_grid_container) {
        int16_t maxx;
        int16_t minx;
        int16_t maxy;
        int16_t miny;

        if (player) {
            const uint32_t visible_width = 
                    TILES_SCREEN_WIDTH / 2 + TILES_SCREEN_WIDTH_PAD;
            const uint32_t visible_height = 
                    TILES_SCREEN_HEIGHT / 2 + TILES_SCREEN_HEIGHT_PAD;

            maxx = player->x + visible_width;
            minx = player->x - visible_width;
            maxy = player->y + visible_height;
            miny = player->y - visible_height;

            if (minx < 0) {
                minx = 0;
            }
            if (maxx > MAP_WIDTH) {
                maxx = MAP_WIDTH;
            }

            if (miny < 0) {
                miny = 0;
            }
            if (maxy > MAP_HEIGHT) {
                maxy = MAP_HEIGHT;
            }
        } else {
            minx = 0;
            maxx = MAP_WIDTH;
            miny = 0;
            maxy = MAP_HEIGHT;
        }

        int32_t x, y;
        uint8_t z;

        uint8_t black_and_white = 0;
        if (!player || thing_is_dead_or_dying(player)) {
            black_and_white = 1;
        }

        wid_light_init();

        /*
         * Level shake
         */
        double shake_x = 0;
        double shake_y = 0;

        if (unlikely(w->shaking)) {
            wid_get_shake(w, &shake_x, &shake_y);
        }

        for (z = 0; z < MAP_DEPTH_MAX; z++) {
            for (x = maxx - 1; x >= minx; x--) {
                for (y = miny; y < maxy; y++) {
                    tree_root **tree = 
                        w->grid->grid_of_trees[z] + (y * w->grid->width) + x;
                    widgridnode *node;

                    TREE_WALK_REVERSE_UNSAFE_INLINE(
                                        *tree, 
                                        node,
                                        tree_prev_tree_wid_compare_func_fast) {

                        widp w = node->wid;
 
                        if (unlikely(wid_this_is_hidden(w))) {
                        } else if (w->text[0]) {
                            /*
                             * Do later post light source.
                             */
                        } else {
                            wid_display_fast(w,
                                             shake_x,
                                             shake_y,
                                             0, black_and_white);
                        }
                    }
                }
            }
        }

        blit_flush();

        if ((w == wid_game_map_client_grid_container) && !debug) {
            /*
             * Light soure.
             */
            glBindFramebuffer_EXT(GL_FRAMEBUFFER, fbo_id1);

            glClearColor(0,0,0,0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glcolor(WHITE);

            /*
             * We want to merge successive light sources together.
             */
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);

            blit_init();

            int i;
            for (i = 0; i < wid_light_count; i++) {
                /*
                 * Calculate ray lengths for all passes.
                 */
                wid_lighting_calculate(w, i);

                /*
                 * Draw the light sources. First pass is for solid obstacles.
                 */
                wid_lighting_render(w, i, 0);

                /*
                 * This for soft shadows.
                 */
                wid_lighting_render(w, i, 1);
            }

            glBindFramebuffer_EXT(GL_FRAMEBUFFER, 0);

            if (level_explosion_flash_effect > 0) {
                level_explosion_flash_effect--;
                /*
                 * God light effect.
                 */
                glBlendFunc(GL_DST_ALPHA, GL_SRC_ALPHA);
            } else {
                glBlendFunc(GL_ZERO, GL_SRC_COLOR);
            }

            uint32_t tw = global_config.video_pix_width;
            uint32_t th = global_config.video_pix_height;
            double window_w = tw;
            double window_h = th;

            /*
             * Stretch the shadow so when the screen shakes the borders are 
             * black as well. This is a bit of a hack but it also makes the
             * lights look nice with a kind of halo effect.
             */
            blit_init();
            glcolor(WHITE);
            blit(fbo_tex_id1, 0.0, 1.0, 1.0, 0.0, 0, 0, window_w, window_h);
            blit_flush();

            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            /*
             * Redraw light sources on top of the light they cast.
             */
            blit_init();

            /*
             * Need to keep track of lit cells as a lava block at the bottom 
             * of the z depth must also lit all things sitting on top of it.
             */
            static uint32_t cell_lit[MAP_WIDTH][MAP_HEIGHT];
            static uint32_t cell_lit_value;

            cell_lit_value++;

            for (x = maxx - 1; x >= minx; x--) {
                for (y = miny; y < maxy; y++) {
                    for (z = 0; z < MAP_DEPTH_MAX; z++) {
                        tree_root **tree = 
                            w->grid->grid_of_trees[z] + (y * w->grid->width) + x;
                        widgridnode *node;

                        TREE_WALK_REVERSE_UNSAFE_INLINE(
                                            *tree, 
                                            node,
                                            tree_prev_tree_wid_compare_func_fast) {

                            widp w = node->wid;
                            thingp t = wid_get_thing(w);

                            uint8_t lit = (cell_lit[x][y] == cell_lit_value);

                            if (t) {
                                if (thing_is_player_or_owned_by_player(t)) {
                                    /*
                                     * So players standing one tile below a 
                                     * light source like rock do not have the 
                                     * rock overlaid over their heads!
                                     */
                                    lit = 1;
                                } else if (thing_is_light_source_noverify(t)) {
                                    /*
                                     * Do a quick dmap check so that lights 
                                     * that are enclosed in a room do not 
                                     * shine
                                     */
                                    if (thing_is_ethereal(t)) {
                                        lit = 1;
                                    }

#if 0
                                    if (thing_is_monst(t)       ||
                                        thing_is_lava(t)        ||
                                        thing_is_projectile(t)  ||
                                        thing_is_teleport(t)    ||
                                        thing_is_torch(t)       ||
                                        thing_is_acid(t)) {
                                        int sx, sy;
                                        thing_real_to_map(t, &sx, &sy);

                                        int distance = dmap_distance_to_player(sx, sy);
                                        if (distance != -1) {
                                            lit = 1;
                                        }
                                    }
#endif
                                } else if ((t->lit == 0) && 
                                    (t->torch_light_radius == 0) &&
                                    thing_is_cats_eyes_noverify(t)) {
                                    lit = 1;
                                }
                            }

                            if (lit || w->text[0]) {
                                if (unlikely(wid_this_is_hidden(node->wid))) {
                                } else if (w->text[0]) {
                                    uint8_t child_updated_scissors = false;
        
                                    wid_display(w, true, &child_updated_scissors);
                                } else {
                                    cell_lit[x][y] = cell_lit_value;

                                    wid_display_fast(node->wid, 
                                                     shake_x,
                                                     shake_y,
                                                     1, black_and_white);
                                }
                            }
                        }
                    }
                }
            }

            blit_flush();

            if (debug) {
                for (i = 0; i < wid_light_count; i++) {
                    /*
                    * Calculate ray lengths for all passes.
                    */
                    wid_lighting_calculate(w, i);
                    /*
                    * Draw the light sources. First pass is for solid obstacles.
                    */
                    color c = RED;
                    c.a = 100;
                    glcolor(c);

                    wid_lighting_debug(w, i, 0);

                    c = YELLOW;
                    c.a = 50;
                    glcolor(c);

                    wid_lighting_debug(w, i, 1);
                }
            }
        }
    } else {
        widp child;

        TREE_WALK_REVERSE_UNSAFE_INLINE(w->children_display_sorted, child,
                                        tree_prev_tree_wid_compare_func) {
            uint8_t child_updated_scissors = false;

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

        if (wid_time >= w->timestamp_moving_end) {

            wid_move_dequeue(w);

            /*
             * If nothing else in the move queue, we're dine.
             */
            if (!w->moving) {
                continue;
            }
        }

        double time_step =
            (double)(wid_time - w->timestamp_moving_begin) /
            (double)(w->timestamp_moving_end - w->timestamp_moving_begin);

        x = (time_step * (double)(w->moving_end.x - w->moving_start.x)) +
            w->moving_start.x;
        y = (time_step * (double)(w->moving_end.y - w->moving_start.y)) +
            w->moving_start.y;

        wid_move_to_abs(w, x, y);
    }
}

/*
 * Do stuff for all widgets.
 */
void wid_gc_all (void)
{
    widp w;

    { TREE_OFFSET_WALK(wid_top_level4, w, tree4_wids_being_destroyed) {
        wid_gc(w);
    } }
}

void wid_gc_all_force (void)
{
    widp w;

    { TREE_OFFSET_WALK(wid_top_level4, w, tree4_wids_being_destroyed) {
        wid_gc_force(w);
    } }
}

/*
 * Do stuff for all widgets.
 */
void wid_tick_all (void)
{
    wid_time = time_get_time_ms();

    widp w;

    { TREE_OFFSET_WALK_UNSAFE(wid_top_level5, w) {
        verify(w);

        if (!w->on_tick) {
            ERR("wid on ticker tree, but no callback set");
        }

        (w->on_tick)(w);
    } }

    action_timers_tick(&wid_timers);
}

static int saved_mouse_x;
static int saved_mouse_y;

void wid_mouse_hide (int visible)
{
    if (visible != wid_mouse_visible) {
        wid_mouse_visible = visible;

        if (visible) {
            sdl_mouse_warp(saved_mouse_x, saved_mouse_y);
        } else {
            saved_mouse_x = mouse_x;
            saved_mouse_y = mouse_y;
        }
    }
}

void wid_mouse_warp (widp w)
{
    int32_t tlx, tly, brx, bry;

    wid_get_abs_coords(w, &tlx, &tly, &brx, &bry);

    int32_t x = (tlx + brx) / 2.0;
    int32_t y = (tly + bry) / 2.0;

    sdl_mouse_warp(x, y);
}

void wid_mouse_move (widp w)
{
    int32_t tlx, tly, brx, bry;

    wid_get_abs_coords(w, &tlx, &tly, &brx, &bry);

    int32_t x = (tlx + brx) / 2.0;
    int32_t y = (tly + bry) / 2.0;

    saved_mouse_x = mouse_x;
    saved_mouse_y = mouse_y;

    mouse_x = x;
    mouse_y = y;
}

static void wid_mouse_blit (void)
{
    static uint32_t ts;
    static tilep tile;
    static double mx;
    static double my;

    if (!tile) {
        tile = tile_find("mouse");
        if (!tile) {
            return;
        }
    }

    double x = mouse_x;
    double y = mouse_y;

    if ((x != mx) || (y != my)) {
        ts = wid_time;
        mx = x;
        my = y;
    }

    /*
     * Hide the mouse after some inactivity.
     */
    if (wid_time - ts > 2000) {
        return;
    }

    double size = ((double)global_config.video_gl_width) / 80.0;
    point tl;
    point br;

    tl.x = x - size;
    tl.y = y - size;
    br.x = x + size;
    br.y = y + size;

    glcolor(WHITE);

    tile_blit_at(tile, 0, tl, br);
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

        if (w->on_display_top_level) {
            if (!wid_is_hidden(w)) {
                (w->on_display_top_level)(w);
            }
        }
    } }

    glDisable(GL_SCISSOR_TEST);

    if (wid_mouse_visible) {
        wid_mouse_blit();
    }
}

void wid_fade_in (widp w, uint32_t delay)
{
    fast_verify(w);

    w->timestamp_fading_begin = wid_time;
    w->timestamp_fading_end = w->timestamp_fading_begin + delay;
    w->fade_out = false;
    w->hidden = false;

    if (delay == 0) {
        w->fade_in = false;
        w->hidden = false;
        w->visible = true;
    } else {
        w->fade_in = true;
    }
}

void wid_fade_out (widp w, uint32_t delay)
{
    fast_verify(w);

    /*
     * If already faded out, do not fade out again, this causes a flicker.
     */
    if (w->fade_out) {
        return;
    }

    w->timestamp_fading_begin = wid_time;
    w->timestamp_fading_end = w->timestamp_fading_begin + delay;
    w->fade_in = false;

    if (delay == 0) {
        w->fade_out = false;
        w->hidden = true;
        w->visible = false;
    } else {
        w->fade_out = true;
    }
}

void wid_fade_in_out (widp w, uint32_t delay, uint32_t repeat, uint8_t in)
{
    fast_verify(w);

    w->timestamp_fading_begin = wid_time;
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

uint8_t wid_is_hidden (widp w)
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

uint8_t inline wid_this_is_hidden (widp w)
{
    return (w->hidden);
}

uint8_t wid_is_always_hidden (widp w)
{
    fast_verify(w);

    if (w->always_hidden) {
        return (true);
    }

    return (false);
}

uint8_t wid_is_fading (widp w)
{
    fast_verify(w);

    if (!w) {
        return (false);
    }

    /*
     * I may be hidden already in a parent that is fading. Just keep hidden.
     */
    if (!w->fade_count) {
        if (w->hidden) {
            return (false);
        }
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

uint8_t wid_is_scaling (widp w)
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

    if (wid_time >= w->timestamp_fading_end) {
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
                w->hidden = false;
                w->visible = true;
            }

            return (0.0f);
        }

        return (1.0f);
    }

    int32_t effect_duration = w->timestamp_fading_end -
                    w->timestamp_fading_begin;

    if (w->fade_in) {
        fade = (double)(wid_time - w->timestamp_fading_begin) /
               (double)effect_duration;

        return (fade);
    }

    if (w->fade_out) {
        fade = 1.0f -
                ((double)(wid_time - w->timestamp_fading_begin) /
                 (double)effect_duration);

        return (fade);
    }

    return (1.0f);
}

void wid_move_to_pct (widp w, double x, double y)
{
    fast_verify(w);

    x *= (double)global_config.video_gl_width;
    y *= (double)global_config.video_gl_height;

    double dx = x - wid_get_tl_x(w);
    double dy = y - wid_get_tl_y(w);

    wid_move_delta(w, dx, dy);
}

void wid_move_to_abs (widp w, double x, double y)
{
    fast_verify(w);

    double dx = x - wid_get_tl_x(w);
    double dy = y - wid_get_tl_y(w);

    wid_move_delta(w, dx, dy);
}

void wid_move_to_pct_centered (widp w, double x, double y)
{
    fast_verify(w);

    x *= (double)global_config.video_gl_width;
    y *= (double)global_config.video_gl_height;

    double dx = x - wid_get_tl_x(w);
    double dy = y - wid_get_tl_y(w);

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

static void wid_move_enqueue (widp w,
                              double moving_start_x,
                              double moving_start_y,
                              double moving_end_x,
                              double moving_end_y,
                              uint32_t ms)
{
    fast_verify(w);

    if (w->moving) {
        /*
         * If this is not a widget with a thing, then just zoom it to the 
         * destination. We don't need queues.
         */
        thingp t = wid_get_thing(w);
        if (!t) {
            w->moving_end.x = moving_end_x;
            w->moving_end.y = moving_end_y;
            w->timestamp_moving_begin = wid_time;
            w->timestamp_moving_end = wid_time + ms;
            return;
        }

        if (w->moving == WID_MAX_MOVE_QUEUE) {
            thingp t = wid_get_thing(w);

            ERR("too many moves queued up for widget %s", 
                wid_logname(w));

            if (t) {
                THING_LOG(t, "too many moves queued up");
            }

#ifdef DEBUG_WID_MOVE
            int i;
            CON("    [-] to %f,%f in %d", 
                w->moving_end.x, w->moving_end.y, 
                w->timestamp_moving_end - wid_time);

            for (i = 0; i < w->moving - 1; i++) {
                wid_move_t *c = &w->move[i];

                CON("    [%d] to %f,%f in %d", i, 
                    c->moving_end.x, c->moving_end.y, 
                    c->timestamp_moving_end - wid_time);
            }
#endif
        }

        wid_move_t *c = &w->move[w->moving - 1];

        c->timestamp_moving_end = wid_time + ms;
        c->moving_end.x = moving_end_x;
        c->moving_end.y = moving_end_y;
    } else {
        w->moving_start.x = moving_start_x;
        w->moving_start.y = moving_start_y;
        w->moving_end.x = moving_end_x;
        w->moving_end.y = moving_end_y;
        w->timestamp_moving_begin = wid_time;
        w->timestamp_moving_end = wid_time + ms;

        wid_tree3_moving_wids_insert(w);
    }

    w->moving++;
}

static void wid_move_dequeue (widp w)
{
    fast_verify(w);

    if (!w->moving) {
        return;
    }

    wid_move_to_abs(w, w->moving_end.x, w->moving_end.y);

    w->moving--;
    if (!w->moving) {
        wid_tree3_moving_wids_remove(w);
        return;
    }

    wid_move_t *c = &w->move[0];

    w->moving_start.x = w->moving_end.x;
    w->moving_start.y = w->moving_end.y;
    w->moving_end.x = c->moving_end.x;
    w->moving_end.y = c->moving_end.y;
    w->timestamp_moving_begin = wid_time;
    w->timestamp_moving_end = c->timestamp_moving_end;

    uint8_t i;
    for (i = 0; i < w->moving; i++) {
        if (i < WID_MAX_MOVE_QUEUE - 1) {
            wid_move_t *c = &w->move[i];
            memcpy(c, c+1, sizeof(*c));
        }
    }

    {
        wid_move_t *c = &w->move[i];
        memset(c, 0, sizeof(*c));
    }
}

void wid_move_to_pct_in (widp w, double x, double y, uint32_t ms)
{
    fast_verify(w);

    x *= (double)global_config.video_gl_width;
    y *= (double)global_config.video_gl_height;

    wid_move_enqueue(w, wid_get_tl_x(w), wid_get_tl_y(w), x, y, ms);
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

void wid_move_to_abs_in (widp w, double x, double y, uint32_t ms)
{
    wid_move_enqueue(w, wid_get_tl_x(w), wid_get_tl_y(w), x, y, ms);
}

void wid_move_to_pct_centered_in (widp w, double x, double y, uint32_t ms)
{
    fast_verify(w);

    x *= (double)global_config.video_gl_width;
    y *= (double)global_config.video_gl_height;

    x -= (wid_get_br_x(w) - wid_get_tl_x(w))/2;
    y -= (wid_get_br_y(w) - wid_get_tl_y(w))/2;

    wid_move_enqueue(w, wid_get_tl_x(w), wid_get_tl_y(w), x, y, ms);
}

void wid_move_delta_pct_in (widp w, double x, double y, uint32_t ms)
{
    fast_verify(w);

    x *= (double)global_config.video_gl_width;
    y *= (double)global_config.video_gl_height;

    wid_move_enqueue(w,
                     wid_get_tl_x(w),
                     wid_get_tl_y(w),
                     wid_get_tl_x(w) + x,
                     wid_get_tl_y(w) + y,
                     ms);
}

void wid_move_to_abs_centered_in (widp w, double x, double y, uint32_t ms)
{
    fast_verify(w);

    x -= (wid_get_br_x(w) - wid_get_tl_x(w))/2;
    y -= (wid_get_br_y(w) - wid_get_tl_y(w))/2;

    wid_move_enqueue(w, wid_get_tl_x(w), wid_get_tl_y(w), x, y, ms);
}

void wid_move_to_centered_in (widp w, double x, double y, uint32_t ms)
{
    fast_verify(w);

    wid_move_enqueue(w, wid_get_tl_x(w), wid_get_tl_y(w), x, y, ms);
}

void wid_scaling_to_pct_in (widp w,
                            double scaling_start,
                            double scaling_end,
                            uint32_t ms,
                            uint32_t scaling_repeat_count)
{
    fast_verify(w);

    w->timestamp_scaling_w_begin = wid_time;
    w->timestamp_scaling_h_begin = wid_time;

    w->timestamp_scaling_w_end = w->timestamp_scaling_w_begin + ms;
    w->timestamp_scaling_h_end = w->timestamp_scaling_h_begin + ms;

    w->scaling_w_start = scaling_start;
    w->scaling_h_start = scaling_start;

    w->scaling_w_end = scaling_end;
    w->scaling_h_end = scaling_end;

    w->scaling_w_repeat_count = scaling_repeat_count;
    w->scaling_h_repeat_count = scaling_repeat_count;

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
        wid_scaling_to_pct_in(w, 1.0, 1.15, wid_pulse_delay, 1);
    } else {
        wid_scaling_to_pct_in(w, 1.0, 1.05, wid_pulse_delay, 1);
    }
}

void wid_effect_pulse_forever (widp w)
{
    fast_verify(w);

    if (wid_get_height(w) > 100) {
        wid_scaling_to_pct_in(w, 1.0, 1.15, wid_scaling_forever_delay, 10000);
    } else {
        wid_scaling_to_pct_in(w, 1.0, 1.05, wid_scaling_forever_delay, 10000);
    }
}

void wid_effect_pulse_stop (widp w)
{
    fast_verify(w);

    w->scaling_w = false;
    w->scaling_h = false;
    w->blit_scaling_w = false;
    w->blit_scaling_h = false;
}

double wid_get_scaling_w (widp w)
{
    double scaling;

    if (!w->scaling_w && !w->scaled_w) {
        return (1.0);
    }

    const uint32_t ts = wid_time;

    if (ts >= w->timestamp_scaling_w_end) {

        scaling = w->scaling_w_end;
        w->scaling_w = false;

        if (w->scaling_w_repeat_count) {
            wid_scaling_to_pct_in(w,
                                  w->scaling_w_end,
                                  w->scaling_w_start,
                                  w->timestamp_scaling_w_end -
                                  w->timestamp_scaling_w_begin,
                                  w->scaling_w_repeat_count - 1);
        }

        return (scaling + w->scale_w_base);
    }

    double time_step =
        (double)(ts - w->timestamp_scaling_w_begin) /
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

    const uint32_t ts = wid_time;

    if (ts >= w->timestamp_scaling_h_end) {

        scaling = w->scaling_h_end;
        w->scaling_h = false;

        if (w->scaling_h_repeat_count) {
            wid_scaling_to_pct_in(w,
                                  w->scaling_h_end,
                                  w->scaling_h_start,
                                  w->timestamp_scaling_h_end -
                                  w->timestamp_scaling_h_begin,
                                  w->scaling_h_repeat_count - 1);
        }

        return (scaling + w->scale_h_base);
    }

    double time_step =
        (double)(ts - w->timestamp_scaling_h_begin) /
        (double)(w->timestamp_scaling_h_end - w->timestamp_scaling_h_begin);

    scaling = (time_step * (double)(w->scaling_h_end - w->scaling_h_start)) +
                    w->scaling_h_start;

    return (scaling + w->scale_h_base);
}

void wid_scaling_blit_to_pct_in (widp w,
                            double blit_scaling_start,
                            double blit_scaling_end,
                            uint32_t ms,
                            uint32_t blit_scaling_repeat_count)
{
    fast_verify(w);

    const uint32_t ts = wid_time;

    w->timestamp_blit_scaling_w_begin = ts;
    w->timestamp_blit_scaling_h_begin = ts;

    w->timestamp_blit_scaling_w_end = w->timestamp_blit_scaling_w_begin + ms;
    w->timestamp_blit_scaling_h_end = w->timestamp_blit_scaling_h_begin + ms;

    w->blit_scaling_w_start = blit_scaling_start;
    w->blit_scaling_h_start = blit_scaling_start;

    w->blit_scaling_w_end = blit_scaling_end;
    w->blit_scaling_h_end = blit_scaling_end;

    w->blit_scaling_w_repeat_count = blit_scaling_repeat_count;
    w->blit_scaling_h_repeat_count = blit_scaling_repeat_count;

    w->blit_scaling_w = true;
    w->blit_scaling_h = true;
}

void wid_blit_scale_immediate (widp w, double val)
{
    fast_verify(w);

    w->blit_scale_w_base = val;
    w->blit_scale_h_base = val;
    w->blit_scaled_w = true;
    w->blit_scaled_h = true;

    if (val == 0.0) {
        w->blit_scaled_w = false;
        w->blit_scaled_h = false;
    }
}

void wid_blit_scale_w_immediate (widp w, double val)
{
    fast_verify(w);

    w->blit_scale_w_base = val;
    w->blit_scaled_w = true;

    if (val == 0.0) {
        w->blit_scaled_w = false;
    }
}

void wid_blit_scale_h_immediate (widp w, double val)
{
    fast_verify(w);

    w->blit_scale_h_base = val;
    w->blit_scaled_h = true;

    if (val == 0.0) {
        w->blit_scaled_h = false;
    }
}

void wid_blit_effect_pulses (widp w)
{
    fast_verify(w);

    wid_scaling_blit_to_pct_in(w, 1.0, 1.1, wid_pulse_delay, 1);
}

double wid_get_blit_scaling_w (widp w)
{
    double blit_scaling;

    if (!w->blit_scaling_w && !w->blit_scaled_w) {
        return (1.0);
    }

    const uint32_t ts = wid_time;

    if (ts >= w->timestamp_blit_scaling_w_end) {

        blit_scaling = w->blit_scaling_w_end;
        w->blit_scaling_w = false;

        if (w->blit_scaling_w_repeat_count) {
            wid_scaling_blit_to_pct_in(w,
                                  w->blit_scaling_w_end,
                                  w->blit_scaling_w_start,
                                  w->timestamp_blit_scaling_w_end -
                                  w->timestamp_blit_scaling_w_begin,
                                  w->blit_scaling_w_repeat_count - 1);
        }

        return (blit_scaling + w->blit_scale_w_base);
    }

    double time_step =
        (double)(ts - w->timestamp_blit_scaling_w_begin) /
        (double)(w->timestamp_blit_scaling_w_end - w->timestamp_blit_scaling_w_begin);

    blit_scaling = (time_step * (double)(w->blit_scaling_w_end - w->blit_scaling_w_start)) +
                    w->blit_scaling_w_start;

    return (blit_scaling + w->blit_scale_w_base);
}

double wid_get_blit_scaling_h (widp w)
{
    double blit_scaling;

    if (!w->blit_scaling_h && !w->blit_scaled_h) {
        return (1.0);
    }

    if (wid_time >= w->timestamp_blit_scaling_h_end) {

        blit_scaling = w->blit_scaling_h_end;
        w->blit_scaling_h = false;

        if (w->blit_scaling_h_repeat_count) {
            wid_scaling_blit_to_pct_in(w,
                                  w->blit_scaling_h_end,
                                  w->blit_scaling_h_start,
                                  w->timestamp_blit_scaling_h_end -
                                  w->timestamp_blit_scaling_h_begin,
                                  w->blit_scaling_h_repeat_count - 1);
        }

        return (blit_scaling + w->blit_scale_h_base);
    }

    double time_step =
        (double)(wid_time - w->timestamp_blit_scaling_h_begin) /
        (double)(w->timestamp_blit_scaling_h_end - w->timestamp_blit_scaling_h_begin);

    blit_scaling = (time_step * (double)(w->blit_scaling_h_end - w->blit_scaling_h_start)) +
                    w->blit_scaling_h_start;

    return (blit_scaling + w->blit_scale_h_base);
}

void wid_rotate_to_pct_in (widp w,
                            double rotate_start,
                            double rotate_end,
                            uint32_t ms,
                            uint32_t rotate_sways_count)
{
    fast_verify(w);

    w->timestamp_rotate_begin = wid_time;
    w->timestamp_rotate_end = w->timestamp_rotate_begin + ms;

    w->rotate_start = rotate_start;
    w->rotate_end = rotate_end;
    w->rotate_sways_count = rotate_sways_count;
    w->rotating = true;
}

void wid_bounce_to_pct_in (widp w,
                            double bounce_height,
                            double bounce_fade,
                            uint32_t ms,
                            uint32_t bounce_count)
{
    fast_verify(w);

    w->timestamp_bounce_begin = wid_time;
    w->timestamp_bounce_end = w->timestamp_bounce_begin + ms;

    w->bounce_height = bounce_height;
    w->bounce_fade = bounce_fade;
    w->bounce_count = bounce_count;
    w->bouncing = true;
}

void wid_shake_to_pct_in (widp w,
                          double shake_amount,
                          double shake_fade,
                          uint32_t ms,
                          uint32_t shake_count)
{
    fast_verify(w);

    w->timestamp_shake_begin = wid_time;
    w->timestamp_shake_end = w->timestamp_shake_begin + ms;

    w->shake_amount = shake_amount;
    w->shake_fade = shake_fade;
    w->shake_count = shake_count;
    w->shaking = true;
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

void wid_flip_horiz (widp w, uint8_t val)
{
    fast_verify(w);

    w->flip_vert = val;
}

void wid_flip_vert (widp w, uint8_t val)
{
    fast_verify(w);

    w->flip_horiz = val;
}

uint8_t wid_get_flip_horiz (widp w)
{
    fast_verify(w);

    return (w->flip_vert);
}

uint8_t wid_get_flip_vert (widp w)
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

    if (wid_time >= w->timestamp_rotate_end) {

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
        (double)(wid_time - w->timestamp_rotate_begin) /
        (double)(w->timestamp_rotate_end - w->timestamp_rotate_begin);

    rotating = (time_step * (double)(w->rotate_end - w->rotate_start)) +
                    w->rotate_start;

    return (w->rotate_base + rotating);
}

double wid_get_bounce (widp w)
{
    if (!w->bouncing) {
        return (0.0);
    }

    if (wid_time >= w->timestamp_bounce_end) {

        w->bouncing = false;

        if (w->bounce_count) {
            wid_bounce_to_pct_in(
                w,
                w->bounce_height * w->bounce_fade,
                w->bounce_fade,
                (double)(w->timestamp_bounce_end - w->timestamp_bounce_begin) * w->bounce_fade,
                w->bounce_count - 1);
        }

        return (0);
    }

    double time_step =
        (double)(wid_time - w->timestamp_bounce_begin) /
        (double)(w->timestamp_bounce_end - w->timestamp_bounce_begin);

    double height = sin(time_step * RAD_180);

    height *= (double) wid_get_height(w);

    height *= w->bounce_height;

    return (height);
}

void wid_get_shake (widp w, double *x, double *y)
{
    if (!w->shaking) {
        *x = 0.0;
        *y = 0.0;
        return;
    }

    if (wid_time >= w->timestamp_shake_end) {

        w->shaking = false;
        w->shaking_set = false;

        *x = 0.0;
        *y = 0.0;

        if (w->shake_count) {
            wid_shake_to_pct_in(
                w,
                w->shake_amount * w->shake_fade,
                w->shake_fade,
                (double)(w->timestamp_shake_end - w->timestamp_shake_begin) * w->shake_fade,
                w->shake_count - 1);
        }

        return;
    }

    /*
     * Set the shake amount to a starting point.
     */
    if (!w->shaking_set) {
        w->shaking_set = 1;

        w->shake_x = wid_get_width(w) * 
            (((double) (myrand() % 100)) / 100.0) * 
            w->shake_amount;

        w->shake_y = wid_get_height(w) * 
            (((double) (myrand() % 100)) / 100.0) * 
            w->shake_amount;

        if ((myrand() % 10) < 5) {
            w->shake_x *= -1.0;
        }

        if ((myrand() % 10) < 5) {
            w->shake_y *= -1.0;
        }
    }

    double time_step =
        (double)(wid_time - w->timestamp_shake_begin) /
        (double)(w->timestamp_shake_end - w->timestamp_shake_begin);

    *x = w->shake_x * (1.0 - time_step);
    *y = w->shake_y * (1.0 - time_step);
}

uint8_t wids_overlap (widp A, widp B)
{
    /*
     * The rectangles don't overlap if one rectangle's minimum in some 
     * dimension is greater than the other's maximum in that dimension.
     */
    uint8_t no_overlap = (A->tree.tl.x > B->tree.br.x) ||
                      (B->tree.tl.x > A->tree.br.x) ||
                      (A->tree.tl.y > B->tree.br.y) ||
                      (B->tree.tl.y > A->tree.br.y);

    return (!no_overlap);
}
