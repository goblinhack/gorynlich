/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "color.h"
#include "wid_server_edit.h"
#include "wid_server_join.h"
#include "wid_choose_game_type.h"
#include "string_util.h"
#include "wid_game_map_server.h"
#include "wid_game_map_client.h"
#include "socket_util.h"
#include "wid_menu.h"
#include "wid_numpad.h"
#include "wid_keyboard.h"
#include "server.h"

#define WID_SERVER_CREATE_MAX_SETTINGS  3
#define WID_SERVER_CREATE_MAX_VAL      30 

enum {
    WID_SERVER_CREATE_ROW_IP,
    WID_SERVER_CREATE_ROW_PORT,
    WID_SERVER_CREATE_ROW_JOIN_SERVER,
};

static const char *
    wid_server_edit_button_name[WID_SERVER_CREATE_MAX_SETTINGS] = {
    "%%fmt=left$Server IP",
    "%%fmt=left$Port",
    "%%fmt=left$%%fg=red$Join server",
};

static int saved_focus;
static widp menu;

static uint8_t wid_server_edit_ip_mouse_down(widp w, 
                                             int32_t x, int32_t y,
                                             uint32_t button);
static uint8_t wid_server_edit_port_mouse_down(widp w, 
                                                 int32_t x, int32_t y,
                                                 uint32_t button);
static uint8_t wid_server_edit_join(widp w, int32_t x, int32_t y, 
                                    uint32_t button);

static const on_mouse_down_t 
    wid_server_edit_button_mouse_down[WID_SERVER_CREATE_MAX_SETTINGS] = {

    wid_server_edit_ip_mouse_down,
    wid_server_edit_port_mouse_down,
    wid_server_edit_join,
};

static widp wid_server_edit_bg;
static uint8_t wid_server_edit_init_done;

static void wid_server_edit_menu(void);
static void wid_server_edit_destroy(void);

typedef struct server_ {
    tree_key_two_int tree;

    IPaddress ip;
    char *host;
    uint16_t port;
    uint8_t walked;
    uint8_t started;
} server;

static void wid_server_edit_destroy_internal(server *node);

static tree_rootp edit_servers;

static void wid_server_edit_server_add (const server *s_in)
{
    server *s;

    if (!edit_servers) {
        edit_servers = 
            tree_alloc(TREE_KEY_TWO_INTEGER, "TREE ROOT: local servers");
    }

    s = (typeof(s)) myzalloc(sizeof(*s), "TREE NODE: server");

    memcpy(s, s_in, sizeof(*s));

    uint16_t port;
    uint32_t host;

    /*
     * Need to resolve.
     */
    if ((address_resolve(&s->ip, s_in->host, s_in->port)) == -1) {
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

        TREE_WALK(edit_servers, sw) {
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

    if (!tree_insert(edit_servers, &s->tree.node)) {
        ERR("Cannot add port %u", s->port);
        myfree(s);
        return;
    }
}

static void server_remove (server *s)
{
    if (!edit_servers) {
        return;
    }

    wid_server_edit_destroy_internal(s);
    tree_remove(edit_servers, &s->tree.node);
    myfree(s);
}

static void wid_server_edit_destroy_internal (server *node)
{
    if (node->host) {
        myfree(node->host);
        node->host = 0;
    }
}

void wid_server_edit_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (wid_server_edit_init_done) {
        wid_server_edit_init_done = false;

        wid_server_edit_hide();

        if (edit_servers) {
            tree_destroy(&edit_servers, 
                         (tree_destroy_func)wid_server_edit_destroy_internal);
        }
    }
}

void wid_server_edit_hide (void)
{
CON("%s",__FUNCTION__);
    wid_server_edit_destroy();

    /*
     * Leave server socket open.
     */

    if (wid_server_edit_bg) {
        wid_destroy_in(wid_server_edit_bg, wid_hide_delay * 2);
        wid_server_edit_bg = 0;
    }

    if (menu) {
        wid_destroy(&menu);
    }
}

void wid_server_edit_visible (void)
{
CON("%s",__FUNCTION__);
    wid_server_edit_redo();
}

void wid_server_edit_redo (void)
{
    server *s;

    {
        TREE_WALK(edit_servers, s) {
            s->walked = false;
        }
    }

    TREE_WALK(edit_servers, s) {
        if (s->walked) {
            continue;
        }

        s->started = false;
        s->walked = true;

        gsocketp sp = socket_find(s->ip, SOCKET_LISTEN);
        if (!sp) {
            continue;
        }

        if (sp == server_socket) {
            s->started = true;
        }

        break;
    }

    wid_server_edit_destroy();
    wid_server_edit_menu();
}

static uint8_t wid_server_edit_go_back (widp w, int32_t x, int32_t y, 
                                        uint32_t button)
{
    wid_menu_ctx *ctx = (typeof(ctx)) wid_get_client_context(w);
    verify(ctx);

    saved_focus = ctx->focus;

    wid_server_edit_hide();
    wid_choose_game_type_visible();

    return (true);
}

static uint8_t wid_server_edit_join (widp w, int32_t x, int32_t y, 
                                     uint32_t button)
{
    wid_menu_ctx *ctx = (typeof(ctx)) wid_get_client_context(w);
    verify(ctx);

    saved_focus = ctx->focus;
    wid_destroy(&menu);
    wid_server_edit_hide();

    server *s;
  
    TREE_WALK_REVERSE(edit_servers, s) {

        wid_server_join(s->host, s->port);
        break;
    }

    return (true);
}

static widp wid_port_number;

static void wid_port_number_ok (widp w, const char *text)
{
    wid_destroy(&wid_port_number);

    server *s = 0;

    TREE_WALK_REVERSE(edit_servers, s) {
        break;
    }

    server sn;
    memset(&sn, 0, sizeof(sn));

    int port;
    int success = sscanf(text, "%u", &port);
    if (success != 1) {
        /*
         * Fail
         */
        MSG_BOX("Failed to parse port number");
        wid_server_edit_redo();
        return;
    }

    if ((port > 65535) || (port < 1024)) {
        /*
         * Fail
         */
        MSG_BOX("Failed to parse port number, "
                "must be in the 1024 to 65535 range");
        wid_server_edit_redo();
        return;
    }

    sn.port = port;

    if (s) {
        sn.host = dupstr(s->host, "wid port change");
    }

    server_remove(s);
    wid_server_edit_server_add(&sn);
    wid_server_edit_redo();
    myfree(sn.host);
}

static void wid_port_number_cancel (widp w, const char *text)
{
    wid_destroy(&wid_port_number);

    wid_server_edit_menu();
}

static uint8_t wid_server_edit_port_mouse_down (widp w, 
                                                  int32_t x, int32_t y,
                                                  uint32_t button)
{
    wid_menu_ctx *ctx = (typeof(ctx)) wid_get_client_context(w);
    verify(ctx);

    saved_focus = ctx->focus;

    wid_destroy(&menu);

    wid_port_number = wid_numpad("",
                                 "Enter port number",
                                 wid_port_number_ok,
                                 wid_port_number_cancel);

    return (true);
}

static widp wid_ip;

static void wid_ip_ok (widp w, const char *text)
{
    wid_destroy(&wid_ip);

    server *s = 0;

    TREE_WALK_REVERSE(edit_servers, s) {
        break;
    }

    server sn;
    memset(&sn, 0, sizeof(sn));

    int a, b, c, d;
    int success = sscanf(text, "%u.%u.%u.%u", &a, &b, &c, &d);
    if (success != 1) {
        /*
         * Fail
         */
        MSG_BOX("Failed to parse IP address, expecting a.b.c.d format");
        wid_server_edit_redo();
        return;
    }

    if ((a > 255) || (a < 0) ||
        (b > 255) || (b < 0) ||
        (c > 255) || (c < 0) ||
        (d > 255) || (d < 0)) {
        /*
         * Fail
         */
        MSG_BOX("Address must be in 0-255 range");
        wid_server_edit_redo();
        return;
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
        return;
    }

    /*
     * Replace the server.
     */
    if (!sn.host || !*sn.host) {
        server_remove(s);
        wid_server_edit_redo();
        return;
    }

    sn.port = s->port;

    server_remove(s);
    wid_server_edit_server_add(&sn);
    wid_server_edit_redo();
    myfree(sn.host);
}

static void wid_ip_cancel (widp w, const char *text)
{
    wid_destroy(&wid_ip);

    wid_server_edit_menu();
}

static uint8_t wid_server_edit_ip_mouse_down (widp w,
                                              int32_t x, int32_t y,
                                              uint32_t button)
{
    wid_menu_ctx *ctx = (typeof(ctx)) wid_get_client_context(w);
    verify(ctx);

    saved_focus = ctx->focus;

    wid_destroy(&menu);

    wid_ip = wid_numpad("", "Enter IP address", wid_ip_ok, wid_ip_cancel);

    return (true);
}

void wid_server_edit_destroy (void)
{
    if (menu) {
        wid_menu_ctx *ctx = 
                (typeof(ctx)) wid_get_client_context(menu);
        verify(ctx);

        /*
         * Save the focus so when we remake the menu we are at the same entry.
         */
        saved_focus = ctx->focus;
    }
}

uint8_t wid_server_edit_init (const char *host, int port)
{
    if (edit_servers) {
        tree_destroy(&edit_servers, 0);
    }

    server s;

    memset(&s, 0, sizeof(s));

    s.host = (char*) host;
    s.port = port;

    wid_server_edit_server_add(&s);

    wid_server_edit_visible();

    return (true);
}

static void wid_server_edit_menu (void)
{
    server *s = 0;

CON("%s",__FUNCTION__);
    TREE_WALK_REVERSE(edit_servers, s) {
        break;
    }

    char *keys[WID_SERVER_CREATE_MAX_SETTINGS];
    char *values[WID_SERVER_CREATE_MAX_SETTINGS];

    memset(keys, 0, sizeof(keys));
    memset(values, 0, sizeof(values));

    int i;
    for (i = WID_SERVER_CREATE_ROW_IP; 
         i < WID_SERVER_CREATE_MAX_SETTINGS; i++) {

        keys[i] = dynprintf("%s", wid_server_edit_button_name[i]);

        switch (i) {
        case WID_SERVER_CREATE_ROW_IP:
            if (s) {
                values[i] = iprawtodynstr(s->ip);
            }
            break;

        case WID_SERVER_CREATE_ROW_PORT:
            if (s) {
                values[i] = iprawporttodynstr(s->ip);
            }

            break;

        case WID_SERVER_CREATE_ROW_JOIN_SERVER:
            if (s) {
                values[i] = dupstr("", "dummy");
            }

            break;
        default:
            break;
        }

        if (values[i]) {
            continue;
        }

        values[i] = dynprintf("-");
    }

    i = WID_SERVER_CREATE_ROW_IP;

    menu = wid_menu(0,
                vlarge_font,
                large_font,
                0.5, /* x */
                0.4, /* y */
                2, /* columns */
                saved_focus, /* focus */
                WID_SERVER_CREATE_MAX_SETTINGS + 1, /* items */

                /*
                 * column width
                 */
                (double)0.5, (double)0.2,

                (int) '1', 
                keys[i],
                values[i], 
                wid_server_edit_button_mouse_down[i],

                (int) '2',
                keys[i + 1],
                values[i + 1], 
                wid_server_edit_button_mouse_down[i + 1],

                (int) '3',
                keys[i + 2],
                values[i + 2], 
                wid_server_edit_button_mouse_down[i + 2],

                (int) 'b', 
                "%%fmt=left$Back",
                (char*) 0,
                wid_server_edit_go_back);

    for (i = WID_SERVER_CREATE_ROW_IP; 
         i < WID_SERVER_CREATE_MAX_SETTINGS; i++) {
        myfree(values[i]);
        myfree(keys[i]);
    }
}
