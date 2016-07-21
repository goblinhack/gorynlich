/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "color.h"
#include "wid_server_create.h"
#include "wid_choose_game_type.h"
#include "string_util.h"
#include "wid_game_map_server.h"
#include "wid_game_map_client.h"
#include "socket_util.h"
#include "wid_menu.h"
#include "wid_numpad.h"
#include "wid_keyboard.h"
#include "server.h"

#define WID_SERVER_CREATE_MAX_SETTINGS  4
#define WID_SERVER_CREATE_MAX_VAL      30 

enum {
    WID_SERVER_CREATE_ROW_SERVER_NAME,
    WID_SERVER_CREATE_ROW_PORT,
    WID_SERVER_CREATE_ROW_MAX_PLAYERS,
    WID_SERVER_CREATE_ROW_START_SERVER,
};

static const char *
    wid_server_create_button_name[WID_SERVER_CREATE_MAX_SETTINGS] = {
    "%%fmt=left$Server name",
    "%%fmt=left$Port",
    "%%fmt=left$Max Players",
    "%%fmt=left$Start the server",
};

static const char *wid_server_create_button_value_string
        [WID_SERVER_CREATE_MAX_SETTINGS][WID_SERVER_CREATE_MAX_VAL] = {
    { 0 },
    { 0 },
    { 0 },
    { "Off", "On", 0 },
};

static int saved_focus;
static widp menu;

static uint8_t wid_server_create_name_mouse_down(widp w, 
                                                 int32_t x, int32_t y,
                                                 uint32_t button);
static uint8_t wid_server_create_port_mouse_down(widp w, 
                                                 int32_t x, int32_t y,
                                                 uint32_t button);
static uint8_t wid_server_create_mouse_event(widp w,
                                             int32_t x, int32_t y,
                                             uint32_t button);
static uint8_t wid_server_start(widp w, int32_t x, int32_t y, uint32_t button);

static const on_mouse_down_t 
    wid_server_create_button_mouse_down[WID_SERVER_CREATE_MAX_SETTINGS] = {

    wid_server_create_name_mouse_down,
    wid_server_create_port_mouse_down,
    wid_server_create_mouse_event,
    wid_server_start,
};

static uint32_t wid_server_create_button_val[WID_SERVER_CREATE_MAX_SETTINGS];
static char wid_server_create_button_sval[WID_SERVER_CREATE_MAX_SETTINGS][MAXSTR];

static widp wid_server_create_bg;
static uint8_t wid_server_create_init_done;

static void wid_server_create_menu(void);
static void wid_server_config_changed(void);
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
    if ((address_resolve(&s->ip, "0.0.0.0", s_in->port)) == -1) {
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

    if (wid_server_create_bg) {
        wid_destroy_in(wid_server_create_bg, wid_hide_delay * 2);
        wid_server_create_bg = 0;
    }

    if (menu) {
        wid_destroy(&menu);
    }
}

void wid_server_create_visible (void)
{
    wid_server_create_redo();
}

void wid_server_create_redo (void)
{
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

        gsocketp sp = socket_find(s->ip, SOCKET_LISTEN);
        if (!sp) {
            continue;
        }

        if (sp == server_socket) {
            s->started = true;
        }

        break;
    }

    wid_server_create_destroy();
    wid_server_create_menu();
}

static uint8_t wid_server_create_go_back (widp w, int32_t x, int32_t y, uint32_t button)
{
    wid_menu_ctx *ctx = (typeof(ctx)) wid_get_client_context(w);
    verify(ctx);

    saved_focus = ctx->focus;

    wid_server_create_hide();
    wid_choose_game_type_visible();

    return (true);
}

static uint8_t wid_server_start (widp w, int32_t x, int32_t y, uint32_t button)
{
    wid_menu_ctx *ctx = (typeof(ctx)) wid_get_client_context(w);
    verify(ctx);

    saved_focus = ctx->focus;
    wid_destroy(&menu);

    server_stop();

    server *s;

    TREE_WALK_REVERSE(local_servers, s) {

        IPaddress ip = {0};

        if (address_resolve(&ip, SERVER_DEFAULT_HOST, 
                               global_config.server_port)) {
            MSG_BOX("Open socket, cannot resolve %s:%u",
                SERVER_DEFAULT_HOST, global_config.server_port);
            wid_server_create_redo();

            return (false);
        }

        if (!server_start(ip)) {
            wid_server_create_redo();

            return (true);
        }

        break;
    }

    MESG(POPUP, "Server started! Woot!");

    wid_server_create_hide();
    wid_choose_game_type_visible();

    return (true);
}

static widp wid_port_number;

static void wid_port_number_ok (widp w, const char *text)
{
    wid_destroy(&wid_port_number);

    server *s = 0;

    TREE_WALK_REVERSE(local_servers, s) {
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
        wid_server_create_redo();
        return;
    }

    if ((port > 65535) || (port < 1024)) {
        /*
         * Fail
         */
        MSG_BOX("Failed to parse port number, "
                "must be in the 1024 to 65535 range");
        wid_server_create_redo();
        return;
    }

    sn.port = port;
    global_config.server_port = sn.port;

    if (s) {
        sn.name = dupstr(s->name, "wid port change");
    }

    server_remove(s);
    wid_server_local_server_add(&sn);
    wid_server_create_redo();
    myfree(sn.name);
}

static void wid_port_number_cancel (widp w, const char *text)
{
    wid_destroy(&wid_port_number);

    wid_server_create_menu();
}

static uint8_t wid_server_create_port_mouse_down (widp w, 
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

static widp wid_server_name;

static void wid_server_name_ok (widp w, const char *text)
{
    wid_destroy(&wid_server_name);

    server *s = 0;

    TREE_WALK_REVERSE(local_servers, s) {
        break;
    }

    server sn;
    memset(&sn, 0, sizeof(sn));

    sn.name = (char *)text;
    sn.port = s->port;

    strlcpy(global_config.server_name, sn.name,
            sizeof(global_config.server_name));

    if (global_config.user_server_name[0]) {
        global_config.user_server_name[0] = 0;
    }

    server_remove(s);
    wid_server_local_server_add(&sn);
    wid_server_create_redo();

}

static void wid_server_name_cancel (widp w, const char *text)
{
    wid_destroy(&wid_server_name);

    wid_server_create_menu();
}

static uint8_t wid_server_create_name_mouse_down (widp w,
                                                  int32_t x, int32_t y,
                                                  uint32_t button)
{
    wid_menu_ctx *ctx = (typeof(ctx)) wid_get_client_context(w);
    verify(ctx);

    saved_focus = ctx->focus;

    wid_destroy(&menu);

    wid_server_name = wid_keyboard("",
                                   "Enter server name",
                                   wid_server_name_ok,
                                   wid_server_name_cancel);

    return (true);
}

void wid_server_create_destroy (void)
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

static uint8_t wid_server_create_col2_mouse_event_ (widp w,
                                                    int32_t x, int32_t y,
                                                    uint32_t button, 
                                                    int32_t delta)
{
    /*
     * Increment.
     */
    wid_menu_ctx *ctx = (typeof(ctx)) wid_get_client_context(w);
    verify(ctx);

    int32_t row = (typeof(row)) (intptr_t) wid_get_client_context2(w);

    if (row >= WID_SERVER_CREATE_MAX_SETTINGS) {
        ERR("bad row");
    }

    if (row == WID_SERVER_CREATE_ROW_MAX_PLAYERS) {
        if (delta == 1) {
            if (global_config.server_max_players >= MAX_PLAYERS) {
                global_config.server_max_players = 1;
                delta = 0;
            }
        } else {
            if (global_config.server_max_players <= 1) {
                MESG(POPUP, "Don't be silly");
                delta = 0;
            }
        }

        global_config.server_max_players += delta;
    } else {
        wid_server_create_button_val[row] += delta;
    }

    wid_server_config_changed();
    wid_server_create_redo();

    return (true);
}

static void wid_server_create_read (void)
{
    server *s;

    TREE_WALK_REVERSE(local_servers, s) {

        strlcpy(wid_server_create_button_sval
                    [WID_SERVER_CREATE_ROW_SERVER_NAME],
                s->name,
                sizeof(wid_server_create_button_sval
                        [WID_SERVER_CREATE_ROW_SERVER_NAME]));

        {
            char *tmp = iprawporttodynstr(s->ip);
            strlcpy(wid_server_create_button_sval[WID_SERVER_CREATE_ROW_PORT],
                    tmp,
                    sizeof(wid_server_create_button_sval
                           [WID_SERVER_CREATE_ROW_PORT]));
            myfree(tmp);
        }

        {
            char *tmp = dynprintf("%u", global_config.server_max_players);
            strlcpy(wid_server_create_button_sval
                        [WID_SERVER_CREATE_ROW_MAX_PLAYERS],
                    tmp,
                    sizeof(wid_server_create_button_sval
                            [WID_SERVER_CREATE_ROW_MAX_PLAYERS]));
            myfree(tmp);
        }

        break;
    }
}

static void wid_server_config_changed (void)
{
    config_save();
}

static uint8_t wid_server_create_mouse_event (widp w,
                                              int32_t x, int32_t y,
                                              uint32_t button)
{
    wid_menu_ctx *ctx = (typeof(ctx)) wid_get_client_context(w);
    verify(ctx);

    saved_focus = ctx->focus;

    wid_destroy(&menu);

    wid_server_create_col2_mouse_event_(w, x, y, SDL_BUTTON_LEFT, 1);

    return (true);
}

static void wid_server_create_menu (void)
{
    wid_server_create_read();

    server *s = 0;

    TREE_WALK_REVERSE(local_servers, s) {
        break;
    }

    char *keys[WID_SERVER_CREATE_MAX_SETTINGS];
    char *values[WID_SERVER_CREATE_MAX_SETTINGS];

    memset(keys, 0, sizeof(keys));
    memset(values, 0, sizeof(values));

    int i;
    for (i = WID_SERVER_CREATE_ROW_SERVER_NAME; 
         i < WID_SERVER_CREATE_MAX_SETTINGS; i++) {

        const char *val;

        keys[i] = dynprintf("%s", wid_server_create_button_name[i]);

        switch (i) {
        case WID_SERVER_CREATE_ROW_PORT:
            if (s) {
                values[i] = iprawporttodynstr(s->ip);
            }

            break;

        case WID_SERVER_CREATE_ROW_SERVER_NAME:
            if (s) {
                values[i] = dupstr(s->name, "server name");
            }

            break;

        case WID_SERVER_CREATE_ROW_MAX_PLAYERS:
            values[i] = dynprintf("%u", global_config.server_max_players);
            break;

        case WID_SERVER_CREATE_ROW_START_SERVER:
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

        val = wid_server_create_button_value_string[i]
                        [wid_server_create_button_val[i]];
        if (val) {
            values[i] = dynprintf("%s", val);
        } else {
            values[i] = dynprintf("-");
        }
    }

    i = WID_SERVER_CREATE_ROW_SERVER_NAME;

    menu = wid_menu(0,
                vlarge_font,
                large_font,
                0, // on_update
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
                wid_server_create_button_mouse_down[i],

                (int) '2',
                keys[i + 1],
                values[i + 1], 
                wid_server_create_button_mouse_down[i + 1],

                (int) '3',
                keys[i + 2],
                values[i + 2], 
                wid_server_create_button_mouse_down[i + 2],

                (int) '4',
                keys[i + 3],
                values[i + 3], 
                wid_server_create_button_mouse_down[i + 3],

                (int) 'b', 
                "%%fmt=left$Back",
                (char*) 0,
                wid_server_create_go_back);

    for (i = WID_SERVER_CREATE_ROW_SERVER_NAME; 
         i < WID_SERVER_CREATE_MAX_SETTINGS; i++) {
        myfree(values[i]);
        myfree(keys[i]);
    }
}
