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

static const char *server_dir_and_file = "gorynlich-servers.txt";

static widp wid_server_window;
static widp wid_server_container;
static boolean wid_server_init_done;

static void wid_server_create(void);
static void wid_server_destroy(void);

typedef struct server_ {
    tree_key_two_int tree;

    IPaddress ip;
    char *host;
    uint16_t port;
    char *host_and_port_str;
} server;

tree_rootp servers;

static void server_add (const server *s)
{
    static uint32_t port_tiebreak;
    server *h;

    if (!servers) {
        servers = tree_alloc(TREE_KEY_TWO_INTEGER, "TREE ROOT: servers");
    }

    h = (typeof(h)) myzalloc(sizeof(*h), "TREE NODE: server");
    h->host = dupstr(s->host, "hostname");
    h->port = s->port;

    if ((SDLNet_ResolveHost(&h->ip, 
                            h->host,
                            h->port)) == -1) {
        LOG("Cannot resolve host %s port %u", 
            h->host, 
            h->port);

        h->host_and_port_str = dynprintf("%s:%u", h->host, h->port);
    } else {
        h->host_and_port_str = iptodynstr(h->ip);

        LOG("Resolve host %s port %u", h->host, h->port);
    }

    h->tree.key1 = SDLNet_Read32(&h->ip.host);
    h->tree.key2 = SDLNet_Read16(&h->ip.port);

    while (!tree_insert(servers, &h->tree.node)) {
        h->tree.key2 = port_tiebreak++;
    }
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

static boolean wid_server_mouse_event (widp w, int32_t x, int32_t y,
                                        uint32_t button)
{
    wid_server_hide();

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

static void wid_server_create (void)
{
    if (wid_server_window) {
        return;
    }

    widp w = wid_server_window = wid_new_rounded_window("wid settings");

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
    wid_set_on_mouse_up(w, wid_server_mouse_event);
    wid_set_on_key_down(w, wid_server_key_event);

    wid_set_on_mouse_motion(w, wid_server_receive_mouse_motion);

    {
        widp w = wid_server_container =
            wid_new_container(wid_server_window, "wid settings container");

        fpoint tl = {0.0, 0.1};
        fpoint br = {1.0, 0.9};

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

        wid_set_tl_br_pct(w, tl, br);

        wid_set_text(w, "Servers");
        wid_set_font(w, large_font);
        wid_set_color(w, WID_COLOR_TEXT, STEELBLUE);
        wid_set_color(w, WID_COLOR_BG, BLACK);

        wid_set_text_outline(w, true);
    }

    {
        uint32_t i = 0;
        server *h;

        TREE_WALK_REVERSE(servers, h) {
            widp w = wid_new_square_button(wid_server_container,
                                           "server name");

            fpoint tl = {0.05, 0.0};
            fpoint br = {0.51, 0.1};

            float height = 0.08;

            if (i < 1) {
                wid_set_color(w, WID_COLOR_TEXT, YELLOW);
            } else if (i < 4) {
                wid_set_color(w, WID_COLOR_TEXT, GREEN);
            } else {
                wid_set_color(w, WID_COLOR_TEXT, SKYBLUE);
            }

            br.y += (float)i * height;
            tl.y += (float)i * height;

            wid_set_tl_br_pct(w, tl, br);

            wid_set_text(w, h->host_and_port_str);

            color c = BLACK;

            c.a = 100;
            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_OVER);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_NORMAL);

            wid_set_bevel(w,0);
            wid_set_no_shape(w);
            wid_set_text_outline(w, true);
            wid_set_font(w, large_font);
            wid_set_text_lhs(w, true);

            i++;
        }
    }

    {
        uint32_t i = 0;
        server *h;

        TREE_WALK_REVERSE(servers, h) {
            widp w = wid_new_square_button(wid_server_container,
                                           "server value");

            fpoint tl = {0.52, 0.0};
            fpoint br = {0.95, 0.1};

            float height = 0.08;

            if (i < 1) {
                wid_set_color(w, WID_COLOR_TEXT, YELLOW);
            } else if (i < 4) {
                wid_set_color(w, WID_COLOR_TEXT, GREEN);
            } else {
                wid_set_color(w, WID_COLOR_TEXT, SKYBLUE);
            }

            br.y += (float)i * height;
            tl.y += (float)i * height;

            wid_set_tl_br_pct(w, tl, br);

            char *tmp = dynprintf("%08d", h->tree.key1);
            wid_set_text(w, tmp);
            myfree(tmp);

            color c = BLUE;

            c.a = 100;
            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_OVER);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_NORMAL);

            wid_set_bevel(w,0);
            wid_set_no_shape(w);
            wid_set_text_outline(w, true);
            wid_set_text_fixed_width(w, true);
            wid_set_font(w, large_font);

            i++;
        }
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
}

static boolean demarshal_server (demarshal_p ctx, server *p)
{
    boolean rc;

    rc = true;

    rc = rc && GET_OPT_NAMED_STRING(ctx, "host", p->host);
    rc = rc && GET_OPT_NAMED_UINT16(ctx, "port", p->port);

    return (rc);
}

static void marshal_server (marshal_p ctx, server *p)
{
//    if (p->host) {
        PUT_NAMED_STRING(ctx, "host", p->host);
        PUT_NAMED_INT16(ctx, "port", p->port);
//    }
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

    server *h;

    TREE_WALK_REVERSE(servers, h) {
        marshal_server(ctx, h);
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

    server h;

    if ((ctx = demarshal(file))) {
        while (demarshal_server(ctx, &h)) {
            server_add(&h);
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
    server *h;

    TREE_WALK_REVERSE(servers, h) {

        if (score_in > (uint32_t) h->tree.key1) {
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
