/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "color.h"
#include "wid_server_join.h"
#include "wid_server_edit.h"
#include "wid_game_map_client.h"
#include "wid_choose_game_type.h"
#include "wid_intro.h"
#include "string_util.h"
#include "marshal.h"
#include "socket_util.h"
#include "client.h"
#include "string_ext.h"
#include "wid_tooltip.h"
#include "wid_menu.h"

static const char *config_file = "gorynlich-remote-servers.txt";

static widp wid_server_join_window;
static widp wid_server_join_background;
static uint8_t wid_server_join_init_done;

static void wid_server_join_destroy(void);
static void wid_server_join_menu_destroy(void);
static void wid_server_join_bg_create(void);
static uint8_t wid_server_load_remote_server_list(void);

#define WID_MAX_SERVERS 100
#define WID_MAX_SERVERS_ON_SCREEN 20
static widp wid_server_join_menu;
static int saved_focus = 1;
static void wid_server_join_menu_create(void);

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
    gsocketp socket;
    uint8_t walked;
    char name[SMALL_STRING_LEN_MAX];
} server;

static server *row_server[WID_MAX_SERVERS];
static widp wid_server_stats_window;
static widp wid_server_stats_bars;

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

    if ((address_resolve(&s->ip, s->host, s->port)) == -1) {
        LOG("Cannot resolve host [%s] port %u", s->host, s->port);

        s->host_and_port_str = dynprintf("%s:%u", s->host, s->port);
    } else {
        s->host_and_port_str = iptodynstr(s->ip);
    }

    LOG("Add server %s", s->host_and_port_str);

    /*
     * Connector.
     */
    if (is_client) {
        gsocketp sp = socket_connect_from_client(s->ip);
        if (sp) {
            s->quality = socket_get_quality(sp);
            s->avg_latency_rtt = socket_get_avg_latency_rtt(sp);
            s->min_latency_rtt = socket_get_min_latency_rtt(sp);
            s->max_latency_rtt = socket_get_max_latency_rtt(sp);
            strlcpy(s->name, socket_get_name(sp), sizeof(s->name) - 1);
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
        wid_destroy_nodelay(&wid_server_stats_bars);
    }

    if (!remote_servers) {
        return;
    }

    /*
     * Remove this socket.
     */
    server *s;
    TREE_WALK(remote_servers, s) {
        gsocketp sp = socket_find(s->ip, SOCKET_CONNECT + 1);
        if (sp) {
            socket_disconnect(sp);
            break;
        }
    }

    LOG("Remove server %s", s_in->host_and_port_str);

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

    wid_server_join_menu_destroy();

    if (wid_server_join_background) {
        wid_destroy(&wid_server_join_background);
    }

    if (wid_server_stats_window) {
        wid_destroy_nodelay(&wid_server_stats_window);
        wid_destroy_nodelay(&wid_server_stats_bars);
    }

    /*
     * Leave all other sockets other than the joined one.
     */
    server *s;

    /*
     * Leave all other sockets.
     */
    TREE_WALK(remote_servers, s) {
        gsocketp sp = socket_find(s->ip, SOCKET_CONNECT + 1);
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
            "10.0.1.10",
            "10.0.1.11",
            "10.0.1.12",
            "10.0.1.13",
            "10.0.1.14",
            "10.0.1.15",
            "10.0.1.16",
            "10.0.1.17",
            "10.0.1.18",
            "10.0.1.19",
            "10.0.1.20",
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

    wid_server_join_redo(false);
}

static uint8_t wid_server_join_go_back (widp w, int32_t x, int32_t y, 
                                        uint32_t button)
{
    wid_server_join_hide();
    wid_choose_game_type_visible();

    return (true);
}

uint8_t wid_server_join (char *host, int port)
{
    if (!client_socket_join(host, 0, port, true /* quiet */)) {
        return (false);
    }

    wid_server_join_hide();

    /*
     * Leave all other sockets.
     */
    server *s;
    TREE_WALK(remote_servers, s) {
        gsocketp sp = socket_find(s->ip, SOCKET_CONNECT + 1);
        if (sp && (sp != client_joined_server)) {
            socket_disconnect(sp);
            continue;
        }
    }

    wid_intro_hide();
    wid_game_map_client_visible();

    return (true);
}

uint8_t wid_server_replace (char *host, int port)
{
    if (saved_focus) {
        server *s;
        s = row_server[saved_focus - 1];
        if (s) {
            server_remove(s);
        }
        row_server[saved_focus] = 0;
    }

    server s;
    memset(&s, 0, sizeof(s));
    s.host = host;
    s.port = port;
    server_add(&s);

    wid_server_join_visible();
    wid_server_join_redo(true);

    return (true);
}

static void wid_server_join_display (server *s)
{
    if (!s) {
        return;
    }

    if (wid_server_stats_window) {
        wid_destroy_nodelay(&wid_server_stats_window);
        wid_destroy_nodelay(&wid_server_stats_bars);
    }

    gsocketp sp = socket_find(s->ip, SOCKET_CONNECT + 1);
    if (!sp) {
        return;
    }

    color c = BLACK;
    c.a = 100;

    /*
     * Add server stats.
     */
    char *tmp = 0;

    {
        int size;
        int used;

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
    }

    char *tmp2 = dynprintf(
        "%%%%fg=green$%%%%fmt=left$Server name\t\t%%%%fg=red$%-45s\n"
        "%%%%fg=green$%%%%fmt=left$Avg latency round trip time\t%%%%fg=red$%u ms\n"
        "%%%%fg=green$%%%%fmt=left$Min latency round trip time\t%%%%fg=red$%u ms\n"
        "%%%%fg=green$%%%%fmt=left$Max latency round trip time\t%%%%fg=red$%u ms\n"
        "%%%%fg=green$%%%%fmt=left$Current players, %%%%fg=red$%u, %u max\n\n"
        "%s",
        s->name,
        sp->avg_latency_rtt,
        sp->min_latency_rtt,
        sp->max_latency_rtt,
        socket_get_max_players(sp),
        socket_get_current_players(sp),
        tmp);

    wid_server_stats_window = wid_tooltip(tmp2, 0.05, 0.65, vsmall_font);
    myfree(tmp);
    myfree(tmp2);

    wid_move_to_pct(wid_server_stats_window, 0.1, 0.74);
    wid_set_tex(wid_server_stats_window, 0, 0);
    wid_set_square(wid_server_stats_window);
    wid_set_color(wid_server_stats_window, WID_COLOR_BG, c);
    wid_set_color(wid_server_stats_window, WID_COLOR_TL, c);
    wid_set_color(wid_server_stats_window, WID_COLOR_BR, c);
    wid_set_bevel(wid_server_stats_window, 0);

    wid_server_stats_bars = wid_new_square_window("bars");
    fpoint tl = {0.50, 0.74};
    fpoint br = {0.85, 0.99};
    wid_set_tl_br_pct(wid_server_stats_bars, tl, br);
    wid_set_color(wid_server_stats_bars, WID_COLOR_BG, c);
    wid_set_color(wid_server_stats_bars, WID_COLOR_TL, c);
    wid_set_color(wid_server_stats_bars, WID_COLOR_BR, c);

    /*
     * Draw bars.
     */
    {
        uint32_t i;

        for (i = 0; i < SOCKET_PING_SEQ_NO_RANGE; i++) {
            double dx = 1.0 / (double)SOCKET_PING_SEQ_NO_RANGE;
            
            widp w = wid_new_square_button(wid_server_stats_bars, "bar");
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

    wid_update(wid_server_stats_bars);
    wid_visible(wid_server_stats_bars, 0);
    wid_raise(wid_server_stats_bars);
}

void wid_server_join_redo (uint8_t redo)
{
    if (redo) {
        if (!wid_server_join_menu) {
            return;
        }
    }

    if (redo) {
        /*
         * Save the row we're interested in.
         */
        wid_server_join_menu_destroy();
    }

    wid_server_join_bg_create();

    wid_server_join_menu_create();

    /*
     * Get the server we're interested in from the focus.
     */
    wid_menu_ctx *ctx = (typeof(ctx)) wid_get_client_context(wid_server_join_menu);
    verify(ctx);

    int32_t row = ctx->focus - 1;

    server *s = row_server[row];
    if (s) {
        wid_server_join_display(s);
    }
}

void wid_server_join_destroy (void)
{
    if (wid_server_join_window) {
        wid_destroy_nodelay(&wid_server_join_window);
    }
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

static uint8_t wid_server_mouse_event (widp w,
                                       int32_t x, int32_t y,
                                       uint32_t button)
{
    wid_menu_ctx *ctx = (typeof(ctx)) wid_get_client_context(w);
    verify(ctx);

    int32_t row = ctx->focus - 1;
    server *s = row_server[row];
    if (s) {
        wid_server_edit_init(s->host, s->port);
    } else {
        wid_server_edit_init("", DEFAULT_PORT);
    }

    wid_server_join_menu_destroy();
    wid_destroy_nodelay(&wid_server_stats_window);
    wid_destroy_nodelay(&wid_server_stats_bars);

    return (true);
}

static uint8_t wid_server_back_mouse_event (widp w,
                                            int32_t x, int32_t y,
                                            uint32_t button)
{
    wid_destroy(&wid_server_join_menu);
    wid_server_join_go_back(w, x, y, button);

    return (true);
}

static void wid_server_join_menu_destroy (void)
{
    if (!wid_server_join_menu) {
        return;
    }

    wid_menu_ctx *ctx = 
                    (typeof(ctx)) wid_get_client_context(wid_server_join_menu);
    verify(ctx);

    /*
     * Save the focus so when we remake the menu we are at the same entry.
     */
    saved_focus = ctx->focus;

    wid_destroy_nodelay(&wid_server_join_menu);
}

static void wid_server_join_bg_create (void)
{
    widp wid;

    if (wid_server_join_background) {
        return;
    }

    {
        wid = wid_server_join_background = wid_new_window("bg");

        float f = (1024.0 / 680.0);

        fpoint tl = { 0.0, 0.0 };
        fpoint br = { 1.0, f };

        wid_set_tl_br_pct(wid, tl, br);

        wid_set_tex(wid, 0, "title5");

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

static void wid_server_join_tick (widp w)
{
    if (!wid_server_join_menu) {
        return;
    }

    wid_menu_ctx *ctx = 
                    (typeof(ctx)) wid_get_client_context(wid_server_join_menu);
    verify(ctx);

    /*
     * Save the focus so when we remake the menu we are at the same entry.
     */
    if (ctx->focus != saved_focus) {
        saved_focus = ctx->focus;
        wid_server_join_redo(true);
    }
}

static void wid_server_join_menu_create (void)
{
    if (wid_server_join_menu) {
        return;
    }

    int i;
    server *s;
    char *col_name[WID_MAX_SERVERS];
    char *col_host[WID_MAX_SERVERS];
    char *col_ipaddr[WID_MAX_SERVERS];
    char *col_port[WID_MAX_SERVERS];
    char *col_quality[WID_MAX_SERVERS];

    memset(col_name, 0, sizeof(col_name));
    memset(col_host, 0, sizeof(col_host));
    memset(col_ipaddr, 0, sizeof(col_ipaddr));
    memset(col_port, 0, sizeof(col_port));
    memset(col_quality, 0, sizeof(col_quality));
    memset(row_server, 0, sizeof(row_server));

    /*
     * First add only servers the user explicitly added. Then add all the 
     * stuff we automatically added as likely addresses.
     */
    i = 0;
    { TREE_WALK(remote_servers, s) {

        if (i >= WID_MAX_SERVERS) {
            break;
        }

        if (s->auto_add) {
            continue;
        }

        gsocketp sp = socket_find(s->ip, SOCKET_CONNECT);
        if (!sp) {
            continue;
        }

        /*
         * Don't check quality for stuff we manually added, always show.
         */
        row_server[i] = s;

        {
            char *tmp = iprawtodynstr(s->ip);
            col_ipaddr[i] = dynprintf("%%%%fmt=left$%s", tmp);
            myfree(tmp);
        }

        {
            char *tmp = iprawporttodynstr(s->ip);
            col_port[i] = dynprintf("%s", tmp);
            myfree(tmp);
        }

        col_host[i] = dynprintf("%%%%fmt=left$%s", s->host);
        col_quality[i] = dynprintf("bar:%d,%d", sp->quality, 100);
        i++;
    } }

    /*
     * Second pass.
     */
    { TREE_WALK(remote_servers, s) {

        if (i >= WID_MAX_SERVERS) {
            break;
        }

        if (!s->auto_add) {
            continue;
        }

        gsocketp sp = socket_find(s->ip, SOCKET_CONNECT);
        if (!sp) {
            continue;
        }

        if (!sp->quality) {
            continue;
        }

        row_server[i] = s;

        if (s->name[0]) {
            col_name[i] = dynprintf("%%%%fmt=left$%s", s->name);
        }

        col_host[i] = dynprintf("%%%%fmt=left$%s", s->host);

        {
            char *tmp = iprawtodynstr(s->ip);
            col_ipaddr[i] = dynprintf("%%%%fmt=left$%s", tmp);
            myfree(tmp);
        }

        {
            char *tmp = iprawporttodynstr(s->ip);
            col_port[i] = dynprintf("%s", tmp);
            myfree(tmp);
        }

        col_quality[i] = dynprintf("bar:%d,%d", sp->quality, 100);
        i++;
    } }

    for (i = 0; i < WID_MAX_SERVERS; i++) {
        if (!col_name[i] || (col_name[i] && col_name[i][0] == '\0')) {
            col_name[i] = dynprintf("-");
        }
        if (!col_host[i]) {
            col_host[i] = dynprintf("-");
        }
        if (!col_ipaddr[i]) {
            col_ipaddr[i] = dynprintf("-");
        }
        if (!col_port[i]) {
            col_port[i] = dynprintf("-");
        }
        if (!col_quality[i]) {
            col_quality[i] = dynprintf("bar:%d,%d", 0, 100);
        }
    }

    wid_server_join_menu = wid_menu(0,
                med_font,
                med_font,
                0, // on_update
                0.5, /* x */
                0.4, /* y */
                5, /* columns */
                saved_focus, /* focus */
                WID_MAX_SERVERS_ON_SCREEN + 2, /* items */

                /*
                 * Column widths
                 */
                (double) 0.2, (double) 0.2, (double) 0.22, (double) 0.22,
                (double) 0.08,

                (int) '\0', 
                "Server", "Host", "IP addr", "Port", "Quality",
                (on_mouse_down_t) 0,

                (int) '\0', 
                col_name[0], 
                col_host[0], 
                col_ipaddr[0], 
                col_port[0], 
                col_quality[0], 
                wid_server_mouse_event,

                (int) '\0', 
                col_name[1], 
                col_host[1], 
                col_ipaddr[1], 
                col_port[1], 
                col_quality[1], 
                wid_server_mouse_event,

                (int) '\0', 
                col_name[2], 
                col_host[2], 
                col_ipaddr[2], 
                col_port[2], 
                col_quality[2], 
                wid_server_mouse_event,

                (int) '\0', 
                col_name[3], 
                col_host[3], 
                col_ipaddr[3], 
                col_port[3], 
                col_quality[3], 
                wid_server_mouse_event,

                (int) '\0', 
                col_name[4], 
                col_host[4], 
                col_ipaddr[4], 
                col_port[4], 
                col_quality[4], 
                wid_server_mouse_event,

                (int) '\0', 
                col_name[5], 
                col_host[5], 
                col_ipaddr[5], 
                col_port[5], 
                col_quality[5], 
                wid_server_mouse_event,

                (int) '\0', 
                col_name[6], 
                col_host[6], 
                col_ipaddr[6], 
                col_port[6], 
                col_quality[6], 
                wid_server_mouse_event,

                (int) '\0', 
                col_name[7], 
                col_host[7], 
                col_ipaddr[7], 
                col_port[7], 
                col_quality[7], 
                wid_server_mouse_event,

                (int) '\0', 
                col_name[8], 
                col_host[8], 
                col_ipaddr[8], 
                col_port[8], 
                col_quality[8], 
                wid_server_mouse_event,

                (int) '\0', 
                col_name[9], 
                col_host[9], 
                col_ipaddr[9], 
                col_port[9], 
                col_quality[9], 
                wid_server_mouse_event,

                (int) '\0', 
                col_name[10], 
                col_host[10], 
                col_ipaddr[10], 
                col_port[10], 
                col_quality[10], 
                wid_server_mouse_event,

                (int) '\0', 
                col_name[11], 
                col_host[11], 
                col_ipaddr[11], 
                col_port[11], 
                col_quality[11], 
                wid_server_mouse_event,

                (int) '\0', 
                col_name[12], 
                col_host[12], 
                col_ipaddr[12], 
                col_port[12], 
                col_quality[12], 
                wid_server_mouse_event,

                (int) '\0', 
                col_name[13], 
                col_host[13], 
                col_ipaddr[13], 
                col_port[13], 
                col_quality[13], 
                wid_server_mouse_event,

                (int) '\0', 
                col_name[14], 
                col_host[14], 
                col_ipaddr[14], 
                col_port[14], 
                col_quality[14], 
                wid_server_mouse_event,

                (int) '\0', 
                col_name[15], 
                col_host[15], 
                col_ipaddr[15], 
                col_port[15], 
                col_quality[15], 
                wid_server_mouse_event,

                (int) '\0', 
                col_name[16], 
                col_host[16], 
                col_ipaddr[16], 
                col_port[16], 
                col_quality[16], 
                wid_server_mouse_event,

                (int) '\0', 
                col_name[17], 
                col_host[17], 
                col_ipaddr[17], 
                col_port[17], 
                col_quality[17], 
                wid_server_mouse_event,

                (int) '\0', 
                col_name[18], 
                col_host[18], 
                col_ipaddr[18], 
                col_port[18], 
                col_quality[18], 
                wid_server_mouse_event,

                (int) '\0', 
                col_name[19], 
                col_host[19], 
                col_ipaddr[19], 
                col_port[19], 
                col_quality[19], 
                wid_server_mouse_event,

                (int) 'b', 
                "Back", 
                (char*) 0, // host
                (char*) 0, // ipaddr
                (char*) 0, // port
                (char*) 0, // quality
                wid_server_back_mouse_event);

    for (i = 0; i < WID_MAX_SERVERS; i++) {
        myfree(col_name[i]);
        myfree(col_host[i]);
        myfree(col_ipaddr[i]);
        myfree(col_port[i]);
        myfree(col_quality[i]);
    }

    wid_set_on_tick(wid_server_join_menu, wid_server_join_tick);
}
