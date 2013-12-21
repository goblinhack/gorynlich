/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>
#include <errno.h>

#include "main.h"
#include "wid.h"
#include "tex.h"
#include "color.h"
#include "tile.h"
#include "thing_tile.h"
#include "thing_template.h"
#include "tree.h"
#include "string.h"
#include "marshal.h"
#include "wid_popup.h"
#include "wid_textbox.h"
#include "wid_game_map.h"
#include "wid_tooltip.h"
#include "wid_intro.h"
#include "wid_editor.h"
#include "wid_editor_map.h"
#include "gl.h"
#include "level.h"
#include "player.h"
#include "thing.h"
#include "item.h"
#include "map.h"
#include "wid_button.h"
#include "sdl.h"
#include "config.h"
#include "level.h"
#include "level_private.h"

levelp level_game;
widp wid_game_map_window;
widp wid_game_map_grid_container;
widp wid_scoreline_container_top;
widp wid_scoreline_container_bot;
widp wid_score;
widp wid_health;
widp wid_score_title;
widp wid_health_title;
widp wid_name_title;
widp wid_title;
widp wid_level;

static const char *wid_game_selected_item_name;
static widp wid_game_last_selected_tile;
static boolean wid_game_init_done;

static float tile_width_pct = 1.0f / TILES_SCREEN_WIDTH;
static float tile_height_pct = 1.0f / TILES_SCREEN_HEIGHT;
uint32_t tile_width;
uint32_t tile_height;

static void wid_game_selected_item_name_reset(void);

static void wid_game_map_set_thing_template (widp w, thing_templatep t)
{
    wid_set_thing_template(w, t);

    if (thing_template_is_effect_pulse(t)) {
        wid_scaling_to_pct_in(w, 1.0, 1.2, 500, 9999999);
    }

    if (thing_template_is_effect_sway(t)) {
        wid_rotate_to_pct_in(w, -2, 2, 500, 9999999);
    }
}

static boolean wid_game_map_tile_receive_mouse_down(widp w,
                                                    int32_t x, int32_t y,
                                                    uint32_t button);

static boolean wid_game_map_tile_receive_mouse_up(widp w,
                                                  int32_t x, int32_t y,
                                                  uint32_t button);
boolean wid_game_init (void)
{
    wid_game_init_done = true;

    return (true);
}

void wid_game_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (wid_game_init_done) {
        wid_game_init_done = false;

        wid_game_map_wid_destroy();
    }
}

void wid_game_hide (void)
{
    if (wid_game_map_window) {
        wid_hide(wid_game_map_window, wid_swipe_delay);

        wid_detach_from_grid(wid_game_map_grid_container);

        wid_destroy_grid(wid_game_map_grid_container);

        /*
         * Reset the tile that we had last highlighted.
         */
        wid_game_selected_item_name_reset();
    }

    wid_intro_visible();
}

void wid_game_visible (void)
{
    if (wid_game_map_window) {
        wid_visible(wid_game_map_window, 0);
    } else {
        wid_game_map_wid_create();
    }

    wid_raise(wid_game_map_window);
}

/*
 * Mouse down etc...
 */
static boolean wid_game_map_receive_mouse_down (widp w,
                                                int32_t x,
                                                int32_t y,
                                                uint32_t button)
{
    if (SDL_BUTTON(SDL_BUTTON_LEFT) & SDL_GetMouseState(0, 0)) {
    }

    if (SDL_BUTTON(SDL_BUTTON_RIGHT) & SDL_GetMouseState(0, 0)) {
    }

    return (false);
}

static boolean wid_game_map_receive_mouse_up (widp w,
                                              int32_t x, int32_t y,
                                              uint32_t button)
{
    return (false);
}

static boolean wid_game_map_tile_receive_mouse_up (widp w,
                                                   int32_t x, int32_t y,
                                                   uint32_t button)
{
    return (false);
}

static boolean wid_game_map_tile_can_place_here (int32_t x, int32_t y)
{
    thing_templatep thing_template;
    boolean can_place_here;
    widp t;

    if (!wid_game_selected_item_name) {
        return (false);
    }

    can_place_here = false;

    /*
     * Look for a floor tile where we can place stuff.
     */
    t = wid_grid_find_first(wid_game_map_grid_container, x, y);
    while (t) {
        thing_template = wid_get_thing_template(t);
        if (thing_template) {
            if (thing_template_is_floor(thing_template)) {
                can_place_here = true;
            }
        }

        t = wid_grid_find_next(wid_game_map_grid_container, t, x, y);
    }

    /*
     * Check for exclusions.
     */
    t = wid_grid_find_first(wid_game_map_grid_container, x, y);
    while (t) {
        thing_template = wid_get_thing_template(t);
        if (thing_template) {
            if (thing_template_is_wall(thing_template)) {
                can_place_here = false;
            }

            if (thing_template_is_pipe(thing_template)) {
                levelp level;

                level = thing_level(player);
                if (level->end_pipe[x][y] == ' ') {
                    can_place_here = false;
                }
            }

            if (thing_template_is_food(thing_template)) {
                /*
                 * Ok to place food on food.
                 *
                can_place_here = false;
                 */
            }
        }

        t = wid_grid_find_next(wid_game_map_grid_container, t, x, y);
    }

    return (can_place_here);
}

static boolean wid_game_map_tile_receive_mouse_down (widp w,
                                                     int32_t x, int32_t y,
                                                     uint32_t button)
{
    thing_templatep thing_template;
    boolean can_place_here;
    const char *item_name;
    tree_rootp items;
    fpoint offset;
    thingp thing;
    itemp item;

    if (!wid_game_selected_item_name) {
        wid_tooltip_transient("Nothing left to place", 0);
        return (true);
    }

    /*
     * Map the mouse to tile coordinate.
     */
    wid_get_offset(wid_game_map_grid_container, &offset);

    x += -offset.x;
    y += -offset.y;

    x /= tile_width;
    y /= tile_height;

    can_place_here = wid_game_map_tile_can_place_here(x, y);
    if (!can_place_here) {
        /*
         * Too noisy
         *
        wid_tooltip_transient("You cannot place any items here.", 0);
         */
        return (true);
    }

    item_name = wid_game_selected_item_name;
    thing_template = thing_template_find(item_name);
    if (!thing_template) {
        char *tmp = dynprintf("item [%s] has no thing template (1)",
                              item_name);
        (void) wid_popup_error(tmp);
        myfree(tmp);
        return (false);
    }

    items = thing_carried_items(player);
    if (!items) {
        wid_tooltip_transient("Nothing left to place. Sorry!", 0);

        /*
         * So we select a new thing.
         */
        wid_game_map_item_update(level_game);

        return (false);
    }

    item = item_find_template(items, thing_template);
    if (!item) {
        char *tmp = dynprintf("item [%s] is not being carried",
                              thing_template_name(thing_template));
        (void) wid_popup_error(tmp);
        myfree(tmp);
        return (false);
    }

    thing = item_thing(item);

    /*
     * If this is something we can use, then do not place a star for example,
     * but use it as a powerup.
     */
    if (thing_item_use(player, thing)) {
        /*
         * Do not place on map. Destroyed.
         */
    } else {
        /*
         * Place on map.
         */
        widp w = wid_game_map_replace_tile(wid_game_map_grid_container,
                                  x, y, 0, thing_template);

        thing_destroy(thing, "copy placed on map");

        thingp t = wid_get_thing(w);

        /*
         * Pull the strings! Pull the strings!
         */
        if (thing_is_bomb(t)) {
            thing_set_is_open(t, true);
        }
    }

    wid_game_map_item_update(level_game);

    return (true);
}

static void
wid_game_map_tile_receive_on_mouse_over_begin (widp w)
{
    boolean can_place_here;
    fpoint offset;
    int32_t x;
    int32_t y;

    /*
     * Map the mouse to tile coordinate.
     */
    (void) SDL_GetMouseState(&x, &y);

    x *= global_config.xscale;
    y *= global_config.yscale;

    wid_get_offset(wid_game_map_grid_container, &offset);

    x += -offset.x;
    y += -offset.y;

    x /= tile_width;
    y /= tile_height;

    can_place_here = wid_game_map_tile_can_place_here(x, y);

    wid_effect_pulses(w);

    if (can_place_here) {
        wid_set_square_outline(w);
        wid_set_bevel(w, 10);
        wid_set_tile(w, 0);
    } else {
        wid_set_tilename(w, "noentry");
        wid_set_no_shape(w);
    }

    wid_game_last_selected_tile = w;
}

/*
 * Reset the tile that we had last highlighted.
 */
static void
wid_game_selected_item_name_reset (void)
{
    if (!wid_game_last_selected_tile) {
        return;
    }

    verify(wid_game_last_selected_tile);

    wid_set_no_shape(wid_game_last_selected_tile);
    wid_set_tile(wid_game_last_selected_tile, 0);

    wid_game_last_selected_tile = 0;
}

static void
wid_game_map_tile_receive_on_mouse_over_end (widp w)
{
    wid_set_no_shape(w);
    wid_set_tile(w, 0);
}

static boolean wid_game_map_tile_key_down_event (widp w,
                                                 const SDL_KEYSYM *key)
{
    /*
     * Console.
     */
    switch (key->sym) {
        case '`':
            return (false);
        default:
            break;
    }

    switch (key->sym) {
        case 'q':
        case SDLK_ESCAPE:
            wid_game_hide();
            return (true);

        default:
            return (true);
    }

    return (false);
}

static boolean wid_game_map_tile_key_up_event (widp w,
                                               const SDL_KEYSYM *key)
{
    switch (key->sym) {
        default:
            return (true);
    }

    return (false);
}

/*
 * This is an item being select.
 */
static boolean wid_game_map_button_receive_mouse_down (widp w,
                                                       int32_t x, int32_t y,
                                                       uint32_t button)
{
    thing_templatep thing_template;
    tree_rootp items;
    thingp thing;
    itemp item;
    
    wid_game_selected_item_name = (typeof(wid_game_selected_item_name))
                    wid_get_client_context(w);

    if (!wid_game_selected_item_name) {
        char *tmp = dynprintf("wid [%s] has no item", wid_name(w));
        (void) wid_popup_error(tmp);
        myfree(tmp);
        return (false);
    }

    thing_template = thing_template_find(wid_game_selected_item_name);
    if (!thing_template) {
        return (false);
    }

    if (!player) {
        return (false);
    }

    items = thing_carried_items(player);
    if (!items) {
        char *tmp = dynprintf("item [%s] has no items",
                              wid_game_selected_item_name);
        (void) wid_popup_error(tmp);
        myfree(tmp);
        return (false);
    }

    item = item_find_template(items, thing_template);
    if (!item) {
        char *tmp = dynprintf("item [%s] not found (2)",
                              wid_game_selected_item_name);
        (void) wid_popup_error(tmp);
        myfree(tmp);
        return (false);
    }

    thing = item_thing(item);
    thing_item_use(player, thing);

    return (true);
}

/*
 * Create the wid_game_map
 */
void wid_game_map_wid_create (void)
{
    wid_game_selected_item_name = 0;

    if (sdl_is_exiting()) {
        return;
    }

    if (wid_game_map_window) {
        return;
    }

    {
        fpoint tl = {0.0f, 0.0f};
        fpoint br = {1.0f, 1.0f};

        wid_game_map_window = wid_new_square_window("wid_game_map");
        wid_set_movable(wid_game_map_window, false);
        wid_set_do_not_raise(wid_game_map_window, true);

        wid_set_mode(wid_game_map_window, WID_MODE_NORMAL);

        wid_set_text_advance(wid_game_map_window, 0.9f);
        wid_set_text_scaling(wid_game_map_window, 2.0f);
        wid_set_text_pos(wid_game_map_window, true, 0.5f, 0.10f);
        wid_set_text_outline(wid_game_map_window, true);

        wid_set_on_mouse_down(wid_game_map_window,
                              wid_game_map_receive_mouse_down);
        wid_set_on_mouse_up(wid_game_map_window,
                            wid_game_map_receive_mouse_up);
        wid_set_on_key_down(wid_game_map_window,
                            wid_game_map_tile_key_down_event);
        wid_set_on_key_up(wid_game_map_window,
                          wid_game_map_tile_key_up_event);

        wid_set_text_bot(wid_game_map_window, true);
        wid_set_text_lhs(wid_game_map_window, true);
        wid_set_tl_br_pct(wid_game_map_window, tl, br);

        fsize sz = {0.0f, 0.0f};
        wid_set_tex_tl(wid_game_map_window, sz);

        fsize sz2 = {1.0f, 1.0f};
        wid_set_tex_br(wid_game_map_window, sz2);
    }

    {
        fpoint tl = {0.00f, 0.00f};
        fpoint br = {1.00f, 1.00f};

        wid_game_map_grid_container =
                        wid_new_container(wid_game_map_window,
                                          "wid game grid container");

        wid_set_color(wid_game_map_grid_container, WID_COLOR_TL, BLACK);
        wid_set_color(wid_game_map_grid_container, WID_COLOR_BG, BLACK);
        wid_set_color(wid_game_map_grid_container, WID_COLOR_BR, BLACK);

        wid_set_tl_br_pct(wid_game_map_grid_container, tl, br);
        wid_set_tex(wid_game_map_grid_container, 0, 0);

        wid_set_on_mouse_down(wid_game_map_grid_container,
                              wid_game_map_receive_mouse_down);
        wid_set_on_mouse_up(wid_game_map_grid_container,
                            wid_game_map_receive_mouse_up);
    }

    {
        int32_t x;
        int32_t y;

        widp child;

        /*
         * An array of tiles that we use for the cursor placement.
         */
        for (x = 0; x < TILES_MAP_EDITABLE_WIDTH; x++) {
            for (y = 0; y < TILES_MAP_EDITABLE_HEIGHT; y++) {

                if (y < 1 ) {
                    /*
                     * Nothing in the title area.
                     */
                    continue;
                }

                fpoint tl = {
                    (tile_width_pct * (float)(x)),
                    (tile_height_pct * (float)(y))
                };

                fpoint br = {
                    (tile_width_pct * (float)(x+1)),
                    (tile_height_pct * (float)(y+1))
                };

                child = wid_new_square_button(wid_game_map_grid_container,
                                              "map base tile");

                wid_set_color(child, WID_COLOR_BG, BLACK);
                color c = WHITE;
                c.a = 100;

                wid_set_color(child, WID_COLOR_TEXT, c);

                if ((y >= 1) && (y < TILES_MAP_EDITABLE_HEIGHT - 1)) {
                    wid_set_on_key_down(child,
                                        wid_game_map_tile_key_down_event);
                    wid_set_on_key_up(child,
                                        wid_game_map_tile_key_up_event);
                    wid_set_on_mouse_down(child,
                                        wid_game_map_tile_receive_mouse_down);
                    wid_set_on_mouse_up(child,
                                        wid_game_map_tile_receive_mouse_up);
                    wid_set_on_mouse_over_begin(
                            child,
                            wid_game_map_tile_receive_on_mouse_over_begin);
                    wid_set_on_mouse_over_end(
                            child,
                            wid_game_map_tile_receive_on_mouse_over_end);

                    wid_set_mode(child, WID_MODE_OVER);
                    wid_set_color(child, WID_COLOR_TL, STEELBLUE);
                    wid_set_color(child, WID_COLOR_BR, STEELBLUE);

                    wid_set_mode(child, WID_MODE_NORMAL);
                    wid_set_color(child, WID_COLOR_TL, STEELBLUE);
                    wid_set_color(child, WID_COLOR_BR, STEELBLUE);

                    wid_set_z_depth(child, 100);
                    wid_set_z_order(child, 0);
                }

                wid_set_no_shape(child);

                wid_set_tl_br_pct(child, tl, br);

                tile_width = wid_get_width(child);
                tile_height = wid_get_height(child);
            }
        }

        wid_new_grid(wid_game_map_grid_container,
                     TILES_MAP_WIDTH,
                     TILES_MAP_HEIGHT, tile_width, tile_height);
    }

    wid_visible(wid_game_map_window, 0);

    wid_game_selected_item_name = 0;

    if (!player) {
        player_new(0 /* level */, "data/things/warrior");
    }

    wid_editor_map_loading = true;

    level_game = level_load(thing_level_no(player),
                            wid_game_map_grid_container);
    if (!level_game) {
        WARN("failed to load level");
        return;
    }

    wid_editor_map_loading = false;

    things_level_start(level_game);
}

void wid_game_map_wid_destroy (void)
{
    FINI_LOG("Destroy game map");

    /*
     * Reset the tile that we had last highlighted.
     */
    wid_game_selected_item_name_reset();

    if (level_game) {
        level_destroy_immediate(&level_game);
    }

    if (wid_game_map_window) {
        wid_destroy(&wid_game_map_window);
    }

    if (wid_scoreline_container_top) {
        wid_destroy(&wid_scoreline_container_top);
    }

    if (wid_scoreline_container_bot) {
        wid_destroy(&wid_scoreline_container_bot);
    }
}

/*
 * Replace or place a tile.
 */
widp
wid_game_map_replace_tile (widp w,
                           int32_t x, int32_t y,
                           uint32_t count,
                           thing_templatep thing_template)
{
    tree_rootp thing_tiles;
    const char *tilename;
    widp existing;
    tilep tile;
    widp child;
    levelp level;

    verify(w);

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
    if (!thing_template) {
        DIE("no thing template");
    }

    thing_tiles = thing_template_get_tiles(thing_template);
    if (!thing_tiles) {
        DIE("thing template [%s] has no tiles",
            thing_template_shortname(thing_template));
    }

    thing_tilep thing_tile;

    /*
     * Get the first anim tile.
     */
    thing_tile = (typeof(thing_tile)) tree_root_first(thing_tiles);

    /*
     * Find the real tile that corresponds to this name.
     */
    tilename = thing_tile_name(thing_tile);
    tile = tile_find(tilename);

    if (!tile) {
        DIE("tile name %s from thing %s not found",
            tilename,
            thing_template_shortname(thing_template));
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

    existing = wid_grid_find_thing_template_is(wid_game_map_grid_container,
                                               x, y,
                                               thing_template);
    if (existing)  {
        child = existing;

        count = (typeof(count)) (uintptr_t)
                        wid_get_client_context(child);
        count++;

        wid_game_map_set_thing_template(child, thing_template);

        wid_update(child);

        return (child);
    }

    /*
     * Give this thing to the player and do not put it on the level.
     */
    if (count) {
        while (count--) {
            thingp thing = thing_new(level,
                                     thing_template_name(thing_template));

            if (!thing_item_collect_no_auto_use(player, thing)) {
                DIE("failed to collect %s",
                    thing_template_name(thing_template));
            }
        }

        return (wid_game_map_grid_container);
    }

    /*
     * Make a new thing.
     */
    child = wid_new_square_button(wid_game_map_grid_container,
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
    wid_game_map_set_thing_template(child, thing_template);

    if (thing_template_is_player(thing_template)) {
        if (!player) {
            player = player_new(level,
                                thing_template_name(thing_template));
        }

        wid_set_thing(child, player);

        thing_restarted(player, level);
    } else if (thing_template_is_thing(thing_template)) {
        wid_set_thing(child,
                      thing_new(level,
                                thing_template_name(thing_template)));
    }

    /*
     * This adds it to the grid wid.
     */
    wid_update(child);

    if (wid_editor_map_loading) {
        return (child);
    }

    map_fixup(level);

    return (child);
}

void wid_game_map_score_update (levelp level)
{
    if (!player) {
        return;
    }

    if (!wid_game_map_window) {
        return;
    }

    wid_destroy(&wid_scoreline_container_top);

    /*
     * Create the area for the scores at the top.
     */
    {
        fpoint tl;
        fpoint br;

        tl.x = 0.67;
        tl.y = 0.0;
        br.x = 1.0;
        br.y = 1.0;

        wid_scoreline_container_top =
            wid_new_square_button(wid_game_map_window, "scoreline top");

        wid_set_tl_br_pct(wid_scoreline_container_top, tl, br);
        wid_set_color(wid_scoreline_container_top, WID_COLOR_TL, BLACK);
        wid_set_color(wid_scoreline_container_top, WID_COLOR_BG, BLACK);
        wid_set_color(wid_scoreline_container_top, WID_COLOR_BR, BLACK);
    }

    float atx1 = 0.75;
    float atx2 = 0.90;
    float aty1 = 0.35;
    float dy = 0.15;
    float dy2 = 0.03;
    float dy3 = 0.05;

    /*
     * Print the score.
     */
    int y;
    for (y = 0; y < 4; y++) 
    {
        color c;
        char *name_title;

        switch (y) {
        case 0:
            c = RED;
            name_title = "WARRIOR";
            break;
        case 1:
            c = SKYBLUE;
            name_title = "VALKYRIE";
            break;
        case 2:
            c = YELLOW;
            name_title = "WIZARD";
            break;
        case 3:
            c = GREEN;
            name_title = "ELF";
            break;
        }

        /*
         * Score
         */
        char *tmp = dynprintf("%06u", thing_score(player));

        widp wid_score_container;

        wid_score_container = wid_textbox_fixed_width(
                                    wid_scoreline_container_top,
                                    &wid_score,
                                    tmp, 
                                    atx1, aty1 + dy*(float)y, 
                                    small_font);
        myfree(tmp);

        wid_set_no_shape(wid_score_container);

        /*
         * Health
         */
        tmp = dynprintf("%06u", thing_score(player));

        widp wid_health_container;

        wid_health_container = wid_textbox_fixed_width(
                                    wid_scoreline_container_top,
                                    &wid_health,
                                    tmp,  
                                    atx2, aty1 + dy*(float)y, 
                                    small_font);
        myfree(tmp);

        wid_set_no_shape(wid_health_container);

        /*
         * Score title
         */
        widp wid_score_title_container;

        wid_score_title_container = wid_textbox_fixed_width(
                                    wid_scoreline_container_top,
                                    &wid_score_title,
                                    "SCORE", 
                                    atx1, aty1 + dy*(float)y - dy2,
                                    vsmall_font);

        wid_set_no_shape(wid_score_title_container);

        /*
         * Health title
         */
        widp wid_health_title_container;

        wid_health_title_container = wid_textbox_fixed_width(
                                    wid_scoreline_container_top,
                                    &wid_health_title,
                                    "HEALTH",  
                                    atx2, aty1 + dy*(float)y - dy2,
                                    vsmall_font);

        wid_set_no_shape(wid_health_title_container);

        /*
         * Score title
         */
        widp wid_name_title_container;

        wid_name_title_container = wid_textbox_fixed_width(
                                    wid_scoreline_container_top,
                                    &wid_name_title,
                                    name_title,
                                    (atx1 + atx2) / 2,
                                    aty1 + dy*(float)y - dy3,
                                    vsmall_font);

        wid_set_no_shape(wid_name_title_container);

        switch (y) {
        case 0:
            wid_set_color(wid_score, WID_COLOR_TEXT, RED);
            wid_set_color(wid_health, WID_COLOR_TEXT, RED);
            wid_set_color(wid_score_title, WID_COLOR_TEXT, RED);
            wid_set_color(wid_health_title, WID_COLOR_TEXT, RED);
            break;
        case 1:
            wid_set_color(wid_score, WID_COLOR_TEXT, SKYBLUE);
            wid_set_color(wid_health, WID_COLOR_TEXT, SKYBLUE);
            wid_set_color(wid_score_title, WID_COLOR_TEXT, SKYBLUE);
            wid_set_color(wid_health_title, WID_COLOR_TEXT, SKYBLUE);
            break;
        case 2:
            wid_set_color(wid_score, WID_COLOR_TEXT, YELLOW);
            wid_set_color(wid_health, WID_COLOR_TEXT, YELLOW);
            wid_set_color(wid_score_title, WID_COLOR_TEXT, YELLOW);
            wid_set_color(wid_health_title, WID_COLOR_TEXT, YELLOW);
            break;
        case 3:
            wid_set_color(wid_score, WID_COLOR_TEXT, GREEN);
            wid_set_color(wid_health, WID_COLOR_TEXT, GREEN);
            wid_set_color(wid_score_title, WID_COLOR_TEXT, GREEN);
            wid_set_color(wid_health_title, WID_COLOR_TEXT, GREEN);
            break;
        }

        wid_set_color(wid_score, WID_COLOR_TEXT, c);
        wid_set_color(wid_health, WID_COLOR_TEXT, c);
        wid_set_color(wid_score_title, WID_COLOR_TEXT, c);
        wid_set_color(wid_health_title, WID_COLOR_TEXT, c);
        wid_set_color(wid_name_title, WID_COLOR_TEXT, c);
    }

    /*
     * Print the level title.
     */
    if (level_get_title(level) &&
        strcmp(level_get_title(level), "(null)")) {

        widp wid_title_container;

        wid_title_container = wid_textbox(
                    wid_scoreline_container_top,
                    &wid_title,
                    level_get_title(level), 0.5, 0.07, med_font);

        wid_set_no_shape(wid_title_container);
        wid_raise(wid_title_container);
        wid_set_do_not_lower(wid_title_container, true);
        wid_set_color(wid_title, WID_COLOR_TEXT, STEELBLUE);
    }

    /*
     * Print the level.
     */
    {
        char *tmp = dynprintf("Level %u", thing_level_no(player));
        widp wid_level_container;

        wid_level_container = wid_textbox(wid_scoreline_container_top,
                                            &wid_level,
                                            tmp, atx1, 0.2, small_font);
        myfree(tmp);

        wid_set_no_shape(wid_level_container);
        wid_set_color(wid_level, WID_COLOR_TEXT, WHITE);
    }

    wid_raise(wid_scoreline_container_top);
    wid_update(wid_scoreline_container_top);

    {
        widp wid = wid_new_container(wid_scoreline_container_top, "title");

        fpoint tl = { 0, 0.0 };
        fpoint br = { 1.0, 0.12 };

        wid_set_tl_br_pct(wid, tl, br);

        wid_set_tex(wid, 0, "title");

        fsize sz = {2.8,1.9};

        wid_set_tex_br(wid, sz);

        wid_set_mode(wid, WID_MODE_NORMAL);
        wid_set_color(wid, WID_COLOR_TL, WHITE);
        wid_set_color(wid, WID_COLOR_BR, WHITE);
        wid_set_color(wid, WID_COLOR_BG, WHITE);

        wid_update(wid);
        wid_raise(wid);
    }


    wid_update_mouse();
}

void wid_game_map_item_update (levelp level)
{
    boolean wid_game_selected_item_name_is_valid;

    wid_game_selected_item_name_is_valid = false;

    if (!player) {
        wid_game_selected_item_name = 0;
        return;
    }

    if (!wid_game_map_window) {
        wid_game_selected_item_name = 0;
        return;
    }

    wid_destroy(&wid_scoreline_container_bot);

    /*
     * Create the area for the items at the bottom.
     */
    {
        fpoint tl;
        fpoint br;

        tl.x = 0;
        tl.y = 0.8;
        br.x = 1.0;
        br.y = 1.0;

        wid_scoreline_container_bot =
            wid_new_plain(wid_game_map_window, "scoreline bot");

        wid_set_tl_br_pct(wid_scoreline_container_bot, tl, br);
        wid_set_no_shape(wid_scoreline_container_bot);
    }

    /*
     * Check we have an item to use.
     */
    {
        itemp item;
        uint32_t i;

        i = 0;
        item = item_get_last(thing_carried_items(player));

        while (item) {

            if (!wid_game_selected_item_name) {
                wid_game_selected_item_name = item_name(item);
                wid_game_selected_item_name_is_valid = true;
            } else {
                if (!strcmp(wid_game_selected_item_name, item_name(item))) {
                    wid_game_selected_item_name_is_valid = true;
                }
            }
            item = item_get_prev(thing_carried_items(player), item);
            i++;
        }
    }

    if (!wid_game_selected_item_name_is_valid) {
        wid_game_selected_item_name = 0;
    }

    /*
     * Print all items.
     */
    {
        thing_templatep thing_template = 0;
        const uint32_t item_per_line = 8;
        float item_height = 0.05;
        float item_pad = item_height + 0.065;
        float item_at = 0.050;
        itemp item;
        uint32_t i;

        i = 0;
        item = item_get_last(thing_carried_items(player));

        while (item && (i < item_per_line)) {

            thingp thing = item_thing(item);
            if (!thing) {
                ERR_POPUP("item [%s] has no thing template (3)",
                          wid_game_selected_item_name);
            }

            if (thing_template == thing_get_template(thing)) {
                item = item_get_prev(thing_carried_items(player), item);
                continue;
            }

            thing_template = thing_get_template(thing);

            if (thing_is_item_hidden(thing)) {
                item = item_get_prev(thing_carried_items(player), item);
                continue;
            }

            tree_rootp items = thing_carried_items(player);

            widp wid_item_box;
            widp wid_items_count;
            widp wid_item;

            color c;
            c = BLACK;
            c.a = 150;

            /*
             * Item container box.
             */
            wid_item_box =
                wid_new_square_button(wid_scoreline_container_bot, 
                                      item_name(item));

            fpoint tl;
            fpoint br;

            tl.x = item_at + item_pad * i;
            tl.y = 0.32;

            br.x = tl.x + item_height + (item_pad - item_height) * 0.7;
            br.y = 1.0;

            wid_set_tl_br_pct(wid_item_box, tl, br);
            wid_set_color(wid_item_box, WID_COLOR_BG, BLACK);
            wid_set_on_mouse_down(wid_item_box,
                                  wid_game_map_button_receive_mouse_down);

            const char *name = item_name(item);
            if (!name) {
                ERR_POPUP("item [%s] has no name",
                          wid_game_selected_item_name);
            }

            wid_set_client_context(wid_item_box, (void*)name);

            wid_set_tooltip(wid_item_box,
                            thing_template_get_tooltip(thing_template));

            boolean highlight = false;

            if (!wid_game_selected_item_name) {
                wid_game_selected_item_name = item_name(item);
                wid_game_selected_item_name_is_valid = true;
                highlight = true;
            } else {
                if (!strcmp(wid_game_selected_item_name, item_name(item))) {
                    wid_game_selected_item_name_is_valid = true;
                    highlight = true;
                }
            }

            if (highlight) {
                wid_set_color(wid_item_box, WID_COLOR_TL, STEELBLUE);
                wid_set_color(wid_item_box, WID_COLOR_BR, STEELBLUE);
            }

            wid_set_color(wid_item_box, WID_COLOR_BG, c);
            wid_set_bevel(wid_item_box, 3);
            wid_set_mode(wid_item_box, WID_MODE_OVER);
            wid_set_color(wid_item_box, WID_COLOR_TL, BLUE);
            wid_set_color(wid_item_box, WID_COLOR_BR, BLUE);
            wid_set_mode(wid_item_box, WID_MODE_NORMAL);

            /*
             * Item tile box.
             */
            wid_item = wid_new_plain(wid_item_box, "item");

            tl.x = 0.20;
            tl.y = 0.05;
            br.x = 0.80;
            br.y = 0.8;

            wid_set_tl_br_pct(wid_item, tl, br);

            tree_rootp tiles = thing_template_get_tiles(thing_template);
            if (!tiles) {
                ERR_POPUP("item [%s] has no tiles",
                          wid_game_selected_item_name);
                return;
            }

            thing_tilep tile = (typeof(tile)) tree_root_first(tiles);
            if (!tile) {
                ERR_POPUP("item [%s] has no tile",
                          wid_game_selected_item_name);
                return;
            }

            wid_set_tilename(wid_item, thing_tile_name(tile));

            /*
             * Item count box.
             */
            wid_items_count = wid_new_plain(wid_item_box, "item count");

            tl.x = 0.05;
            tl.y = 0.35;
            br.x = 0.95;
            br.y = 1.15;

            wid_set_tl_br_pct(wid_items_count, tl, br);

            char *tmp = dynprintf("%u",
                                  items_count_is_thing_template(items,
                                                             thing_template));
            wid_set_text(wid_items_count, tmp);
            myfree(tmp);

            wid_set_color(wid_items_count, WID_COLOR_TEXT, STEELBLUE);
            wid_set_font(wid_items_count, med_font);
            wid_set_text_bot(wid_items_count, true);

            item = item_get_prev(thing_carried_items(player), item);
            i++;
        }
    }

    wid_raise(wid_scoreline_container_bot);

    wid_update(wid_scoreline_container_bot);

    wid_update_mouse();
}
