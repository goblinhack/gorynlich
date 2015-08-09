/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "tile.h"
#include "thing_tile.h"
#include "wid_game_map_server.h"
#include "thing.h"
#include "map.h"
#include "level.h"
#include "server.h"
#include "socket_util.h"
#include "string_util.h"
#include "file.h"
#include "thing_template.h"
#include "wid_editor.h"
#include "sound.h"

levelp server_level;
widp wid_game_map_server_window;
widp wid_game_map_server_grid_container;

uint32_t server_tile_width;
uint32_t server_tile_height;
int server_level_is_being_loaded;

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

        level_load_new();
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
}

void wid_game_map_server_wid_destroy (uint8_t keep_players)
{
    if (!keep_players) {
        LOG("Server: Reset back to first level");

        memset(&global_config.server_level_pos, 0, 
               sizeof(global_config.server_level_pos));
    }

    if (server_level) {
        LOG("Server: Destroy game map");

        if (keep_players) {
            LOG("Server: Destroy level but keep players");
        } else {
            LOG("Server: Destroy level and kill players");
        }

        level_destroy(&server_level, keep_players);
    }

    if (wid_game_map_server_window) {
        LOG("Server: Destroy game window`");

        wid_game_map_server_hide();

        wid_destroy(&wid_game_map_server_window);
    }
}

static void level_set_new_tp (levelp level,
                              int x, int y, int z, 
                              tpp tp, tpp_data data)
{
    memset(&level->map_grid.tile[x][y][z], 0, sizeof(level_map_tile));
    level->map_grid.tile[x][y][z].tp = tp;

    if (data) {
        level->map_grid.tile[x][y][z].data = *data;
    }
}

static void level_get_tp (levelp level,
                          int x, int y, int z, 
                          tpp *tp)
{
    *tp = level->map_grid.tile[x][y][z].tp;
}

/*
 * Replace or place a tile.
 */
widp
wid_game_map_server_replace_tile (widp w,
                                  double x, double y,
                                  thingp t,
                                  tpp tp,
                                  tpp_data data,
                                  itemp item,
                                  thing_statsp stats)
{
#if 0
    static levelp last_level;
    static int flip_x;
    static int flip_y;
    static int rot;
#endif

    verify(w);
    levelp level = (typeof(level)) wid_get_client_context(w);
    verify(level);

#if 0
    /*
     * Does not handle rotation of triggers correctly, so commented out for 
     * now
     */
    if (server_level_is_being_loaded) {
        if (level != last_level) {
            last_level = level;

            /*
             * Need to change direction triggers too... Not sure if this is 
             * worth the complexity...
             */
            flip_x = 0;
            flip_y = 0;
            rot = 0;

            if ((myrand() % 100) < 50) {
                flip_x = 1;
            }

            if ((myrand() % 100) < 50) {
                flip_y = 1;
            }

            if ((myrand() % 100) < 50) {
                rot = 1;
            }
        }

        if (flip_x) {
            x = MAP_WIDTH - x - 1;
        }

        if (flip_y) {
            y = MAP_HEIGHT - y - 1;
        }

        if (rot) {
            swap(x, y);
        }

        if ((x < 0) || (y < 0) || (x >= MAP_WIDTH) || (y >= MAP_HEIGHT)) {
            ERR("%s placing thing at bad co-ords %f %f", tp_name(tp), x, y);
            return (0);
        }
    }
#endif

    int z = tp_get_z_depth(tp);
    tree_rootp thing_tiles;
    const char *tilename;
    tilep tile;
    widp child;
    int ix = x;
    int iy = y;
    int depth = ((level->level_pos.y - 1) * LEVELS_ACROSS) + level->level_pos.x;

    if (server_level_is_being_loaded == 1) {
        /*
         * Map random things to real things.
         */
        tpp otp = tp;

        /*
         * Some things are only valid in shops.
         */
        tpp floor_tp;
        level_get_tp(level, x, y, MAP_DEPTH_FLOOR, &floor_tp);
        int shop_floor = false;

        if (floor_tp) {
            if (tp_is_shop_floor(floor_tp)) {
                shop_floor = true;
            }
        }

        switch (tp_to_id(tp)) {
            case THING_POTION_ANY:
                tp = random_potion(shop_floor);
                break;
            case THING_FOOD_ANY:
                tp = random_food();
                break;
            case THING_MOB_ANY:
                tp = random_mob(depth);
                break;
            case THING_MONST_ANY:
                tp = random_monst(depth);
                break;
            case THING_TREASURE_ANY:
                tp = random_treasure(shop_floor);
                break;
            case THING_WEAPON_ANY:
                tp = random_weapon(shop_floor);
                break;
        }

        if (!tp) {
            ERR("failed to make random %s", tp_name(otp));
            return (0);
        }
    } else {
        switch (tp_to_id(tp)) {
            case THING_POTION_ANY:
            case THING_FOOD_ANY:
            case THING_MOB_ANY:
            case THING_MONST_ANY:
            case THING_TREASURE_ANY:
            case THING_WEAPON_ANY:
                level_get_tp(level, x, y, z, &tp);
                break;
        }
    }

    /*
     * First pass? Only interested in location of triggers.
     */
    if (server_level_is_being_loaded == 1) {
        /*
         * Record what triggers exist on the level.
         */
        if (tp_is_action_trigger(tp)) {
            if (!data) {
                ERR("expecting trigger data for %s", tp_name(tp));
                return (0);
            }

            level_trigger_alloc(level, data->col_name);
        }

        level_set_new_tp(level, x, y, z, tp, data);

        return (0);
    }

    /*
     * Second pass, do not create things that are to only be created on 
     * triggers.
     */
    if (server_level_is_being_loaded == 2) {
        /*
         * If there is a trigger here 
         */
        if (z > MAP_DEPTH_FLOOR) {
            tpp trigger = level->map_grid.tile[ix][iy][MAP_DEPTH_ACTIONS].tp;

            if (trigger) {
                if (tp_is_action_sleep(trigger)) {
                    return (0);
                }
            }
        }
    }

    /*
     * Second pass, create the things.
     */

    if ((x < 0) || (y < 0) || (x >= MAP_WIDTH) || (y >= MAP_WIDTH)) {
        /*
         * Trying to fire a weapon off the end of the level?
         */
        ERR("server: thing template [%s] cannot be placed at %f %f",
            tp_short_name(tp), x, y);
        return (0);
    }

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
                ERR("Too many player start positions in level %d.%d",
                    global_config.server_level_pos.x,
                    global_config.server_level_pos.y);
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
        ERR("no thing template to place on server map");
        return (0);
    }

    thing_tiles = tp_get_tiles(tp);
    if (!thing_tiles) {
        ERR("thing template [%s] has no tiles", tp_short_name(tp));
        return (0);
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
        ERR("tile name %s from thing %s not found on server",
            tilename,
            tp_short_name(tp));
        return (0);
    }

    /*
     * Make a new thing.
     */
    child = wid_new_square_button(wid_game_map_server_grid_container,
                                  "server map tile");

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

    if (!t) {
        t = thing_server_new(tp_name(tp), x, y, stats);
    } else {
        thing_server_init(t, x, y);
    }

    if (data) {
        if (!t->data) {
            t->data = myzalloc(sizeof(thing_template_data), "thing data");
        }

        memcpy(t->data, data, sizeof(thing_template_data));

        if (thing_is_exit(t)) {
            if (t->data->exit_set) {
                LOG("Create exit to level %d.%d", 
                    t->data->exit.y, t->data->exit.x);
            } else {
                /*
                 * Not an issue, just choose the next one.
                 */
                LOG("An exit was present on level %d.%d with no exit level set",
                    t->data->exit.y, t->data->exit.x);
            }
        }

        if (t->data->col_name) {
            t->data->col = color_find(t->data->col_name);
            wid_set_color(w, WID_COLOR_BLIT, t->data->col);
        }
    }

    if (item) {
        memcpy(&t->item, item, sizeof(item_t));
        t->item.quantity = 1;

        if (!item->id) {
            ERR("trying to create thing %s with an item, "
                "but the item has no id set in it", tp_name(tp));
        }
    }

    wid_set_thing(child, t);

    /*
     * Round the co-ordinates to a location the client can represent.
     */
    thing_round(t, &x, &y);

    /*
     * Does it appear as a different size on screen?
     */
    double scale = tp_get_scale(tp);
    if (scale != 1.0) {
        wid_scaling_blit_to_pct_in(child, scale, scale, 500, 9999999);
    }

    thing_server_wid_update(t, x, y, true /* is_new */);

    /*
     * This adds it to the grid wid.
     */
    wid_update(child);

    /*
     * Avoid doing the map fixup all the time as it is expensive.
     */
    if (server_level_is_being_loaded) {
        return (child);
    }

    if (thing_is_wall(t) ||
        thing_is_door(t)) {
        map_fixup(level);
    }

    /*
     * STUFF ADDED HERE WILL BE SKIPPED BY THE ABOVE CHECK
     */

    return (child);
}
