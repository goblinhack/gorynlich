/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "color.h"
#include "tile.h"
#include "thing_tile.h"
#include "thing_template.h"
#include "string.h"
#include "wid_textbox.h"
#include "wid_game_map_client.h"
#include "wid_intro.h"
#include "wid_editor.h"
#include "thing.h"
#include "map.h"
#include "level.h"
#include "server.h"
#include "client.h"
#include "socket.h"
#include "timer.h"

levelp client_level;
widp wid_game_map_client_window;
widp wid_game_map_client_grid_container;
widp wid_scoreline_container_top;
widp wid_scoreline_container_bot;
widp wid_score;
widp wid_health;
widp wid_score_title;
widp wid_health_title;
widp wid_name_title;
widp wid_title;
widp wid_level;

uint32_t client_tile_width;
uint32_t client_tile_height;

static void wid_game_map_client_set_thing_template (widp w, thing_templatep t)
{
    wid_set_thing_template(w, t);

    if (thing_template_is_effect_pulse(t)) {
        wid_scaling_to_pct_in(w, 1.0, 1.2, 500, 9999999);
    }

    if (thing_template_is_effect_sway(t)) {
        wid_rotate_to_pct_in(w, -2, 2, 500, 9999999);
    }
}

boolean wid_game_map_client_init (void)
{
    return (true);
}

void wid_game_map_client_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    wid_game_map_client_wid_destroy();
}

void wid_game_map_client_hide (void)
{
    if (wid_game_map_client_window) {
        wid_hide(wid_game_map_client_window, wid_swipe_delay);

        wid_detach_from_grid(wid_game_map_client_grid_container);

        wid_destroy_grid(wid_game_map_client_grid_container);
    }

    wid_intro_visible();
}

static void wid_game_map_client_start_cb (void *context)
{
    uint16_t portno;

    portno = global_config.server_port;

    if (server_socket) {
        IPaddress addr = socket_get_local_ip(server_socket);
        portno = SDLNet_Read16(&addr.port);
    }

    if (!client_socket_join(SERVER_DEFAULT_HOST, 0, portno,
                            true /* quiet */)) {
        action_timer_create(
                &wid_timers,
                (action_timer_callback)wid_game_map_client_start_cb,
                0, /* context */
                "join server",
                100,
                0 /* jitter */);
    }
}

void wid_game_map_client_visible (void)
{
    is_server = true;

    if (!client_joined_server) {
        action_timer_create(
                &wid_timers,
                (action_timer_callback)wid_game_map_client_start_cb,
                0, /* context */
                "join server",
                100,
                0 /* jitter */);
    }

    wid_game_map_client_wid_create();
    wid_intro_hide();

    wid_visible(wid_game_map_client_window, 0);
    wid_raise(wid_game_map_client_window);
}

static boolean wid_game_map_key_event (widp w, const SDL_KEYSYM *key)
{
#if SDL_MAJOR_VERSION == 1 && SDL_MINOR_VERSION == 2 /* { */
    uint8_t *state = SDL_GetKeyState(0);

    boolean right = state[SDLK_RIGHT];
    boolean left  = state[SDLK_LEFT];
    boolean up    = state[SDLK_UP];
    boolean down  = state[SDLK_DOWN];
#else /* } { */
    const uint8_t *state = SDL_GetKeyboardState(0);

    boolean right = state[SDL_SCANCODE_RIGHT];
    boolean left  = state[SDL_SCANCODE_LEFT];
    boolean up    = state[SDL_SCANCODE_UP];
    boolean down  = state[SDL_SCANCODE_DOWN];
#endif /* } */

    if (!player) {
        return (false);
    }

    if (!client_joined_server) {
        return (false);
    }

    if (up) {
        if (left) {
            thing_set_is_dir_tl(player);
        } else if (right) {
            thing_set_is_dir_tr(player);
        } else {
            thing_set_is_dir_up(player);
        }
    } else if (down) {
        if (left) {
            thing_set_is_dir_bl(player);
        } else if (right) {
            thing_set_is_dir_br(player);
        } else {
            thing_set_is_dir_down(player);
        }
    } else if (left) {
        thing_set_is_dir_left(player);
    } else if (right) {
        thing_set_is_dir_right(player);
    }

    socket_tx_client_move(client_joined_server, up, down, left, right);

    return (false);
}

/*
 * Create the wid_game_map_client
 */
void wid_game_map_client_wid_create (void)
{
    if (sdl_is_exiting()) {
        return;
    }

    if (wid_game_map_client_window) {
        return;
    }

    {
        fpoint tl = {0.0f, 0.0f};
        fpoint br = {1.0f, 1.0f};

        wid_game_map_client_window = 
                        wid_new_square_window("wid_game_map_client");
        wid_set_movable(wid_game_map_client_window, false);
        wid_set_do_not_raise(wid_game_map_client_window, true);

        wid_set_mode(wid_game_map_client_window, WID_MODE_NORMAL);

        wid_set_text_advance(wid_game_map_client_window, 0.9f);
        wid_set_text_scaling(wid_game_map_client_window, 2.0f);
        wid_set_text_pos(wid_game_map_client_window, true, 0.5f, 0.10f);
        wid_set_text_outline(wid_game_map_client_window, true);

        wid_set_text_bot(wid_game_map_client_window, true);
        wid_set_text_lhs(wid_game_map_client_window, true);
        wid_set_tl_br_pct(wid_game_map_client_window, tl, br);

        fsize sz = {0.0f, 0.0f};
        wid_set_tex_tl(wid_game_map_client_window, sz);

        fsize sz2 = {1.0f, 1.0f};
        wid_set_tex_br(wid_game_map_client_window, sz2);

        wid_set_on_key_down(wid_game_map_client_window, wid_game_map_key_event);
    }

    {
        fpoint tl = {0.00f, 0.00f};
        fpoint br = {0.67f, 1.00f};

        wid_game_map_client_grid_container =
                        wid_new_container(wid_game_map_client_window,
                                          "wid game grid container");

        wid_set_color(wid_game_map_client_grid_container, WID_COLOR_TL, BLACK);
        wid_set_color(wid_game_map_client_grid_container, WID_COLOR_BG, BLACK);
        wid_set_color(wid_game_map_client_grid_container, WID_COLOR_BR, BLACK);

        wid_set_tl_br_pct(wid_game_map_client_grid_container, tl, br);
        wid_set_tex(wid_game_map_client_grid_container, 0, 0);
    }

    {
        float base_tile_width =
                ((1.0f / ((float)TILES_SCREEN_WIDTH) / TILES_CLIENT_SCALE) *
                    (float)global_config.video_gl_width);

        float base_tile_height =
                ((1.0f / ((float)TILES_SCREEN_HEIGHT) / TILES_CLIENT_SCALE) *
                    (float)global_config.video_gl_height);

        fpoint tl = { 0, 0 };
        fpoint br = { 0, 0 };

        br.x += base_tile_width;
        br.y += base_tile_height;

        client_tile_width = br.x - tl.x;
        client_tile_height = br.y - tl.y;
        client_tile_width = br.x - tl.x;
        client_tile_height = br.y - tl.y;

        if (!client_tile_width) {
            client_tile_width = TILE_WIDTH;
        }

        if (!client_tile_height) {
            client_tile_height = TILE_HEIGHT;
        }

        wid_new_grid(wid_game_map_client_grid_container,
                     TILES_MAP_WIDTH,
                     TILES_MAP_HEIGHT, client_tile_width, client_tile_height);
    }

    wid_visible(wid_game_map_client_window, 0);

    client_level = level_new(wid_game_map_client_grid_container, 0);
    if (!client_level) {
        WARN("failed to load level");
        return;
    }

    wid_game_map_client_score_update(client_level);
}

void wid_game_map_client_wid_destroy (void)
{
    FINI_LOG("Destroy game map");

    if (client_level) {
        level_destroy(&client_level);
    }

    if (wid_game_map_client_window) {
        wid_destroy(&wid_game_map_client_window);
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
wid_game_map_client_replace_tile (widp w, int32_t x, int32_t y, thingp thing)
{
    tree_rootp thing_tiles;
    const char *tilename;
    tilep tile;
    widp child;

    verify(w);

    /*
     * Grow tl and br to fit the template thing. Use the first tile.
     */
    thing_templatep thing_template = thing_get_template(thing);
    if (!thing_template) {
        DIE("no thing template");
    }

    thing_tiles = thing_template_get_tiles(thing_template);
    if (!thing_tiles) {
        DIE("thing template [%s] has no tiles",
            thing_template_short_name(thing_template));
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
            thing_template_short_name(thing_template));
    }

    /*
     * Make a new thing.
     */
    child = wid_new_square_button(wid_game_map_client_grid_container,
                                  "map tile");

    wid_set_text_lhs(child, true);
    wid_set_text_top(child, true);
    wid_set_mode(child, WID_MODE_NORMAL);
    wid_set_color(child, WID_COLOR_TEXT, STEELBLUE);
    wid_set_color(child, WID_COLOR_TL, BLACK);
    wid_set_color(child, WID_COLOR_BG, BLACK);
    wid_set_color(child, WID_COLOR_BR, BLACK);
    wid_set_text_outline(child, true);
    wid_set_font(child, small_font);
    wid_set_no_shape(child);

    /*
     * "paint" the thing.
     */
    wid_game_map_client_set_thing_template(child, thing_template);

    wid_set_thing(child, thing);

    thing_client_wid_update(thing, x, y, false /* smooth */);

    /*
     * This adds it to the grid wid.
     */
    wid_update(child);

    return (child);
}

void wid_game_map_client_score_update (levelp level)
{
    if (!player) {
        return;
    }

    if (!wid_game_map_client_window) {
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
            wid_new_square_button(wid_game_map_client_window, "scoreline top");

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

        msg_player_state *p = client_get_player(y);
        name_title = p->pclass;
        if (!p->pclass[0]) {
            name_title = "No player";
        }

        switch (y) {
        case 0:
            c = RED;
            break;
        case 1:
            c = SKYBLUE;
            break;
        case 2:
            c = YELLOW;
            break;
        case 3:
            c = GREEN;
            break;
        }

        /*
         * Score
         */
        char *tmp = dynprintf("%06u", p->score);

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
        tmp = dynprintf("%06u", p->health);

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
                    level_get_title(level), 0.5, 0.07, small_font);

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
        widp wid = wid_new_square_button(wid_scoreline_container_top, "title");

        fpoint tl = { 0.0, 0.0 };
        fpoint br = { 1.0, 0.20 };

        wid_set_tl_br_pct(wid, tl, br);

        wid_set_tex(wid, 0, "title_small");

        wid_set_mode(wid, WID_MODE_NORMAL);
        wid_set_color(wid, WID_COLOR_TL, WHITE);
        wid_set_color(wid, WID_COLOR_BR, WHITE);
        wid_set_color(wid, WID_COLOR_BG, WHITE);

        wid_update(wid);
        wid_raise(wid);
    }

    wid_update_mouse();
}
