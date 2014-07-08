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
#include "string.h"
#include "wid_textbox.h"
#include "wid_game_map_client.h"
#include "wid_intro.h"
#include "thing.h"
#include "level.h"
#include "server.h"
#include "client.h"
#include "socket.h"
#include "timer.h"
#include "time.h"
#include "thing_tile.h"
#include "wid_chat.h"
#include "wid_game_quit.h"
#include "wid_notify.h"

levelp client_level;
widp wid_game_map_client_window;
widp wid_game_map_client_grid_container;
static widp wid_game_map_client_horiz_scroll;
static widp wid_game_map_client_vert_scroll;
#define MAX_SHORTCUTS 10
static thing_templatep wid_key_shortcuts[MAX_SHORTCUTS];
static thing_templatep wid_key_shortcuts_user[MAX_SHORTCUTS];
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

uint8_t wid_game_map_client_init (void)
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
        LOG("Client: Map hide");

        wid_hide(wid_game_map_client_window, 0);

        wid_detach_from_grid(wid_game_map_client_grid_container);

        wid_destroy_grid(wid_game_map_client_grid_container);

        wid_destroy(&wid_game_map_client_grid_container);

        wid_chat_hide();
    }
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
                (action_timer_destroy_callback)0,
                0, /* context */
                "join server",
                100,
                0 /* jitter */);
    }
}

void wid_game_map_client_visible (void)
{
    if (!client_joined_server) {
        action_timer_create(
                &wid_timers,
                (action_timer_callback)wid_game_map_client_start_cb,
                (action_timer_destroy_callback)0,
                0, /* context */
                "join server",
                100,
                0 /* jitter */);
    }

    wid_game_map_client_wid_create();
}

static uint8_t wid_game_map_client_receive_mouse_motion (
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

    /*
     * Block moving the window.
     */
    return (true);
}

void wid_game_map_client_scroll_adjust (uint8_t adjust) 
{
    widp w = player->wid;
    if (!w) {
        return;
    }

    uint32_t gridw;
    uint32_t gridh;

    wid_get_grid_dim(wid_game_map_client_grid_container, &gridw, &gridh);

    double fgridw = (double)gridw;
    double fgridh = (double)gridh;

    double winw = wid_get_width(wid_game_map_client_window) * 0.75;
    double winh = wid_get_height(wid_game_map_client_window);

    gridw -= winw;
    gridh -= winh;

    double playerx = wid_get_cx(w);
    double playery = wid_get_cy(w);

    playerx -= winw / 2.0;
    playery -= winh / 2.0;

    playerx /= fgridw;
    playery /= fgridh;

    static double last_playery;
    static double last_playerx;

    if (adjust || (last_playery != playery)) {
        wid_move_to_vert_pct(wid_game_map_client_vert_scroll, playery);
    }

    if (adjust || (last_playerx != playerx)) {
        wid_move_to_horiz_pct(wid_game_map_client_horiz_scroll, playerx);
    }

    last_playerx = playerx;
    last_playery = playery;
}

uint8_t wid_game_map_client_player_move (void)
{
#if SDL_MAJOR_VERSION == 1 && SDL_MINOR_VERSION == 2 /* { */
    uint8_t *state = SDL_GetKeyState(0);

    uint8_t right = state[SDLK_RIGHT] ? 1 : 0;
    uint8_t left  = state[SDLK_LEFT] ? 1 : 0;
    uint8_t up    = state[SDLK_UP] ? 1 : 0;
    uint8_t down  = state[SDLK_DOWN] ? 1 : 0;
    uint8_t fire  = state[SDLK_SPACE] ? 1 : 0;
#else /* } { */
    const uint8_t *state = SDL_GetKeyboardState(0);

    uint8_t right = state[SDL_SCANCODE_RIGHT] ? 1 : 0;
    uint8_t left  = state[SDL_SCANCODE_LEFT] ? 1 : 0;
    uint8_t up    = state[SDL_SCANCODE_UP] ? 1 : 0;
    uint8_t down  = state[SDL_SCANCODE_DOWN] ? 1 : 0;
    uint8_t fire  = state[SDL_SCANCODE_SPACE] ? 1 : 0;
#endif /* } */

    if (!player) {
        return (false);
    }

    if (!client_joined_server) {
        return (false);
    }

    if (!up && !down && !left && !right && !fire) {
        return (false);
    }

    /*
     * If no longer visible it may mean we have finished the level and are 
     * waiting for others to finish.
     */
    if (!thing_is_visible(player)) {
        return (false);
    }

    /*
     * Check if we are allowed to fire our gun again so soon.
     */
    if (fire) {
        if (!player->weapon) {
            fire = 0;
        }

        if (fire) {
            static uint32_t last_fired = 0;

            uint32_t delay = 
                thing_template_get_weapon_fire_delay_tenths(player->weapon);
            
            if (!time_have_x_tenths_passed_since(delay, last_fired)) {
                fire = 0;
            }

            if (fire) {
                last_fired = time_get_time_cached();
            }
        }
    }

    double x = player->x;
    double y = player->y;

    double delta = 0.10;

    x += (double)right * delta;
    x -= (double)left * delta;
    y -= (double)up * delta;
    y += (double)down * delta;

    thing_client_move(player, x, y, up, down, left, right, fire);

    /*
     * If no key then we allow the console.
     */
    return (up || down || left || right || fire);
}

static uint8_t wid_game_map_key_event (widp w, const SDL_KEYSYM *key)
{
    thing_templatep thing_template;
    uint32_t shortcut;

    switch (key->sym) {
    case SDLK_0:
    case SDLK_1:
    case SDLK_2:
    case SDLK_3:
    case SDLK_4:
    case SDLK_5:
    case SDLK_6:
    case SDLK_7:
    case SDLK_8:
    case SDLK_9:
        shortcut = ((uint32_t)key->sym) - SDLK_0;

        thing_template = wid_key_shortcuts[shortcut];
        if (thing_template) {
            socket_tx_player_action(client_joined_server, player, 
                                    PLAYER_ACTION_USE,
                                    thing_template_to_id(thing_template));
            return (true);
        }

        MSG(WARNING, "No carried item is using that key");
        break;

    case 'q':
        wid_game_quit_visible();
        return (true);

    default:
        break;
    }

    return (false);
}

static int32_t
wid_game_map_find_shortcut (thing_templatep temp)
{
    uint32_t i;

    for (i = 0; i < MAX_SHORTCUTS; i++) {
        if (wid_key_shortcuts[i] == temp) {
            return (i);
        }
    }

    return (-1);
}

static void
wid_game_map_update_shortcuts (void)
{
    uint32_t i;
    uint32_t shortcut;

    if (!player) {
        return;
    }

    memcpy(wid_key_shortcuts, wid_key_shortcuts_user, 
           sizeof(wid_key_shortcuts));

    shortcut = 0;
    for (i = 0; i < THING_MAX; i++) {
        if (!player->carrying[i]) {
            continue;
        }

        /*
         * Is this thing used with a shortcut key?
         */
        thing_templatep temp = id_to_thing_template(i);
        if (!thing_template_is_valid_for_shortcut_key(temp)) {
            continue;
        }

        /*
         * Already has a key?
         */
        if (wid_game_map_find_shortcut(temp) != -1) {
            continue;
        }

        /*
         * If this key is grabbed by the user, try the next key.
         */
        while (wid_key_shortcuts_user[shortcut]) {
            shortcut++;
            if (shortcut >= MAX_SHORTCUTS) {
                return;
            }
        }

        wid_key_shortcuts[shortcut++] = temp;
        if (shortcut >= MAX_SHORTCUTS) {
            return;
        }
    }
}

static void
wid_game_map_new_shortcut (uint32_t shortcut, thing_templatep temp)
{
    uint32_t i;

    /*
     * Check this thing is not used by another key elsewhere.
     */
    for (i = 0; i < MAX_SHORTCUTS; i++) {
        if (wid_key_shortcuts_user[i] == temp) {
            wid_key_shortcuts_user[i] = 0;
        }

        if (wid_key_shortcuts[i] == temp) {
            wid_key_shortcuts[i] = 0;
        }
    }

    if (shortcut >= MAX_SHORTCUTS) {
        return;
    }

    wid_key_shortcuts_user[shortcut] = temp;

    wid_game_map_update_shortcuts();
}

static uint8_t 
wid_game_map_item_shortcut_key_event_common (const SDL_KEYSYM *key,
                                             thing_templatep thing_template)
{
    uint32_t shortcut;

    switch (key->sym) {
    case SDLK_0:
    case SDLK_1:
    case SDLK_2:
    case SDLK_3:
    case SDLK_4:
    case SDLK_5:
    case SDLK_6:
    case SDLK_7:
    case SDLK_8:
    case SDLK_9:

        shortcut = ((uint32_t)key->sym) - SDLK_0;

        /*
         * Replace the old key.
         */
        wid_game_map_new_shortcut(shortcut, thing_template);

        /*
         * Redo the widgets with the new shortcut key
         */
        wid_game_map_client_score_update(client_level, true /* redo */);

        MSG(INFO, "Shortcut key changed to %u", shortcut);
        return (true);

    default:
        break;
    }

    return (false);
}

static uint8_t 
wid_game_map_item_mouse_event_common (uint32_t button,
                                      thing_templatep thing_template)
{
    if (button == 1) {
        socket_tx_player_action(client_joined_server, player, 
                                PLAYER_ACTION_USE,
                                thing_template_to_id(thing_template));
    } else{
        socket_tx_player_action(client_joined_server, player, 
                                PLAYER_ACTION_DROP,
                                thing_template_to_id(thing_template));
    }

    return (true);
}

static uint8_t wid_game_map_item_shortcut_key_event (widp w, 
                                                     const SDL_KEYSYM *key)
{
    thing_templatep thing_template = wid_get_client_context(w);

    if (wid_get_mode(w) != WID_MODE_OVER) {
        return (false);
    }

    return (wid_game_map_item_shortcut_key_event_common(key, thing_template));
}

static uint8_t wid_game_map_item_shortcut_mouse_event (widp w, 
                                                       int32_t x, int32_t y,
                                                       uint32_t button)
{
    thing_templatep thing_template = wid_get_client_context(w);

    return (wid_game_map_item_mouse_event_common(button, thing_template));
}

static uint8_t wid_game_map_item_key_event (widp w, const SDL_KEYSYM *key)

{
    thing_templatep thing_template;

    if (wid_get_mode(w) != WID_MODE_OVER) {
        return (false);
    }
    thing_template = wid_get_thing_template(w);

    return (wid_game_map_item_shortcut_key_event_common(key, thing_template));
}

static uint8_t wid_game_map_item_mouse_event (widp w, int32_t x, int32_t y, 
                                              uint32_t button)
{
    thing_templatep thing_template;

    thing_template = wid_get_thing_template(w);

    return (wid_game_map_item_mouse_event_common(button, thing_template));
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

    wid_notify_flush();

    LOG("Client: Create map");

    {
        fpoint tl = {0.0f, 0.0f};
        fpoint br = {1.0f, 1.0f};

        wid_game_map_client_window = 
                        wid_new_square_window("wid_game_map_client");
        wid_set_movable(wid_game_map_client_window, false);
        wid_set_do_not_raise(wid_game_map_client_window, true);
        wid_set_no_shape(wid_game_map_client_window);

        wid_set_mode(wid_game_map_client_window, WID_MODE_NORMAL);

        wid_set_text_advance(wid_game_map_client_window, 0.9f);
        wid_set_text_scaling(wid_game_map_client_window, 2.0f);
        wid_set_text_pos(wid_game_map_client_window, true, 0.5f, 0.10f);

        wid_set_text_bot(wid_game_map_client_window, true);
        wid_set_text_lhs(wid_game_map_client_window, true);
        wid_set_tl_br_pct(wid_game_map_client_window, tl, br);

        fsize sz = {0.0f, 0.0f};
        wid_set_tex_tl(wid_game_map_client_window, sz);

        fsize sz2 = {1.0f, 1.0f};
        wid_set_tex_br(wid_game_map_client_window, sz2);

        wid_set_on_key_down(wid_game_map_client_window, 
                            wid_game_map_key_event);
    }

    {
        fpoint tl = {0.00f, 0.00f};
        fpoint br = {MAP_WINDOW_WIDTH, 1.00f};

        wid_game_map_client_grid_container =
                        wid_new_container(wid_game_map_client_window,
                                          "wid game grid container");

        wid_set_no_shape(wid_game_map_client_grid_container);

        wid_set_color(wid_game_map_client_grid_container, WID_COLOR_TL, BLACK);
        wid_set_color(wid_game_map_client_grid_container, WID_COLOR_BG, BLACK);
        wid_set_color(wid_game_map_client_grid_container, WID_COLOR_BR, BLACK);
        wid_set_on_mouse_motion(wid_game_map_client_grid_container,
                                wid_game_map_client_receive_mouse_motion);

        wid_set_tl_br_pct(wid_game_map_client_grid_container, tl, br);
        wid_set_tex(wid_game_map_client_grid_container, 0, 0);

        wid_set_on_key_down(wid_game_map_client_grid_container, 
                            wid_game_map_key_event);

        LOG("Client: Created map container window");
    }

    {
        double base_tile_width =
                ((1.0f / ((double)TILES_SCREEN_WIDTH) / TILES_CLIENT_SCALE) *
                    (double)global_config.video_gl_width);

        double base_tile_height =
                ((1.0f / ((double)TILES_SCREEN_HEIGHT) / TILES_CLIENT_SCALE) *
                    (double)global_config.video_gl_height);

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
                     MAP_WIDTH,
                     MAP_HEIGHT, client_tile_width, client_tile_height);

        LOG("Client: Created map container window grid");
    }

    client_level = level_new(wid_game_map_client_grid_container, 
                             0, 
                             false /* is_editor */,
                             false /* on_server */);

    LOG("Client: Created level");

    if (!client_level) {
        WARN("failed to load level");
    }

    wid_game_map_client_vert_scroll =
        wid_new_vert_scroll_bar(wid_game_map_client_window,
                                wid_game_map_client_grid_container);
    wid_game_map_client_horiz_scroll =
        wid_new_horiz_scroll_bar(wid_game_map_client_window,
                                 wid_game_map_client_grid_container);

    wid_visible(wid_get_parent(wid_game_map_client_vert_scroll), 0);
    wid_visible(wid_get_parent(wid_game_map_client_horiz_scroll), 0);
    wid_visible(wid_game_map_client_vert_scroll, 0);
    wid_visible(wid_game_map_client_horiz_scroll, 0);

    wid_update(wid_game_map_client_vert_scroll);
    wid_update(wid_game_map_client_horiz_scroll);

    wid_game_map_client_score_update(client_level, true /* redo */);

    wid_hide(wid_game_map_client_window, 0);

    wid_visible(wid_chat_window, 0);
}

void wid_game_map_client_wid_destroy (void)
{
    LOG("Client: Destroy game map");

    if (client_level) {
        LOG("Client: Destroy game level");

        level_destroy(&client_level, false /* keep players */);
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
wid_game_map_client_replace_tile (widp w, double x, double y, thingp t)
{
    tree_rootp thing_tiles;
    const char *tilename;
    tilep tile;
    widp child;

    verify(w);

    /*
     * Grow tl and br to fit the template thing. Use the first tile.
     */
    thing_templatep thing_template = thing_get_template(t);
    if (!thing_template) {
        DIE("no thing template");
    }

    if ((x < 0) || (y < 0) || (x >= MAP_WIDTH) || (y >= MAP_WIDTH)) {
        DIE("thing template [%s] cannot be placed at %f %f",
            thing_template_short_name(thing_template), x, y);
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
    thing_tile = (typeof(thing_tile)) thing_tile_random(thing_tiles);

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
    wid_set_font(child, small_font);
    wid_set_no_shape(child);

    /*
     * "paint" the thing.
     */
    wid_game_map_client_set_thing_template(child, thing_template);

    wid_set_thing(child, t);
    wid_set_tile(child, tile);

    thing_client_wid_update(t, x, y, false /* smooth */);

    /*
     * This adds it to the grid wid.
     */
#ifdef DEBUG_CLIENT_THING
    wid_update(child);
    char name[20];
    sprintf(name, "%d",thing->thing_id);
    wid_set_text(child,name);
#endif

    return (child);
}

void wid_game_map_client_score_update (levelp level, uint8_t redo)
{
    if (!player) {
        return;
    }

    if (redo) {
        if (wid_scoreline_container_top) {
            wid_destroy(&wid_scoreline_container_top);
        }
    }

    uint8_t update;

    if (wid_scoreline_container_top) {
        update = true;
    } else {
        update = false;
    }

    /*
     * Create the area for the scores at the top.
     */
    if (!update) {
        fpoint tl;
        fpoint br;

        tl.x = MAP_WINDOW_WIDTH;
        tl.y = 0.0;
        br.x = 1.0;
        br.y = 1.0;

        wid_scoreline_container_top =
            wid_new_container(wid_game_map_client_window, "scoreline top");

        wid_set_no_shape(wid_scoreline_container_top);
        wid_set_tl_br_pct(wid_scoreline_container_top, tl, br);

        wid_set_color(wid_scoreline_container_top, WID_COLOR_TL, BLACK);
        wid_set_color(wid_scoreline_container_top, WID_COLOR_BG, BLACK);
        wid_set_color(wid_scoreline_container_top, WID_COLOR_BR, BLACK);
    }

    double score_x = 0.75;
    double health_x = 0.90;
    double player_y_offset = 0.24; // player start y
    double next_player_y_delta = 0.1;
    double score_and_health_title_offset = 0.03;
    double score_and_health_value_offset = 0.05;

    double items_x = 0.01; // items x
    double items_y = 0.31; // items y
    double items_y_offset = 0.04;
    uint32_t item_columns = 6;
    double item_width = 0.15;
    double item_height = 0.08;

    /*
     * Print the score.
     */
    int32_t y;
    for (y = 0; y < 4; y++) {
        msg_player_state *p = client_get_player(y);

        /*
         * Score
         */
        char tmp[20];
        snprintf(tmp, sizeof(tmp), "%06u", p->score);

        static widp wid_score_container[MAX_PLAYERS];

        if (!update) {
            wid_set_no_shape(
                wid_textbox_fixed_width(wid_scoreline_container_top,
                                        &wid_score, tmp,
                                        score_x, player_y_offset, small_font));

            wid_score_container[y] = wid_score;
        } else {
            wid_set_text(wid_score_container[y], tmp);
        }

        /*
         * Health
         */
        snprintf(tmp, sizeof(tmp), "%06u", p->health);

        /*
         * If we see a drop in health, flash the widget.
         */
        thingp t = thing_client_find(p->thing_id);
        if (t) {
            widp w = t->wid;

            if (w && t->health && (p->health < t->health)) {
                /*
                 * Flash briefly red.
                 */
                wid_set_mode(w, WID_MODE_ACTIVE);
                wid_set_color(w, WID_COLOR_BLIT, RED);
            }

            t->health = p->health;
        }

        static widp wid_health_container[MAX_PLAYERS];

        if (!update) {
            wid_set_no_shape(
                wid_textbox_fixed_width(wid_scoreline_container_top,
                                        &wid_health, tmp,
                                        health_x, player_y_offset, small_font));

            wid_health_container[y] = wid_health;
        } else {
            wid_set_text(wid_health_container[y], tmp);
        }

        if (update) {
            continue;
        }

        color c;
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
         * Score title
         */
        widp wid_score_title_container;

        wid_score_title_container = wid_textbox_fixed_width(
                                    wid_scoreline_container_top,
                                    &wid_score_title,
                                    "SCORE", 
                                    score_x, player_y_offset - score_and_health_title_offset,
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
                                    health_x, player_y_offset - score_and_health_title_offset,
                                    vsmall_font);

        wid_set_no_shape(wid_health_title_container);

        /*
         * Score title
         */
        widp wid_name_title_container;

        char *name_title;
        if (p->pclass[0] && p->name[0]) {
            name_title = dynprintf("%s, %s", p->name, p->pclass);
        } else if (p->pclass[0]) {
            name_title = dynprintf("%s", p->pclass);
        } else if (p->name[0]) {
            name_title = dynprintf("%s", p->name);
        } else {
            name_title = 0;
        }

        wid_name_title_container = wid_textbox_fixed_width(
                                    wid_scoreline_container_top,
                                    &wid_name_title,
                                    name_title ? name_title : "No player",
                                    (score_x + health_x) / 2,
                                    player_y_offset - score_and_health_value_offset,
                                    vsmall_font);
        if (name_title) {
            myfree(name_title);
        }

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

        {
            uint32_t c;
            uint32_t count = 0;

            thingp t = thing_client_find(p->thing_id);
            if (!t) {
                player_y_offset += next_player_y_delta;
                continue;
            }

            /*
             * Only print the items of the local player.
             */
            if (t == player) {
                wid_game_map_update_shortcuts();

                for (c = 0; c < THING_MAX; c++) {
                    if (!t->carrying[c]) {
                        continue;
                    }

                    widp w;

                    /*
                     * It is faster to use square here, but this looks better.
                     */
#ifdef MORE_SPEED_LESS_VISUALS
                    w = wid_new_square_button(
                                        wid_scoreline_container_top,
                                        "item");
#else
                    w = wid_new_rounded_small_button(
                                        wid_scoreline_container_top,
                                        "item");
#endif
                    fpoint tl;
                    fpoint br;

                    tl.x = items_x + 
                        (item_width * (count % item_columns));
                    tl.y = items_y + 
                        next_player_y_delta*(double)y - items_y_offset + 
                        (item_height * (count / item_columns));

                    br.x = tl.x + item_width * 0.9;
                    br.y = tl.y + item_height;

                    if (!(count % item_columns)) {
                        player_y_offset += item_height;
                    }

                    wid_set_tl_br_pct(w, tl, br);

                    thing_templatep temp = 
                                    id_to_thing_template(c);

                    wid_set_thing_template(w, temp);

                    thing_tilep tile = 
                            thing_tile_first(thing_template_get_tiles(temp));

                    /*
                     * If this item has a tooltip create the text.
                     */
                    const char *tooltip = thing_template_get_tooltip(temp);
                    if (!tooltip) {
                        ERR("need a tooltip defined for %s", 
                            thing_template_name(temp));
                    } else {
                        char *full_tooltip;

                        full_tooltip = strappend(
                                tooltip,
                                "\n\n%%fg=green$left click to use%%fg=reset$\n"
                                "%%fg=red$right click to drop%%fg=reset$\n");

                        /*
                         * Add on the shortcut key if one exists fo r this 
                         * thing.
                         */
                        uint32_t shortcut;
                        for (shortcut = 0; shortcut < 10; shortcut++) {
                            thing_templatep t = wid_key_shortcuts[shortcut];
                            if (t && (t == temp)) {
                                /*
                                 * Yes there is a shortcut and it matches this
                                 * thing template.
                                 */
                                char *extra = dynprintf(
                                    "\npress '%%%%fg=green$"
                                    "%c"
                                    "%%%%fg=reset$' to use\n",
                                    '0' + shortcut);

                                char *old_full_tooltip = full_tooltip;
                                full_tooltip = strappend(full_tooltip, extra);
                                myfree(old_full_tooltip);
                                myfree(extra);
                                break;
                            }
                        }

                        wid_set_tooltip(w, full_tooltip);

                        myfree(full_tooltip);
                    }

                    wid_set_tilename(w, thing_tile_name(tile));
                    wid_set_do_not_raise(w, true);

                    color col = {0};

                    if (thing_template_is_item_unusable(temp)) {
                        col = BLACK;
                    } else if (thing_template_is_weapon(temp)) {
                        col.r = 0x66;
                        col.g = 0x10;
                        col.b = 0x10;
                    } else if (thing_template_is_wearable(temp)) {
                        col.r = 0;
                        col.g = 0x66;
                        col.b = 0xff;
                    } else if (thing_template_is_food(temp)) {
                        col.r = 0;
                        col.g = 0x66;
                        col.b = 0x1f;
                    } else if (thing_template_is_treasure(temp)) {
                        col.r = 0x20;
                        col.g = 0x20;
                        col.b = 0x20;
                    }
                    col.a = 0xdf;

                    if (player->weapon && (temp == player->weapon)) {
                        wid_set_color(w, WID_COLOR_TL, RED);
                        wid_set_color(w, WID_COLOR_BR, RED);
                    } else {
                        wid_set_color(w, WID_COLOR_TL, LIGHTBLUE);
                        wid_set_color(w, WID_COLOR_BR, LIGHTBLUE);
                    }

                    wid_set_color(w, WID_COLOR_TEXT, WHITE);
                    wid_set_color(w, WID_COLOR_BG, col);
                    wid_set_bevel(w, 2);

                    wid_set_mode(w, WID_MODE_OVER);

                    wid_set_color(w, WID_COLOR_TEXT, WHITE);
                    col.a = 0xff;
                    wid_set_color(w, WID_COLOR_BG, col);

                    if (player->weapon && (temp == player->weapon)) {
                        wid_set_color(w, WID_COLOR_TL, ORANGE);
                        wid_set_color(w, WID_COLOR_BR, ORANGE);
                    } else {
                        wid_set_color(w, WID_COLOR_TL, WHITE);
                        wid_set_color(w, WID_COLOR_BR, WHITE);
                    }

                    wid_set_bevel(w, 2);

                    wid_set_mode(w, WID_MODE_NORMAL);

                    if (t->carrying[c] > 1) {
                        char tmp[20];
                        snprintf(tmp, sizeof(tmp) - 1, "%d", t->carrying[c]);

                        wid_set_text(w, tmp);
                        wid_set_font(w, vsmall_font);
                        wid_set_text_rhs(w, true);
                        wid_set_text_bot(w, true);
                    }

                    wid_set_on_mouse_down(w, wid_game_map_item_mouse_event);
                    wid_set_on_key_down(w, wid_game_map_item_key_event);

                    /*
                     * Add a shortcut key if this thing is useful to have
                     * one.
                     */
                    int32_t shortcut = wid_game_map_find_shortcut(temp);
                    if (shortcut != -1) {
                        widp w;

                        w = wid_new_square_button(wid_scoreline_container_top,
                                                  "item shortcut");

                        br.x = tl.x + ((br.x - tl.x) / 2.5);
                        br.y = tl.y + ((br.y - tl.y) / 2.5);
                        tl.x -= 0.01;
                        tl.y -= 0.01;
                        br.x -= 0.01;
                        br.y -= 0.01;

                        wid_set_tl_br_pct(w, tl, br);

                        wid_set_client_context(w, temp);

                        wid_set_color(w, WID_COLOR_TEXT, WHITE);
                        wid_set_color(w, WID_COLOR_BG, RED);
                        wid_set_color(w, WID_COLOR_TL, LIGHTBLUE);
                        wid_set_color(w, WID_COLOR_BR, LIGHTBLUE);
                        wid_set_bevel(w, 1);

                        snprintf(tmp, sizeof(tmp) - 1, "%d", shortcut);

                        wid_set_text(w, tmp);
                        wid_set_font(w, vsmall_font);
                        wid_raise(w);

                        wid_set_on_mouse_down(w, 
                                    wid_game_map_item_shortcut_mouse_event);
                        wid_set_on_key_down(w, 
                                    wid_game_map_item_shortcut_key_event);
                    }

                    count++;
                }
            }
        }

        player_y_offset += next_player_y_delta;
    }

    if (update) {
        wid_raise(wid_chat_window);
        wid_visible(wid_chat_window, 0);
        return;
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
    if (client_level) {
        char *tmp = dynprintf("Level %u", level_get_level_no(client_level));
        widp wid_level_container;

        wid_level_container = wid_textbox(wid_scoreline_container_top,
                                            &wid_level,
                                            tmp, score_x, 0.15, vsmall_font);
        myfree(tmp);

        wid_set_no_shape(wid_level_container);

        wid_set_color(wid_level, WID_COLOR_TEXT, WHITE);
    }

    wid_raise(wid_scoreline_container_top);
    wid_update(wid_scoreline_container_top);

    {
        widp wid = wid_new_container(wid_scoreline_container_top, "title");

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

    wid_set_focus(wid_game_map_client_grid_container);

    wid_raise(wid_chat_window);
    wid_visible(wid_chat_window, 0);
}
