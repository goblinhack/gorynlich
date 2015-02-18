/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>
#include <SDL_mouse.h>

#include "main.h"
#include "wid.h"
#include "tile.h"
#include "thing_tile.h"
#include "wid_editor.h"
#include "wid_editor_map.h"
#include "wid_editor_buttons.h"
#include "wid_textbox.h"
#include "map.h"
#include "level.h"
#include "thing.h"
#include "string_util.h"

widp wid_editor_map_window;
widp wid_editor_map_grid_container;

static widp wid_editor_map_horiz_scroll;
static widp wid_editor_map_vert_scroll;

static uint32_t tile_width;
static uint32_t tile_height;
levelp level_ed;
uint8_t server_level_is_being_loaded;
uint8_t wid_editor_got_line_start;

static uint8_t wid_editor_ignore_events (widp w)
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
                                            double x,
                                            double y,
                                            thingp t,
                                            tpp tp,
                                            itemp item,
                                            thing_statsp stats)
{
    tree_rootp thing_tiles;
    const char *tilename;
    uint8_t z_depth;
    widp existing;
    tilep tile;
    widp child;
    levelp level;

    level = (typeof(level)) wid_get_client_context(w);
    verify(level);

    if ((x < 0) || (y < 0) ||
        (x >= MAP_WIDTH) ||
        (y >= MAP_HEIGHT)) {
        ERR("overflow on placing tile at %f,%f", x,y); 
        return (0);
    }

    /*
     * Grow tl and br to fit the template thing. Use the first tile.
     */
    thing_tiles = tp_get_tiles(tp);
    if (!thing_tiles) {
        DIE("thing template [%s] has no tiles",
            tp_short_name(tp));
    }

    /*
     * Get the first anim tile.
     */
    thing_tilep thing_tile = (typeof(thing_tile))
                    tree_root_first(thing_tiles);

    /*
     * Find the real tile that corresponds to this name.
     */
    tilename = thing_tile_name(thing_tile);
    tile = tile_find(tilename);

    if (!tile) {
        DIE("tile name %s from thing %s not found",
            tilename,
            tp_short_name(tp));
    }

    /*
     * Find the midpoint of the tile.
     */
    x *= tile_width;
    y *= tile_height;

    x += tile_width / 2;
    y += tile_height / 2;

    fpoint tl = { x, y };
    fpoint br = { x, y };

    double base_tile_width =
            ((1.0f / ((double)TILES_SCREEN_EDITOR_WIDTH)) *
                (double)global_config.video_gl_width);

    double base_tile_height =
            ((1.0f / ((double)TILES_SCREEN_EDITOR_HEIGHT)) *
                (double)global_config.video_gl_height);

    double tw = tile_get_width(tile);
    double th = tile_get_height(tile);
    double scale_x = tw / TILE_WIDTH; 
    double scale_y = th / TILE_HEIGHT; 

    if (scale_x > 1) {
        base_tile_width *= scale_x;
        base_tile_height *= scale_y;
    }

    br.x += base_tile_width / 2.0;
    br.y += base_tile_height / 2.0;
    tl.x -= base_tile_width / 2.0;
    tl.y -= base_tile_height / 2.0;

    /*
     * Now the tile itself has a shadow that is 1/4 of the pixels.
     * The center is 24x24 and with shadow it is 32x32. We need to
     * stretch the tile so it overlaps so the inner 24x24 if seamless.
     */
    double tile_width = ((br.x - tl.x) / 
                         (double)TILE_PIX_WIDTH) * 
                            (double)TILE_PIX_WITH_SHADOW_WIDTH;

    double tile_height = ((br.y - tl.y) / 
                         (double)TILE_PIX_HEIGHT) * 
                            (double)TILE_PIX_WITH_SHADOW_HEIGHT;

    if (scale_x == 1) {
        tl.y -= tile_height / 4.0;
        br.x += tile_width / 4.0;
    }

    z_depth = tp_get_z_depth(tp);

    existing = wid_find_matching(wid_editor_map_grid_container,
                                 tl, br, z_depth);
    if (existing)  {
        child = existing;

        wid_set_thing_template(child, tp);

        wid_update(child);

        /*
         * Do the fixup at the end as it is slow.
         */
        if (server_level_is_being_loaded) {
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
                                  "editor map tile");

    wid_set_tl_br(child, tl, br);
    wid_set_text_lhs(child, true);
    wid_set_text_top(child, true);

    wid_set_mode(child, WID_MODE_NORMAL);
    wid_set_color(child, WID_COLOR_TEXT, WHITE);
    wid_set_color(child, WID_COLOR_TL, BLACK);
    wid_set_color(child, WID_COLOR_BG, BLACK);
    wid_set_color(child, WID_COLOR_BR, BLACK);

    wid_set_text_outline(child, true);
    wid_set_font(child, med_font);
    wid_set_no_shape(child);

    /*
     * "paint" the thing.
     */
    wid_set_thing_template(child, tp);

    /*
     * This adds it to the grid wid.
     */
    wid_update(child);

    /*
     * Do the fixup at the end as it is slow.
     */
    if (server_level_is_being_loaded) {
        return (child);
    }

    map_fixup(level);
    wid_raise(wid_editor_filename_and_title);
    wid_update(wid_editor_map_window);
    wid_update(child);

    return (child);
}

/*
 * Flood file.
 */
void wid_editor_map_thing_flood_fill_template (int32_t x, int32_t y,
                                               tpp tp)
{
    tree_rootp thing_tiles;
    const char *tilename;
    widp existing;
    tilep tile;

    uint32_t xin;
    uint32_t yin;

    if ((x < 0) || (y < 0) ||
        (x >= MAP_WIDTH) ||
        (y >= MAP_HEIGHT)) {
        return;
    }

    xin = x;
    yin = y;

    /*
     * Grow tl and br to fit the template thing. Use the first tile.
     */
    thing_tiles = tp_get_tiles(tp);
    if (!thing_tiles) {
        DIE("thing template [%s] has no tiles",
            tp_short_name(tp));
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
            tp_short_name(tp));
    }

    /*
     * Check to see there is nothing here blocking us.
     */
    uint8_t min_z;
    uint8_t z;

    /*
     * Bound certain things by others. e.g. flood fill ghosts limited by 
     * walls.
     */
    switch (tp_get_z_depth(tp)) {
        case MAP_DEPTH_EDITOR: 
            min_z = 0; 
            break;
        case MAP_DEPTH_FLOOR: 
            min_z = MAP_DEPTH_FLOOR; 
            break;
        case MAP_DEPTH_WALL: 
            min_z = MAP_DEPTH_WALL; 
            break;
        case MAP_DEPTH_MONST: 
            min_z = MAP_DEPTH_WALL; 
            break;
        case MAP_DEPTH_PLAYER: 
            min_z = MAP_DEPTH_WALL; 
            break;
        default:
            min_z = 0;
            break;

    }
    for (z = min_z; z < MAP_DEPTH; z++) {
        existing = wid_grid_find_first(
                        wid_editor_map_grid_container, xin, yin, z);
        while (existing) {
            if (wid_get_thing_template(existing)) {
                return;
            }

            existing = wid_grid_find_next(wid_editor_map_grid_container,
                                        existing, xin, yin, z);
        }
    }

    wid_editor_map_thing_replace_template(wid_editor_map_grid_container,
                                          xin, yin, 
                                          0, /* thing */
                                          tp,
                                          0 /* item */,
                                          0 /* stats */);

    wid_editor_map_thing_flood_fill_template(xin + 1, yin, tp);
    wid_editor_map_thing_flood_fill_template(xin - 1, yin, tp);
    wid_editor_map_thing_flood_fill_template(xin, yin + 1, tp);
    wid_editor_map_thing_flood_fill_template(xin, yin - 1, tp);
}

/*
 * Remove the top tile.
 */
static uint8_t wid_editor_map_thing_remove_template (
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

    existing = wid_grid_find_top(wid_editor_map_grid_container, tl, br);
    if (existing)  {
        tpp t = wid_get_thing_template(existing);
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
static uint8_t wid_editor_map_thing_replace (widp w,
                                             int32_t x,
                                             int32_t y,
                                             uint8_t scaled)
{
    tpp tp;
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

    tp = wid_get_thing_template(focus);
    if (!tp) {
        return (false);
    }

    (void) wid_editor_map_thing_replace_template(
                                            wid_editor_map_grid_container,
                                            x, y,
                                            0, /* thing */
                                            tp,
                                            0 /* item */,
                                            0 /* stats */);

    return (true);
}

/*
 * Mouse down etc...
 */
static uint8_t wid_editor_map_thing_flood_fill (widp w, int32_t x, int32_t y)
{
    tpp tp;
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

    tp = wid_get_thing_template(focus);
    if (!tp) {
        return (false);
    }

    (void) wid_editor_map_thing_flood_fill_template(x, y, tp);

    return (true);
}

/*
 * Mouse down etc...
 */
static uint8_t wid_editor_map_thing_remove (widp w,
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

static uint8_t wid_editor_map_thing_replace_wrap (widp w,
                                                  int32_t x,
                                                  int32_t y)
{
    static int32_t line_start_x;
    static int32_t line_start_y;

    if (wid_editor_mode_fill) {
        wid_editor_save_point();

        server_level_is_being_loaded = true;
        wid_editor_map_thing_flood_fill(w, x, y);
        server_level_is_being_loaded = false;

        map_fixup(level_ed);
        wid_raise(wid_editor_filename_and_title);
        wid_update(wid_editor_map_window);
        return (true);
    }

    if (wid_editor_mode_line) {
        fpoint offset;
        wid_get_offset(wid_editor_map_grid_container, &offset);

#if SDL_MAJOR_VERSION == 1 && SDL_MINOR_VERSION == 2 /* { */
        uint8_t *state = SDL_GetKeyState(0);

        uint8_t lshift = state[SDLK_LSHIFT] ? 1 : 0;
        uint8_t rshift = state[SDLK_RSHIFT] ? 1 : 0;
#else /* } { */
        const uint8_t *state = SDL_GetKeyboardState(0);

        uint8_t lshift = state[SDL_SCANCODE_LSHIFT] ? 1 : 0;
        uint8_t rshift = state[SDL_SCANCODE_RSHIFT] ? 1 : 0;
#endif /* } */
        uint8_t shift = lshift | rshift;

        if (shift) {
            wid_editor_got_line_start = true;
        }

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

        server_level_is_being_loaded = true;
        wid_editor_draw_line(w, line_start_x, line_start_y, x, y);
        server_level_is_being_loaded = false;
        
        map_fixup(level_ed);
        wid_raise(wid_editor_filename_and_title);
        wid_update(wid_editor_map_window);

        line_start_x = x;
        line_start_y = y;

        return (true);
    }

    wid_editor_save_point();

    return (wid_editor_map_thing_replace(w, x, y, false /* scaled */));
}

/*
 * Mouse down etc...
 */
static uint8_t wid_editor_map_receive_mouse_down (widp w,
                                                  int32_t x,
                                                  int32_t y,
                                                  uint32_t button)
{
    if (mouse_down & SDL_BUTTON_LEFT) {
        return (wid_editor_map_thing_replace_wrap(w, x, y));
    }

    if (mouse_down & SDL_BUTTON_RIGHT) {
        return (wid_editor_map_thing_remove(w, x, y));
    }

    return (false);
}

static uint8_t wid_editor_map_receive_joy_down (widp w,
                                                int32_t x,
                                                int32_t y)
{
    if (sdl_joy_button[SDL_JOY_BUTTON_A]) {
        return (wid_editor_map_thing_replace_wrap(w, x, y));
    }
    if (sdl_joy_button[SDL_JOY_BUTTON_B]) {
        if (!wid_editor_mode_draw) {
            wid_editor_draw();
            return (true);
        }

        wid_editor_hide();
        return (true);
    }
    if (sdl_joy_button[SDL_JOY_BUTTON_X]) {
        return (wid_editor_map_thing_remove(w, x, y));
    }
    if (sdl_joy_button[SDL_JOY_BUTTON_Y]) {
    }
    if (sdl_joy_button[SDL_JOY_BUTTON_TOP_LEFT]) {
    }
    if (sdl_joy_button[SDL_JOY_BUTTON_TOP_RIGHT]) {
    }
    if (sdl_joy_button[SDL_JOY_BUTTON_LEFT_STICK_DOWN]) {
    }
    if (sdl_joy_button[SDL_JOY_BUTTON_RIGHT_STICK_DOWN]) {
    }
    if (sdl_joy_button[SDL_JOY_BUTTON_START]) {
    }
    if (sdl_joy_button[SDL_JOY_BUTTON_XBOX]) {
    }
    if (sdl_joy_button[SDL_JOY_BUTTON_BACK]) {
    }

    static double scroll_x = 0;
    static double scroll_y = 0;

    if (sdl_joy_button[SDL_JOY_BUTTON_UP]) {
        scroll_y -= 0.1;
        if (scroll_y < 0.0) {
            scroll_y = 0.0;
        }
        wid_move_to_vert_pct(wid_editor_map_vert_scroll, scroll_y);
    }
    if (sdl_joy_button[SDL_JOY_BUTTON_DOWN]) {
        scroll_y += 0.1;
        if (scroll_y > 1.0) {
            scroll_y = 1.0;
        }
        wid_move_to_vert_pct(wid_editor_map_vert_scroll, scroll_y);
    }
    if (sdl_joy_button[SDL_JOY_BUTTON_LEFT]) {
        scroll_x -= 0.1;
        if (scroll_x < 0.0) {
            scroll_x = 0.0;
        }
        wid_move_to_horiz_pct(wid_editor_map_horiz_scroll, scroll_x);
    }
    if (sdl_joy_button[SDL_JOY_BUTTON_RIGHT]) {
        scroll_x += 0.1;
        if (scroll_x > 1.0) {
            scroll_x = 1.0;
        }
        wid_move_to_horiz_pct(wid_editor_map_horiz_scroll, scroll_x);
    }
    if (sdl_joy_button[SDL_JOY_BUTTON_LEFT_FIRE]) {
    }
    if (sdl_joy_button[SDL_JOY_BUTTON_RIGHT_FIRE]) {
    }

    return (true);
}

static uint8_t wid_editor_map_receive_mouse_motion (
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

#if 0
    /*
     * MACOS is bugged here and randomly leaves the mouse stuck down.
     */
    int mouse_x;
    int mouse_y;
    mouse_down = SDL_GetMouseState(&mouse_x, &mouse_y);

    if (mouse_down & SDL_BUTTON_LEFT) {
        return (wid_editor_map_thing_replace_wrap(w, x, y));
    }

    /*
     * MACOS seems bugged in SDL with this
     */
    if (mouse_down & SDL_BUTTON_RIGHT) {
        return (wid_editor_map_thing_remove(w, x, y));
    }
#endif

    /*
     * Block moving the window.
     */
    return (true);
}

/*
 * Mouse up etc...
 */
static uint8_t wid_editor_map_receive_mouse_up (widp w, int32_t x, int32_t y,
                                                uint32_t button)
{
    return (false);
}

static uint8_t wid_editor_map_tile_key_down_event (widp w,
                                                   const SDL_KEYSYM *key)
{
    int32_t x;
    int32_t y;
    uint8_t rc;

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

static uint8_t wid_editor_map_tile_mouse_motion (widp w,
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

    if (mouse_down & SDL_BUTTON_LEFT) {
        return (wid_editor_map_thing_replace(w, x, y, false /* scaled */));
    }

    if (mouse_down & SDL_BUTTON_RIGHT) {
        return (wid_editor_map_thing_remove(w, x, y));
    }

    /*
     * Block moving the window.
     */
    return (true);
}

static uint8_t wid_editor_map_tile_key_up_event (widp w,
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
        double x;
        double y;
        int32_t ix;
        int32_t iy;

        widp child;

        for (ix = 0; ix < MAP_WIDTH; ix++) {
            for (iy = 0; iy < MAP_HEIGHT; iy++) {

                /*
                 * Find the midpoint of the tile.
                 */
                x = (double)ix * tile_width;
                y = (double)iy * tile_height;

                x += tile_width / 2;
                y += tile_height / 2;

                fpoint tl = { x, y };
                fpoint br = { x, y };

                double base_tile_width =
                        ((1.0f / ((double)TILES_SCREEN_EDITOR_WIDTH)) *
                            (double)global_config.video_gl_width);

                double base_tile_height =
                        ((1.0f / ((double)TILES_SCREEN_EDITOR_HEIGHT)) *
                            (double)global_config.video_gl_height);

                br.x += base_tile_width / 2.0;
                br.y += base_tile_height / 2.0;
                tl.x -= base_tile_width / 2.0;
                tl.y -= base_tile_height / 2.0;

                child = wid_new_square_button(wid_editor_map_grid_container,
                                              "editor grid map tile");

                wid_set_color(child, WID_COLOR_BG, BLACK);
                color c = WHITE;
                c.a = 100;

                wid_set_color(child, WID_COLOR_TEXT, c);
                wid_set_font(child, vsmall_font);

                if ((ix < MAP_WIDTH) &&
                    (iy < MAP_HEIGHT)) {
                    wid_set_on_key_down(child,
                                        wid_editor_map_tile_key_down_event);
                    wid_set_on_key_up(child,
                                      wid_editor_map_tile_key_up_event);
                    wid_set_on_mouse_motion(child,
                                            wid_editor_map_tile_mouse_motion);
                } else {
                    tpp noentry;

                    noentry = tp_find("data/things/noentry");
                    if (!noentry) {
                        DIE("noentry icon not found");
                    }

                    wid_set_thing_template(child, noentry);
                }

                wid_set_color(child, WID_COLOR_TEXT, WHITE);
                wid_set_z_depth(child, MAP_DEPTH_EDITOR);
                wid_set_bevel(child, 1);

                wid_set_tl_br(child, tl, br);

                wid_set_mode(child, WID_MODE_NORMAL);
                c = RED;
                c.a = 0;
                wid_set_color(child, WID_COLOR_BG, c);

                if ((!(ix % TILES_SCREEN_WIDTH)) || 
                    (!(iy % TILES_SCREEN_HEIGHT)) || 
                    (ix == MAP_WIDTH - 1) || (iy == MAP_HEIGHT - 1)) {
                    char tmp[20];
                    sprintf(tmp, "%u,%u",ix,iy);
                    wid_set_text(child, tmp);
                    c = GREEN;
                    c.a = 100;
                    wid_set_color(child, WID_COLOR_TL, c);
                    wid_set_color(child, WID_COLOR_BR, c);
                } else {
                    c = WHITE;
                    c.a = 100;
                    wid_set_color(child, WID_COLOR_TL, c);
                    wid_set_color(child, WID_COLOR_BR, c);
                }

                wid_set_mode(child, WID_MODE_OVER);
                c = RED;
                c.a = 0;
                wid_set_color(child, WID_COLOR_BG, c);

                c = RED;
                c.a = 100;
                wid_set_color(child, WID_COLOR_TL, c);
                wid_set_color(child, WID_COLOR_BR, c);

                wid_set_mode(child, WID_MODE_NORMAL);

                wid_update(child);
            }
        }
    }
    wid_update(wid_editor_map_grid_container);
}

static void wid_editor_title_set (const char *title)
{
    level_set_title(level_ed, title);

    wid_destroy(&wid_editor_filename_and_title);

    if (level_get_title(level_ed) &&
        strcasecmp(level_get_title(level_ed), "(null)")) {

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
        wid_set_on_joy_button(wid_editor_map_window,
                              wid_editor_map_receive_joy_down);
        wid_set_on_mouse_motion(wid_editor_map_window,
                                wid_editor_map_receive_mouse_motion);
        wid_set_on_mouse_up(wid_editor_map_window,
                            wid_editor_map_receive_mouse_up);
        wid_set_on_key_down(wid_editor_map_window,
                            wid_editor_map_tile_key_down_event);
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

    double base_tile_width =
            ((1.0f / ((double)TILES_SCREEN_EDITOR_WIDTH)) *
                (double)global_config.video_gl_width);

    double base_tile_height =
            ((1.0f / ((double)TILES_SCREEN_EDITOR_HEIGHT)) *
                (double)global_config.video_gl_height);

    fpoint tl = { 0, 0 };
    fpoint br = { 0, 0 };

    br.x += base_tile_width;
    br.y += base_tile_height;

    tile_width = br.x - tl.x;
    tile_height = br.y - tl.y;
    tile_width = br.x - tl.x;
    tile_height = br.y - tl.y;

    if (!tile_width) {
        tile_width = TILE_WIDTH;
    }

    if (!tile_height) {
        tile_height = TILE_HEIGHT;
    }

    wid_editor_add_grid();

    wid_new_grid(wid_editor_map_grid_container,
                    MAP_WIDTH,
                    MAP_HEIGHT, tile_width, tile_height);

    level_ed = level_new(wid_editor_map_grid_container, 0, 
                         true, /* is_editor */
                         false /* on_server */);

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
        level_destroy(&level_ed, false /* keep players */);
    }
}

void wid_editor_marshal (marshal_p ctx)
{
    marshal_level(ctx, level_ed);
}

uint8_t wid_editor_demarshal (demarshal_p ctx)
{
    uint8_t rc;

    rc = demarshal_level(ctx, level_ed);

    wid_update(wid_editor_map_window);

    wid_raise(wid_editor_filename_and_title);

    return (rc);
}
