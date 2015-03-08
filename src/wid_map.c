/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "dir.h"
#include "color.h"
#include "string_ext.h"
#include "ttf.h"
#include "wid_map.h"
#include "time_util.h"
#include "timer.h"
#include "level.h"
#include "math_util.h"
#include "thing_template.h"
#include "thing_tile.h"
#include "wid_editor.h"
#include "tile.h"

int wid_map_visible;

static widp wid_map_window;
static wid_map_ctx *wid_map_window_ctx;
static widp wid_map_background;
static void wid_map_destroy(widp w);
static void wid_map_set_focus(wid_map_ctx *ctx, int focusx, int focusy);
static int saved_focus_x = -1;
static int saved_focus_y = -1;

static void wid_map_update_buttons (widp w)
{
    wid_map_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    double width = 1.0 / (double)(LEVELS_DOWN + 1);
    double height = 1.0 / (double)(LEVELS_ACROSS + 1);

    int x, y;

    ctx->b = 0;

    for (x = 0; x < LEVELS_DOWN; x++) {
    for (y = 0; y < LEVELS_ACROSS; y++) {

        widp b = ctx->buttons[y][x];
        verify(b);

        fpoint tl;
        fpoint br;
        fontp font;
        color c;

        tl.x = (double) x * width;
        tl.y = (double) y * height;
        tl.x += width / 2.0;
        tl.y += height / 2.0;

        br.x = tl.x;
        br.y = tl.y;
        br.x += width;
        br.y += height;

        font = small_font;

        double zoom = 0.005;
        if ((x == ctx->focusx) && (y == ctx->focusy)) {
            tl.x -= zoom;
            tl.y -= zoom;
            br.x += zoom * 2.0;
            br.y += zoom * 2.0;
            c = GREEN;

            ctx->b = b;

            wid_raise(b);
        } else {
            tl.x += zoom;
            tl.y += zoom;
            br.x -= zoom * 2.0;
            br.y -= zoom * 2.0;

            wid_lower(b);

            c = GRAY70;
        }

        wid_set_tl_br_pct(b, tl, br);
        wid_set_color(b, WID_COLOR_TEXT, c);
        wid_set_font(b, font);

        if (ctx->levels[y][x].level) {
            color c = GREEN;
            c.a = 255;
            wid_set_color(b, WID_COLOR_BG, c);
        } else {
            color c = GRAY;
            c.a = 50;
            wid_set_color(b, WID_COLOR_BG, c);
        }
    }
    }

    wid_update(w);

    /*
     * Zoom buttons in
     */
    if (ctx->is_new) {
        ctx->is_new = false;

        for (x = 0; x < LEVELS_DOWN; x++) {
            for (y = 0; y < LEVELS_ACROSS; y++) {

                widp b = ctx->buttons[y][x];

                fpoint tl;
                fpoint br;

                wid_get_tl_br(b, &tl, &br);

                double x = gauss(0.0, 2.0);
                double y = gauss(0.0, 2.0);

                wid_move_to_pct_centered(b, x, y);
                wid_move_to_centered_in(b, tl.x, tl.y, 500);
            }
        }
    }
}

static uint8_t wid_map_mouse_event (widp w,
                                         int focusx, int focusy)
{
    return (true);
}

static uint8_t wid_map_button_mouse_event (widp w,
                                                int32_t x, int32_t y,
                                                uint32_t button)
{
    int focus = (typeof(focus)) (uintptr_t) wid_get_client_context2(w);
    int focusx = (focus & 0xff);
    int focusy = (focus & 0xff00) >> 8;

    return (wid_map_mouse_event(w, focusx, focusy));
}

static void wid_map_focus_right (wid_map_ctx *ctx)
{
    ctx->focusx++;
    if (ctx->focusx > LEVELS_DOWN - 1) {
        ctx->focusx = 0;
    }

    wid_map_update_buttons(ctx->w);
}

static void wid_map_focus_left (wid_map_ctx *ctx)
{
    ctx->focusx--;
    if (ctx->focusx < 0) {
        ctx->focusx = LEVELS_DOWN - 1;
    }

    wid_map_update_buttons(ctx->w);
}

static void wid_map_focus_down (wid_map_ctx *ctx)
{
    ctx->focusy++;
    if (ctx->focusy > LEVELS_ACROSS - 1) {
        ctx->focusy = 0;
    }

    wid_map_update_buttons(ctx->w);
}

static void wid_map_focus_up (wid_map_ctx *ctx)
{
    ctx->focusy--;
    if (ctx->focusy < 0) {
        ctx->focusy = LEVELS_ACROSS - 1;
    }

    wid_map_update_buttons(ctx->w);
}

static void wid_map_last_focus (wid_map_ctx *ctx)
{
    ctx->focusx = LEVELS_DOWN - 1;
    ctx->focusy = LEVELS_ACROSS - 1;

    wid_map_update_buttons(ctx->w);
}

static void wid_map_first_focus (wid_map_ctx *ctx)
{
    ctx->focusx = 0;
    ctx->focusy = 0;

    wid_map_update_buttons(ctx->w);
}

static void wid_map_set_focus (wid_map_ctx *ctx, 
                                    int focusx, int focusy)
{
    ctx->focusx = focusx;
    ctx->focusy = focusy;

    wid_map_update_buttons(ctx->w);
}

static uint8_t wid_map_parent_key_down (widp w, 
                                             const SDL_KEYSYM *key)
{
    wid_map_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    switch (key->sym) {
        case '`':
            return (false);

        case SDLK_ESCAPE:
            (ctx->cancelled)(ctx->w);
            return (true);

        case SDLK_RETURN: {
            (ctx->selected)(ctx->w);
            return (true);

        case SDLK_LEFT:
            wid_map_focus_left(ctx);
            break;

        case SDLK_RIGHT:
            wid_map_focus_right(ctx);
            break;

        case SDLK_UP:
            wid_map_focus_up(ctx);
            break;

        case SDLK_DOWN:
            wid_map_focus_down(ctx);
            break;

        case SDLK_HOME:
            wid_map_first_focus(ctx);
            break;

        case SDLK_END:
            wid_map_last_focus(ctx);
            break;

        default:
            return (true);
        }
    }

    return (true);
}

static uint8_t wid_map_parent_joy_button (widp w, 
                                               int32_t x,
                                               int32_t y)
{
    wid_map_ctx *ctx = wid_get_client_context(w);
    verify(ctx);
    int ret = false;

    if (sdl_joy_buttons[SDL_JOY_BUTTON_A]) {
        (ctx->selected)(ctx->w);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_B]) {
        (ctx->cancelled)(ctx->w);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_X]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_Y]) {
        (ctx->selected)(ctx->w);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_TOP_LEFT]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_TOP_RIGHT]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_LEFT_STICK_DOWN]) {
        (ctx->selected)(ctx->w);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_RIGHT_STICK_DOWN]) {
        (ctx->selected)(ctx->w);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_START]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_XBOX]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_BACK]) {
        (ctx->cancelled)(ctx->w);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_UP]) {
        wid_map_focus_up(ctx);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_DOWN]) {
        wid_map_focus_down(ctx);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_LEFT]) {
        wid_map_focus_left(ctx);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_RIGHT]) {
        wid_map_focus_right(ctx);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_LEFT_FIRE]) {
        (ctx->selected)(ctx->w);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_RIGHT_FIRE]) {
        (ctx->selected)(ctx->w);
        ret = true;
    }

    wid_map_update_buttons(ctx->w);
    if (ctx->b) {
        wid_mouse_warp(ctx->b);
    }

    return (true);
}

static uint8_t wid_map_button_key_event (widp w, const SDL_KEYSYM *key)
{
    wid_map_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    switch (key->sym) {
        case '`':
            return (false);

        case SDLK_ESCAPE:
            (ctx->cancelled)(ctx->w);
            return (true);

        case SDLK_RETURN:
            (ctx->selected)(ctx->w);
            return (true);

        case SDLK_DOWN:
        case SDLK_HOME:
        case SDLK_END:
        case SDLK_BACKSPACE:
        case SDLK_DELETE:
        case SDLK_LEFT:
        case SDLK_RIGHT:
        case SDLK_UP:
            break;

        default:
            return (true);
    }

    return (false);
}

static uint8_t wid_map_button_joy_down_event (widp w, 
                                                     int32_t x, int32_t y)
{
    wid_map_ctx *ctx = wid_get_client_context(w);
    verify(ctx);
    int ret = false;

    if (sdl_joy_buttons[SDL_JOY_BUTTON_A]) {
        ret = wid_map_mouse_event(w, ctx->focusx, ctx->focusy);
    }

    if (sdl_joy_buttons[SDL_JOY_BUTTON_B]) {
        (ctx->selected)(ctx->w);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_X]) {
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_Y]) {
        (ctx->selected)(ctx->w);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_TOP_LEFT]) {
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_TOP_RIGHT]) {
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_LEFT_STICK_DOWN]) {
        wid_map_mouse_event(w, ctx->focusx, ctx->focusy);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_RIGHT_STICK_DOWN]) {
        wid_map_mouse_event(w, ctx->focusx, ctx->focusy);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_START]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_XBOX]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_BACK]) {
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
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_RIGHT_FIRE]) {
        wid_map_mouse_event(w, ctx->focusx, ctx->focusy);
        ret = true;
    }

    wid_map_update_buttons(ctx->w);
    if (ctx->b) {
        wid_mouse_warp(ctx->b);
    }

    return (ret);
}

static void wid_map_mouse_over (widp w,
                                int32_t relx, int32_t rely,
                                int32_t wheelx, int32_t wheely)
{
    wid_map_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    if (!relx && !rely && !wheelx && !wheely) {
        return;
    }

    /*
     * If we recreate the map with a fixed focus we will be told about
     * a mouse over event immediately which may not be over the focus item
     * and will cause us to move. Annoying.
     */
    if (time_get_time_ms() - ctx->created < 500) {
        return;
    }

    int focus = (typeof(focus)) (uintptr_t) wid_get_client_context2(w);
    int focusx = (focus & 0xff);
    int focusy = (focus & 0xff00) >> 8;

    wid_map_set_focus(ctx, focusx, focusy);
}

static void wid_map_destroy (widp w)
{
    wid_map_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    wid_set_client_context(w, 0);

    wid_map_visible = false;

    if (wid_map_background) {
        wid_destroy(&wid_map_background);
    }

    int x, y;

    for (x = 0; x < LEVELS_DOWN; x++) {
        for (y = 0; y < LEVELS_ACROSS; y++) {

            levelp l = ctx->levels[y][x].level;
            if (!l) {
                continue;
            }

            level_destroy(&l, false /* keep players */);
            ctx->levels[y][x].level = 0;
        }
    }

    saved_focus_x = ctx->focusx;
    saved_focus_y = ctx->focusy;

    myfree(ctx);
    wid_map_window = 0;
    wid_map_window_ctx = 0;
}

static void wid_map_tick (widp w)
{
    wid_map_ctx *ctx = wid_get_client_context(w);
    if (!ctx) {
        return;
    }

    verify(ctx);

    static int val;
    static int delta = 1;
    static int step = 2;

    val += delta * step;

    if (val > 255) {
        val = 255;
        delta = -1;
    }

    if (val < 200) {
        val = 200;
        delta = 1;
    }

    int x, y;

    for (x = 0; x < LEVELS_DOWN; x++) {
    for (y = 0; y < LEVELS_ACROSS; y++) {

        if ((x != ctx->focusx) || (y != ctx->focusy)) {
            continue;
        }

        widp b = ctx->buttons[y][x];
        verify(b);

        color c;
        c = GREEN;
        c.g = val;

        /*
         * Make sure the other widgets look plain in all modes.
         */
        int mode;
        for (mode = WID_MODE_NORMAL; mode < WID_MODE_LAST; mode++) {
            wid_set_mode(b, mode);
            wid_set_color(b, WID_COLOR_TEXT, c);
            wid_set_text_outline(b, true);
        }

        wid_set_mode(w, WID_MODE_NORMAL);
    }
    }
}

static void wid_map_destroy_begin (widp w)
{
    wid_map_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    /*
     * Zoom buttons in
     */
    int x, y;

    for (x = 0; x < LEVELS_DOWN; x++) {
        for (y = 0; y < LEVELS_ACROSS; y++) {

            widp b = ctx->buttons[y][x];
            fpoint tl;
            fpoint br;

            wid_get_tl_br(b, &tl, &br);
            double x = gauss(0.0, 2.0);
            double y = gauss(0.0, 2.0);

            wid_move_to_pct_centered_in(b, x, y, 500);
        }
    }
}

static void wid_map_cell_selected (widp w)
{
    wid_map_ctx *ctx = wid_map_window_ctx;
    verify(ctx);
    verify(ctx->w);

    /*
     * If we recreate the map with a fixed focus we will be told about
     * a mouse over event immediately which may not be over the focus item
     * and will cause us to move. Annoying.
     */
    if (time_get_time_ms() - ctx->created < 500) {
        return;
    }
CON("selected");

    level_pos_t level_pos;

    level_pos.x = ctx->focusx;
    level_pos.y = ctx->focusy;

    if (level_pos.x == -1) {
        return;
    }

    if (level_pos.y == -1) {
        return;
    }

    wid_editor_visible(level_pos);

    wid_destroy(&wid_map_window);
}

static void wid_map_cell_cancelled (widp w)
{
}

static void wid_map_bg_create (void)
{
    widp wid;

    if (wid_map_background) {
        return;
    }

    {
        wid = wid_map_background = wid_new_window("bg");

        float f = (1024.0 / 680.0);

        fpoint tl = { 0.0, 0.0 };
        fpoint br = { 1.0, f };

        wid_set_tl_br_pct(wid, tl, br);

        wid_set_tex(wid, 0, "title5");

        wid_lower(wid);

        color c;
        c = WHITE;
        wid_set_mode(wid, WID_MODE_NORMAL);
        wid_set_color(wid, WID_COLOR_TL, c);
        wid_set_color(wid, WID_COLOR_BR, c);
        wid_set_color(wid, WID_COLOR_BG, c);

        wid_update(wid);
    }
}

static void wid_map_preview (widp w)
{
    wid_map_ctx *ctx = wid_map_window_ctx;
    verify(ctx);
    verify(ctx->w);

    wid_map_level *map = &ctx->levels[ctx->focusy][ctx->focusx];
    if (!map) {
        return;
    }

    int x, y, z;

    blit_init();

    for (x = 0; x < MAP_WIDTH; x++) 
    for (y = 0; y < MAP_HEIGHT; y++) 
    for (z = 0; z < MAP_DEPTH; z++) {
        tpp tp = map->tiles[x][y][z];
        if (!tp) {
            continue;
        }

        thing_tilep thing_tile;
        tree_rootp tiles;

        tiles = tp_get_tiles(tp);
        if (!tiles) {
            return;
        }

        thing_tile = thing_tile_first(tiles);
        if (!thing_tile) {
            continue;
        }

        fpoint tl;
        fpoint br;

        double dx = 0.010;
        double dy = 0.010;

        tl.x = ((double)x) * dx;
        br.x = ((double)x+1.5) * dx;
        tl.y = ((double)y) * dy;
        br.y = ((double)y+1.5) * dy;

        const char *tilename = thing_tile_name(thing_tile);
        if (!tilename) {
            ERR("cannot find tile %s", tilename);
            continue;
        }

        tilep tile = tile_find(tilename);
        if (!tile) {
            ERR("cannot find tilep for tile %s", tilename);
        }

        if (ctx->focusx > MAP_WIDTH / 2) {
            tl.x -= dx * (double)MAP_WIDTH;
            br.x -= dx * (double)MAP_WIDTH;
        }

        if (ctx->focusx > MAP_HEIGHT / 2) {
            tl.y -= dy * (double)MAP_HEIGHT;
            br.y -= dy * (double)MAP_HEIGHT;
        }

        tl.x *= (double) global_config.video_gl_width;
        tl.y *= (double) global_config.video_gl_height;
        br.x *= (double) global_config.video_gl_width;
        br.y *= (double) global_config.video_gl_height;

        tl.x += mouse_x;
        tl.y += mouse_y;
        br.x += mouse_x;
        br.y += mouse_y;

        glcolor(WHITE);
        tile_blit_fat(tile, 0, tl, br);
    }

    blit_flush();
}

/*
 * Replace or place a tile.
 */
widp wid_editor_level_map_thing_replace_template (widp w,
                                                  double x,
                                                  double y,
                                                  thingp t,
                                                  tpp tp,
                                                  itemp item,
                                                  thing_statsp stats)
{
    /*
     * Can't use w as it has the level set as its context now.
     */
    wid_map_ctx *ctx = wid_map_window_ctx;
    verify(ctx);
    verify(ctx->w);

    wid_map_level *map = &ctx->levels[ctx->loading_y][ctx->loading_x];

    int ix = (int)x;
    int iy = (int)y;

    if ((ix >= MAP_WIDTH) || (iy >= MAP_HEIGHT) || (ix < 0) || (iy < 0)) {
        DIE("overflow in reading position (%f,%f) -> (%d,%d) in level %u.%u, "
            "map bounds (%d,%d) -> (%d,%d)", 
            x, y, ix, iy, ctx->loading_x, ctx->loading_y,
            0, 0, MAP_DEPTH, MAP_HEIGHT);
    }

    int z = tp_get_z_depth(tp);
    map->tiles[ix][iy][z] = tp;

    return (0);
}

static void wid_map_load_levels (wid_map_ctx *ctx)
{
    tree_file_node *n;
    tree_root *d;

    d = dirlist(LEVELS_PATH,
                0 /* context->include_suffix */,
                0 /* context->exclude_suffix */,
                0 /* context->include_dirs */,
                false /* include_ramdisk */);

    { TREE_WALK(d, n) {

        const char *name = n->tree.key;

        /*
         * Ignore dot files
         */
        if (name[0] == '.') {
            continue;
        }

        level_pos_t level_pos;
        int x, y;

        if (sscanf(name, "%d.%d", &x, &y) != 2) {
            WARN("bad format in level name %s, expecting a,b format", name);
            continue;
        }

        level_pos.x = x;
        level_pos.y = y;

        ctx->loading_x = x;
        ctx->loading_y = y;

        levelp l = level_load(level_pos, 
                              ctx->w,
                              false, /* is_editor */
                              true, /* is_map_editor */
                              false /* on_server */);

        ctx->levels[y][x].level = l;

        widp b = ctx->buttons[y][x];

        wid_set_tooltip(b, level_get_title(l), med_font);
        wid_set_font(b, vsmall_font);
    } }

    dirlist_free(&d);
}

widp wid_map (void)
{
    const char *title = "Choose epic level";
    wid_map_event_t selected = wid_map_cell_selected;
    wid_map_event_t cancelled = wid_map_cell_cancelled;

    wid_map_visible = true;

    /*
     * Create a context to hold button info so we can update it when the focus 
     * changes
     */
    wid_map_ctx *ctx = myzalloc(sizeof(*ctx), "wid map");
    wid_map_window_ctx = ctx;

    ctx->focusx = -1;
    ctx->focusx = -1;
    ctx->cancelled = cancelled;
    ctx->selected = selected;

    widp window;
    ctx->w = wid_map_window = window = wid_new_window("wid map");
    wid_set_client_context(window, ctx);
    ctx->is_new = true;

    /*
     * Main window
     */
    {
        fpoint tl = { 0.0, 0.0};
        fpoint br = { 1.0, 1.0};

        wid_set_tl_br_pct(window, tl, br);

        color c = BLACK;
        c.a = 100;
        wid_set_color(window, WID_COLOR_BG, c);

        wid_set_on_destroy_begin(window, wid_map_destroy_begin);
        wid_set_on_key_down(window, wid_map_parent_key_down);
        wid_set_on_joy_down(window, wid_map_parent_joy_button);
        wid_set_on_destroy(window, wid_map_destroy);
    }

    /*
     * Create the title
     */
    {
        widp w = wid_new_square_button(window, "wid map title");
        wid_set_no_shape(w);

        fpoint tl = { 0.0, 0.0};
        fpoint br = { 1.0, 0.15};

        wid_set_tl_br_pct(w, tl, br);
        wid_set_text(w, title);
        wid_set_text_outline(w, true);
        wid_set_font(w, vvlarge_font);

        wid_set_color(w, WID_COLOR_TEXT, YELLOW);
    }

    /*
     * Create the button container
     */
    {
        widp button_container = wid_new_square_button(window, 
                                                      "wid map buttons");
        wid_set_no_shape(button_container);
        wid_set_on_tick(button_container, wid_map_tick);

        fpoint tl = { 0.0, 0.1};
        fpoint br = { 1.0, 1.0};

        wid_set_tl_br_pct(button_container, tl, br);
        wid_set_client_context(button_container, ctx);

        /*
         * Create the buttons
         */
        int x, y;

        for (x = 0; x < LEVELS_DOWN; x++) {
        for (y = 0; y < LEVELS_ACROSS; y++) {
            widp b = wid_new_square_button(button_container,
                                           "wid map button");
            ctx->buttons[y][x] = b;
            ctx->levels[y][x].x = x;
            ctx->levels[y][x].y = y;

            wid_set_on_mouse_over_begin(b, wid_map_mouse_over);
            wid_set_on_key_down(b, wid_map_button_key_event);
            wid_set_on_joy_down(b, wid_map_button_joy_down_event);
            wid_set_on_mouse_down(b, wid_map_button_mouse_event);

            wid_set_color(b, WID_COLOR_BG, GRAY20);
            wid_set_color(b, WID_COLOR_TL, GRAY60);
            wid_set_color(b, WID_COLOR_BR, GRAY10);
            wid_set_color(b, WID_COLOR_TEXT, GREEN);

            wid_set_mode(b, WID_MODE_OVER);

            wid_set_color(b, WID_COLOR_BG, GRAY20);
            wid_set_color(b, WID_COLOR_TL, GRAY70);
            wid_set_color(b, WID_COLOR_BR, GRAY10);
            wid_set_color(b, WID_COLOR_TEXT, GREEN);

            wid_set_mode(b, WID_MODE_NORMAL);

            wid_set_client_context(b, ctx);
            int focus = (y << 8) | x;
            wid_set_client_context2(b, (void*) (uintptr_t) focus);
        }
        }
    }

    if (saved_focus_x == -1) {
        ctx->focusx = LEVELS_ACROSS / 2;
    } else {
        ctx->focusx = saved_focus_x;
    }

    if (saved_focus_y == -1) {
        ctx->focusy = LEVELS_DOWN / 2;
    } else {
        ctx->focusy = saved_focus_y;
    }

    /*
     * Load all levels
     */
    wid_map_load_levels(ctx);

    /*
     * Repair the context so it is not pointing at the last level loaded.
     */
    wid_set_client_context(window, ctx);

    wid_map_update_buttons(window);
    wid_update(window);
    wid_raise(window);
    wid_map_update_buttons(window);
    wid_update(window);
    wid_raise(window);
    wid_map_bg_create();

    wid_set_on_display(window, wid_map_preview);

    ctx->created = time_get_time_ms();

    return (window);
}
