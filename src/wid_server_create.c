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
#include "wid_choose_game_type.h"
#include "string.h"
#include "wid_game_map_server.h"
#include "wid_game_map_client.h"
#include "socket.h"
#include "server.h"

#define WID_SERVER_CREATE_MAX_SETTINGS  5
#define WID_SERVER_CREATE_MAX_VAL      30 

enum {
    WID_SERVER_CREATE_ROW_SERVER_NAME,
    WID_SERVER_CREATE_ROW_PORT,
    WID_SERVER_CREATE_ROW_MAX_PLAYERS,
    WID_SERVER_CREATE_ROW_DEATH_MATCH,
    WID_SERVER_CREATE_ROW_INCLUDE_MONSTERS,
};

static const char *wid_server_create_button_col1[WID_SERVER_CREATE_MAX_SETTINGS] = {
    "Server name",
    "Port",
    "Max Players",
    "Death Match",
    "Monsters",
};

static const char *wid_server_create_button_col2[WID_SERVER_CREATE_MAX_SETTINGS] = {
    0,
    "+",
    "+",
    0,
    0,
};

static const char *wid_server_create_button_col3[WID_SERVER_CREATE_MAX_SETTINGS] = {
    0,
    "-",
    "-",
    0,
    0,
};

static const char *wid_server_create_button_col4
                        [WID_SERVER_CREATE_MAX_SETTINGS][WID_SERVER_CREATE_MAX_VAL] = {
    { 0 },
    { 0 },
    { 0 },
    { "Off", "On", 0 },
    { "Off", "On", 0 },
};

static uint8_t wid_server_create_name_mouse_down(widp w, 
                                                 int32_t x, int32_t y,
                                                 uint32_t button);
static uint8_t wid_server_create_port_mouse_down(widp w, 
                                                 int32_t x, int32_t y,
                                                 uint32_t button);
static uint8_t wid_server_create_max_players_mouse_down(widp w, 
                                                        int32_t x, int32_t y,
                                                        uint32_t button);

static const on_mouse_down_t wid_server_create_button_mouse_down[WID_SERVER_CREATE_MAX_SETTINGS] = {
    wid_server_create_name_mouse_down,
    wid_server_create_port_mouse_down,
    wid_server_create_max_players_mouse_down,
    0,
    0,
};

static uint32_t wid_server_create_button_val[WID_SERVER_CREATE_MAX_SETTINGS];
static char wid_server_create_button_sval[WID_SERVER_CREATE_MAX_SETTINGS][MAXSTR];

static widp wid_choose_game_type_background;
static widp wid_server_create_window;
static widp wid_server_create_go_back_button;
static widp wid_server_create_container;
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

    if (wid_choose_game_type_background) {
        wid_destroy_in(wid_choose_game_type_background, wid_hide_delay * 2);
        wid_choose_game_type_background= 0;
    }
}

void wid_server_create_visible (void)
{
    wid_server_create_menu();
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

        socketp sp = socket_find(s->ip, SOCKET_LISTEN);
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
    wid_server_create_hide();
    wid_choose_game_type_visible();

    return (true);
}

static uint8_t wid_server_start (widp w, int32_t x, int32_t y, uint32_t button)
{
    server_stop();

    server *s;

    TREE_WALK_REVERSE(local_servers, s) {

        IPaddress ip = {0};

        if (SDLNet_ResolveHost(&ip, SERVER_DEFAULT_HOST, 
                               global_config.server_port)) {
            MSG_BOX("Open socket, cannot resolve %s:%u",
                SERVER_DEFAULT_HOST, global_config.server_port);
            return (false);
        }

        if (!server_start(ip)) {
            return (true);
        }

        break;
    }

    wid_server_create_redo();

    MSG(POPUP, "Server started! Woot!");

    wid_server_create_hide();
    wid_choose_game_type_visible();

    return (true);
}

static uint8_t wid_server_stop (widp w, int32_t x, int32_t y, uint32_t button)
{
    server_stop();

    wid_server_create_redo();

    return (true);
}

static uint8_t wid_server_create_key_event (widp w, const SDL_KEYSYM *key)
{
    switch (key->sym) {
        case 'q':
        case 'b':
        case SDLK_ESCAPE:
            wid_server_create_hide();
            wid_choose_game_type_visible();
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

            if (a > MAX_PLAYERS) {
                MSG_BOX("Max players limited to %d", global_config.server_max_players);
            }

            global_config.server_max_players = min(MAX_PLAYERS, a);
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

void wid_server_create_destroy (void)
{
    if (wid_server_create_window) {
        wid_destroy(&wid_server_create_window);
    }

    if (wid_server_create_go_back_button) {
        wid_destroy(&wid_server_create_go_back_button);
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

static uint8_t wid_server_create_col1_mouse_event (widp w,
                                                    int32_t x, int32_t y,
                                                    uint32_t button)
{
    return (true);
}

static uint8_t wid_server_create_col2_mouse_event (widp w,
                                                    int32_t x, int32_t y,
                                                    uint32_t button)
{
    /*
     * Increment.
     */
    int32_t row = (typeof(row)) (intptr_t) wid_get_client_context(w);
    int32_t val = wid_server_create_button_val[row];

    if (!wid_server_create_button_col4[row][val+1]) {
        return (true);
    }

    wid_server_create_button_val[row]++;

    wid_destroy_nodelay(&wid_server_create_container);
    wid_server_create_menu();

    wid_server_config_changed();

    return (true);
}

static uint8_t wid_server_create_col3_mouse_event (widp w,
                                                    int32_t x, int32_t y,
                                                    uint32_t button)
{
    /*
     * Decrement.
     */
    int32_t row = (typeof(row)) (intptr_t) wid_get_client_context(w);
    int32_t val = wid_server_create_button_val[row];

    if (!val) {
        return (true);
    }

    wid_server_create_button_val[row]--;

    wid_destroy_nodelay(&wid_server_create_container);
    wid_server_create_menu();

    wid_server_config_changed();

    return (true);
}

static uint8_t wid_server_create_col4_mouse_event (widp w,
                                                    int32_t x, int32_t y,
                                                    uint32_t button)
{
    /*
     * Invert.
     */
    int32_t row = (typeof(row)) (intptr_t) wid_get_client_context(w);

    wid_server_create_button_val[row] = !wid_server_create_button_val[row];

    wid_destroy_nodelay(&wid_server_create_container);
    wid_server_create_menu();

    wid_server_config_changed();

    return (true);
}

static void wid_server_create_read (void)
{
    server *s;

    TREE_WALK_REVERSE(local_servers, s) {

        strncpy(wid_server_create_button_sval[WID_SERVER_CREATE_ROW_SERVER_NAME],
                s->name,
                sizeof(wid_server_create_button_sval[WID_SERVER_CREATE_ROW_SERVER_NAME]));

        {
            char *tmp = iprawporttodynstr(s->ip);
            strncpy(wid_server_create_button_sval[WID_SERVER_CREATE_ROW_PORT],
                    tmp,
                    sizeof(wid_server_create_button_sval[WID_SERVER_CREATE_ROW_PORT]));
            myfree(tmp);
        }

        {
            char *tmp = dynprintf("%u", global_config.server_max_players);
            strncpy(wid_server_create_button_sval[WID_SERVER_CREATE_ROW_MAX_PLAYERS],
                    tmp,
                    sizeof(wid_server_create_button_sval[WID_SERVER_CREATE_ROW_MAX_PLAYERS]));
            myfree(tmp);
        }

        wid_server_create_button_val[WID_SERVER_CREATE_ROW_MAX_PLAYERS] =
            global_config.server_max_players;
        wid_server_create_button_val[WID_SERVER_CREATE_ROW_DEATH_MATCH] =
            0;
        wid_server_create_button_val[WID_SERVER_CREATE_ROW_INCLUDE_MONSTERS] =
            0;

        break;
    }
}

static void wid_server_config_changed (void)
{
    config_save();
}

static void wid_choose_game_type_bg_create (void)
{
    widp wid;

    if (wid_choose_game_type_background) {
        return;
    }

    {
        wid = wid_choose_game_type_background = wid_new_window("bg");

        float f = (1024.0 / 680.0);

        fpoint tl = { 0.0, 0.0 };
        fpoint br = { 1.0, f };

        wid_set_tl_br_pct(wid, tl, br);

        wid_set_tex(wid, 0, "title3");

        wid_lower(wid);

        color c;
        c = WHITE;
        wid_set_mode(wid, WID_MODE_NORMAL);
        wid_set_color(wid, WID_COLOR_TL, c);
        wid_set_color(wid, WID_COLOR_BR, c);
        wid_set_color(wid, WID_COLOR_BG, c);

        wid_update(wid);
    }
}

static void wid_server_create_menu (void)
{
    if (!wid_server_create_window) {
        wid_server_create_read();

        widp w = wid_server_create_window = wid_new_rounded_window("wid settings");

        fpoint tl = {0.2, 0.05};
        fpoint br = {0.8, 0.95};

        wid_set_tl_br_pct(w, tl, br);
        wid_set_font(w, small_font);

        wid_set_tex(w, 0, "title2");
    }

    {
        widp w = wid_server_create_container =
            wid_new_container(wid_server_create_window, "wid settings container");

        fpoint tl = {0.0, 0.0};
        fpoint br = {1.0, 1.0};

        wid_set_tl_br_pct(w, tl, br);
    }

    {
        fpoint tl = {0.0, 0.0};
        fpoint br = {1.0, 0.15};

        widp w = wid_new_container(wid_server_create_window, "wid settings title");

        wid_set_tl_br_pct(w, tl, br);

        wid_set_text(w, "Server Settings");
        wid_set_font(w, small_font);
        wid_set_color(w, WID_COLOR_TEXT, GOLD);

        wid_set_text_outline(w, true);
    }

    {
        uint32_t i;

        for (i=0; i<ARRAY_SIZE(wid_server_create_button_col1); i++)
        {
            widp w = wid_new_square_button(wid_server_create_container,
                                           wid_server_create_button_col1[i]);

            fpoint tl = {0.05, 0.2};
            fpoint br = {0.48, 0.28};

            double height = 0.08;

            br.y += (double)i * height;
            tl.y += (double)i * height;

            wid_set_tl_br_pct(w, tl, br);
            wid_set_text(w, wid_server_create_button_col1[i]);
            wid_set_font(w, small_font);

            color c = BLACK;

            c.a = 200;
            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_color(w, WID_COLOR_BG, c);

            c.a = 255;
            wid_set_mode(w, WID_MODE_OVER);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_NORMAL);

            wid_set_on_mouse_down(w, wid_server_create_col1_mouse_event);
            wid_set_client_context(w, (void*)(uintptr_t)i);
            wid_set_bevel(w,0);
        }
    }

    {
        uint32_t i;

        for (i=0; i<ARRAY_SIZE(wid_server_create_button_col1); i++) {

            if (!wid_server_create_button_col2[i]) {
                continue;
            }

            widp w = wid_new_square_button(wid_server_create_container,
                                           wid_server_create_button_col2[i]);

            fpoint tl = {0.49, 0.2};
            fpoint br = {0.595, 0.28};

            double height = 0.08;

            br.y += (double)i * height;
            tl.y += (double)i * height;

            wid_set_tl_br_pct(w, tl, br);
            wid_set_text(w, wid_server_create_button_col2[i]);
            wid_set_font(w, small_font);

            color c = WHITE;

            c.a = 200;
            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_color(w, WID_COLOR_BG, c);

            c.a = 250;
            wid_set_mode(w, WID_MODE_OVER);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_NORMAL);

            wid_set_on_mouse_down(w, wid_server_create_col2_mouse_event);
            wid_set_client_context(w, (void*)(uintptr_t)i);
            wid_set_bevel(w,0);

            wid_set_tex(w, 0, "button_green");
            wid_set_square(w);
        }
    }

    {
        uint32_t i;

        for (i=0; i<ARRAY_SIZE(wid_server_create_button_col1); i++) {

            if (!wid_server_create_button_col3[i]) {
                continue;
            }

            widp w = wid_new_square_button(wid_server_create_container,
                                           wid_server_create_button_col3[i]);

            fpoint tl = {0.605, 0.2};
            fpoint br = {0.71, 0.28};

            double height = 0.08;

            br.y += (double)i * height;
            tl.y += (double)i * height;

            wid_set_tl_br_pct(w, tl, br);
            wid_set_text(w, wid_server_create_button_col3[i]);
            wid_set_font(w, small_font);

            color c = WHITE;

            c.a = 200;
            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_color(w, WID_COLOR_BG, c);

            c.a = 250;
            wid_set_mode(w, WID_MODE_OVER);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_NORMAL);

            wid_set_on_mouse_down(w, wid_server_create_col3_mouse_event);
            wid_set_client_context(w, (void*)(uintptr_t)i);
            wid_set_bevel(w,0);

            wid_set_tex(w, 0, "button_red");
            wid_set_square(w);
        }
    }

    {
        uint32_t i;

        for (i=0; i<ARRAY_SIZE(wid_server_create_button_col1); i++) {

            if (!wid_server_create_button_col4[i]) {
                continue;
            }

            widp w = wid_new_square_button(wid_server_create_container,
                                           wid_server_create_button_col3[i]);

            fpoint tl = {0.72, 0.2};
            fpoint br = {0.95, 0.28};

            double height = 0.08;

            br.y += (double)i * height;
            tl.y += (double)i * height;

            wid_set_tl_br_pct(w, tl, br);
            if (wid_server_create_button_sval[i][0]) {
                wid_set_text(w, wid_server_create_button_sval[i]);
            } else {
                wid_set_text(w, wid_server_create_button_col4[i][wid_server_create_button_val[i]]);
            }
            wid_set_font(w, small_font);
 
            color c = WHITE;

            c.a = 200;
            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_color(w, WID_COLOR_BG, c);

            c.a = 255;
            wid_set_mode(w, WID_MODE_OVER);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_NORMAL);

            if (wid_server_create_button_mouse_down[i]) {
                wid_set_on_mouse_down(w, wid_server_create_button_mouse_down[i]);
            } else {
                wid_set_on_mouse_down(w, wid_server_create_col4_mouse_event);

            }
            wid_set_client_context(w, (void*)(uintptr_t)i);
            wid_set_bevel(w,0);

            wid_set_tex(w, 0, "button_black");
            wid_set_square(w);
        }
    }

    {
        widp w = wid_new_rounded_small_button(wid_server_create_container,
                                              "back");

        fpoint tl = {0.70, 0.80};
        fpoint br = {0.90, 0.90};

        wid_set_tl_br_pct(w, tl, br);
        wid_set_text(w, "%%tile=button_b$Back");
        wid_set_font(w, small_font);
        wid_set_no_shape(w);

        wid_set_mode(w, WID_MODE_NORMAL);
        wid_set_color(w, WID_COLOR_TEXT, GRAY);

        wid_set_mode(w, WID_MODE_OVER);
        wid_set_color(w, WID_COLOR_TEXT, WHITE);

        wid_set_mode(w, WID_MODE_NORMAL);

        wid_set_on_mouse_down(w, wid_server_create_go_back);
        wid_set_on_key_down(w, wid_server_create_key_event);

        wid_set_tex(w, 0, "button_black");
    }

    {
        fpoint tl = {0.1, 0.8};
        fpoint br = {0.45, 0.88};

        widp w = wid_new_square_button(wid_server_create_container,
                                       "wid server add");
        wid_visible(w, 0);

        wid_set_tl_br_pct(w, tl, br);

        wid_set_text(w, "Start the server");

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

        wid_set_on_mouse_down(w, wid_server_start);
    }

    wid_raise(wid_server_create_window);

    wid_update(wid_server_create_window);

    wid_set_tex(wid_server_create_window, 0, "window_gothic");
    wid_set_square(wid_server_create_window);

    wid_move_to_pct_centered(wid_server_create_window, 0.5, 0.5);
    wid_raise(wid_server_create_window);

    wid_choose_game_type_bg_create();
}
