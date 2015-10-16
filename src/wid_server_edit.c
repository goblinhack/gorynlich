/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
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
    "%%fmt=left$Join the server",
};

static int saved_focus;
static widp menu;

static char *new_ip;
static int new_port;

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

void wid_server_edit_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (wid_server_edit_init_done) {
        wid_server_edit_init_done = false;

        wid_server_edit_hide();

        if (new_ip) {
            myfree(new_ip);
            new_ip = 0;
        }
    }
}

void wid_server_edit_hide (void)
{
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
    wid_server_edit_redo();
}

void wid_server_edit_redo (void)
{
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
    wid_server_replace(new_ip, new_port);

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

    wid_server_replace(new_ip, new_port);
    wid_server_join(new_ip, new_port);

    return (true);
}

static widp wid_port_number;

static void wid_port_number_ok (widp w, const char *text)
{
    wid_destroy(&wid_port_number);

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

    new_port = port;

    wid_server_edit_redo();
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

    char *tmp = dynprintf("%d", new_port ? 0 : DEFAULT_PORT);

    wid_port_number = wid_numpad(tmp ? tmp : "",
                                 "Enter port number",
                                 wid_port_number_ok,
                                 wid_port_number_cancel);

    myfree(tmp);

    return (true);
}

static widp wid_ip;

static void wid_ip_ok (widp w, const char *text)
{
    wid_destroy(&wid_ip);

    int a, b, c, d;
    int success = sscanf(text, "%u.%u.%u.%u", &a, &b, &c, &d);
    if (success != 4) {
        /*
         * Fail
         */
        MSG_BOX("Failed to parse IP address \"%s\", expecting a.b.c.d format",
                text);
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

    if (new_ip) {
        myfree(new_ip);
    }

    new_ip = dupstr(text, "new ip");

    wid_server_edit_redo();
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

    wid_ip = wid_numpad(new_ip ? new_ip : "",
                        "Enter IP address", wid_ip_ok, wid_ip_cancel);

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
    if (new_ip) {
        myfree(new_ip);
        new_ip = 0;
    }

    new_ip = dupstr(host, "new ip");
    new_port = port;

    wid_server_edit_visible();

    return (true);
}

static void wid_server_edit_menu (void)
{
    char *keys[WID_SERVER_CREATE_MAX_SETTINGS];
    char *values[WID_SERVER_CREATE_MAX_SETTINGS];

    memset(keys, 0, sizeof(keys));
    memset(values, 0, sizeof(values));

    if (!new_ip) {
        new_ip = dupstr("0.0.0.0", "new ip");
    }

    int i;
    for (i = WID_SERVER_CREATE_ROW_IP; 
         i < WID_SERVER_CREATE_MAX_SETTINGS; i++) {

        keys[i] = dynprintf("%s", wid_server_edit_button_name[i]);

        switch (i) {
        case WID_SERVER_CREATE_ROW_IP:
            values[i] = dynprintf("%%%%fmt=left$%s", new_ip);
            break;

        case WID_SERVER_CREATE_ROW_PORT:
            values[i] = dynprintf("%%%%fmt=left$%u", new_port);
            break;

        case WID_SERVER_CREATE_ROW_JOIN_SERVER:
            values[i] = dupstr("", "dummy");
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
