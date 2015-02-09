/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "tile.h"
#include "string_util.h"
#include "wid_textbox.h"
#include "wid_game_map_client.h"
#include "wid_player_stats.h"
#include "wid_player_info.h"
#include "wid_player_inventory.h"
#include "wid_player_action.h"
#include "wid_menu.h"
#include "thing.h"
#include "level.h"
#include "server.h"
#include "client.h"
#include "socket_util.h"
#include "timer.h"
#include "time_util.h"
#include "thing_tile.h"
#include "wid_chat.h"
#include "wid_game_quit.h"
#include "wid_notify.h"
#include "wid_player_action.h"
#include "math_util.h"

levelp client_level;
widp wid_game_map_client_window;
widp wid_game_map_client_grid_container;
static widp wid_game_map_client_horiz_scroll;
static widp wid_game_map_client_vert_scroll;
widp wid_scoreline_container_top;
widp wid_scoreline_container_bot;
widp wid_score;
widp wid_cash;
widp wid_score_title;
widp wid_cash_title;
widp wid_name_title;
widp wid_title;
widp wid_level;

double last_playery;
double last_playerx;

uint32_t client_tile_width;
uint32_t client_tile_height;

uint32_t player_action_bar_changed_at;

static void wid_game_map_client_set_thing_template (widp w, tpp t)
{
    wid_set_thing_template(w, t);

    if (tp_is_effect_pulse(t)) {
        wid_scaling_to_pct_in(w, 1.0, 1.2, 500, 9999999);
    }

    if (tp_is_effect_sway(t)) {
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

        wid_player_stats_hide(true /* fast */);
        wid_player_info_hide(true /* fast */);
        wid_player_inventory_hide(true /* fast */);
        wid_player_action_hide(true /* fast */);

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

    if (!client_socket_join(0, 0, portno, true /* quiet */)) {
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
    if (!player) {
        return;
    }

    widp w = player->wid;
    if (!w) {
        return;
    }

    uint32_t gridw;
    uint32_t gridh;

    wid_get_grid_dim(wid_game_map_client_grid_container, &gridw, &gridh);

    double fgridw = (double)gridw;
    double fgridh = (double)gridh;

    double winw = wid_get_width(wid_game_map_client_window);
    double winh = wid_get_height(wid_game_map_client_window);

    gridw -= winw;
    gridh -= winh;

    double playerx = wid_get_cx(w);
    double playery = wid_get_cy(w);

    playerx -= winw / 2.0;
    playery -= winh / 2.0;

    playerx /= fgridw;
    playery /= fgridh;

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
    uint8_t right = 0;
    uint8_t left  = 0;
    uint8_t up    = 0;
    uint8_t down  = 0;
    uint8_t fire  = 0;

    if (!sdl_shift_held) {
#if SDL_MAJOR_VERSION == 1 && SDL_MINOR_VERSION == 2 /* { */
        {
            uint8_t *state = SDL_GetKeyState(0);

            right = state[SDLK_RIGHT] ? 1 : 0;
            left  = state[SDLK_LEFT] ? 1 : 0;
            up    = state[SDLK_UP] ? 1 : 0;
            down  = state[SDLK_DOWN] ? 1 : 0;
            fire  = state[SDLK_SPACE] ? 1 : 0;
        }
#else /* } { */
        {
            const uint8_t *state = SDL_GetKeyboardState(0);

            right = state[SDL_SCANCODE_RIGHT] ? 1 : 0;
            left  = state[SDL_SCANCODE_LEFT] ? 1 : 0;
            up    = state[SDL_SCANCODE_UP] ? 1 : 0;
            down  = state[SDL_SCANCODE_DOWN] ? 1 : 0;
            fire  = state[SDL_SCANCODE_SPACE] ? 1 : 0;
        }
#endif /* } */
    }

    if (!player) {
        LOG("No player, cannot move");
        return (false);
    }

    if (wid_menu_visible) {
        LOG("Menu present, ignore moves");
        return (false);
    }

    if (!client_joined_server) {
        LOG("Have not joined server, cannot move");
        return (false);
    }

    if (!up && !down && !left && !right && !fire) {
        return (false);
    }

    if (thing_is_dead_or_dying(player)) {
        return (true);
    }

    /*
     * If no longer visible it may mean we have finished the level and are 
     * waiting for others to finish.
     */
    if (!thing_is_visible(player)) {
        LOG("Player is not visible, cannot move");
        return (false);
    }

    /*
     * Check if we are allowed to fire our gun again so soon.
     */
    if (fire) {
        tpp weapon = thing_weapon(player);

        if (!weapon) {
            THING_LOG(player, "tried to fire but no weapon");
            fire = 0;
        }

        if (fire) {
            static uint32_t last_fired = 0;

            uint32_t delay = tp_get_weapon_fire_delay_tenths(weapon);
            
            if (!time_have_x_tenths_passed_since(delay, last_fired)) {
                fire = 0;

                if (!up && !down && !left && !right) {
                    return (false);
                }
            }

            if (fire) {
                last_fired = time_get_time_ms();
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
    tpp tp;

    if (!player) {
        wid_player_stats_hide(true /* fast */);
        wid_player_info_hide(true /* fast */);
        wid_player_inventory_hide(true /* fast */);
        return (true);
    }

    int redraw_action_bar = 0;
    int action_bar_index = thing_stats_get_action_bar_index(player);

    if (key->mod & KMOD_SHIFT) {
        int weapon_switch_delta = 0;

        switch (key->sym) {
        case SDLK_LEFT:
            weapon_switch_delta = -1;
            break;
        case SDLK_RIGHT:
            weapon_switch_delta = 1;
            break;
        default:
            break;
        }

        /*
         * Weapon switch?
         */
        if (!weapon_switch_delta) {
            return (true);
        }

        /*
         * Only switch between weapons
         */
        int tries = THING_ACTION_BAR_MAX;

        while (tries-- > 0) {
            action_bar_index += weapon_switch_delta;
            if (action_bar_index < 0) {
                action_bar_index = THING_ACTION_BAR_MAX - 1;
            }

            if (action_bar_index >= THING_ACTION_BAR_MAX) {
                action_bar_index = 0;
            }

            uint32_t id = player->stats.action_bar[action_bar_index].id;
            if (!id) {
                continue;
            }

            tp = id_to_tp(id);
            if (!tp) {
                continue;
            }

            if (!tp_is_weapon(tp)) {
                continue;
            }

            /*
             * Found a weapon, switch to it.
             */
            break;
        }

        if (tries <= 0) {
            return (true);
        }

        redraw_action_bar = 1;
    } else {
        switch (key->sym) {
        case SDLK_1:
            action_bar_index = 0; 
            redraw_action_bar = 1;
            break;
        case SDLK_2:
            action_bar_index = 1; 
            redraw_action_bar = 1;
            break;
        case SDLK_3:
            action_bar_index = 2; 
            redraw_action_bar = 1;
            break;
        case SDLK_4:
            action_bar_index = 3; 
            redraw_action_bar = 1;
            break;
        case SDLK_5:
            action_bar_index = 4; 
            redraw_action_bar = 1;
            break;
        case SDLK_6:
            action_bar_index = 5; 
            redraw_action_bar = 1;
            break;
        case SDLK_7:
            action_bar_index = 6; 
            redraw_action_bar = 1;
            break;
        case SDLK_8:
            action_bar_index = 7; 
            redraw_action_bar = 1;
            break;
        case SDLK_9:
            action_bar_index = 8; 
            redraw_action_bar = 1;
            break;
        case SDLK_0:
            action_bar_index = 9; 
            redraw_action_bar = 1;
            break;
        case 'd':
            debug = !debug;
            CON("debug %d", debug);
            break;

        case '\t': {
            /*
             * Show the inventory.
             */
            thing_statsp s;

            s = &player->stats;

            if (!wid_player_stats || wid_is_hidden(wid_player_stats)) {
                wid_player_stats_visible(s, false /* fast */);
                wid_player_info_visible(s, false /* fast */);
                wid_player_inventory_visible(s, false /* fast */);
            } else {
                wid_player_stats_hide(false /* fast */);
                wid_player_info_hide(false /* fast */);
                wid_player_inventory_hide(false /* fast */);
            }
            return (true);
        }

        case 'q':
            wid_game_quit_visible();
            return (true);

        default:
            return (false);
        }
    }

    uint32_t id = player->stats.action_bar[action_bar_index].id;
    if (!id) {
        MSG(WARNING, "Nothing in that slot");
        return (true);
    }

    tp = id_to_tp(id);
    if (!tp) {
        MSG(WARNING, "No carried item is using that key");
        return (true);
    }

    if (!client_joined_server) {
        MSG(WARNING, "Not connected to server");
        return (true);
    }

    if (redraw_action_bar) {
        /*
         * Assume the server will accept the change and update locally else it 
         * looks laggy.
         *
         * If potions and the like then we do not switch to them.
         */
        if (tp_is_weapon(tp)) {
            player_action_bar_changed_at = time_get_time_ms();

            thing_stats_set_action_bar_index(player, action_bar_index);

            wid_player_action_hide(true /* fast */);
            wid_player_action_visible(&player->stats, true /* fast */);
        }

        wid_player_stats_redraw(true /* fast */);
    }

    socket_tx_player_action(client_joined_server, player, 
                            PLAYER_ACTION_USE,
                            action_bar_index);

    return (true);
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
        fpoint br = {1.00f, 1.00f};

        wid_game_map_client_grid_container =
                        wid_new_container(wid_game_map_client_window,
                                          "wid game client grid container");

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
                ((1.0f / ((double)TILES_SCREEN_WIDTH)) *
                    (double)global_config.video_gl_width);

        double base_tile_height =
                ((1.0f / ((double)TILES_SCREEN_HEIGHT)) *
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

    /*
     * Mark that we want to learn the starting stats so we can use those
     * when starting again with this player type.
     */
    global_config.starting_stats_inited = false;

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

    if (global_config.server_current_players > 1) {
        wid_visible(wid_chat_window, 0);
    }
}

void wid_game_map_client_wid_destroy (void)
{
    if (client_level) {
        LOG("Client: Destroy game level");

        level_destroy(&client_level, false /* keep players */);
    }

    if (wid_game_map_client_window) {
        LOG("Client: Destroy game window");

        wid_player_stats_hide(true /* fast */);
        wid_player_info_hide(true /* fast */);
        wid_player_inventory_hide(true /* fast */);
        wid_player_action_hide(true /* fast */);

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
wid_game_map_client_replace_tile (widp w, 
                                  double x, double y, 
                                  thingp t,
                                  tpp tp)
{
    tree_rootp thing_tiles;
    const char *tilename;
    tilep tile;
    widp child;

    verify(w);

    /*
     * Grow tl and br to fit the template thing. Use the first tile.
     */
    if (!tp) {
        tp = thing_tp(t);
        if (!tp) {
            DIE("no thing template");
        }
    }

    if ((x < 0) || (y < 0) || (x >= MAP_WIDTH) || (y >= MAP_WIDTH)) {
        DIE("thing template [%s] cannot be placed at %f %f",
            tp_short_name(tp), x, y);
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
    thing_tile = (typeof(thing_tile)) thing_tile_random(thing_tiles);

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
    child = wid_new_square_button(wid_game_map_client_grid_container,
                                  "client map tile");

    wid_set_mode(child, WID_MODE_NORMAL);
    wid_set_no_shape(child);

    /*
     * "paint" the thing.
     */
    wid_game_map_client_set_thing_template(child, tp);

    if (!t) {
        t = thing_client_local_new(tp);
    }

    wid_set_thing(child, t);
    wid_set_tile(child, tile);

    double dx = 0;
    double dy = 0;

    if (thing_is_mob_spawner(t)) {
        wid_scaling_blit_to_pct_in(child, 1.0, 1.1, 1000, 10000);
    }

    if (thing_is_explosion(t)) {
        wid_scaling_blit_to_pct_in(child, 1.0, 3.0, 500, 0);
        dx = ((double)(((int)(myrand() % 100)) - 50)) / 100.0;
        dy = ((double)(((int)(myrand() % 100)) - 50)) / 100.0;
        wid_fade_out(child, 500);
    }


    thing_client_wid_update(t, x + dx, y + dy, false /* smooth */);

    /*
     * If this is a pre-existing thing perhaps being recreated ona new level
     * then it will have a direction already. Update it.
     */
    if (thing_is_animated(t)) {
        thing_animate(t);
    }

    /*
     * This adds it to the grid wid.
     */
#ifdef DEBUG_CLIENT_THING
    wid_update(child);
    char name[20];
    sprintf(name, "%d",thing->thing_id);
    wid_set_text(child,name);
#endif

    /*
     * We've been told about the epicenter of an explsion, now emulate the 
     * blast.
     */
    if (t->is_epicenter && thing_is_explosion(t) ) {
        if ((tp->id == THING_EXPLOSION1) ||
            (tp->id == THING_EXPLOSION2) ||
            (tp->id == THING_EXPLOSION3) ||
            (tp->id == THING_EXPLOSION4)) {

            level_place_fireball(client_level, 0, t->x, t->y);

        } else if ((tp->id == THING_POISON1) ||
                   (tp->id == THING_POISON2) ||
                   (tp->id == THING_POISON3) ||
                   (tp->id == THING_POISON4)) {

            level_place_poison(client_level, 0, t->x, t->y);

        } else if ((tp->id == THING_CLOUDKILL1) ||
                   (tp->id == THING_CLOUDKILL2) ||
                   (tp->id == THING_CLOUDKILL3) ||
                   (tp->id == THING_CLOUDKILL4)) {

            level_place_cloudkill(client_level, 0, t->x, t->y);

        } else {
            ERR("unknown explosion %s", thing_logname(t));
            level_place_explosion(client_level, 0, t->x, t->y);
        }
    }

    return (child);
}

void wid_game_map_client_score_update (levelp level, uint8_t redo)
{
    if (!player) {
        return;
    }

    if (player->stats.pclass[0]) {
        wid_player_action_hide(true /* fast */);
        wid_player_action_visible(&player->stats, true /* fast */);
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

        tl.x = 0.7;
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

    double score_x = 0.82;
    double cash_x = 0.92;
    double player_y_offset = 0.14; // player start y
    double next_player_y_delta = 0.1;
    double score_and_cash_title_offset = 0.025;
    double score_and_cash_value_offset = 0.055;

    /*
     * Print the score.
     */
    for (;;) {
        msg_player_state *p = client_get_player();

        /*
         * Experience
         */
        char tmp[20];
        snprintf(tmp, sizeof(tmp), "%05u", p->stats.xp);

        static widp wid_score_container;

        if (!update) {
            wid_set_no_shape(
                wid_textbox(wid_scoreline_container_top,
                            &wid_score, tmp,
                            score_x, player_y_offset, med_font));

            wid_score_container = wid_score;
        } else {
            wid_set_text(wid_score_container, tmp);
            wid_set_text_outline(wid_score_container, true);
        }

        /*
         * cash
         */
        snprintf(tmp, sizeof(tmp), "%05u", p->stats.cash);

        static widp wid_cash_container;

        if (!update) {
            wid_set_no_shape(
                wid_textbox(wid_scoreline_container_top,
                            &wid_cash, tmp,
                            cash_x, player_y_offset, med_font));

            wid_cash_container = wid_cash;
        } else {
            wid_set_text(wid_cash_container, tmp);
            wid_set_text_outline(wid_cash_container, true);
        }

        if (update) {
            break;
        }

        /*
         * Score title
         */
        widp wid_score_title_container;

        wid_score_title_container = wid_textbox(
                                    wid_scoreline_container_top,
                                    &wid_score_title,
                                    "XP", 
                                    score_x, player_y_offset - 
                                    score_and_cash_title_offset,
                                    med_font);

        wid_set_no_shape(wid_score_title_container);

        /*
         * cash title
         */
        widp wid_cash_title_container;

        wid_cash_title_container = wid_textbox(
                                    wid_scoreline_container_top,
                                    &wid_cash_title,
                                    "$$$",  
                                    cash_x, player_y_offset - 
                                    score_and_cash_title_offset,
                                    med_font);

        wid_set_no_shape(wid_cash_title_container);

        /*
         * Score title
         */
        widp wid_name_title_container;

        char *name_title;
        if (p->stats.pclass[0] && p->stats.pname[0]) {
            name_title = dynprintf("%s, %s", p->stats.pname, p->stats.pclass);
        } else if (p->stats.pclass[0]) {
            name_title = dynprintf("%s", p->stats.pclass);
        } else if (p->stats.pname[0]) {
            name_title = dynprintf("%s", p->stats.pname);
        } else {
            name_title = 0;
        }

        wid_name_title_container = wid_textbox(
                                    wid_scoreline_container_top,
                                    &wid_name_title,
                                    name_title ? name_title : "No player",
                                    (score_x + cash_x) / 2,
                                    player_y_offset - 
                                    score_and_cash_value_offset,
                                    med_font);
        if (name_title) {
            myfree(name_title);
        }

        wid_set_no_shape(wid_name_title_container);

        wid_set_color(wid_score, WID_COLOR_TEXT, RED);
        wid_set_color(wid_cash, WID_COLOR_TEXT, GOLD);
        wid_set_color(wid_score_title, WID_COLOR_TEXT, RED);
        wid_set_color(wid_cash_title, WID_COLOR_TEXT, GOLD);

        player_y_offset += next_player_y_delta;

        break;
    }

    if (update) {
        if (global_config.server_current_players > 1) {
            wid_raise(wid_chat_window);
            wid_visible(wid_chat_window, 0);
        }
        return;
    }

    /*
     * Print the level title.
     */
    if (level_get_title(level) &&
        strcasecmp(level_get_title(level), "(null)")) {

        widp wid_title_container;

        wid_title_container = wid_textbox(
                    wid_scoreline_container_top,
                    &wid_title,
                    level_get_title(level), 0.5, 0.07, med_font);

        wid_set_no_shape(wid_title_container);

        wid_raise(wid_title_container);
        wid_set_do_not_lower(wid_title_container, true);
        wid_set_color(wid_title, WID_COLOR_TEXT, WHITE);
    }

    /*
     * Print the level.
     */
    if (client_level) {
        char *tmp = dynprintf("Level %u", level_get_level_no(client_level));
        widp wid_level_container;

        wid_level_container = wid_textbox(wid_scoreline_container_top,
                                            &wid_level,
                                            tmp, 
                                            (score_x + cash_x) / 2.0, 0.05, med_font);
        myfree(tmp);

        wid_set_no_shape(wid_level_container);

        wid_set_color(wid_level, WID_COLOR_TEXT, WHITE);
    }

    wid_raise(wid_scoreline_container_top);
    wid_update(wid_scoreline_container_top);

    wid_update_mouse();

    wid_set_focus(wid_game_map_client_grid_container);

    if (global_config.server_current_players > 1) {
        wid_raise(wid_chat_window);
        wid_visible(wid_chat_window, 0);
    }
}
