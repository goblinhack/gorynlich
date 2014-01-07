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

static boolean wid_game_init_done;

uint32_t tile_width;
uint32_t tile_height;

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

boolean wid_game_init (void)
{
    wid_game_init_done = true;

    if (is_server) {
        if (!is_client) {
            wid_game_visible();
        }
    }
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
 * Create the wid_game_map
 */
void wid_game_map_wid_create (void)
{
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
    }

    {
        float base_tile_width =
                ((1.0f / (float)TILES_SCREEN_WIDTH) *
                    (float)global_config.video_gl_width);

        float base_tile_height =
                ((1.0f / (float)TILES_SCREEN_HEIGHT) *
                    (float)global_config.video_gl_height);

        fpoint tl = { 0, 0 };
        fpoint br = { 0, 0 };

        br.x += base_tile_width;
        br.y += base_tile_height;

        tile_width = br.x - tl.x;
        tile_height = br.y - tl.y;

        if (!tile_width) {
            tile_width = TILE_WIDTH;
        }

        if (!tile_height) {
            tile_height = TILE_HEIGHT;
        }

        wid_new_grid(wid_game_map_grid_container,
                     TILES_MAP_WIDTH,
                     TILES_MAP_HEIGHT, tile_width, tile_height);
    }

    wid_visible(wid_game_map_window, 0);

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
                                    med_font);
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
                                    med_font);
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
                                            tmp, atx1, 0.2, med_font);
        myfree(tmp);

        wid_set_no_shape(wid_level_container);
        wid_set_color(wid_level, WID_COLOR_TEXT, WHITE);
    }

    wid_raise(wid_scoreline_container_top);
    wid_update(wid_scoreline_container_top);

    {
        widp wid = wid_new_square_button(wid_scoreline_container_top, "title");

        fpoint tl = { 0.1, 0.0 };
        fpoint br = { 0.9, 0.09 };

        wid_set_tl_br_pct(wid, tl, br);

        wid_set_tex(wid, 0, "title2");

        wid_set_mode(wid, WID_MODE_NORMAL);
        wid_set_color(wid, WID_COLOR_TL, WHITE);
        wid_set_color(wid, WID_COLOR_BR, WHITE);
        wid_set_color(wid, WID_COLOR_BG, WHITE);

        wid_update(wid);
        wid_raise(wid);
    }

    wid_update_mouse();
}
