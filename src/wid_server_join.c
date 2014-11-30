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
#include "wid_choose_game_type.h"
#include "wid_intro.h"
#include "string.h"
#include "marshal.h"
#include "socket.h"
#include "client.h"
#include "string_ext.h"
#include "wid_tooltip.h"

static const char *config_file = "gorynlich-remote-servers.txt";

static widp wid_server_join_window;
static widp wid_server_join_window_go_back_button;
static widp wid_server_join_server_list_container;
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
    uint8_t auto_add;
    uint32_t avg_latency_rtt;
    uint32_t min_latency_rtt;
    uint32_t max_latency_rtt;
    uint32_t avg_latency_us_to_peer;
    uint32_t min_latency_us_to_peer;
    uint32_t max_latency_us_to_peer;
    uint32_t avg_latency_peer_to_us;
    uint32_t min_latency_peer_to_us;
    uint32_t max_latency_peer_to_us;
    socketp socket;
    uint8_t walked;
    char name[SMALL_STRING_LEN_MAX];
} server;

static widp wid_server_stats_window;
static widp wid_server_stats_window2;
static server *wid_server_stats_window_server;

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

    LOG("Add server %s", s->host_and_port_str);

    /*
     * Connector.
     */
    if (is_client) {
        socketp sp = socket_connect_from_client(s->ip);
        if (sp) {
            s->quality = socket_get_quality(sp);
            s->avg_latency_rtt = socket_get_avg_latency_rtt(sp);
            s->min_latency_rtt = socket_get_min_latency_rtt(sp);
            s->max_latency_rtt = socket_get_max_latency_rtt(sp);
            strncpy(s->name, socket_get_name(sp), sizeof(s->name) - 1);
        }
    }

    s->tree.key2 = (s->quality * 10000) - s->avg_latency_rtt;
    s->tree.key3 = s->avg_latency_rtt;
    s->tree.key4 = SDLNet_Read16(&s->ip.port);
    s->tree.key5 = SDLNet_Read32(&s->ip.host);
    s->auto_add = s_in->auto_add;

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

static void server_remove (server *s_in)
{
    if (wid_server_stats_window) {
        wid_destroy_nodelay(&wid_server_stats_window);
        wid_destroy_nodelay(&wid_server_stats_window2);
        wid_server_stats_window_server = 0;
    }

    if (!remote_servers) {
        return;
    }

    /*
     * Remove this socket.
     */
    server *s;
    TREE_WALK(remote_servers, s) {
        socketp sp = socket_find(s->ip, SOCKET_CONNECT);
        if (sp) {
            socket_disconnect(sp);
            break;
        }
    }

    CON("Remove server %s", s_in->host_and_port_str);

    wid_server_join_destroy_internal(s_in);
    tree_remove(remote_servers, &s_in->tree.node);
    myfree(s_in);
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

    if (wid_server_stats_window) {
        wid_destroy_nodelay(&wid_server_stats_window);
        wid_destroy_nodelay(&wid_server_stats_window2);
    }

    /*
     * Leave all other sockets other than the joined on.
     */
    server *s;

    /*
     * Leave all other sockets.
     */
    TREE_WALK(remote_servers, s) {
        socketp sp = socket_find(s->ip, SOCKET_CONNECT);
        if (sp && (sp != client_joined_server)) {
            socket_disconnect(sp);
            continue;
        }
    }
}

void wid_server_join_visible (void)
{
    if (!remote_servers || !tree_root_size(remote_servers)) {
        LOG("No servers found, add defaults");

        static const char *common_ips[] = {
            "0.0.0.0",
            "192.168.0.1",
            "192.168.0.2",
            "192.168.0.3",
            "192.168.0.4",
            "192.168.0.5",
            "192.168.1.1",
            "192.168.1.2",
            "192.168.1.3",
            "192.168.1.4",
            "192.168.1.5",
            "192.168.100.1",
            "192.168.100.2",
            "192.168.100.3",
            "192.168.100.4",
            "192.168.100.5",
            "10.0.0.1",
            "10.0.0.2",
            "10.0.0.3",
            "10.0.0.4",
            "10.0.0.5",
            "10.0.0.6",
            "10.0.0.7",
            "10.0.0.8",
            "10.0.0.9",
            "10.0.1.1",
            "10.0.1.2",
            "10.0.1.3",
            "10.0.1.4",
            "10.0.1.5",
            "10.0.1.6",
            "10.0.1.7",
            "10.0.1.8",
            "10.0.1.9",
        };

        int i;
        for (i = 0; i < ARRAY_SIZE(common_ips); i++) {
            server s;

            memset(&s, 0, sizeof(s));
            s.host = dupstr(common_ips[i], "an ip");
            s.port = global_config.server_port; 
            s.auto_add = 1;
            server_add(&s);
        }
    }

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

        socketp sp = socket_find(s->ip, SOCKET_CONNECT);
        if (!sp) {
            /*
             * Connector.
             */
            if (is_client) {
                socket_connect_from_client(s->ip);
            }
        }

        sp = socket_find(s->ip, SOCKET_CONNECT);
        if (!sp) {
            continue;
        }

        s->quality = socket_get_quality(sp);
        s->avg_latency_rtt = socket_get_avg_latency_rtt(sp);
        s->min_latency_rtt = socket_get_min_latency_rtt(sp);
        s->max_latency_rtt = socket_get_max_latency_rtt(sp);

        /*
         * Re-sort the server.
         */
        if (!tree_remove(remote_servers, &s->tree.node)) {
            ERR("Cannot find to re-sort host %s port %u", s->host, s->port);
        }

        s->tree.key2 = (s->quality * 10000) - s->avg_latency_rtt;
        s->tree.key3 = s->avg_latency_rtt;
        s->tree.key4 = SDLNet_Read16(&s->ip.port);
        s->tree.key5 = SDLNet_Read32(&s->ip.host);

        if (!tree_insert(remote_servers, &s->tree.node)) {
            ERR("Cannot re-sort host %s port %u qual %d lat %d", 
                s->host, s->port, s->quality, s->avg_latency_rtt);
        }

        if (s->tooltip) {
            myfree(s->tooltip);
        }

        const char *server_name = socket_get_server_name(sp);

        strncpy(s->name, server_name, sizeof(s->name) - 1);

        if (s->quality) {
            int size;
            int used;
            char *tmp;

            tmp = 0;
            size = 1024;

            uint32_t idx = 0;
            uint32_t p;

            for (p = 0; p < MAX_PLAYERS; p++) {
                const char *player_name = socket_get_other_player_name(sp, p);

                if (player_name[0]) {
                    snprintf_realloc(&tmp, &size, &used, 
                                     "%%%%fmt=left$[%d] %-10s\n",
                                     idx++,
                                     player_name);
                } else {
                    snprintf_realloc(&tmp, &size, &used, 
                                     "%%%%fmt=left$[%d] %-10s\n",
                                     idx++,
                                     "<available player slot>");
                }
            }

            snprintf_realloc(&tmp, &size, &used, "\n");

            char *tmp2;

            tmp2 = dynprintf(
                "%%%%fg=green$%%%%fmt=left$Server name\t\t%%%%fg=red$%-45s\n"
                "%%%%fg=green$%%%%fmt=left$Avg latency round trip time\t%%%%fg=red$%u ms\n"
                "%%%%fg=green$%%%%fmt=left$Min latency round trip time\t%%%%fg=red$%u ms\n"
                "%%%%fg=green$%%%%fmt=left$Max latency round trip time\t%%%%fg=red$%u ms\n"
                "%%%%fg=green$%%%%fmt=left$Maximum players\t\t%%%%fg=red$%u\n"
                "%%%%fg=green$%%%%fmt=left$Current players\t\t%%%%fg=red$%u\n\n"
                "%s",
                server_name,
                s->avg_latency_rtt,
                s->min_latency_rtt,
                s->max_latency_rtt,
                socket_get_max_players(sp),
                socket_get_current_players(sp),
                tmp);

            s->tooltip = tmp2;
            myfree(tmp);
        } else {
            s->tooltip = 0;
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
    wid_choose_game_type_visible();

    return (true);
}

static uint8_t wid_server_join (widp w, int32_t x, int32_t y, uint32_t button)
{
    server *s = (typeof(s)) wid_get_client_context(w);
    if (!s) {
        return (false);
    }

    if (!client_socket_join(s->host, 0, s->port, true /* quiet */)) {
        return (false);
    }

    wid_server_join_hide();

    /*
     * Leave all other sockets.
     */
    TREE_WALK(remote_servers, s) {
        socketp sp = socket_find(s->ip, SOCKET_CONNECT);
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
    server *s = (typeof(s)) wid_get_client_context(w);
    if (!s) {
        return (false);
    }

    client_socket_leave();

    /*
     * Rescan all sockets to get new stats.
     */
    TREE_WALK(remote_servers, s) {
        socketp sp = socket_find(s->ip, SOCKET_CONNECT);
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
    server *s = (typeof(s)) wid_get_client_context(w);
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

    s.host = (char*) SERVER_DEFAULT_HOST; 
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
        case 'b':
        case SDLK_ESCAPE:
            wid_server_join_hide();
            wid_choose_game_type_visible();
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
            wid_choose_game_type_visible();
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
            wid_choose_game_type_visible();
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
            wid_choose_game_type_visible();
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
}

static void wid_server_join_display (server *s)
{
    if (!s) {
        return;
    }

    wid_server_stats_window_server = s;

    if (wid_server_stats_window) {
        wid_destroy_nodelay(&wid_server_stats_window);
        wid_destroy_nodelay(&wid_server_stats_window2);
    }

    if (!s->tooltip) {
        return;
    }

    color c = BLACK;
    c.a = 255;

    wid_server_stats_window = wid_tooltip(s->tooltip, 0.2, 0.8, fixed_font);
    wid_move_to_pct(wid_server_stats_window, 0.05, 0.72);
    wid_move_stop(wid_server_stats_window);
    wid_set_tex(wid_server_stats_window, 0, 0);
    wid_set_square(wid_server_stats_window);
    wid_set_color(wid_server_stats_window, WID_COLOR_BG, c);
    wid_set_color(wid_server_stats_window, WID_COLOR_TL, c);
    wid_set_color(wid_server_stats_window, WID_COLOR_BR, c);
    wid_set_bevel(wid_server_stats_window, 0);

    wid_server_stats_window2 = wid_new_square_window("bars");
    fpoint tl = {0.5, 0.70};
    fpoint br = {0.9, 0.95};
    wid_set_tl_br_pct(wid_server_stats_window2, tl, br);
    wid_set_color(wid_server_stats_window2, WID_COLOR_BG, c);
    wid_set_color(wid_server_stats_window2, WID_COLOR_TL, c);
    wid_set_color(wid_server_stats_window2, WID_COLOR_BR, c);

    socketp sp = socket_find(s->ip, SOCKET_CONNECT);
    if (sp) {
        uint32_t i;

        for (i = 0; i < SOCKET_PING_SEQ_NO_RANGE; i++) {
            double dx = 1.0 / (double)SOCKET_PING_SEQ_NO_RANGE;
            
            widp w = wid_new_square_button(wid_server_stats_window2, "bar");
            wid_visible(w, 0);

            fpoint tl;
            fpoint br;

            tl.x = dx * (double)i;
            br.x = tl.x + dx;
            tl.x -= 0.01;

            tl.y = 1.0 - ((double)(sp->latency_rtt[i]) / 300.0);
            br.y = 1.0;

            if (sp->latency_rtt[i] == -1) {
                continue;
            }

            char *tmp = dynprintf("%d%% ms", sp->latency_rtt[i]);
            wid_set_tooltip(w, tmp, 0 /* font */);
            myfree(tmp);

            color c;

            if (sp->latency_rtt[i] < 50) {
                c = GREEN;
            } else if (sp->latency_rtt[i] < 100) {
                c = ORANGE;
            } else {
                c = RED;
            }

            c.a = 100;

            wid_set_bevel(w, 2);
            wid_set_tl_br_pct(w, tl, br);
            wid_set_color(w, WID_COLOR_BG, c);
            wid_set_color(w, WID_COLOR_TL, c);
            wid_set_color(w, WID_COLOR_BR, c);
        }
    }

    wid_update(wid_server_stats_window);
    wid_visible(wid_server_stats_window, 0);
    wid_raise(wid_server_stats_window);

    wid_update(wid_server_stats_window2);
    wid_visible(wid_server_stats_window2, 0);
    wid_raise(wid_server_stats_window2);
}

static void wid_server_join_mouse_over_server (widp w)
{
    server *s = (typeof(s)) wid_get_client_context(w);

    wid_server_join_display(s);
}

static uint8_t wid_server_join_mouse_motion_over_server (
                                    widp w,
                                    int32_t x, int32_t y,
                                    int32_t relx, int32_t rely,
                                    int32_t wheelx, int32_t wheely)
{
    server *s = (typeof(s)) wid_get_client_context(w);

    wid_server_join_display(s);

    return (true);
}

static void wid_server_join_mouse_over_end_server (widp w)
{
    wid_server_stats_window_server = 0;
}

static void wid_server_join_create (uint8_t redo)
{
    float scroll_delta = 0;

    wid_server_stats_window_server = 0;

    if (redo) {
        wid_destroy_nodelay(&wid_server_join_window_container);
        if (wid_server_join_container_vert_scroll) {
            scroll_delta = 
                wid_get_tl_y(wid_get_parent(wid_server_join_container_vert_scroll)) -
                wid_get_tl_y(wid_server_join_container_vert_scroll);
        }

    } else {
        if (wid_server_join_window) {
            return;
        }

        widp w = wid_server_join_window = 
                        wid_new_square_window("wid join main window");
        wid_visible(w, 0);

        fpoint tl = {0.0, 0.0};
        fpoint br = {1.0, 1.0};

        wid_set_tl_br_pct(w, tl, br);
        wid_set_font(w, small_font);
        wid_set_color(w, WID_COLOR_TEXT, WHITE);

        color c = BLACK;
        wid_set_color(w, WID_COLOR_BG, c);

        c = BLACK;
        wid_set_color(w, WID_COLOR_TL, c);
        wid_set_color(w, WID_COLOR_BR, c);
        wid_set_bevel(w, 4);
        wid_set_on_key_down(w, wid_server_join_key_event);

        wid_set_on_mouse_motion(w, wid_server_join_receive_mouse_motion);

        {
            widp w = wid_new_container(wid_server_join_window, "bg");
            wid_visible(w, 0);

            float f = (1024.0 / 680.0);

            fpoint tl = { 0.0, 0.0 };
            fpoint br = { 1.0, f };

            wid_set_tl_br_pct(w, tl, br);

            wid_set_tex(w, 0, "title2");

            wid_lower(w);

            color c;
            c = WHITE;
            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_color(w, WID_COLOR_TL, c);
            wid_set_color(w, WID_COLOR_BR, c);
            wid_set_color(w, WID_COLOR_BG, c);
        }
    }

    wid_server_join_window_container =
        wid_new_container(wid_server_join_window, "wid server join container");
    wid_visible(wid_server_join_window_container, 0);

    {
        fpoint tl = {0.0, 0.0};
        fpoint br = {1.0, 1.0};

        wid_set_tl_br_pct(wid_server_join_window_container, tl, br);
    }

    {
        widp w = wid_server_join_server_list_container =
            wid_new_container(wid_server_join_window_container,
                              "wid server list container");
        wid_visible(w, 0);

        fpoint tl = {0.0, 0.15};
        fpoint br = {1.0, 0.9};

        wid_set_tl_br_pct(w, tl, br);
    }

    const float width1 = 0.20;
    const float width2 = 0.10;
    const float width3 = 0.1;
    const float width4 = 0.23;
    const float width5 = 0.1;
    const float width6 = 0.08;
    const float width7 = 0.08;
    float width_at = 0.05;
    const float line_height = 0.05;

    {
        fpoint tl = {0.0, 0.0};
        fpoint br = {1.0, line_height};

        widp w = wid_new_container(wid_server_join_window_container,
                                       "server name");
        wid_visible(w, 0);

        wid_set_tooltip(w, "Click on a server to edit it", 0 /* font */);
        wid_set_tl_br_pct(w, tl, br);

        wid_set_text(w, "Join a game");
        wid_set_font(w, large_font);
        wid_set_color(w, WID_COLOR_TEXT, WHITE);

        color c = BLACK;
        c.a = 0;
        wid_set_color(w, WID_COLOR_BG, c);
        wid_set_color(w, WID_COLOR_TL, c);
        wid_set_color(w, WID_COLOR_BR, c);
        wid_set_square(w);
        wid_set_bevelled(w, true);
        wid_set_bevel(w, 2);
        wid_set_text_outline(w, true);
    }

    {
        fpoint tl = {width_at, line_height * 2};
        fpoint br = {width_at + width1, line_height * 3};

        widp w = wid_new_container(wid_server_join_window_container,
                                       "server hostname");
        wid_visible(w, 0);

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
            if (s->auto_add && !s->quality) {
                continue;
            }

            widp w = wid_new_square_button(wid_server_join_server_list_container,
                                           "server name");
            wid_visible(w, 0);

            fpoint tl = {width_at, 0.0};
            fpoint br = {width_at + width1, line_height};

            float height = line_height;

            wid_server_join_set_color(w, s);

            br.y += (float)i * height;
            tl.y += (float)i * height;

            wid_set_tl_br_pct(w, tl, br);
            wid_set_text(w, s->host);

            color c = DARKSLATEBLUE;
            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_OVER);
            wid_set_color(w, WID_COLOR_BG, RED);

            wid_set_mode(w, WID_MODE_NORMAL);

            wid_set_text_outline(w, true);
            wid_set_font(w, small_font);
            wid_set_text_lhs(w, true);

            wid_set_on_mouse_down(w, wid_server_join_hostname_mouse_down);
            wid_set_on_key_down(w, wid_server_join_hostname_receive_input);
            wid_set_on_mouse_over_begin(w, wid_server_join_mouse_over_server);
            wid_set_on_mouse_motion(w, wid_server_join_mouse_motion_over_server);
            wid_set_on_mouse_over_end(w, wid_server_join_mouse_over_end_server);
            wid_set_client_context(w, s);

            i++;
        }
    }

    width_at += width1;

    {
        fpoint tl = {width_at, line_height * 2};
        fpoint br = {width_at + width2, line_height * 3};

        widp w = wid_new_container(wid_server_join_window_container,
                                       "server ip");
        wid_visible(w, 0);

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
            if (s->auto_add && !s->quality) {
                continue;
            }

            widp w = wid_new_square_button(wid_server_join_server_list_container,
                                           "server name");
            wid_visible(w, 0);

            fpoint tl = {width_at, 0.0};
            fpoint br = {width_at + width2, line_height};

            float height = line_height;

            wid_server_join_set_color(w, s);

            br.y += (float)i * height;
            tl.y += (float)i * height;

            wid_set_tl_br_pct(w, tl, br);

            char *tmp = iprawtodynstr(s->ip);
            wid_set_text(w, tmp);
            myfree(tmp);

            color c = DARKSLATEBLUE;
            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_OVER);
            wid_set_color(w, WID_COLOR_BG, RED);

            wid_set_mode(w, WID_MODE_NORMAL);

            wid_set_text_outline(w, true);
            wid_set_font(w, small_font);
            wid_set_text_lhs(w, true);

            wid_set_on_mouse_down(w, wid_server_join_ip_mouse_down);
            wid_set_on_key_down(w, wid_server_join_ip_receive_input);
            wid_set_on_mouse_over_begin(w, wid_server_join_mouse_over_server);
            wid_set_on_mouse_motion(w, wid_server_join_mouse_motion_over_server);
            wid_set_on_mouse_over_end(w, wid_server_join_mouse_over_end_server);
            wid_set_client_context(w, s);

            i++;
        }
    }

    width_at += width2;

    {
        fpoint tl = {width_at, line_height * 2};
        fpoint br = {width_at + width3, line_height * 3};

        widp w = wid_new_container(wid_server_join_window_container,
                                       "server port");
        wid_visible(w, 0);

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
            if (s->auto_add && !s->quality) {
                continue;
            }

            widp w = wid_new_square_button(wid_server_join_server_list_container,
                                           "server name");
            wid_visible(w, 0);

            fpoint tl = {width_at, 0.0};
            fpoint br = {width_at + width3, line_height};

            float height = line_height;

            wid_server_join_set_color(w, s);

            br.y += (float)i * height;
            tl.y += (float)i * height;

            wid_set_tl_br_pct(w, tl, br);

            char *tmp = iprawporttodynstr(s->ip);
            wid_set_text(w, tmp);
            myfree(tmp);

            color c = DARKSLATEBLUE;
            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_OVER);
            wid_set_color(w, WID_COLOR_BG, RED);

            wid_set_mode(w, WID_MODE_NORMAL);

            wid_set_text_outline(w, true);
            wid_set_font(w, small_font);
            wid_set_text_lhs(w, true);

            wid_set_on_mouse_down(w, wid_server_join_port_mouse_down);
            wid_set_on_key_down(w, wid_server_join_port_receive_input);
            wid_set_on_mouse_over_begin(w, wid_server_join_mouse_over_server);
            wid_set_on_mouse_motion(w, wid_server_join_mouse_motion_over_server);
            wid_set_on_mouse_over_end(w, wid_server_join_mouse_over_end_server);
            wid_set_client_context(w, s);

            i++;
        }
    }

    width_at += width3;

    {
        fpoint tl = {width_at, line_height * 2};
        fpoint br = {width_at + width4, line_height * 3};

        widp w = wid_new_container(wid_server_join_window_container,
                                       "server name");
        wid_visible(w, 0);

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
            if (s->auto_add && !s->quality) {
                continue;
            }

            widp w = wid_new_square_button(wid_server_join_server_list_container,
                                           "server name");
            wid_visible(w, 0);

            fpoint tl = {width_at, 0.0};
            fpoint br = {width_at + width4, line_height};

            float height = line_height;

            wid_server_join_set_color(w, s);

            br.y += (float)i * height;
            tl.y += (float)i * height;

            wid_set_tl_br_pct(w, tl, br);

            char *tmp = dynprintf("%s", s->name);
            wid_set_text(w, tmp);
            myfree(tmp);

            color c = DARKSLATEBLUE;
            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_OVER);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_NORMAL);

            wid_set_text_outline(w, true);
            wid_set_font(w, small_font);
            wid_set_text_lhs(w, true);
            wid_set_on_mouse_over_begin(w, wid_server_join_mouse_over_server);
            wid_set_on_mouse_motion(w, wid_server_join_mouse_motion_over_server);
            wid_set_on_mouse_over_end(w, wid_server_join_mouse_over_end_server);
            wid_set_client_context(w, s);

            i++;
        }
    }

    width_at += width4;

    {
        fpoint tl = {width_at, line_height * 2};
        fpoint br = {width_at + width5, line_height * 3};

        widp w = wid_new_container(wid_server_join_window_container,
                                       "server quality");
        wid_visible(w, 0);

        wid_set_tl_br_pct(w, tl, br);

        wid_set_text(w, "Quality");
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
            if (s->auto_add && !s->quality) {
                continue;
            }

            widp w = wid_new_square_button(wid_server_join_server_list_container,
                                           "server quality");
            wid_visible(w, 0);

            fpoint tl = {width_at, 0.0};
            fpoint br = {width_at + width5, line_height};

            float height = line_height;

            wid_server_join_set_color(w, s);

            br.y += (float)i * height;
            tl.y += (float)i * height;

            wid_set_tl_br_pct(w, tl, br);

            char *tmp = dynprintf("%u%%", s->quality);
            wid_set_text(w, tmp);
            myfree(tmp);

            color c = DARKSLATEBLUE;

            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_OVER);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_NORMAL);

            wid_set_text_outline(w, true);
            wid_set_font(w, small_font);
            wid_set_text_centerx(w, true);
            wid_set_on_mouse_over_begin(w, wid_server_join_mouse_over_server);
            wid_set_on_mouse_motion(w, wid_server_join_mouse_motion_over_server);
            wid_set_on_mouse_over_end(w, wid_server_join_mouse_over_end_server);
            wid_set_client_context(w, s);

            i++;
        }
    }

    width_at += width5;

    {
        uint32_t i = 0;
        server *s;

        TREE_WALK_REVERSE(remote_servers, s) {
            if (s->auto_add && !s->quality) {
                continue;
            }

            socketp sp = socket_find(s->ip, SOCKET_CONNECT);

            if (sp && (sp == client_joined_server)) {
                /*
                 * Ignore
                 */
                continue;
            } else {
                if (!s->auto_add) {
                } else {
                    continue;
                }
            }

            widp w = wid_new_rounded_small_button(wid_server_join_server_list_container,
                                           "server remove");
            wid_visible(w, 0);

            fpoint tl = {width_at, 0.0};
            fpoint br = {width_at + width6, line_height};

            float height = line_height;

            wid_set_color(w, WID_COLOR_TEXT, RED);

            br.y += (float)i * height;
            tl.y += (float)i * height;

            wid_set_tl_br_pct(w, tl, br);

            wid_set_font(w, small_font);
            color c = DARKSLATEBLUE;

            wid_set_text(w, "Delete");
            wid_set_on_mouse_down(w, wid_server_join_delete);

            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_color(w, WID_COLOR_BG, c);

            c = RED;
            wid_set_mode(w, WID_MODE_OVER);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_text_outline(w, true);

            wid_set_on_mouse_over_begin(w, wid_server_join_mouse_over_server);
            wid_set_on_mouse_motion(w, wid_server_join_mouse_motion_over_server);
            wid_set_on_mouse_over_end(w, wid_server_join_mouse_over_end_server);
            wid_set_client_context(w, s);

            i++;
        }
    }

    width_at += width6;

    {
        uint32_t i = 0;
        server *s;

        TREE_WALK_REVERSE(remote_servers, s) {
            if (s->auto_add && !s->quality) {
                continue;
            }

            widp w = wid_new_rounded_small_button(wid_server_join_server_list_container,
                                           "server join");
            wid_visible(w, 0);

            fpoint tl = {width_at, 0.0};
            fpoint br = {width_at + width7, line_height};

            float height = line_height;

            if (s->quality > 0) {
                wid_set_color(w, WID_COLOR_TEXT, GREEN);
            } else {
                wid_set_color(w, WID_COLOR_TEXT, WHITE);
            }

            br.y += (float)i * height;
            tl.y += (float)i * height;

            wid_set_tl_br_pct(w, tl, br);

            socketp sp = socket_find(s->ip, SOCKET_CONNECT);
            if (sp && (sp == client_joined_server)) {
                wid_set_text(w, "Leave");
                wid_set_on_mouse_down(w, wid_server_join_leave);
            } else {
                wid_set_text(w, "Join");
                wid_set_on_mouse_down(w, wid_server_join);
            }

            wid_set_font(w, small_font);
            color c = DARKSLATEBLUE;
            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_color(w, WID_COLOR_BG, c);

            c = RED;
            wid_set_mode(w, WID_MODE_OVER);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_text_outline(w, true);

            wid_set_on_mouse_over_begin(w, wid_server_join_mouse_over_server);
            wid_set_on_mouse_motion(w, wid_server_join_mouse_motion_over_server);
            wid_set_on_mouse_over_end(w, wid_server_join_mouse_over_end_server);
            wid_set_client_context(w, s);

            i++;
        }
    }

    {
        fpoint tl = {0.0, 0.95};
        fpoint br = {0.2, 1.0};

        widp w = wid_new_square_button(wid_server_join_window_container,
                                       "wid server add");
        wid_visible(w, 0);

        wid_set_tl_br_pct(w, tl, br);

        wid_set_text(w, "Add a server");
        wid_set_no_shape(w);

        wid_fade_in_out(w, 1000, 1000, false /* fade out first */);

        wid_set_font(w, med_font);
        wid_set_color(w, WID_COLOR_TEXT, WHITE);
        wid_set_color(w, WID_COLOR_BG, BLACK);

        color c = WHITE;

        wid_set_mode(w, WID_MODE_NORMAL);
        wid_set_color(w, WID_COLOR_TEXT, c);

        c = RED;
        wid_set_mode(w, WID_MODE_OVER);
        wid_set_color(w, WID_COLOR_TEXT, c);

        wid_set_mode(w, WID_MODE_NORMAL);
        wid_set_text_outline(w, true);
        wid_raise(w);

        wid_set_on_mouse_down(w, wid_server_join_add);
    }

    if (!redo) {
        wid_server_join_container_vert_scroll =
            wid_new_vert_scroll_bar(wid_server_join_window, 
                                    wid_server_join_server_list_container);

        wid_hide(wid_get_parent(wid_server_join_container_vert_scroll), 0);
        wid_hide(wid_server_join_container_vert_scroll, 0);

        {
            widp child;

            child = wid_new_square_button(wid_server_join_window, "Go back");
            wid_visible(child, 0);

            wid_set_font(child, small_font);

            fpoint tl = {0.9f, 0.95f};
            fpoint br = {1.0f, 1.00f};

            wid_set_tl_br_pct(child, tl, br);
            wid_set_text(child, "%%fmt=left$%%tile=button_b$Go back");

            wid_set_no_shape(child);
            wid_set_color(child, WID_COLOR_TEXT, GRAY90);
            wid_set_mode(child, WID_MODE_OVER);
            wid_set_color(child, WID_COLOR_TEXT, WHITE);
            wid_set_mode(child, WID_MODE_NORMAL);

            wid_set_on_mouse_down(child, wid_server_join_go_back);
            wid_raise(child);
            wid_set_do_not_lower(child, true);
        }
    } else {
        wid_move_delta(wid_server_join_container_vert_scroll, 0, -scroll_delta);
    }

    wid_update(wid_server_join_window);

    /*
     * Make sure the mouse thinks we're over new widgets again.
     */
    wid_server_stats_window_server = 0;

    /*
     * Just check we cannot get stuck in a loop.
     */
    static int recurstion;
    if (recurstion) {
        return;
    }

    recurstion = 1;
    wid_update_mouse();
    recurstion = 0;

    if (wid_server_stats_window_server) {
        return;
    }

    /*
     * If we're not over a server, look for the best one to show.
     */
    {
        int gotone = 0;
        server *s;

        TREE_WALK_REVERSE(remote_servers, s) {
            if (s->auto_add && !s->quality) {
                continue;
            }

            if (s == wid_server_stats_window_server) {
                wid_server_join_display(s);
                gotone = 1;
                break;
            }
        }

        if (!gotone) {
            TREE_WALK_REVERSE(remote_servers, s) {
                if (s->auto_add && !s->quality) {
                    continue;
                }

                wid_server_join_display(s);
                break;
            }
        }
    }
}

void wid_server_join_destroy (void)
{
    if (wid_server_join_window) {
        wid_destroy_nodelay(&wid_server_join_window);
    }

    if (wid_server_join_window_go_back_button) {
        wid_destroy_nodelay(&wid_server_join_window_go_back_button);
    }

    user_is_typing = false;
}

static uint8_t demarshal_servers (demarshal_p ctx, server *s)
{
    uint8_t rc;

    rc = true;

    rc = rc && GET_OPT_NAMED_STRING(ctx, "host", s->host);
    rc = rc && GET_OPT_NAMED_UINT16(ctx, "port", s->port);
    rc = rc && GET_OPT_NAMED_UINT8(ctx, "auto_add", s->auto_add);

    return (rc);
}

static void marshal_servers (marshal_p ctx, server *s)
{
    char *host = s->host;
    uint16_t port = SDLNet_Read16(&s->ip.port);

    if (s->host) {
        PUT_NAMED_STRING(ctx, "host", host);
        PUT_NAMED_INT16(ctx, "port", port);
        PUT_NAMED_UINT8(ctx, "auto_add", s->auto_add);
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

    LOG("Load %s", file);

    int added = 0;

    server s;

    if ((ctx = demarshal(file))) {
        memset(&s, 0, sizeof(s));

        while (demarshal_servers(ctx, &s)) {
            server_add(&s);
            myfree(s.host);
            added++;
        }

        demarshal_fini(ctx);
    }

    myfree(file);

    return (true);
}
