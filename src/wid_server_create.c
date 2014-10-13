/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "color.h"
#include "wid_server_create.h"
#include "wid_intro.h"
#include "wid_server_join.h"
#include "string.h"
#include "wid_game_map_server.h"
#include "wid_game_map_client.h"
#include "socket.h"
#include "server.h"

static widp wid_server_create_window;
static widp wid_server_create_container;
static widp wid_server_create_window_container;
static uint8_t wid_server_create_init_done;

static void wid_server_create_create(uint8_t redo);
static void wid_server_create_destroy(void);
static uint8_t wid_server_load_local_server(void);

typedef struct server_ {
    tree_key_two_int tree;

    IPaddress ip;
    char *name;
    uint16_t port;
    char *tooltip;
    uint8_t walked;
    uint8_t started;
} server;

static void wid_server_create_destroy_internal(server *node);
static uint8_t wid_server_create_name_receive_input(widp w,
                                                    const SDL_KEYSYM *key);
static uint8_t wid_server_create_port_receive_input(widp w, 
                                                    const SDL_KEYSYM *key);
static uint8_t wid_server_create_max_players_receive_input(widp w, 
                                                    const SDL_KEYSYM *key);

static tree_rootp local_servers;

static void wid_server_local_server_add (const server *s_in)
{
    server *s;

    if (!local_servers) {
        local_servers = 
            tree_alloc(TREE_KEY_TWO_INTEGER, "TREE ROOT: local servers");
    }

    s = (typeof(s)) myzalloc(sizeof(*s), "TREE NODE: server");

    memcpy(s, s_in, sizeof(*s));

    if (!s_in->name) {
        s->name = dupstr("unnamed server", "default server name");
    } else {
        s->name = dupstr(s_in->name, "server name");
    }

    uint16_t port;
    uint32_t host;

    /*
     * Use 0.0.0.0 for listening always as it is different from the client
     * connecting on 127.0.0.1
     */

    /*
     * Need to resolve.
     */
    if ((SDLNet_ResolveHost(&s->ip, "0.0.0.0", s_in->port)) == -1) {
        LOG("Cannot resolve port %u", s_in->port);
    }

    port = SDLNet_Read16(&s->ip.port);
    host = SDLNet_Read32(&s->ip.host);

    s->tree.key2 = port;
    s->tree.key3 = host;

    /*
     * Check this ip and port combination is not added already.
     */
    uint8_t collision = false;

    do {
        server *sw;

        collision = false;

        TREE_WALK(local_servers, sw) {
            if (cmp_address(&sw->ip, &s->ip)) {
                collision = true;
                break;
            }
        }

        if (collision) {
            s->tree.key2++;
            SDLNet_Write16(s->tree.key2, &s->ip.port);
            s->port = s->tree.key2;
        }
    } while (collision);

    if (!tree_insert(local_servers, &s->tree.node)) {
        ERR("Cannot add port %u", s->port);
        myfree(s);
        return;
    }
}

static void server_remove (server *s)
{
    if (!local_servers) {
        return;
    }

    wid_server_create_destroy_internal(s);
    tree_remove(local_servers, &s->tree.node);
    myfree(s);
}

uint8_t wid_server_create_init (void)
{
    if (!wid_server_create_init_done) {
        wid_server_load_local_server();
    }

    wid_server_create_init_done = true;

    return (true);
}

static void wid_server_create_destroy_internal (server *node)
{
    if (node->tooltip) {
        myfree(node->tooltip);
        node->tooltip = 0;
    }

    if (node->name) {
        myfree(node->name);
        node->name = 0;
    }
}

void wid_server_create_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (wid_server_create_init_done) {
        wid_server_create_init_done = false;

        wid_server_create_hide();

        if (local_servers) {
            tree_destroy(&local_servers, 
                         (tree_destroy_func)wid_server_create_destroy_internal);
        }
    }
}

void wid_server_create_hide (void)
{
    wid_server_create_destroy();

    /*
     * Leave server socket open.
     */
}

void wid_server_create_visible (void)
{
    wid_server_create_create(false);
}

void wid_server_create_redo (void)
{
    if (!wid_server_create_window) {
        return;
    }

    server *s;

    {
        TREE_WALK(local_servers, s) {
            s->walked = false;
        }
    }

    TREE_WALK(local_servers, s) {
        if (s->walked) {
            continue;
        }

        s->started = false;
        s->walked = true;

        socketp sp = socket_find(s->ip);
        if (!sp) {
            continue;
        }

        if (sp == server_socket) {
            s->started = true;
        }

        break;
    }

    wid_server_create_destroy();
    wid_server_create_create(false);
}

static uint8_t wid_server_create_go_back (widp w, int32_t x, int32_t y, uint32_t button)
{
    wid_server_create_hide();
    wid_server_join_hide();
    wid_intro_visible();

    return (true);
}

static uint8_t wid_server_start (widp w, int32_t x, int32_t y, uint32_t button)
{
    on_server = true;

    server_stop();

    server *s;

    TREE_WALK_REVERSE(local_servers, s) {

        if (!server_start(s->ip)) {
            return (true);
        }

        break;
    }

    wid_server_create_redo();

    wid_game_map_server_visible();
    wid_game_map_client_visible();
    wid_server_create_hide();
    wid_server_join_hide();
    wid_intro_hide();

    MSG(INFO, "Server started");

    return (true);
}

static uint8_t wid_server_stop (widp w, int32_t x, int32_t y, uint32_t button)
{
    server_stop();

    on_server = false;

    wid_server_create_redo();

    return (true);
}

static uint8_t wid_server_create_key_event (widp w, const SDL_KEYSYM *key)
{
    switch (key->sym) {
        case SDLK_ESCAPE:
            wid_server_create_hide();
            wid_server_join_hide();
            wid_intro_visible();
            return (true);

        case ' ':
            if (server_socket) {
                wid_server_stop(w, 0, 0, 0);
            } else {
                wid_server_start(w, 0, 0, 0);
            }
            return (true);

        default:
            break;
    }

    return (false);
}

static uint8_t wid_server_create_receive_mouse_motion (
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

static uint8_t wid_server_create_name_mouse_down (widp w, 
                                                  int32_t x, int32_t y,
                                                  uint32_t button)
{
    wid_set_show_cursor(w, true);
    wid_set_on_key_down(w, wid_server_create_name_receive_input);

    return (true);
}

static uint8_t wid_server_create_port_mouse_down (widp w, 
                                                  int32_t x, int32_t y,
                                                  uint32_t button)
{
    wid_set_show_cursor(w, true);
    wid_set_on_key_down(w, wid_server_create_port_receive_input);

    return (true);
}

static uint8_t wid_server_create_max_players_mouse_down (widp w, 
                                                         int32_t x, int32_t y,
                                                         uint32_t button)
{
    wid_set_show_cursor(w, true);
    wid_set_on_key_down(w, wid_server_create_max_players_receive_input);

    return (true);
}

/*
 * Key down etc...
 */
static uint8_t wid_server_create_name_receive_input (widp w,
                                                     const SDL_KEYSYM *key)
{
    server *s;

    switch (key->sym) {
        case SDLK_ESCAPE:
            wid_server_create_hide();
            wid_server_join_hide();
            wid_intro_visible();
            return (true);
        default:
            break;
    }

    s = (typeof(s)) wid_get_client_context(w);
    if (!s) {
        return (false);
    }

    switch (key->sym) {
        case SDLK_RETURN: {
            /*
             * Change name.
             */
            wid_set_show_cursor(w, false);

            server sn;

            memset(&sn, 0, sizeof(sn));

            sn.name = (char*) wid_get_text(w);
            sn.port = s->port;

            strncpy(global_config.server_name, sn.name,
                    sizeof(global_config.server_name));

            if (global_config.user_server_name[0]) {
                global_config.user_server_name[0] = 0;
            }

            server_remove(s);
            wid_server_local_server_add(&sn);
            wid_server_create_redo();

            break;
        }

        default:
            break;
    }

    /*
     * Feed to the general input handler
     */
    return (wid_receive_input(w, key));
}

/*
 * Key down etc...
 */
static uint8_t wid_server_create_port_receive_input (widp w, 
                                                     const SDL_KEYSYM *key)
{
    server *s;

    switch (key->sym) {
        case SDLK_ESCAPE:
            wid_server_create_hide();
            wid_server_join_hide();
            wid_intro_visible();
            return (true);
        default:
            break;
    }

    s = (typeof(s)) wid_get_client_context(w);
    if (!s) {
        return (false);
    }

    switch (key->sym) {
        case SDLK_RETURN: {
            /*
             * Change port address.
             */
            wid_set_show_cursor(w, false);

            server sn;

            memset(&sn, 0, sizeof(sn));

            const char *port_str = wid_get_text(w);
            int a;
            int success = sscanf(port_str, "%u", &a);
            if (success != 1) {
                /*
                 * Fail
                 */
                MSG_BOX("Failed to parse port number");

                return (true);
            }

            if ((a > 65535) || (a < 1024)) {
                /*
                 * Fail
                 */
                MSG_BOX("Failed to parse port number, "
                       "must be in the 1024 to 65535 range");

                return (true);
            }

            sn.port = a;
            global_config.server_port = sn.port;
            sn.name = dupstr(s->name, "wid port change");

            server_remove(s);
            wid_server_local_server_add(&sn);
            wid_server_create_redo();
            myfree(sn.name);

            break;
        }

        default:
            break;
    }

    /*
     * Feed to the general input handler
     */
    return (wid_receive_input(w, key));
}

/*
 * Key down etc...
 */
static uint8_t wid_server_create_max_players_receive_input (widp w, 
                                                     const SDL_KEYSYM *key)
{
    server *s;

    switch (key->sym) {
        case SDLK_ESCAPE:
            wid_server_create_hide();
            wid_server_join_hide();
            wid_intro_visible();
            return (true);
        default:
            break;
    }

    s = (typeof(s)) wid_get_client_context(w);
    if (!s) {
        return (false);
    }

    switch (key->sym) {
        case SDLK_RETURN: {
            /*
             * Change max_players address.
             */
            wid_set_show_cursor(w, false);

            server sn;

            memset(&sn, 0, sizeof(sn));

            const char *max_players_str = wid_get_text(w);
            int a;
            int success = sscanf(max_players_str, "%u", &a);
            if (success != 1) {
                /*
                 * Fail
                 */
                MSG_BOX("Failed to parse max players number");

                return (true);
            }

            if (a < 0) {
                a = 0;
            }

            global_config.server_max_players = a;
            wid_server_create_redo();

            break;
        }

        default:
            break;
    }

    /*
     * Feed to the general input handler
     */
    return (wid_receive_input(w, key));
}

static void wid_server_create_set_color (widp w, server *s)
{
    socketp sp = socket_find(s->ip);
    if (sp && (sp == server_socket)) {
        wid_set_color(w, WID_COLOR_TEXT, GREEN);
    } else {
        wid_set_color(w, WID_COLOR_TEXT, GRAY);
    }

    if (s->tooltip) {
        wid_set_tooltip(w, s->tooltip);
    } else {
        wid_set_tooltip(w, "Cick to edit");
    }
}

static void wid_server_create_create (uint8_t redo)
{
    if (wid_server_create_window) {
        return;
    }

    widp w = wid_server_create_window = wid_new_square_window("wid server");

    fpoint tl = {0.01, 0.8};
    fpoint br = {0.99, 1.0};

    wid_set_tl_br_pct(w, tl, br);
    wid_set_font(w, small_font);

    wid_set_color(w, WID_COLOR_TEXT, WHITE);

    color c = BLACK;
    wid_set_color(w, WID_COLOR_BG, c);

    c = STEELBLUE;
    wid_set_color(w, WID_COLOR_TL, c);
    wid_set_color(w, WID_COLOR_BR, c);
    wid_set_bevel(w, 4);
    wid_set_on_key_down(w, wid_server_create_key_event);

    wid_set_on_mouse_motion(w, wid_server_create_receive_mouse_motion);

    wid_server_create_window_container =
        wid_new_container(wid_server_create_window, "wid settings container");

    {
        fpoint tl = {0.0, 0.0};
        fpoint br = {1.0, 1.0};

        wid_set_tl_br_pct(wid_server_create_window_container, tl, br);
    }

    {
        widp w = wid_server_create_container =
            wid_new_container(wid_server_create_window_container,
                              "wid settings container");

        fpoint tl = {0.0, 0.15};
        fpoint br = {1.0, 0.9};

        wid_set_tl_br_pct(w, tl, br);
    }

    {
        fpoint tl = {0.0, 0.0};
        fpoint br = {1.0, 0.3};

        widp w = wid_new_container(wid_server_create_window_container,
                                       "server name container");

        wid_set_tooltip(w, "Click on a server to edit it");
        wid_set_tl_br_pct(w, tl, br);

        wid_set_text(w, "Start a multiplayer game");
        wid_set_font(w, large_font);
        wid_set_color(w, WID_COLOR_TEXT, WHITE);
        wid_set_color(w, WID_COLOR_BG, BLACK);
        wid_set_color(w, WID_COLOR_TL, STEELBLUE);
        wid_set_color(w, WID_COLOR_BR, STEELBLUE);
        wid_set_square(w);
        wid_set_bevelled(w, true);
        wid_set_bevel(w, 2);
        wid_set_text_outline(w, true);
    }

    const float width1 = 0.25;
    const float width2 = 0.2;
    const float width3 = 0.1;
    const float width4 = 0.0;
    const float width5 = 0.0;
    const float width6 = 0.0;
    const float width7 = 0.45;
    float width_at = 0.0;

    {
        fpoint tl = {width_at, 0.3};
        fpoint br = {width_at + width1, 0.5};

        widp w = wid_new_container(wid_server_create_window_container,
                                       "server name container");

        wid_set_tl_br_pct(w, tl, br);

        wid_set_text(w, "name");
        wid_set_font(w, small_font);

        wid_set_color(w, WID_COLOR_BG, BLACK);
        wid_set_color(w, WID_COLOR_TL, STEELBLUE);
        wid_set_color(w, WID_COLOR_BR, STEELBLUE);
        wid_set_square(w);
        wid_set_bevelled(w, true);
        wid_set_bevel(w, 2);
        wid_set_text_outline(w, true);
    }

    {
        uint32_t i = 0;
        server *s;

        TREE_WALK_REVERSE(local_servers, s) {
            widp w = wid_new_square_button(wid_server_create_container,
                                           "server name container 2");

            fpoint tl = {width_at, 0.45};
            fpoint br = {width_at + width1, 0.75};

            wid_server_create_set_color(w, s);

            wid_set_tl_br_pct(w, tl, br);
            wid_set_text(w, s->name);

            color c = BLACK;

            c.a = 100;
            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_OVER);
            wid_set_color(w, WID_COLOR_BG, SKYBLUE);

            wid_set_mode(w, WID_MODE_NORMAL);

            wid_set_text_outline(w, true);
            wid_set_font(w, small_font);
            wid_set_text_lhs(w, true);

            wid_set_on_mouse_down(w, wid_server_create_name_mouse_down);
            wid_set_client_context(w, s);

            i++;
        }
    }

    width_at += width1;

    {
        fpoint tl = {width_at, 0.3};
        fpoint br = {width_at + width2, 0.5};

        widp w = wid_new_container(wid_server_create_window_container,
                                       "server max players container");

        wid_set_tl_br_pct(w, tl, br);

        wid_set_text(w, "Max players");
        wid_set_font(w, small_font);

        wid_set_color(w, WID_COLOR_BG, BLACK);
        wid_set_color(w, WID_COLOR_TL, STEELBLUE);
        wid_set_color(w, WID_COLOR_BR, STEELBLUE);
        wid_set_square(w);
        wid_set_bevelled(w, true);
        wid_set_bevel(w, 2);
        wid_set_text_outline(w, true);
    }

    {
        uint32_t i = 0;
        server *s;

        TREE_WALK_REVERSE(local_servers, s) {
            widp w = wid_new_square_button(wid_server_create_container,
                                           "server max players container2");

            fpoint tl = {width_at, 0.45};
            fpoint br = {width_at + width2, 0.75};

            wid_server_create_set_color(w, s);

            wid_set_tl_br_pct(w, tl, br);

            char *tmp = dynprintf("%u", global_config.server_max_players);
            wid_set_text(w, tmp);
            myfree(tmp);

            color c = BLACK;

            c.a = 100;
            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_OVER);
            wid_set_color(w, WID_COLOR_BG, SKYBLUE);

            wid_set_mode(w, WID_MODE_NORMAL);

            wid_set_text_outline(w, true);
            wid_set_font(w, small_font);
            wid_set_text_centerx(w, true);

            wid_set_on_mouse_down(w, wid_server_create_max_players_mouse_down);
            wid_set_client_context(w, s);

            i++;
        }
    }

    width_at += width2;

    {
        fpoint tl = {width_at, 0.3};
        fpoint br = {width_at + width3, 0.5};

        widp w = wid_new_container(wid_server_create_window_container,
                                       "server port container");

        wid_set_tl_br_pct(w, tl, br);

        wid_set_text(w, "Port");
        wid_set_font(w, small_font);

        wid_set_color(w, WID_COLOR_BG, BLACK);
        wid_set_color(w, WID_COLOR_TL, STEELBLUE);
        wid_set_color(w, WID_COLOR_BR, STEELBLUE);
        wid_set_square(w);
        wid_set_bevelled(w, true);
        wid_set_bevel(w, 2);
        wid_set_text_outline(w, true);
    }

    {
        uint32_t i = 0;
        server *s;

        TREE_WALK_REVERSE(local_servers, s) {
            widp w = wid_new_square_button(wid_server_create_container,
                                           "server port container2");

            fpoint tl = {width_at, 0.45};
            fpoint br = {width_at + width3, 0.75};

            wid_server_create_set_color(w, s);

            wid_set_tl_br_pct(w, tl, br);

            char *tmp = iprawporttodynstr(s->ip);
            wid_set_text(w, tmp);
            myfree(tmp);

            color c = BLACK;

            c.a = 100;
            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_OVER);
            wid_set_color(w, WID_COLOR_BG, SKYBLUE);

            wid_set_mode(w, WID_MODE_NORMAL);

            wid_set_text_outline(w, true);
            wid_set_font(w, small_font);
            wid_set_text_lhs(w, true);

            wid_set_on_mouse_down(w, wid_server_create_port_mouse_down);
            wid_set_client_context(w, s);

            i++;
        }
    }

    width_at += width3;

    width_at += width4;

    width_at += width5;

    width_at += width6;

    {
        uint32_t i = 0;
        server *s;

        TREE_WALK_REVERSE(local_servers, s) {
            widp w = wid_new_rounded_small_button(wid_server_create_container,
                                                  "server join");

            fpoint tl = {width_at, 0.2};
            fpoint br = {width_at + width7, 0.7};

            wid_set_color(w, WID_COLOR_TEXT, WHITE);

            wid_set_tl_br_pct(w, tl, br);

            socketp sp = socket_find(s->ip);
            if (sp && (sp == server_socket)) {
                wid_set_text(w, "Stop");
                wid_set_tooltip(w, "Stop the server");
                wid_set_on_mouse_down(w, wid_server_stop);
            } else {
                wid_set_text(w, "Start");
                wid_set_tooltip(w, "Start the server");
                wid_set_on_mouse_down(w, wid_server_start);
            }

            wid_set_font(w, small_font);
            color c = STEELBLUE;

            c.a = 100;
            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_color(w, WID_COLOR_BG, c);

            c = RED;
            wid_set_mode(w, WID_MODE_OVER);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_text_outline(w, true);

            wid_set_client_context(w, s);

            i++;
        }
    }

    {
        fpoint tl = {0.7, 0.7};
        fpoint br = {0.99, 0.99};

        widp w = wid_new_rounded_small_button(wid_server_create_window_container,
                                              "wid server go back");
        wid_raise(w);

        wid_set_tl_br_pct(w, tl, br);

        wid_set_text(w, "Go back");
        wid_set_font(w, small_font);
        wid_set_color(w, WID_COLOR_TEXT, STEELBLUE);
        wid_set_color(w, WID_COLOR_BG, BLACK);

        wid_set_text_outline(w, true);

        wid_set_on_mouse_down(w, wid_server_create_go_back);
    }

    wid_update(wid_server_create_window);
}

void wid_server_create_destroy (void)
{
    if (wid_server_create_window) {
        wid_destroy(&wid_server_create_window);
    }
}

static uint8_t wid_server_load_local_server (void)
{
    if (local_servers) {
        tree_destroy(&local_servers, 0);
    }

    /*
     * cli overrides the config file.
     */
    if (server_socket) {
        server s;

        memset(&s, 0, sizeof(s));

        IPaddress ip = socket_get_local_ip(server_socket);
        s.ip = ip;
        s.port = SDLNet_Read16(&ip.port);

        if (global_config.user_server_name[0]) {
            s.name = global_config.user_server_name;
        } else {
            s.name = global_config.server_name;
        }

        wid_server_local_server_add(&s);
        return (true);
    }

    server s;

    memset(&s, 0, sizeof(s));

    if (global_config.user_server_name[0]) {
        s.name = global_config.user_server_name;
    } else {
        s.name = global_config.server_name;
    }

    s.port = global_config.server_port;

    wid_server_local_server_add(&s);

    return (true);
}
