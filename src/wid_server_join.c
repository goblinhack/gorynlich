/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "color.h"
#include "wid_server_join.h"
#include "wid_game_map_client.h"
#include "wid_server_create.h"
#include "wid_intro.h"
#include "string.h"
#include "marshal.h"
#include "socket.h"
#include "client.h"
#include "string_ext.h"

static const char *config_file = "gorynlich-remote-servers.txt";

static widp wid_server_join_window;
static widp wid_server_join_container;
static widp wid_server_join_window_container;
static widp wid_server_join_container_vert_scroll;
static uint8_t wid_server_join_init_done;

static void wid_server_join_create(uint8_t redo);
static void wid_server_join_destroy(void);
static uint8_t wid_server_load_remote_server_list(void);

static uint8_t user_is_typing;

typedef struct server_ {
    tree_key_four_int tree;

    IPaddress ip;
    uint16_t port;
    char *host;
    char *host_and_port_str;
    char *tooltip;
    uint8_t quality;
    uint32_t avg_latency;
    uint32_t min_latency;
    uint32_t max_latency;
    socketp socket;
    uint8_t walked;
    char name[SMALL_STRING_LEN_MAX];
} server;

static void wid_server_join_destroy_internal(server *node);

static tree_rootp remote_servers;

static void server_add (const server *s_in)
{
    server *s;

    if (!remote_servers) {
        remote_servers = tree_alloc(TREE_KEY_FOUR_INTEGER, 
                                    "TREE ROOT: remote servers");
    }

    s = (typeof(s)) myzalloc(sizeof(*s), "TREE NODE: server");

    memcpy(s, s_in, sizeof(*s));
    s->host = dupstr(s_in->host, "server hostname");
    s->port = s_in->port;

    if ((SDLNet_ResolveHost(&s->ip, 
                            s->host,
                            s->port)) == -1) {
        LOG("Cannot resolve host %s port %u", 
            s->host, 
            s->port);

        s->host_and_port_str = dynprintf("%s:%u", s->host, s->port);
    } else {
        s->host_and_port_str = iptodynstr(s->ip);
    }

    /*
     * Connector.
     */
    if (is_client) {
        socketp sp = socket_connect_from_client(s->ip);
        if (sp) {
            s->quality = socket_get_quality(sp);
            s->avg_latency = socket_get_avg_latency(sp);
            s->min_latency = socket_get_min_latency(sp);
            s->max_latency = socket_get_max_latency(sp);
            strncpy(s->name, socket_get_name(sp),
                    sizeof(s->name) - 1);
        }
    }

    s->tree.key2 = s->quality;
    s->tree.key3 = s->avg_latency;
    s->tree.key4 = SDLNet_Read16(&s->ip.port);
    s->tree.key5 = SDLNet_Read32(&s->ip.host);

    /*
     * Check this ip and port combination is not added already.
     */
    uint8_t collision = false;

    do {
        server *sw;

        collision = false;

        TREE_WALK(remote_servers, sw) {
            if (cmp_address(&sw->ip, &s->ip)) {
                collision = true;
                break;
            }
        }

        if (collision) {
            s->tree.key4++;
            SDLNet_Write16(s->tree.key4, &s->ip.port);
            s->port = s->tree.key4;
        }
    } while (collision);

    if (!tree_insert(remote_servers, &s->tree.node)) {
        ERR("Cannot add host %s port %u", s->host, s->port);
        myfree(s);
        return;
    }
}

static void server_remove (server *s)
{
    if (!remote_servers) {
        return;
    }

    /*
     * Remove this socket.
     */
    TREE_WALK(remote_servers, s) {
        socketp sp = socket_find(s->ip);
        if (sp) {
            socket_disconnect(sp);
            break;
        }
    }

    wid_server_join_destroy_internal(s);
    tree_remove(remote_servers, &s->tree.node);
    myfree(s);
}

uint8_t wid_server_join_init (void)
{
    if (!wid_server_join_init_done) {
        wid_server_load_remote_server_list();
    }

    wid_server_join_init_done = true;

    return (true);
}

static void wid_server_join_destroy_internal (server *node)
{
    if (!node) {
        return;
    }

    if (node->host) {
        myfree(node->host);
        node->host= 0;
    }

    if (node->host_and_port_str) {
        myfree(node->host_and_port_str);
        node->host_and_port_str = 0;
    }

    if (node->tooltip) {
        myfree(node->tooltip);
        node->tooltip = 0;
    }
}

void wid_server_join_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (wid_server_join_init_done) {
        wid_server_join_init_done = false;

        wid_server_join_hide();

        if (remote_servers) {
            tree_destroy(&remote_servers, 
                         (tree_destroy_func)wid_server_join_destroy_internal);
        }
    }
}

void wid_server_join_hide (void)
{
    wid_server_join_destroy();

    /*
     * Leave all other sockets other than the joined on.
     */
    server *s;

    /*
     * Leave all other sockets.
     */
    TREE_WALK(remote_servers, s) {
        socketp sp = socket_find(s->ip);
        if (sp && (sp != client_joined_server)) {
            socket_disconnect(sp);
            continue;
        }
    }
}

void wid_server_join_visible (void)
{
    wid_server_join_create(false);
}

void wid_server_join_redo (uint8_t soft_refresh)
{
    if (!wid_server_join_window) {
        return;
    }

    if (user_is_typing) {
        return;
    }

    server *s;

    {
        TREE_WALK(remote_servers, s) {
            s->walked = false;
        }
    }

    TREE_WALK(remote_servers, s) {
        if (s->walked) {
            continue;
        }

        s->walked = true;

        socketp sp = socket_find(s->ip);
        if (!sp) {
            /*
             * Connector.
             */
            if (is_client) {
                socket_connect_from_client(s->ip);
            }
        }

        sp = socket_find(s->ip);
        if (!sp) {
            continue;
        }

        s->quality = socket_get_quality(sp);
        s->avg_latency = socket_get_avg_latency(sp);
        s->min_latency = socket_get_min_latency(sp);
        s->max_latency = socket_get_max_latency(sp);

        /*
         * Re-sort the server.
         */
        if (!tree_remove(remote_servers, &s->tree.node)) {
            ERR("Cannot find to re-sort host %s port %u", s->host, s->port);
        }

        s->tree.key2 = s->quality;
        s->tree.key3 = s->avg_latency;
        s->tree.key4 = SDLNet_Read16(&s->ip.port);
        s->tree.key5 = SDLNet_Read32(&s->ip.host);

        if (!tree_insert(remote_servers, &s->tree.node)) {
            ERR("Cannot re-sort host %s port %u qual %d lat %d", 
                s->host, s->port, s->quality, s->avg_latency);
        }

        if (s->tooltip) {
            myfree(s->tooltip);
        }

        msg_server_status *server_status = socket_get_server_status(sp);

        strncpy(s->name, server_status->server_name, sizeof(s->name) - 1);

        if (s->quality) {
            int size;
            int used;
            char *tmp;

            tmp = 0;
            size = 1024;

            snprintf_realloc(&tmp, &size, &used, 
                             "%%%%fmt=left$   Name            Score\n");
            snprintf_realloc(&tmp, &size, &used, 
                             "%%%%fmt=left$   ----           -------\n");

            uint32_t pi;
            uint32_t idx = 0;

            for (pi = 0; pi < MAX_PLAYERS; pi++) {
                msg_player_state *p = &server_status->players[pi];

                if (!p->name[0]) {
                    continue;
                }

                snprintf_realloc(&tmp, &size, &used, 
                                 "%%%%fmt=left$[%d] %-10s %07d\n",
                                 idx++,
                                 p->name,
                                 p->score);
            }

            snprintf_realloc(&tmp, &size, &used, "\n");

            char *tmp2 = dynprintf(
                "%%%%fmt=centerx$%s\n"
                "\n"
                "%%%%fmt=left$Average latency %u ms\n"
                "%%%%fmt=left$Minimum latency %u ms\n"
                "%%%%fmt=left$Maxumum latency %u ms\n\n"
                "%%%%fmt=left$Maxumum players %u\n"
                "%%%%fmt=left$Current players %u\n\n"
                "%s",
                server_status->server_name,
                s->avg_latency,
                s->min_latency,
                s->max_latency,
                server_status->server_max_players,
                server_status->server_current_players,
                tmp);

            s->tooltip = tmp2;
            myfree(tmp);
        } else {
            s->tooltip = dynprintf("server is down");
        }
    }

    if (soft_refresh) {
        wid_server_join_create(true);
    } else {
        wid_server_join_destroy();
        wid_server_join_create(false);
    }
}

static uint8_t wid_server_join_go_back (widp w, int32_t x, int32_t y, uint32_t button)
{
    wid_server_join_hide();
    wid_server_create_hide();
    wid_intro_visible();

    return (true);
}

static uint8_t wid_server_join (widp w, int32_t x, int32_t y, uint32_t button)
{
    server *s = wid_get_client_context(w);
    if (!s) {
        return (false);
    }

    if (!client_socket_join(s->host, 0, s->port, true /* quiet */)) {
        return (false);
    }

    wid_server_join_hide();
    wid_server_create_hide();

    /*
     * Leave all other sockets.
     */
    TREE_WALK(remote_servers, s) {
        socketp sp = socket_find(s->ip);
        if (sp && (sp != client_joined_server)) {
            socket_disconnect(sp);
            continue;
        }
    }

    wid_intro_hide();
    wid_game_map_client_visible();

    return (true);
}

static uint8_t wid_server_join_leave (widp w, int32_t x, int32_t y, 
                                      uint32_t button)
{
    server *s = wid_get_client_context(w);
    if (!s) {
        return (false);
    }

    client_socket_leave();

    /*
     * Rescan all sockets to get new stats.
     */
    TREE_WALK(remote_servers, s) {
        socketp sp = socket_find(s->ip);
        if (sp) {
            socket_connect_from_client(s->ip);
            continue;
        }
    }

    return (true);
}

static uint8_t wid_server_join_delete (widp w, int32_t x, int32_t y, 
                                       uint32_t button)
{
    server *s = wid_get_client_context(w);
    if (!s) {
        return (false);
    }

    server_remove(s);

    wid_server_save_remote_server_list();
    wid_server_join_redo(false /* hard refresh */);

    return (true);
}

static uint8_t wid_server_join_add (widp w, int32_t x, int32_t y, 
                                    uint32_t button)
{
    server s;

    memset(&s, 0, sizeof(s));

    s.host = SERVER_DEFAULT_HOST; 
    s.port = global_config.server_port; 

    server_add(&s);
    wid_server_save_remote_server_list();
    wid_server_join_redo(false /* hard refresh */);

    return (true);
}

static uint8_t wid_server_join_key_event (widp w, const SDL_KEYSYM *key)
{
    switch (key->sym) {
        case 'q':
        case SDLK_ESCAPE:
            wid_server_join_hide();
            wid_server_create_hide();
            wid_intro_visible();
            return (true);

        default:
            break;
    }

    return (false);
}

static uint8_t wid_server_join_receive_mouse_motion (
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

static uint8_t wid_server_join_hostname_mouse_down (widp w, int32_t x, int32_t y, 
                                               uint32_t button)
{
    wid_set_show_cursor(w, true);
    user_is_typing = true;

    return (true);
}

static uint8_t wid_server_join_ip_mouse_down (widp w, int32_t x, int32_t y,
                                         uint32_t button)
{
    wid_set_show_cursor(w, true);
    user_is_typing = true;

    return (true);
}

static uint8_t wid_server_join_port_mouse_down (widp w, int32_t x, int32_t y,
                                           uint32_t button)
{
    wid_set_show_cursor(w, true);
    user_is_typing = true;

    return (true);
}

/*
 * Key down etc...
 */
static uint8_t wid_server_join_hostname_receive_input (widp w, 
                                                  const SDL_KEYSYM *key)
{
    server *s;

    switch (key->sym) {
        case SDLK_ESCAPE:
            wid_server_join_hide();
            wid_server_create_hide();
            wid_intro_visible();
            return (true);

        default:
            break;
    }

    s = wid_get_client_context(w);
    if (!s) {
        return (false);
    }

    switch (key->sym) {
        case SDLK_RETURN: {
            /*
             * Change hostname.
             */
            wid_set_show_cursor(w, false);
            user_is_typing = false;

            server sn;

            memset(&sn, 0, sizeof(sn));

            sn.host = (char*) wid_get_text(w);
            if (!sn.host || !*sn.host) {
                server_remove(s);
                wid_server_save_remote_server_list();
                wid_server_join_redo(false /* hard refresh */);
                return (true);
            }

            sn.port = s->port;

            server_remove(s);
            server_add(&sn);
            wid_server_save_remote_server_list();
            wid_server_join_redo(false /* hard refresh */);

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
static uint8_t wid_server_join_ip_receive_input (widp w, const SDL_KEYSYM *key)
{
    server *s;

    switch (key->sym) {
        case SDLK_ESCAPE:
            wid_server_join_hide();
            wid_server_create_hide();
            wid_intro_visible();
            return (true);
        default:
            break;
    }

    s = wid_get_client_context(w);
    if (!s) {
        return (false);
    }

    switch (key->sym) {
        case SDLK_RETURN: {
            /*
             * Change IP address.
             */
            wid_set_show_cursor(w, false);
            user_is_typing = false;

            server sn;

            memset(&sn, 0, sizeof(sn));

            const char *ip_str = wid_get_text(w);
            int a,b,c,d;
            int success = sscanf(ip_str, "%u.%u.%u.%u", &a, &b, &c, &d);
            if (success != 4) {
                /*
                 * Fail
                 */
                MSG_BOX("Failed to parse IP address, not in A.B.C.D format");
                return (true);
            }

            if ((a > 255) || (b > 255) | (c > 255) | (d > 255)) {
                /*
                 * Fail
                 */
                MSG_BOX("Failed to parse IP address, "
                       "Each number must be in the 0 to 255 range");

                return (true);
            }

            /*
             * Create an IP address for SDL to parse.
             */
            IPaddress ipaddress = {0};
            
            uint32_t ipv4 = (a << 24) | (b << 16) | (c << 8) | d;

            SDLNet_Write32(ipv4, &ipaddress.host);

            sn.host = (char*)SDLNet_ResolveIP(&ipaddress);
            if (!sn.host) {
                /*
                 * Fail
                 */
                MSG_BOX("Failed to resolve IP address to a hostname");

                return (true);
            }

            /*
             * Replace the server.
             */
            if (!sn.host || !*sn.host) {
                server_remove(s);
                wid_server_save_remote_server_list();
                wid_server_join_redo(false /* hard refresh */);
                return (true);
            }

            sn.port = s->port;

            server_remove(s);
            server_add(&sn);
            wid_server_save_remote_server_list();
            wid_server_join_redo(false /* hard refresh */);

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
static uint8_t wid_server_join_port_receive_input (widp w, const SDL_KEYSYM *key)
{
    server *s;

    switch (key->sym) {
        case SDLK_ESCAPE:
            wid_server_join_hide();
            wid_server_create_hide();
            wid_intro_visible();
            return (true);
        default:
            break;
    }

    s = wid_get_client_context(w);
    if (!s) {
        return (false);
    }

    switch (key->sym) {
        case SDLK_RETURN: {
            /*
             * Change port address.
             */
            wid_set_show_cursor(w, false);
            user_is_typing = false;

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

            sn.host = dupstr(s->host, "wid client port change");
            sn.port = a;

            server_remove(s);
            server_add(&sn);
            wid_server_save_remote_server_list();
            wid_server_join_redo(false /* hard refresh */);
            myfree(sn.host);

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

static void wid_server_join_set_color (widp w, server *s)
{
    if (s->quality >= 90) {
        wid_set_color(w, WID_COLOR_TEXT, GREEN);
    } else if (s->quality >= 50) {
        wid_set_color(w, WID_COLOR_TEXT, YELLOW);
    } else if (s->quality >= 25) {
        wid_set_color(w, WID_COLOR_TEXT, ORANGE);
    } else if (s->quality > 0) {
        wid_set_color(w, WID_COLOR_TEXT, RED);
    } else {
        wid_set_color(w, WID_COLOR_TEXT, GRAY);
    }

    if (s->tooltip) {
        wid_set_tooltip(w, s->tooltip);
    } else {
        wid_set_tooltip(w, "Cick to edit");
    }
}

static void wid_server_join_create (uint8_t redo)
{
    float scroll_delta = 0;

    if (redo) {
        wid_destroy(&wid_server_join_window_container);
        if (wid_server_join_container_vert_scroll) {
            scroll_delta = 
                wid_get_tl_y(wid_get_parent(wid_server_join_container_vert_scroll)) -
                wid_get_tl_y(wid_server_join_container_vert_scroll);
        }

    } else {
        if (wid_server_join_window) {
            return;
        }

        widp w = wid_server_join_window = wid_new_square_window("wid server");

        fpoint tl = {0.01, 0.01};
        fpoint br = {0.99, 0.8};

        wid_set_tl_br_pct(w, tl, br);
        wid_set_font(w, small_font);

        wid_set_color(w, WID_COLOR_TEXT, WHITE);

        color c = BLACK;
        wid_set_color(w, WID_COLOR_BG, c);

        c = STEELBLUE;
        wid_set_color(w, WID_COLOR_TL, c);
        wid_set_color(w, WID_COLOR_BR, c);
        wid_set_bevel(w, 4);
        wid_set_on_key_down(w, wid_server_join_key_event);

        wid_set_on_mouse_motion(w, wid_server_join_receive_mouse_motion);
    }

    wid_server_join_window_container =
        wid_new_container(wid_server_join_window, "wid settings container");

    {
        fpoint tl = {0.0, 0.0};
        fpoint br = {1.0, 1.0};

        wid_set_tl_br_pct(wid_server_join_window_container, tl, br);
    }

    {
        widp w = wid_server_join_container =
            wid_new_container(wid_server_join_window_container,
                              "wid settings container");

        fpoint tl = {0.0, 0.15};
        fpoint br = {1.0, 0.9};

        wid_set_tl_br_pct(w, tl, br);
    }

    {
        fpoint tl = {0.0, 0.0};
        fpoint br = {1.0, 0.1};

        widp w = wid_new_container(wid_server_join_window_container,
                                       "server name");

        wid_set_tooltip(w, "Click on a server to edit it");
        wid_set_tl_br_pct(w, tl, br);

        wid_set_text(w, "Join a game");
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
    const float width2 = 0.20;
    const float width3 = 0.1;
    const float width4 = 0.23;
    const float width5 = 0.1;
    const float width6 = 0.05;
    const float width7 = 0.07;
    float width_at = 0.0;

    {
        fpoint tl = {width_at, 0.1};
        fpoint br = {width_at + width1, 0.15};

        widp w = wid_new_container(wid_server_join_window_container,
                                       "server hostname");

        wid_set_tl_br_pct(w, tl, br);

        wid_set_text(w, "Hostname");
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

        TREE_WALK_REVERSE(remote_servers, s) {
            widp w = wid_new_square_button(wid_server_join_container,
                                           "server name");

            fpoint tl = {width_at, 0.0};
            fpoint br = {width_at + width1, 0.1};

            float height = 0.08;

            wid_server_join_set_color(w, s);

            br.y += (float)i * height;
            tl.y += (float)i * height;

            wid_set_tl_br_pct(w, tl, br);
            wid_set_text(w, s->host);

            color c = BLACK;

            c.a = 100;
            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_OVER);
            wid_set_color(w, WID_COLOR_BG, SKYBLUE);

            wid_set_mode(w, WID_MODE_NORMAL);

            wid_set_text_outline(w, true);
            wid_set_font(w, vsmall_font);
            wid_set_text_lhs(w, true);

            wid_set_on_mouse_down(w, wid_server_join_hostname_mouse_down);
            wid_set_on_key_down(w, wid_server_join_hostname_receive_input);
            wid_set_client_context(w, s);

            i++;
        }
    }

    width_at += width1;

    {
        fpoint tl = {width_at, 0.1};
        fpoint br = {width_at + width2, 0.15};

        widp w = wid_new_container(wid_server_join_window_container,
                                       "server ip");

        wid_set_tl_br_pct(w, tl, br);

        wid_set_text(w, "IP");
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

        TREE_WALK_REVERSE(remote_servers, s) {
            widp w = wid_new_square_button(wid_server_join_container,
                                           "server name");

            fpoint tl = {width_at, 0.0};
            fpoint br = {width_at + width2, 0.1};

            float height = 0.08;

            wid_server_join_set_color(w, s);

            br.y += (float)i * height;
            tl.y += (float)i * height;

            wid_set_tl_br_pct(w, tl, br);

            char *tmp = iprawtodynstr(s->ip);
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
            wid_set_font(w, vsmall_font);
            wid_set_text_lhs(w, true);

            wid_set_on_mouse_down(w, wid_server_join_ip_mouse_down);
            wid_set_on_key_down(w, wid_server_join_ip_receive_input);
            wid_set_client_context(w, s);

            i++;
        }
    }

    width_at += width2;

    {
        fpoint tl = {width_at, 0.1};
        fpoint br = {width_at + width3, 0.15};

        widp w = wid_new_container(wid_server_join_window_container,
                                       "server port");

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

        TREE_WALK_REVERSE(remote_servers, s) {
            widp w = wid_new_square_button(wid_server_join_container,
                                           "server name");

            fpoint tl = {width_at, 0.0};
            fpoint br = {width_at + width3, 0.1};

            float height = 0.08;

            wid_server_join_set_color(w, s);

            br.y += (float)i * height;
            tl.y += (float)i * height;

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
            wid_set_font(w, vsmall_font);
            wid_set_text_lhs(w, true);

            wid_set_on_mouse_down(w, wid_server_join_port_mouse_down);
            wid_set_on_key_down(w, wid_server_join_port_receive_input);
            wid_set_client_context(w, s);

            i++;
        }
    }

    width_at += width3;

    {
        fpoint tl = {width_at, 0.1};
        fpoint br = {width_at + width4, 0.15};

        widp w = wid_new_container(wid_server_join_window_container,
                                       "server name");

        wid_set_tl_br_pct(w, tl, br);

        wid_set_text(w, "Name");
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

        TREE_WALK_REVERSE(remote_servers, s) {
            widp w = wid_new_square_button(wid_server_join_container,
                                           "server name");

            fpoint tl = {width_at, 0.0};
            fpoint br = {width_at + width4, 0.1};

            float height = 0.08;

            wid_server_join_set_color(w, s);

            br.y += (float)i * height;
            tl.y += (float)i * height;

            wid_set_tl_br_pct(w, tl, br);

            char *tmp = dynprintf("%s", s->name);
            wid_set_text(w, tmp);
            myfree(tmp);

            color c = BLACK;

            c.a = 100;
            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_OVER);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_NORMAL);

            wid_set_text_outline(w, true);
            wid_set_font(w, vsmall_font);
            wid_set_text_lhs(w, true);

            i++;
        }
    }

    width_at += width4;

    {
        fpoint tl = {width_at, 0.1};
        fpoint br = {width_at + width5, 0.15};

        widp w = wid_new_container(wid_server_join_window_container,
                                       "server quality");

        wid_set_tl_br_pct(w, tl, br);

        wid_set_text(w, "Qual");
        wid_set_font(w, small_font);

        wid_set_color(w, WID_COLOR_BG, BLACK);
        wid_set_color(w, WID_COLOR_TL, STEELBLUE);
        wid_set_color(w, WID_COLOR_BR, STEELBLUE);
        wid_set_square(w);
        wid_set_bevelled(w, true);
        wid_set_bevel(w, 2);
        wid_set_text_outline(w, true);
        wid_set_text_centerx(w, true);
    }

    {
        uint32_t i = 0;
        server *s;

        TREE_WALK_REVERSE(remote_servers, s) {
            widp w = wid_new_square_button(wid_server_join_container,
                                           "server quality");

            fpoint tl = {width_at, 0.0};
            fpoint br = {width_at + width5, 0.1};

            float height = 0.08;

            wid_server_join_set_color(w, s);

            br.y += (float)i * height;
            tl.y += (float)i * height;

            wid_set_tl_br_pct(w, tl, br);

            char *tmp = dynprintf("%u/%u", s->quality, s->avg_latency);
            wid_set_text(w, tmp);
            myfree(tmp);

            color c = BLACK;

            c.a = 100;
            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_OVER);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_NORMAL);

            wid_set_text_outline(w, true);
            wid_set_font(w, vsmall_font);
            wid_set_text_centerx(w, true);

            i++;
        }
    }

    width_at += width5;

    {
        uint32_t i = 0;
        server *s;

        TREE_WALK_REVERSE(remote_servers, s) {
            socketp sp = socket_find(s->ip);

            widp w = wid_new_rounded_small_button(wid_server_join_container,
                                           "server remove");

            fpoint tl = {width_at, 0.0};
            fpoint br = {width_at + width6, 0.1};

            float height = 0.08;

            wid_set_color(w, WID_COLOR_TEXT, RED);

            br.y += (float)i * height;
            tl.y += (float)i * height;

            wid_set_tl_br_pct(w, tl, br);

            if (sp && (sp == client_joined_server)) {
                wid_set_text(w, "-");
            } else {
                wid_set_on_mouse_down(w, wid_server_join_delete);
                wid_set_text(w, "Del");
                wid_set_tooltip(w, "Remove this server from the list");
            }

            wid_set_font(w, vsmall_font);
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

    width_at += width6;

    {
        uint32_t i = 0;
        server *s;

        TREE_WALK_REVERSE(remote_servers, s) {
            widp w = wid_new_rounded_small_button(wid_server_join_container,
                                           "server join");

            fpoint tl = {width_at, 0.0};
            fpoint br = {width_at + width7, 0.1};

            float height = 0.08;

            if (s->quality > 0) {
                wid_set_color(w, WID_COLOR_TEXT, GREEN);
            } else {
                wid_set_color(w, WID_COLOR_TEXT, WHITE);
            }

            br.y += (float)i * height;
            tl.y += (float)i * height;

            wid_set_tl_br_pct(w, tl, br);

            socketp sp = socket_find(s->ip);
            if (sp && (sp == client_joined_server)) {
                wid_set_text(w, "Leave");
                wid_set_tooltip(w, "Exit this game");
                wid_set_on_mouse_down(w, wid_server_join_leave);
            } else {
                wid_set_text(w, "Join");
                wid_set_tooltip(w, "Try to join the game on this server");
                wid_set_on_mouse_down(w, wid_server_join);
            }

            wid_set_font(w, vsmall_font);
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
        fpoint tl = {0.7, 0.90};
        fpoint br = {0.99, 0.99};

        widp w = wid_new_rounded_small_button(wid_server_join_window_container,
                                              "wid server go back");
        wid_raise(w);

        wid_set_tl_br_pct(w, tl, br);

        wid_set_text(w, "Go back");
        wid_set_font(w, small_font);
        wid_set_color(w, WID_COLOR_TEXT, STEELBLUE);
        wid_set_color(w, WID_COLOR_BG, BLACK);

        wid_set_text_outline(w, true);

        wid_set_on_mouse_up(w, wid_server_join_go_back);
    }

    {
        fpoint tl = {0.5, 0.90};
        fpoint br = {0.69, 0.99};

        widp w = wid_new_rounded_small_button(wid_server_join_window_container,
                                              "wid server add");

        wid_set_tl_br_pct(w, tl, br);

        wid_set_text(w, "Add");
        wid_set_tooltip(w, "Add a new server to the list");
        wid_set_font(w, small_font);
        wid_set_color(w, WID_COLOR_TEXT, STEELBLUE);
        wid_set_color(w, WID_COLOR_BG, BLACK);

        wid_set_text_outline(w, true);
        wid_raise(w);

        wid_set_on_mouse_up(w, wid_server_join_add);
    }

    wid_server_join_container_vert_scroll =
        wid_new_vert_scroll_bar(wid_server_join_window, 
                                wid_server_join_container);

    wid_visible(wid_get_parent(wid_server_join_container_vert_scroll), 1);
    wid_visible(wid_server_join_container_vert_scroll, 1);

    if (redo) {
        wid_move_delta(wid_server_join_container_vert_scroll, 0, -scroll_delta);
    }

    wid_update(wid_server_join_window);
}

void wid_server_join_destroy (void)
{
    if (wid_server_join_window) {
        wid_destroy(&wid_server_join_window);
    }

    user_is_typing = false;
}

static uint8_t demarshal_servers (demarshal_p ctx, server *s)
{
    uint8_t rc;

    rc = true;

    rc = rc && GET_OPT_NAMED_STRING(ctx, "host", s->host);
    rc = rc && GET_OPT_NAMED_UINT16(ctx, "port", s->port);

    return (rc);
}

static void marshal_servers (marshal_p ctx, server *s)
{
    char *host = s->host;
    uint16_t port = SDLNet_Read16(&s->ip.port);

    if (s->host) {
        PUT_NAMED_STRING(ctx, "host", host);
        PUT_NAMED_INT16(ctx, "port", port);
    }
}

uint8_t wid_server_save_remote_server_list (void)
{
    char *file = dynprintf("%s", config_file);
    marshal_p ctx;

    ctx = marshal(file);
    if (!ctx) {
        MSG_BOX("Failed to save: %s", file);
        myfree(file);
        return (false);
    }

    server *s;

    TREE_WALK(remote_servers, s) {
        marshal_servers(ctx, s);
    }

    if (marshal_fini(ctx) < 0) {
        ERR("Failed to finalize: %s", file);
        myfree(file);

        return (false);
    }

    myfree(file);

    return (true);
}

static uint8_t wid_server_load_remote_server_list (void)
{
    char *file = dynprintf("%s", config_file);
    demarshal_p ctx;

    if (remote_servers) {
        tree_destroy(&remote_servers, 0);
    }

    DBG("Load %s", file);

    server s;

    if ((ctx = demarshal(file))) {
        memset(&s, 0, sizeof(s));

        while (demarshal_servers(ctx, &s)) {
            server_add(&s);
            myfree(s.host);
        }

        demarshal_fini(ctx);
    }

    myfree(file);

    return (true);
}
