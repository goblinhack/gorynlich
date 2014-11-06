/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "tile.h"
#include "thing_tile.h"
#include "wid_game_map_server.h"
#include "wid_editor.h"
#include "thing.h"
#include "map.h"
#include "level.h"
#include "server.h"
#include "socket.h"

levelp server_level;
widp wid_game_map_server_window;
widp wid_game_map_server_grid_container;

uint32_t server_tile_width;
uint32_t server_tile_height;

uint8_t wid_game_map_server_init (void)
{
    return (true);
}

void wid_game_map_server_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    wid_game_map_server_wid_destroy(false /* keep players */);
}

void wid_game_map_server_hide (void)
{
    if (wid_game_map_server_window) {
        LOG("Server: Hide map");

        wid_hide(wid_game_map_server_window, wid_swipe_delay);

        wid_detach_from_grid(wid_game_map_server_grid_container);

        wid_destroy_grid(wid_game_map_server_grid_container);
    }
}

void wid_game_map_server_visible (void)
{
    on_server = true;

    server_start(server_address);

    if (!wid_game_map_server_window) {
        LOG("Server: Create new map");

        wid_game_map_server_wid_create();
    }

    wid_hide(wid_game_map_server_window, 0);
}

/*
 * Create the wid_game_map_server
 */
void wid_game_map_server_wid_create (void)
{
    if (sdl_is_exiting()) {
        return;
    }

    if (wid_game_map_server_window) {
        return;
    }

    {
        fpoint tl = {0.0f, 0.0f};
        fpoint br = {1.0f, 1.0f};

        wid_game_map_server_window = 
                        wid_new_square_window("wid_game_map_server");
        wid_always_hidden(wid_game_map_server_window, true);
        wid_set_movable(wid_game_map_server_window, false);
        wid_set_do_not_raise(wid_game_map_server_window, true);

        wid_set_mode(wid_game_map_server_window, WID_MODE_NORMAL);

        wid_set_text_advance(wid_game_map_server_window, 0.9f);
        wid_set_text_scaling(wid_game_map_server_window, 2.0f);
        wid_set_text_pos(wid_game_map_server_window, true, 0.5f, 0.10f);
        wid_set_text_outline(wid_game_map_server_window, true);

        wid_set_text_bot(wid_game_map_server_window, true);
        wid_set_text_lhs(wid_game_map_server_window, true);
        wid_set_tl_br_pct(wid_game_map_server_window, tl, br);

        fsize sz = {0.0f, 0.0f};
        wid_set_tex_tl(wid_game_map_server_window, sz);

        fsize sz2 = {1.0f, 1.0f};
        wid_set_tex_br(wid_game_map_server_window, sz2);
    }

    {
        fpoint tl = {0.00f, 0.00f};
        fpoint br = {1.00f, 1.00f};

        wid_game_map_server_grid_container =
                        wid_new_container(wid_game_map_server_window,
                                          "wid game server grid container");

        wid_always_hidden(wid_game_map_server_grid_container, true);
        wid_set_color(wid_game_map_server_grid_container, WID_COLOR_TL, BLACK);
        wid_set_color(wid_game_map_server_grid_container, WID_COLOR_BG, BLACK);
        wid_set_color(wid_game_map_server_grid_container, WID_COLOR_BR, BLACK);
        wid_set_ignore_events(wid_game_map_server_grid_container, true);

        wid_set_tl_br_pct(wid_game_map_server_grid_container, tl, br);
        wid_set_tex(wid_game_map_server_grid_container, 0, 0);
    }

    {
        float base_tile_width =
                ((1.0f / ((float)TILES_SCREEN_WIDTH)) *
                    (float)global_config.video_gl_width);

        float base_tile_height =
                ((1.0f / ((float)TILES_SCREEN_HEIGHT)) *
                    (float)global_config.video_gl_height);

        fpoint tl = { 0, 0 };
        fpoint br = { 0, 0 };

        br.x += base_tile_width;
        br.y += base_tile_height;

        server_tile_width = br.x - tl.x;
        server_tile_height = br.y - tl.y;
        server_tile_width = br.x - tl.x;
        server_tile_height = br.y - tl.y;

        if (!server_tile_width) {
            server_tile_width = TILE_WIDTH;
        }

        if (!server_tile_height) {
            server_tile_height = TILE_HEIGHT;
        }

        wid_new_grid(wid_game_map_server_grid_container,
                     MAP_WIDTH,
                     MAP_HEIGHT, server_tile_width, server_tile_height);
    }

    /*
     * Never show the server things
     */
    wid_hide(wid_game_map_server_window, 0);

    server_level_is_being_loaded = true;

#if 1
    server_level = level_load_random(1, wid_game_map_server_grid_container, 
                                     false /* is_editor */,
                                     true /* server */);
#else
    server_level = level_load(1, wid_game_map_server_grid_container, 
                              false /* is_editor */,
                              true /* server */);
#endif
    if (!server_level) {
        WARN("failed to load level");
        return;
    }

    server_level_is_being_loaded = false;

    level_update_now(server_level);

    level_pause(server_level);
}

void wid_game_map_server_wid_destroy (uint8_t keep_players)
{
    LOG("Server: Destroy game map");

    if (server_level) {
        if (keep_players) {
            LOG("Server: Destroy level but keep players");
        } else {
            LOG("Server: Destroy level");
        }

        level_destroy(&server_level, keep_players);
    }

    if (wid_game_map_server_window) {
        wid_game_map_server_hide();

        wid_destroy(&wid_game_map_server_window);
    }
}

/*
 * Replace or place a tile.
 */
widp
wid_game_map_server_replace_tile (widp w,
                                  double x, double y,
                                  thingp thing,
                                  tpp tp,
                                  itemp item,
                                  player_stats_t *stats)
{
    tree_rootp thing_tiles;
    const char *tilename;
    tilep tile;
    widp child;
    levelp level;

    if ((x < 0) || (y < 0) || (x >= MAP_WIDTH) || (y >= MAP_WIDTH)) {
        DIE("thing template [%s] cannot be placed at %f %f",
            tp_short_name(tp), x, y);
    }

    verify(w);

    level = (typeof(level)) wid_get_client_context(w);
    verify(level);

    /*
     * If we find a player, it is really a placeholder of where to put a 
     * future player who joins.
     */
    if (tp_is_player(tp)) {
        if ((x == 0) && (y == 0)) {
            /*
             * Grab a position from the list loaded.
             */
            x = level->player_start_position[level->player_start_at].x;
            y = level->player_start_position[level->player_start_at].y;
            level->player_start_at++;

            if (level->player_start_at >= level->player_start_max) {
                level->player_start_at = 0;
            }
        } else {
            /*
             * Add a position to the list.
             */
            if (level->player_start_max == MAX_PLAYERS) {
                ERR("Too many player start positions in level");
                return (0);
            }

            /*
             * Append to the array of possible start positions.
             */
            level->player_start_position[level->player_start_max].x = x;
            level->player_start_position[level->player_start_max].y = y;
            level->player_start_max++;

            /*
             * Dummy value just so it is not an error.
             */
            return (w);
        }
    }

    /*
     * Grow tl and br to fit the template thing. Use the first tile.
     */
    if (!tp) {
        DIE("no thing template to place on server map");
    }

    thing_tiles = tp_get_tiles(tp);
    if (!thing_tiles) {
        DIE("thing template [%s] has no tiles",
            tp_short_name(tp));
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
            tp_short_name(tp));
    }

    /*
     * Make a new thing.
     */
    child = wid_new_square_button(wid_game_map_server_grid_container,
                                  "server map tile");

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
    wid_set_thing_template(child, tp);

    if (!thing) {
        thing = thing_server_new(tp_name(tp), x, y, stats);
    } else {
        thing_server_init(thing, x, y);
    }

    if (item) {
        memcpy(&thing->item, &item, sizeof(item_t));
    }

    wid_set_thing(child, thing);

    thing_server_wid_update(thing, x, y, true /* is_new */);

    /*
     * This adds it to the grid wid.
     */
    wid_update(child);

    if (server_level_is_being_loaded) {
        return (child);
    }

    if (thing_is_wall(thing) ||
        thing_is_door(thing) ||
        thing_is_pipe(thing)) {
        map_fixup(level);
    }

    return (child);
}
