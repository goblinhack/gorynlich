/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#pragma once

#define COLORS_ACROSS       30
#define COLORS_DOWN         30

typedef struct wid_cmap_cell_ {
    color color;
    char *name;
    int x;
    int y;
} wid_cmap_color;

typedef void(*wid_cmap_event_t)(widp);
typedef void(*on_selected_t)(const color);
typedef void(*on_cancelled_t)(void);

typedef struct {
    /*
     * Parent widget
     */
    widp w;

    /*
     * Current button
     */
    widp b;

    /*
     * Entered text.
     */
    char text[MAXSTR];

    /*
     * Item currently in focus
     */
    int focusx;
    int focusy;

    /*
     * When the map was made.
     */
    uint32_t created;

    /*
     * Items in the map
     */
    widp buttons[COLORS_ACROSS][COLORS_DOWN];
    wid_cmap_color colors[COLORS_ACROSS][COLORS_DOWN];

    /*
     * What to call on events
     */
    wid_cmap_event_t selected;
    wid_cmap_event_t cancelled;

    /*
     * Just created?
     */
    int is_new;

    on_selected_t on_selected;
    on_cancelled_t on_cancelled;

} wid_cmap_ctx;

widp wid_cmap(const char *title, on_selected_t, on_cancelled_t);

widp wid_editor_color_map_thing_replace_template(widp w,
                                                 double x,
                                                 double y,
                                                 thingp t,
                                                 tpp tp,
                                                 tpp_data,
                                                 itemp item,
                                                 thing_statsp stats);
extern void wid_cmap_cell_play(void);
extern void wid_cmap_cell_load(void);
extern void wid_cmap_visible(void);
extern void wid_cmap_preview(widp);
extern void wid_cmap_preview_thumbnail(widp);
