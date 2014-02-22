/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "tex.h"
#include "color.h"
#include "tile.h"
#include "thing_tile.h"
#include "tree.h"
#include "wid_editor.h"
#include "wid_editor_map.h"
#include "wid_editor_buttons.h"
#include "wid_textbox.h"
#include "thing_template.h"
#include "wid_intro.h"
#include "gl.h"
#include "map.h"
#include "level.h"
#include "config.h"
#include "thing.h"
#include "sdl.h"
#include "string.h"

widp wid_editor_map_window;
widp wid_editor_map_grid_container;

static widp wid_editor_map_horiz_scroll;
static widp wid_editor_map_vert_scroll;

static float tile_width_pct = 1.0f / TILES_SCREEN_WIDTH;
static float tile_height_pct = 1.0f / TILES_SCREEN_HEIGHT;
static uint32_t tile_width;
static uint32_t tile_height;
levelp level_ed;
boolean wid_editor_map_loading;
boolean wid_editor_got_line_start;

static boolean wid_editor_ignore_events (widp w)
{
    if (wid_ignore_for_events(wid_editor_map_window)) {
        return (true);
    }

    return (false);
}

/*
 * Replace or place a tile.
 */
widp wid_editor_map_thing_replace_template (widp w,
                                           int32_t x,
                                           int32_t y,
                                           thing_templatep thing_template)
{
    tree_rootp thing_tiles;
    const char *tilename;
    uint8_t z_depth;
    uint8_t z_order;
    widp existing;
    tilep tile;
    widp child;
    levelp level;


    level = (typeof(level)) wid_get_client_context(w);
    verify(level);

    if ((x < 0) || (y < 0) ||
        (x >= TILES_MAP_EDITABLE_WIDTH) ||
        (y >= TILES_MAP_EDITABLE_HEIGHT)) {
        return (0);
    }

    /*
     * Find the midpoint of the tile.
     */
    x *= tile_width;
    y *= tile_height;

    x += tile_width / 2;
    y += tile_height / 2;

    fpoint tl = {
        (float) x, (float) y
    };

    fpoint br = {
        (float) x, (float) y
    };

    /*
     * Grow tl and br to fit the template thing. Use the first tile.
     */
    thing_tiles = thing_template_get_tiles(thing_template);
    if (!thing_tiles) {
        DIE("thing template [%s] has no tiles",
            thing_template_short_name(thing_template));
    }

    thing_tilep thing_tile;

    /*
     * Get the first anim tile.
     */
    thing_tile = (typeof(thing_tile))
                    tree_root_first(thing_tiles);

    /*
     * Find the real tile that corresponds to this name.
     */
    tilename = thing_tile_name(thing_tile);
    tile = tile_find(tilename);

    if (!tile) {
        DIE("tile name %s from thing %s not found",
            tilename,
            thing_template_short_name(thing_template));
    }

    float base_tile_width =
            ((1.0f / (float)TILES_SCREEN_WIDTH) *
                (float)global_config.video_gl_width);

    float base_tile_height =
            ((1.0f / (float)TILES_SCREEN_HEIGHT) *
                (float)global_config.video_gl_height);

    br.x += base_tile_width;
    br.y += base_tile_height;

    br.x += base_tile_width / 4.0;
    br.y += base_tile_height / 4.0;

    br.x += base_tile_width / 6.0;
    br.y += base_tile_height / 4.0;

    tl.x -= base_tile_height / 2.0;
    br.x -= base_tile_width / 2.0;

    tl.x += base_tile_height / 8.0;
    br.x += base_tile_width / 8.0;

    tl.y -= base_tile_height / 2.0;
    br.y -= base_tile_width / 2.0;

    tl.y -= base_tile_height / 4.0;
    br.y -= base_tile_width / 4.0;

    z_depth = thing_template_get_z_depth(thing_template);
    z_order = thing_template_get_z_order(thing_template);

    existing = wid_find_matching(wid_editor_map_grid_container,
                                 tl, br, z_depth, z_order);
    if (existing)  {
        child = existing;

        wid_set_thing_template(child, thing_template);

        wid_update(child);


        /*
         * Do the fixup at the end as it is slow.
         */
        if (wid_editor_map_loading) {
            return (child);
        }

        map_fixup(level);
        wid_raise(wid_editor_filename_and_title);
        wid_update(wid_editor_map_window);

        return (child);
    }

    /*
     * Make a new thing.
     */
    child = wid_new_square_button(wid_editor_map_grid_container,
                                  "map tile");

    wid_set_tl_br(child, tl, br);
    wid_set_text_lhs(child, true);
    wid_set_text_top(child, true);
    wid_set_mode(child, WID_MODE_NORMAL);
    wid_set_color(child, WID_COLOR_TEXT, STEELBLUE);
    wid_set_color(child, WID_COLOR_TL, BLACK);
    wid_set_color(child, WID_COLOR_BG, BLACK);
    wid_set_color(child, WID_COLOR_BR, BLACK);
    wid_set_text_outline(child, true);
    wid_set_font(child, med_font);
    wid_set_no_shape(child);

    /*
     * "paint" the thing.
     */
    wid_set_thing_template(child, thing_template);

    /*
     * This adds it to the grid wid.
     */
    wid_update(child);

    /*
     * Do the fixup at the end as it is slow.
     */
    if (wid_editor_map_loading) {
        return (child);
    }

    map_fixup(level);
    wid_raise(wid_editor_filename_and_title);
    wid_update(wid_editor_map_window);

    return (child);
}

/*
 * Flood file.
 */
void wid_editor_map_thing_flood_fill_template (int32_t x, int32_t y,
                                               thing_templatep thing_template)
{
    tree_rootp thing_tiles;
    const char *tilename;
    widp existing;
    tilep tile;

    uint32_t xin;
    uint32_t yin;

    if ((x < 0) || (y < 0) ||
        (x >= TILES_MAP_EDITABLE_WIDTH) ||
        (y >= TILES_MAP_EDITABLE_HEIGHT)) {
        return;
    }

    xin = x;
    yin = y;

    /*
     * Grow tl and br to fit the template thing. Use the first tile.
     */
    thing_tiles = thing_template_get_tiles(thing_template);
    if (!thing_tiles) {
        DIE("thing template [%s] has no tiles",
            thing_template_short_name(thing_template));
    }

    thing_tilep thing_tile;

    /*
     * Get the first anim tile.
     */
    thing_tile = (typeof(thing_tile))
                    tree_root_first(thing_tiles);

    /*
     * Find the real tile that corresponds to this name.
     */
    tilename = thing_tile_name(thing_tile);
    tile = tile_find(tilename);

    if (!tile) {
        DIE("tile name %s from thing %s not found",
            tilename,
            thing_template_short_name(thing_template));
    }

    /*
     * Check to see there is nothing here blocking us.
     */
    existing = wid_grid_find_first(wid_editor_map_grid_container, xin, yin);
    while (existing) {
        if (wid_get_thing_template(existing)) {
            return;
        }

        existing = wid_grid_find_next(wid_editor_map_grid_container,
                                      existing, xin, yin);
    }

    wid_editor_map_thing_replace_template(wid_editor_map_grid_container,
                                          xin, yin, thing_template);

    wid_editor_map_thing_flood_fill_template(xin + 1, yin, thing_template);
    wid_editor_map_thing_flood_fill_template(xin - 1, yin, thing_template);
    wid_editor_map_thing_flood_fill_template(xin, yin + 1, thing_template);
    wid_editor_map_thing_flood_fill_template(xin, yin - 1, thing_template);
}

/*
 * Remove the top tile.
 */
static boolean wid_editor_map_thing_remove_template (
                                             int32_t x,
                                             int32_t y)
{
    widp existing;

    /*
     * Find the midpoint of the tile.
     */
    x *= tile_width;
    y *= tile_height;

    x += tile_width / 2;
    y += tile_height / 2;

    fpoint tl = {
        (float) x, (float) y
    };

    fpoint br = {
        (float) x, (float) y
    };

    existing = wid_find_matching_top(wid_editor_map_grid_container, tl, br);
    if (existing)  {
        thing_templatep t = wid_get_thing_template(existing);
        if (!t) {
            return (true);
        }

        wid_destroy(&existing);
    }

    map_fixup(level_ed);

    return (true);
}

/*
 * Mouse down etc...
 */
static boolean wid_editor_map_thing_replace (widp w,
                                             int32_t x,
                                             int32_t y,
                                             boolean scaled)
{
    thing_templatep thing_template;
    widp focus;

    if (!scaled) {
        fpoint offset;

        wid_get_offset(wid_editor_map_grid_container, &offset);

        x += -offset.x;
        y += -offset.y;

        x /= tile_width;
        y /= tile_height;
    }

    if (wid_editor_mode_eraser) {
        wid_editor_map_thing_remove_template(x, y);
        return (true);
    }

    /*
     * Get the thing we are drawing with.
     */
    focus = wid_get_focus(wid_editor_buttons_container2);
    if (!focus) {
        return (false);
    }

    thing_template = wid_get_thing_template(focus);
    if (!thing_template) {
        return (false);
    }

    (void) wid_editor_map_thing_replace_template(wid_editor_map_grid_container,
                                                 x, y,
                                                 thing_template);

    return (true);
}

/*
 * Mouse down etc...
 */
static boolean wid_editor_map_thing_flood_fill (widp w, int32_t x, int32_t y)
{
    thing_templatep thing_template;
    fpoint offset;
    widp focus;

    wid_get_offset(wid_editor_map_grid_container, &offset);

    x += -offset.x;
    y += -offset.y;

    x /= tile_width;
    y /= tile_height;

    /*
     * Get the thing we are drawing with.
     */
    focus = wid_get_focus(wid_editor_buttons_container2);
    if (!focus) {
        return (false);
    }

    thing_template = wid_get_thing_template(focus);
    if (!thing_template) {
        return (false);
    }

    (void) wid_editor_map_thing_flood_fill_template(x, y, thing_template);

    return (true);
}

/*
 * Mouse down etc...
 */
static boolean wid_editor_map_thing_remove (widp w,
                                            int32_t x,
                                            int32_t y)
{
    fpoint offset;

    wid_editor_save_point();

    wid_get_offset(wid_editor_map_grid_container, &offset);

    x += -offset.x;
    y += -offset.y;

    x /= tile_width;
    y /= tile_height;

    wid_editor_map_thing_remove_template(x, y);

    return (true);
}

static void do_wid_editor_line (widp w, 
                                int32_t x0_in, 
                                int32_t y0_in, 
                                int32_t x1_in, 
                                int32_t y1_in, 
                                int32_t flag)
{
    double temp;
    double dx;
    double dy;
    double tdy;
    double dydx;
    double p;
    double x;
    double y;
    double i;

    double x0 = x0_in;
    double y0 = y0_in;
    double x1 = x1_in;
    double y1 = y1_in;

    if (x0 > x1) {
        temp = x0;
        x0 = x1;
        x1 = temp;

        temp = y0;
        y0 = y1;
        y1 = temp;
    }

    dx = x1 - x0;
    dy = y1 - y0;

    tdy = 2.0 * dy;
    dydx = tdy - (2.0 * dx);

    p = tdy - dx;
    x = x0;
    y = y0;

    if (flag == 0) {
        wid_editor_map_thing_replace(w, (int32_t)x, (int32_t)y, true /* scaled */);
    } else if (flag == 1) {
        wid_editor_map_thing_replace(w, (int32_t)y, (int32_t)x, true /* scaled */);
    } else if (flag == 2) {
        wid_editor_map_thing_replace(w, (int32_t)y, (int32_t)-x, true /* scaled */);
    } else if (flag == 3) {
        wid_editor_map_thing_replace(w, (int32_t)x, (int32_t)-y, true /* scaled */);
    }

    for (i = 1; i <= dx; i++){
        x++;

        if (p < 0) {
            p += tdy;
        } else {
            p += dydx;
            y++;
        }

        if (flag == 0) {
            wid_editor_map_thing_replace(w, (int32_t)x, (int32_t)y, true /* scaled */);
        } else if (flag == 1) {
            wid_editor_map_thing_replace(w, (int32_t)y, (int32_t)x, true /* scaled */);
        } else if (flag == 2) {
            wid_editor_map_thing_replace(w, (int32_t)y, (int32_t)-x, true /* scaled */);
        } else if (flag == 3) {
            wid_editor_map_thing_replace(w, (int32_t)x, (int32_t)-y, true /* scaled */);
        }
    }
}

static void wid_editor_draw_line (widp w, int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    double slope = 100.0;

    if (x0 != x1) {
        slope = (y1 - y0) * (1.0 / (x1 - x0));
    }

    if ((0 <= slope) && (slope <= 1)) {
        do_wid_editor_line(w, x0, y0, x1, y1, 0);
    } else if ((-1 <= slope) && (slope <= 0)) {
        do_wid_editor_line(w, x0, -y0, x1, -y1, 3);
    } else if (slope > 1) {
        do_wid_editor_line(w, y0, x0, y1, x1, 1);
    } else {
        do_wid_editor_line(w, -y0, x0, -y1, x1, 2);
    }
}

static boolean wid_editor_map_thing_replace_wrap (widp w,
                                                  int32_t x,
                                                  int32_t y)
{
    static int32_t line_start_x;
    static int32_t line_start_y;

    if (wid_editor_mode_fill) {
        wid_editor_save_point();

        wid_editor_map_loading = true;
        wid_editor_map_thing_flood_fill(w, x, y);
        wid_editor_map_loading = false;

        map_fixup(level_ed);
        wid_raise(wid_editor_filename_and_title);
        wid_update(wid_editor_map_window);
        return (true);
    }

    if (wid_editor_mode_line) {
        fpoint offset;
        wid_get_offset(wid_editor_map_grid_container, &offset);

        if (!wid_editor_got_line_start) {
            wid_editor_got_line_start = true;

            x += -offset.x;
            y += -offset.y;
            x /= tile_width;
            y /= tile_height;

            line_start_x = x;
            line_start_y = y;
            return (true);
        }

        wid_editor_save_point();

        wid_editor_got_line_start = false;

        x += -offset.x;
        y += -offset.y;
        x /= tile_width;
        y /= tile_height;

        wid_editor_map_loading = true;
        wid_editor_draw_line(w, line_start_x, line_start_y, x, y);
        wid_editor_map_loading = false;
        
        map_fixup(level_ed);
        wid_raise(wid_editor_filename_and_title);
        wid_update(wid_editor_map_window);

        return (true);
    }

    wid_editor_save_point();

    return (wid_editor_map_thing_replace(w, x, y, false /* scaled */));
}

/*
 * Mouse down etc...
 */
static boolean wid_editor_map_receive_mouse_down (widp w,
                                                  int32_t x,
                                                  int32_t y,
                                                  uint32_t button)
{
    if (SDL_BUTTON(SDL_BUTTON_LEFT) & SDL_GetMouseState(0, 0)) {
        return (wid_editor_map_thing_replace_wrap(w, x, y));
    }

    if (SDL_BUTTON(SDL_BUTTON_RIGHT) & SDL_GetMouseState(0, 0)) {
        return (wid_editor_map_thing_remove(w, x, y));
    }

    return (false);
}

static boolean wid_editor_map_receive_mouse_motion (
                    widp w,
                    int32_t x, int32_t y,
                    int32_t relx, int32_t rely,
                    int32_t wheelx, int32_t wheely)
{
    if (wheelx || wheely) {
        /*
         * Allow scrolling.
         */
        return (false);
    }

    if (SDL_BUTTON(SDL_BUTTON_LEFT) & SDL_GetMouseState(0, 0)) {
        return (wid_editor_map_thing_replace_wrap(w, x, y));
    }

    if (SDL_BUTTON(SDL_BUTTON_RIGHT) & SDL_GetMouseState(0, 0)) {
        return (wid_editor_map_thing_remove(w, x, y));
    }

    /*
     * Block moving the window.
     */
    return (true);
}

/*
 * Mouse up etc...
 */
static boolean wid_editor_map_receive_mouse_up (widp w, int32_t x, int32_t y,
                                                uint32_t button)
{
    return (false);
}

static boolean wid_editor_map_tile_key_down_event (widp w,
                                                   const SDL_KEYSYM *key)
{
    int32_t x;
    int32_t y;
    boolean rc;

    if (wid_editor_ignore_events(w)) {
        return (false);
    }

    switch (key->sym) {
        case SDLK_TAB:
            return (true);

        case '`':
        case SDLK_LEFT:
        case SDLK_RIGHT:
        case SDLK_RSHIFT:
        case SDLK_LSHIFT:
            break;

        case ' ':
        case SDLK_RETURN:
            (void) SDL_GetMouseState(&x, &y);

            x *= global_config.xscale;
            y *= global_config.yscale;

            rc = wid_editor_map_thing_replace_wrap(wid_editor_map_grid_container, x, y);

            return (rc);


        case SDLK_BACKSPACE:
        case SDLK_DELETE:
            (void) SDL_GetMouseState(&x, &y);

            x *= global_config.xscale;
            y *= global_config.yscale;

            rc = wid_editor_map_thing_remove(wid_editor_map_grid_container, x, y);

            return (rc);

        case 'q':
        case 'b':
            wid_editor_hide();
            return (true);

        case SDLK_ESCAPE:
            if (!wid_editor_mode_draw) {
                wid_editor_draw();
                return (true);
            }

            wid_editor_hide();
            return (true);

        case 's':
            wid_editor_save();
            return (true);

        case 'n':
            wid_editor_line();
            return (true);

        case 'u':
            wid_editor_save_point();
            wid_editor_undo_save_point();
            wid_editor_undo_save_point();
            return (true);

        case 'r':
            wid_editor_redo_save_point();
            return (true);

        case 'l':
            wid_editor_load();
            return (true);

        case 'd':
            wid_editor_draw();
            return (true);

        case 'e':
            wid_editor_erase();
            return (true);

        case 'f':
            wid_editor_fill();
            return (true);

        case 'c':
            wid_editor_clear();
            return (true);

        default:
            return (true);
    }

    return (false);
}

static boolean wid_editor_map_tile_mouse_motion (widp w,
                    int32_t x, int32_t y,
                    int32_t relx, int32_t rely,
                    int32_t wheelx, int32_t wheely)
{
    if (wheelx || wheely) {
        /*
         * Allow scrolling.
         */
        return (false);
    }

    if (SDL_BUTTON(SDL_BUTTON_LEFT) & SDL_GetMouseState(0, 0)) {
        return (wid_editor_map_thing_replace(w, x, y, false /* scaled */));
    }

    if (SDL_BUTTON(SDL_BUTTON_RIGHT) & SDL_GetMouseState(0, 0)) {
        return (wid_editor_map_thing_remove(w, x, y));
    }

    /*
     * Block moving the window.
     */
    return (true);
}

static boolean wid_editor_map_tile_key_up_event (widp w,
                                                 const SDL_KEYSYM *key)
{
    if (wid_editor_ignore_events(w)) {
        return (false);
    }

    switch (key->sym) {
        case SDLK_TAB:
            wid_toggle_hidden(wid_editor_buttons_window,
                              false /* immediate */);

            wid_raise(wid_editor_clear_popup);
            wid_raise(wid_editor_help_popup);
            wid_raise(wid_editor_save_popup);
            wid_raise(wid_editor_title_popup);
            wid_raise(wid_editor_load_popup);
            wid_raise(wid_editor_filename_and_title);

            return (true);

        default:
            return (true);
    }

    return (false);
}

/*
 * Create the wid_editor_map
 */
void wid_editor_add_grid (void)
{
    {
        int32_t x;
        int32_t y;

        widp child;

        for (x = 0; x < TILES_MAP_WIDTH; x++) {
            for (y = 0; y < TILES_MAP_HEIGHT; y++) {

                fpoint tl = {
                    (tile_width_pct * (float)(x)),
                    (tile_height_pct * (float)(y))
                };

                fpoint br = {
                };

                br.x = tl.x + tile_width_pct;
                br.y = tl.y + tile_height_pct;

                child = wid_new_square_button(wid_editor_map_grid_container,
                                              "map tile");

                wid_set_color(child, WID_COLOR_BG, BLACK);
                color c = WHITE;
                c.a = 100;

                wid_set_color(child, WID_COLOR_TEXT, c);
                wid_set_font(child, vsmall_font);

                if (!(x % 10) || !(y % 10)) {
                    char tmp[20];
                    sprintf(tmp, "%u,%u",x,y);
                    wid_set_text(child, tmp);
                }

                if ((x < TILES_MAP_EDITABLE_WIDTH) &&
                    (y < TILES_MAP_EDITABLE_HEIGHT)) {
                    wid_set_on_key_down(child,
                                        wid_editor_map_tile_key_down_event);
                    wid_set_on_key_up(child,
                                      wid_editor_map_tile_key_up_event);
                    wid_set_on_mouse_motion(child,
                                            wid_editor_map_tile_mouse_motion);
                } else {
                    thing_templatep noentry;

                    noentry = thing_template_find("data/things/noentry");
                    if (!noentry) {
                        DIE("noentry icon not found");
                    }

                    wid_set_thing_template(child, noentry);
                }

                wid_set_z_depth(child, 0);
                wid_set_z_order(child, 0);

                wid_set_tl_br_pct(child, tl, br);

                tile_width = wid_get_width(child);
                tile_height = wid_get_height(child);
            }
        }
    }
}

static void wid_editor_title_set (const char *title)
{
    level_set_title(level_ed, title);

    wid_destroy(&wid_editor_filename_and_title);

    if (level_get_title(level_ed) &&
        strcmp(level_get_title(level_ed), "(null)")) {

        LOG("Level titled as: %s", title);

        char *name = dynprintf("%s", level_get_title(level_ed));

        wid_editor_filename_and_title = wid_textbox(
                    wid_editor_map_window,
                    0,
                    name,
                    0.5, 0.07, med_font);

        myfree(name);
    } else {
        wid_editor_filename_and_title = wid_textbox(
                    wid_editor_map_window,
                    0,
                    "Level is unnamed",
                    0.5, 0.07, med_font);
    }

    wid_set_no_shape(wid_editor_filename_and_title);
    wid_raise(wid_editor_filename_and_title);
    wid_set_do_not_lower(wid_editor_filename_and_title, true);

    wid_destroy_in(wid_editor_filename_and_title, 3000);
    wid_editor_filename_and_title = 0;
}

/*
 * Create the wid_editor_map
 */
void wid_editor_map_wid_create (void)
{
    if (wid_editor_map_window) {
        return;
    }

    {
        fpoint tl = {0.0f, 0.0f};
        fpoint br = {1.0f, 1.0f};

        wid_editor_map_window = wid_new_square_window("wid_editor_map");
        wid_set_movable(wid_editor_map_window, false);
        wid_set_do_not_raise(wid_editor_map_window, true);

        wid_set_mode(wid_editor_map_window, WID_MODE_NORMAL);

        wid_set_text_advance(wid_editor_map_window, 0.9f);
        wid_set_text_scaling(wid_editor_map_window, 2.0f);
        wid_set_text_pos(wid_editor_map_window, true, 0.5f, 0.10f);
        wid_set_text_outline(wid_editor_map_window, true);

        wid_set_on_mouse_down(wid_editor_map_window,
                              wid_editor_map_receive_mouse_down);
        wid_set_on_mouse_motion(wid_editor_map_window,
                                wid_editor_map_receive_mouse_motion);
        wid_set_on_mouse_up(wid_editor_map_window,
                            wid_editor_map_receive_mouse_up);
        wid_set_on_key_down(wid_editor_map_window,
                            wid_editor_map_tile_key_down_event);
        wid_set_on_mouse_motion(wid_editor_map_window,
                                wid_editor_map_tile_mouse_motion);
        wid_set_on_key_up(wid_editor_map_window,
                          wid_editor_map_tile_key_up_event);

        wid_set_text_bot(wid_editor_map_window, true);
        wid_set_text_lhs(wid_editor_map_window, true);
        wid_set_tl_br_pct(wid_editor_map_window, tl, br);

        fsize sz = {0.0f, 0.0f};
        wid_set_tex_tl(wid_editor_map_window, sz);

        fsize sz2 = {1.32f, 1.0f};
        wid_set_tex_br(wid_editor_map_window, sz2);
    }

    {
        fpoint tl = {0.00f, 0.00f};
        fpoint br = {1.00f, 1.00f};

        wid_editor_map_grid_container =
                        wid_new_container(wid_editor_map_window,
                                          "wid editor grid container");

        wid_set_color(wid_editor_map_grid_container, WID_COLOR_TL, BLACK);
        wid_set_color(wid_editor_map_grid_container, WID_COLOR_BG, BLACK);
        wid_set_color(wid_editor_map_grid_container, WID_COLOR_BR, BLACK);

        wid_set_tl_br_pct(wid_editor_map_grid_container, tl, br);
        wid_set_tex(wid_editor_map_grid_container, 0, 0);

        wid_set_on_mouse_down(wid_editor_map_grid_container,
                              wid_editor_map_receive_mouse_down);
        wid_set_on_mouse_up(wid_editor_map_grid_container,
                            wid_editor_map_receive_mouse_up);
    }

    wid_editor_add_grid();

    wid_new_grid(wid_editor_map_grid_container,
                    TILES_MAP_WIDTH,
                    TILES_MAP_HEIGHT, tile_width, tile_height);

    level_ed = level_new(wid_editor_map_grid_container, 0);

    wid_editor_map_vert_scroll =
        wid_new_vert_scroll_bar(wid_editor_map_window,
                                wid_editor_map_grid_container);
    wid_editor_map_horiz_scroll =
        wid_new_horiz_scroll_bar(wid_editor_map_window,
                                 wid_editor_map_grid_container);

    wid_visible(wid_get_parent(wid_editor_map_vert_scroll), 1);
    wid_visible(wid_get_parent(wid_editor_map_horiz_scroll), 1);

    wid_visible(wid_editor_map_vert_scroll, 1);
    wid_visible(wid_editor_map_horiz_scroll, 1);

    wid_move_to_top(wid_editor_map_vert_scroll);

    if (!wid_editor_filename_and_title) {
        wid_editor_title_set("Unnamed level");
    }
}

void wid_editor_map_wid_destroy (void)
{
    if (wid_editor_map_window) {
        wid_destroy(&wid_editor_map_window);
    }

    if (level_ed) {
        level_destroy(&level_ed);
    }
}

void wid_editor_marshal (marshal_p ctx)
{
    marshal_level(ctx, level_ed);
}

boolean wid_editor_demarshal (demarshal_p ctx)
{
    boolean rc;

    rc = demarshal_level(ctx, level_ed);

    wid_update(wid_editor_map_window);

    wid_raise(wid_editor_filename_and_title);

    return (rc);
}
