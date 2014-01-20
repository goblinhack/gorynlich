/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <SDL.h>
#include <SDL_net.h>

#include "main.h"
#include "wid.h"
#include "color.h"
#include "wid_server.h"
#include "string.h"
#include "wid_text_input.h"
#include "marshal.h"
#include "sdl.h"
#include "socket.h"
#include "wid_popup.h"
#include "client.h"

static const char *server_dir_and_file = "gorynlich-servers.txt";

static widp wid_server_window;
static widp wid_server_container;
static boolean wid_server_init_done;

static void wid_server_create(void);
static void wid_server_destroy(void);
static boolean user_is_typing;

typedef struct server_ {
    tree_key_four_int tree;

    IPaddress ip;
    uint16_t port;
    char *host;
    char *host_and_port_str;
    uint8_t quality;
    uint32_t avg_latency;
    uint32_t min_latency;
    uint32_t max_latency;
    socketp socket;
} server;

tree_rootp servers;

static void server_add (const server *s_in)
{
    server *s;

    if (!servers) {
        servers = tree_alloc(TREE_KEY_TWO_INTEGER, "TREE ROOT: servers");
    }

    s = (typeof(s)) myzalloc(sizeof(*s), "TREE NODE: server");

    memcpy(s, s_in, sizeof(*s));
    s->host = dupstr(s_in->host, "hostname");
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

        LOG("Resolve host %s port %u", s->host, s->port);
    }

    SDLNet_Write16(s->port, &s->ip.port);

    s->tree.key1 = s->quality;
    s->tree.key2 = s->avg_latency;
    s->tree.key3 = SDLNet_Read32(&s->ip.host);
    s->tree.key4 = SDLNet_Read16(&s->ip.port);

    while (!tree_insert(servers, &s->tree.node)) {
        s->tree.key2++;

        SDLNet_Write16(s->tree.key2, &s->ip.port);
    }
}

static void server_remove (server *s)
{
    if (!servers) {
        return;
    }

    tree_remove(servers, &s->tree.node);

    wid_server_redo();
}

boolean wid_server_init (void)
{
    if (!wid_server_init_done) {
        server_load();
    }

    wid_server_init_done = true;

    return (true);
}

static void server_destroy (server *node)
{
    myfree(node->host_and_port_str);
}

void wid_server_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (wid_server_init_done) {
        wid_server_init_done = false;

        wid_server_destroy();

        tree_destroy(&servers, (tree_destroy_func)server_destroy);
    }
}

void wid_server_hide (void)
{
    wid_server_destroy();
}

void wid_server_visible (void)
{
    wid_server_create();
}

void wid_server_redo (void)
{
    if (!wid_server_window) {
        return;
    }

    if (user_is_typing) {
        return;
    }

    server *s;

    TREE_WALK(servers, s) {
        socketp sp = socket_find(s->ip);
        if (!sp) {
            continue;
        }

        s->quality = socket_get_quality(sp);
        s->avg_latency = socket_get_avg_latency(sp);
        s->min_latency = socket_get_min_latency(sp);
        s->max_latency = socket_get_max_latency(sp);
    }

    wid_server_destroy();
    wid_server_create();
}

static boolean wid_server_refresh (widp w, int32_t x, int32_t y, uint32_t button)
{
    wid_server_redo();

    return (true);
}

static boolean wid_server_go_back (widp w, int32_t x, int32_t y, uint32_t button)
{
    wid_server_hide();

    return (true);
}

static boolean wid_server_join (widp w, int32_t x, int32_t y, uint32_t button)
{
    server *s = wid_get_client_context(w);
    if (!s) {
        return (false);
    }

    client_socket_join(s->host, 0, s->port);

    return (true);
}

static boolean wid_server_delete (widp w, int32_t x, int32_t y, uint32_t button)
{
    server *s = wid_get_client_context(w);
    if (!s) {
        return (false);
    }

    server_remove(s);

    return (true);
}

static boolean wid_server_add (widp w, int32_t x, int32_t y, uint32_t button)
{
    server s;

    memset(&s, 0, sizeof(s));

    s.host = SERVER_DEFAULT_HOST; 
    s.port = SERVER_DEFAULT_PORT; 

    server_add(&s);

    wid_server_redo();

    return (true);
}

static boolean wid_server_key_event (widp w, const SDL_KEYSYM *key)
{
    switch (key->sym) {
        case 'q':
        case SDLK_ESCAPE:
            wid_server_hide();
            return (true);

        default:
            break;
    }

    return (false);
}

static boolean wid_server_receive_mouse_motion (
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

static boolean wid_server_hostname_mouse_down (widp w, int32_t x, int32_t y, 
                                               uint32_t button)
{
    wid_set_show_cursor(w, true);
    user_is_typing = true;

    return (true);
}

static boolean wid_server_ip_mouse_down (widp w, int32_t x, int32_t y,
                                         uint32_t button)
{
    wid_set_show_cursor(w, true);
    user_is_typing = true;

    return (true);
}

static boolean wid_server_port_mouse_down (widp w, int32_t x, int32_t y,
                                           uint32_t button)
{
    wid_set_show_cursor(w, true);
    user_is_typing = true;

    return (true);
}

/*
 * Key down etc...
 */
static boolean wid_server_hostname_receive_input (widp w, 
                                                  const SDL_KEYSYM *key)
{
    server *s;

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
                wid_server_redo();
                return (true);
            }

            sn.port = s->port;

            server_remove(s);
            server_add(&sn);
            wid_server_redo();

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
static boolean wid_server_ip_receive_input (widp w, const SDL_KEYSYM *key)
{
    server *s;

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
                char *popup_str = 
                    dynprintf("Failed to parse IP address, "
                              "not in A.B.C.D format");

                (void) wid_popup_error(popup_str);
                myfree(popup_str);

                return (true);
            }

            if ((a > 255) || (b > 255) | (c > 255) | (d > 255)) {
                /*
                 * Fail
                 */
                char *popup_str = 
                    dynprintf("Failed to parse IP address, "
                              "Each number must be in the 0 to 255 range");

                (void) wid_popup_error(popup_str);
                myfree(popup_str);

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
                char *popup_str = 
                    dynprintf("Failed to resolve IP address %s to a hostname",
                              ip_str);

                (void) wid_popup_error(popup_str);
                myfree(popup_str);

                return (true);
            }

            /*
             * Replace the server.
             */
            if (!sn.host || !*sn.host) {
                server_remove(s);
                wid_server_redo();
                return (true);
            }

            sn.port = s->port;

            server_remove(s);
            server_add(&sn);
            wid_server_redo();

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
static boolean wid_server_port_receive_input (widp w, const SDL_KEYSYM *key)
{
    server *s;

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

            const char *ip_str = wid_get_text(w);
            int a;
            int success = sscanf(ip_str, "%u", &a);
            if (success != 1) {
                /*
                 * Fail
                 */
                char *popup_str = 
                    dynprintf("Failed to parse port number");

                (void) wid_popup_error(popup_str);
                myfree(popup_str);

                return (true);
            }

            if ((a > 65535) || (a < 1024)) {
                /*
                 * Fail
                 */
                char *popup_str = 
                    dynprintf("Failed to parse port number, "
                              "must be in the 1024 to 65535 range");

                (void) wid_popup_error(popup_str);
                myfree(popup_str);

                return (true);
            }

            sn.host = s->host;
            sn.port = a;

            server_remove(s);
            server_add(&sn);
            wid_server_redo();

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

static void wid_server_create (void)
{
    if (wid_server_window) {
        return;
    }

    widp w = wid_server_window = wid_new_square_window("wid settings");

    fpoint tl = {0.05, 0.1};
    fpoint br = {0.95, 0.9};

    wid_set_tl_br_pct(w, tl, br);
    wid_set_font(w, med_font);

    wid_set_color(w, WID_COLOR_TEXT, WHITE);

    color c = BLACK;
    c.a = 200;
    wid_set_color(w, WID_COLOR_BG, c);

    c = STEELBLUE;
    c.a = 200;
    wid_set_color(w, WID_COLOR_TL, c);
    wid_set_color(w, WID_COLOR_BR, c);
    wid_set_bevel(w, 4);
    wid_set_on_key_down(w, wid_server_key_event);

    wid_set_on_mouse_motion(w, wid_server_receive_mouse_motion);

    {
        widp w = wid_server_container =
            wid_new_container(wid_server_window, "wid settings container");

        fpoint tl = {0.0, 0.15};
        fpoint br = {1.0, 1.0};

        wid_set_tl_br_pct(w, tl, br);
    }

    {
        fpoint tl = {0.0, 0.0};
        fpoint br = {1.0, 1.0};

        w = wid_new_container(wid_server_container, "server list");

        wid_set_tl_br_pct(w, tl, br);

        wid_set_text_outline(w, true);
    }

    {
        fpoint tl = {0.0, 0.0};
        fpoint br = {1.0, 0.1};

        widp w = wid_new_square_button(wid_server_window, "server name");

        wid_set_tooltip(w, "Click on a server to edit it");
        wid_set_tl_br_pct(w, tl, br);

        wid_set_text(w, "Servers");
        wid_set_font(w, large_font);
        wid_set_color(w, WID_COLOR_TEXT, STEELBLUE);
        wid_set_color(w, WID_COLOR_BG, BLACK);

        wid_set_text_outline(w, true);
    }

    const float width1 = 0.25;
    const float width2 = 0.25;
    const float width3 = 0.1;
    const float width4 = 0.1;
    const float width5 = 0.1;
    const float width6 = 0.1;
    const float width7 = 0.1;
    float width_at = 0.0;

    {
        fpoint tl = {width_at, 0.1};
        fpoint br = {width_at + width1, 0.15};

        widp w = wid_new_square_button(wid_server_window, "server hostname");

        wid_set_tl_br_pct(w, tl, br);

        wid_set_text(w, "Hostname");
        wid_set_font(w, med_font);
        wid_set_color(w, WID_COLOR_TEXT, STEELBLUE);
        wid_set_color(w, WID_COLOR_BG, BLACK);

        wid_set_text_outline(w, true);
    }

    {
        uint32_t i = 0;
        server *s;

        TREE_WALK(servers, s) {
            widp w = wid_new_square_button(wid_server_container,
                                           "server name");

            fpoint tl = {width_at, 0.0};
            fpoint br = {width_at + width1, 0.1};

            float height = 0.08;

            if (s->quality == 100) {
                wid_set_color(w, WID_COLOR_TEXT, GREEN);
            } else if (s->quality > 75) {
                wid_set_color(w, WID_COLOR_TEXT, YELLOW);
            } else {
                wid_set_color(w, WID_COLOR_TEXT, RED);
            }

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
            wid_set_font(w, small_font);
            wid_set_text_lhs(w, true);

            wid_set_on_mouse_down(w, wid_server_hostname_mouse_down);
            wid_set_on_key_down(w, wid_server_hostname_receive_input);
            wid_set_client_context(w, s);

            wid_set_tooltip(w, "Cick to edit");
            i++;
        }
    }

    width_at += width1;

    {
        fpoint tl = {width_at, 0.1};
        fpoint br = {width_at + width2, 0.15};

        widp w = wid_new_square_button(wid_server_window, "server ip");

        wid_set_tl_br_pct(w, tl, br);

        wid_set_text(w, "IP");
        wid_set_font(w, med_font);
        wid_set_color(w, WID_COLOR_TEXT, STEELBLUE);
        wid_set_color(w, WID_COLOR_BG, BLACK);

        wid_set_text_outline(w, true);
    }

    {
        uint32_t i = 0;
        server *s;

        TREE_WALK(servers, s) {
            widp w = wid_new_square_button(wid_server_container,
                                           "server name");

            fpoint tl = {width_at, 0.0};
            fpoint br = {width_at + width2, 0.1};

            float height = 0.08;

            if (s->quality == 100) {
                wid_set_color(w, WID_COLOR_TEXT, GREEN);
            } else if (s->quality > 75) {
                wid_set_color(w, WID_COLOR_TEXT, YELLOW);
            } else {
                wid_set_color(w, WID_COLOR_TEXT, RED);
            }

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
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_NORMAL);

            wid_set_text_outline(w, true);
            wid_set_font(w, small_font);
            wid_set_text_lhs(w, true);

            wid_set_on_mouse_down(w, wid_server_ip_mouse_down);
            wid_set_on_key_down(w, wid_server_ip_receive_input);
            wid_set_client_context(w, s);

            wid_set_tooltip(w, "Cick to edit");
            i++;
        }
    }

    width_at += width2;

    {
        fpoint tl = {width_at, 0.1};
        fpoint br = {width_at + width3, 0.15};

        widp w = wid_new_square_button(wid_server_window, "server port");

        wid_set_tl_br_pct(w, tl, br);

        wid_set_text(w, "Port");
        wid_set_font(w, med_font);
        wid_set_color(w, WID_COLOR_TEXT, STEELBLUE);
        wid_set_color(w, WID_COLOR_BG, BLACK);

        wid_set_text_outline(w, true);
    }

    {
        uint32_t i = 0;
        server *s;

        TREE_WALK(servers, s) {
            widp w = wid_new_square_button(wid_server_container,
                                           "server name");

            fpoint tl = {width_at, 0.0};
            fpoint br = {width_at + width3, 0.1};

            float height = 0.08;

            if (s->quality == 100) {
                wid_set_color(w, WID_COLOR_TEXT, GREEN);
            } else if (s->quality > 75) {
                wid_set_color(w, WID_COLOR_TEXT, YELLOW);
            } else {
                wid_set_color(w, WID_COLOR_TEXT, RED);
            }

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
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_NORMAL);

            wid_set_text_outline(w, true);
            wid_set_font(w, small_font);
            wid_set_text_lhs(w, true);

            wid_set_on_mouse_down(w, wid_server_port_mouse_down);
            wid_set_on_key_down(w, wid_server_port_receive_input);
            wid_set_client_context(w, s);

            wid_set_tooltip(w, "Cick to edit");
            i++;
        }
    }

    width_at += width3;

    {
        fpoint tl = {width_at, 0.1};
        fpoint br = {width_at + width4, 0.15};

        widp w = wid_new_square_button(wid_server_window, "server latency");

        wid_set_tl_br_pct(w, tl, br);

        wid_set_text(w, "Lag");
        wid_set_font(w, med_font);
        wid_set_color(w, WID_COLOR_TEXT, STEELBLUE);
        wid_set_color(w, WID_COLOR_BG, BLACK);

        wid_set_text_outline(w, true);
    }

    {
        uint32_t i = 0;
        server *s;

        TREE_WALK(servers, s) {
            widp w = wid_new_square_button(wid_server_container,
                                           "server latency");

            fpoint tl = {width_at, 0.0};
            fpoint br = {width_at + width4, 0.1};

            float height = 0.08;

            if (s->quality == 100) {
                wid_set_color(w, WID_COLOR_TEXT, GREEN);
            } else if (s->quality > 75) {
                wid_set_color(w, WID_COLOR_TEXT, YELLOW);
            } else {
                wid_set_color(w, WID_COLOR_TEXT, RED);
            }

            br.y += (float)i * height;
            tl.y += (float)i * height;

            wid_set_tl_br_pct(w, tl, br);

            char *tmp = dynprintf("%u", s->avg_latency);
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
            wid_set_font(w, small_font);
            wid_set_text_lhs(w, true);

            i++;
        }
    }

    width_at += width4;

    {
        fpoint tl = {width_at, 0.1};
        fpoint br = {width_at + width5, 0.15};

        widp w = wid_new_square_button(wid_server_window, "server quality");

        wid_set_tl_br_pct(w, tl, br);

        wid_set_text(w, "Qual");
        wid_set_font(w, med_font);
        wid_set_color(w, WID_COLOR_TEXT, STEELBLUE);
        wid_set_color(w, WID_COLOR_BG, BLACK);

        wid_set_text_outline(w, true);
    }

    {
        uint32_t i = 0;
        server *s;

        TREE_WALK(servers, s) {
            widp w = wid_new_square_button(wid_server_container,
                                           "server quality");

            fpoint tl = {width_at, 0.0};
            fpoint br = {width_at + width5, 0.1};

            float height = 0.08;

            if (s->quality == 100) {
                wid_set_color(w, WID_COLOR_TEXT, GREEN);
            } else if (s->quality > 75) {
                wid_set_color(w, WID_COLOR_TEXT, YELLOW);
            } else {
                wid_set_color(w, WID_COLOR_TEXT, RED);
            }

            br.y += (float)i * height;
            tl.y += (float)i * height;

            wid_set_tl_br_pct(w, tl, br);

            char *tmp = dynprintf("%u", s->quality);
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
            wid_set_font(w, small_font);
            wid_set_text_lhs(w, true);

            i++;
        }
    }

    width_at += width5;

    {
        uint32_t i = 0;
        server *s;

        TREE_WALK(servers, s) {
            widp w = wid_new_rounded_small_button(wid_server_container,
                                           "server remove");

            fpoint tl = {width_at, 0.0};
            fpoint br = {width_at + width6, 0.1};

            float height = 0.08;

            wid_set_color(w, WID_COLOR_TEXT, RED);

            br.y += (float)i * height;
            tl.y += (float)i * height;

            wid_set_tl_br_pct(w, tl, br);
            wid_set_text(w, "Delete");
            wid_set_tooltip(w, "Remove this server from the list");
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

            wid_set_on_mouse_down(w, wid_server_delete);
            wid_set_client_context(w, s);

            i++;
        }
    }

    width_at += width6;

    {
        uint32_t i = 0;
        server *s;

        TREE_WALK(servers, s) {
            widp w = wid_new_rounded_small_button(wid_server_container,
                                           "server join");

            fpoint tl = {width_at, 0.0};
            fpoint br = {width_at + width7, 0.1};

            float height = 0.08;

            wid_set_color(w, WID_COLOR_TEXT, WHITE);

            br.y += (float)i * height;
            tl.y += (float)i * height;

            wid_set_tl_br_pct(w, tl, br);
            wid_set_text(w, "Join");
            wid_set_tooltip(w, "Try to join the game on this server");
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

            wid_set_on_mouse_down(w, wid_server_join);
            wid_set_client_context(w, s);

            i++;
        }
    }

    width_at += width7;

    {
        fpoint tl = {0.7, 0.90};
        fpoint br = {0.99, 0.99};

        widp w = wid_new_rounded_small_button(wid_server_window,
                                              "server finish");

        wid_set_tl_br_pct(w, tl, br);

        wid_set_text(w, "Go back");
        wid_set_font(w, small_font);
        wid_set_color(w, WID_COLOR_TEXT, STEELBLUE);
        wid_set_color(w, WID_COLOR_BG, BLACK);

        wid_set_text_outline(w, true);
        wid_raise(w);
        wid_set_do_not_lower(w, true);

        wid_set_on_mouse_up(w, wid_server_go_back);
    }

    {
        fpoint tl = {0.5, 0.90};
        fpoint br = {0.69, 0.99};

        widp w = wid_new_rounded_small_button(wid_server_window,
                                              "server finish");

        wid_set_tl_br_pct(w, tl, br);

        wid_set_text(w, "Add");
        wid_set_tooltip(w, "Add a new server to the list");
        wid_set_font(w, small_font);
        wid_set_color(w, WID_COLOR_TEXT, STEELBLUE);
        wid_set_color(w, WID_COLOR_BG, BLACK);

        wid_set_text_outline(w, true);
        wid_raise(w);
        wid_set_do_not_lower(w, true);

        wid_set_on_mouse_up(w, wid_server_add);
    }

    {
        fpoint tl = {0.0, 0.90};
        fpoint br = {0.3, 0.99};

        widp w = wid_new_rounded_small_button(wid_server_window,
                                              "server finish");

        wid_set_tl_br_pct(w, tl, br);

        wid_set_text(w, "Refresh");
        wid_set_tooltip(w, "Update server list");
        wid_set_font(w, small_font);
        wid_set_color(w, WID_COLOR_TEXT, STEELBLUE);
        wid_set_color(w, WID_COLOR_BG, BLACK);

        wid_set_text_outline(w, true);
        wid_raise(w);
        wid_set_do_not_lower(w, true);

        wid_set_on_mouse_up(w, wid_server_refresh);
    }

    widp wid_server_container_horiz_scroll;
    widp wid_server_container_vert_scroll;

    wid_server_container_vert_scroll =
        wid_new_vert_scroll_bar(wid_server_window, 
                                wid_server_container);
    wid_server_container_horiz_scroll =
        wid_new_horiz_scroll_bar(wid_server_window, 
                                 wid_server_container);

    wid_raise(wid_server_window);

    wid_update(wid_server_window);
}

void wid_server_destroy (void)
{
    wid_destroy(&wid_server_window);

    user_is_typing = false;
}

static boolean demarshal_server (demarshal_p ctx, server *s)
{
    boolean rc;

    rc = true;

    rc = rc && GET_OPT_NAMED_STRING(ctx, "host", s->host);
    rc = rc && GET_OPT_NAMED_UINT16(ctx, "port", s->port);

    return (rc);
}

static void marshal_server (marshal_p ctx, server *s)
{
    char *host = s->host;
    uint16_t port = SDLNet_Read16(&s->ip.port);

    if (s->host) {
        PUT_NAMED_STRING(ctx, "host", host);
        PUT_NAMED_INT16(ctx, "port", port);
    }
}

boolean server_save (void)
{
    char *file = dynprintf("%s", server_dir_and_file);
    marshal_p ctx;

    ctx = marshal(file);
    if (!ctx) {
        ERR("Failed to save: %s", file);
        myfree(file);
        return (false);
    }

    server *s;

    TREE_WALK(servers, s) {
        marshal_server(ctx, s);
    }

    if (marshal_fini(ctx) < 0) {
        ERR("Failed to finalize: %s", file);
        myfree(file);

        return (false);
    }

    myfree(file);

    return (true);
}

boolean server_load (void)
{
    char *file = dynprintf("%s", server_dir_and_file);
    demarshal_p ctx;

    if (servers) {
        tree_destroy(&servers, 0);
    }

    DBG("Load %s", file);

    server s;

    if ((ctx = demarshal(file))) {
        memset(&s, 0, sizeof(s));

        while (demarshal_server(ctx, &s)) {
            server_add(&s);
        }

        demarshal_fini(ctx);
    }

    myfree(file);

    return (true);
}

static widp wid_server_name_popup;
static uint32_t score;

static void wid_server_name_ok (widp w)
{
    widp top;

    /*
     * We're given the ok or cancel button, so must name the text box.
     */
//    const char *host_and_port_str = wid_get_text(w);

//    server_add(dynprintf("%s", host_and_port_str), score);

    /*
     * Destroy the name dialog.
     */
    top = wid_get_top_parent(w);
    wid_destroy(&top);
    wid_server_name_popup = 0;

    server_save();
}

static void wid_server_name_cancel (widp w)
{
    widp top;

    top = wid_get_top_parent(w);
    wid_destroy(&top);
    wid_server_name_popup = 0;
}

widp server_try_to_add (uint32_t score_in)
{
    uint32_t count = 0;

    score = score_in;
    server *s;

    TREE_WALK(servers, s) {

        if (score_in > (uint32_t) s->tree.key1) {
            break;
        }

        count++;
    }
    const char *which[] = {
        "First",
        "Second",
        "Third",
        "Fourth",
        "Fifth",
        "Sixth",
        "Seventh",
        "Eighth",
        "Nineth",
        "Tenth",
    };

    if (count >= ARRAY_SIZE(which)) {
        return (0);
    }

    char *place_str = dynprintf("%s place!", which[count]);

    wid_server_name_popup = wid_large_text_input(
          place_str,
          0.5, 0.5,                 /* position */
          2,                        /* buttons */
          "Ok", wid_server_name_ok,
          "Cancel", wid_server_name_cancel);

    myfree(place_str);

    return (wid_server_name_popup);
}
