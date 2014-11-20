/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "tex.h"
#include "init_fn.h"
#include "wid_intro_demo.h"
#include "wid_intro.h"
#include "glapi.h"
#include "thing_template.h"
#include "time.h"

#define MAX_DEMO_PLAYERS 20
static int demo_player_count;
static widp demo_players[MAX_DEMO_PLAYERS];
static float demo_player_x[MAX_DEMO_PLAYERS];
static int demo_player_dead[MAX_DEMO_PLAYERS];
static tpp demo_player_tp[MAX_DEMO_PLAYERS];
static int demo_player_speed[MAX_DEMO_PLAYERS];

static float x = -1.0;
static double y = 0.1;
static double dy = 0.0005;

static void wid_rock_tick (void)
{
    y += dy;
    x += 0.009;

    static const double wall_start = 0.25;
    static const double accell_down = 1.11;
    static const double friction_up = 0.90;
    static const double elasticity = 0.6;

    if (y > wall_start) {
        y = wall_start;
        dy = -dy * elasticity;
        y += dy;

        int i;

        static int cnt;

        cnt++;

        if (cnt == 2) {
            for (i = 0; i < demo_player_count; i++) {
                widp wid = demo_players[i];

                if ((demo_player_x[i] > 0.25) && (demo_player_x[i] < 0.4)) {
                    tpp tp = tp_find("data/things/blood2");
                    wid_set_thing_template(wid, tp);
                    demo_player_dead[i] = true;
                    wid_set_animate(wid, true);
                }
            }
        }

        if (cnt == 3) {
            for (i = 0; i < demo_player_count; i++) {
                widp wid = demo_players[i];

                if ((demo_player_x[i] > 0.9) && (demo_player_x[i] < 1.0)) {
                    tpp tp = tp_find("data/things/blood2");
                    wid_set_thing_template(wid, tp);
                    demo_player_dead[i] = true;
                    wid_set_animate(wid, true);
                }
            }
        }
    }

    if (dy < 0) {
        dy *= friction_up;

        if (dy > -0.0001) {
            dy = 0.0001;
        }
    } else {
        dy *= accell_down;
    }
}

static void wid_intro_demo_buttons_tick (widp wid)
{
    uint32_t index = (uint32_t) (uintptr_t) wid_get_client_context(wid);
    float *x = &demo_player_x[index];

    tpp t = demo_player_tp[index];

    if (!demo_player_dead[index]) {
        const char *tn = tp_short_name(t);

        char tilename[40];

        snprintf(tilename, sizeof(tilename) - 1, "%s-demo-right", tn);

        wid_set_tilename(wid, tilename);

        *x += 0.001 * 
            (double) (myrand() % (demo_player_speed[index] + tp_get_speed(t)));

        wid_set_animate(wid, false);
    }

    double y = 0.66;

    y -= ((double)(myrand() % 10)) * 0.001;

    wid_move_to_pct(wid, *x, y);
}

static uint8_t wid_intro_demo_buttons_add_tiles (const tree_node *node, void *arg)
{
    tpp tp;
    widp child;

    tp = (typeof(tp)) 
            (((char*) node) - STRUCT_OFFSET(struct tp_, tree2));

    if (!tp_is_player(tp)) {
        return (true);
    }

    float w = 0.08;
    float h = 0.1;

    fpoint tl = {
        (w * (float)(x)),
        (h * (float)(y))
    };

    fpoint br = {
        (w * (float)(x+1)),
        (h * (float)(y+1))
    };

    demo_players[demo_player_count] = child = wid_new_window("player");
    demo_player_tp[demo_player_count] = tp;
    demo_player_speed[demo_player_count] = 
                myrand() % (tp_get_speed(tp) / 2);

    wid_set_thing_template(child, tp);
    wid_set_tl_br_pct(child, tl, br);
    wid_set_on_tick(child, wid_intro_demo_buttons_tick);

    wid_set_client_context(child, (void*) (uintptr_t) demo_player_count++);

    return (true);
}

int sdl_intro_demo_update (void)
{
    static widp wid_intro;
    static widp wid_rock;
    static widp wid_wall_floor;
    static int first = true;

    if (!wid_intro) {
        wid_intro = wid_new_window("splash icon");

        fpoint tl = { -2.0, 0.0 };
        fpoint br = { 5.0, 1.0 };

        wid_set_tl_br_pct(wid_intro, tl, br);

        wid_set_no_shape(wid_intro);

        wid_raise(wid_intro);
        wid_update(wid_intro);
    }

    if (!wid_wall_floor) {
        wid_wall_floor = wid_new_window("splash icon");

        fpoint tl = { 0.0, 0.0 };
        fpoint br = { 1.0, 1.0 };

        wid_set_tl_br_pct(wid_wall_floor, tl, br);

        wid_set_tex(wid_wall_floor, 0, "wall_floor");

        wid_set_mode(wid_wall_floor, WID_MODE_NORMAL);

        color c = WHITE;
        wid_set_color(wid_wall_floor, WID_COLOR_TL, c);
        wid_set_color(wid_wall_floor, WID_COLOR_BR, c);
        wid_set_color(wid_wall_floor, WID_COLOR_BG, c);

        wid_set_do_not_raise(wid_wall_floor, true);
        wid_update(wid_wall_floor);
    }

    if (!wid_rock) {
        wid_rock = wid_new_container(wid_intro, "splash icon");

        fpoint tl = { 0.0, 0.0 };
        fpoint br = { 0.05, 0.5 };

        wid_set_tl_br_pct(wid_rock, tl, br);

        wid_set_tex(wid_rock, 0, "giant_rock");

        wid_set_mode(wid_rock, WID_MODE_NORMAL);

        color c = WHITE;
        wid_set_color(wid_rock, WID_COLOR_TL, c);
        wid_set_color(wid_rock, WID_COLOR_BR, c);
        wid_set_color(wid_rock, WID_COLOR_BG, c);

        wid_raise(wid_rock);
        wid_update(wid_rock);

        wid_rotate_to_pct_in(wid_rock,
                             0,
                             1000,
                             10000,
                             0);
    }

    wid_rock_tick();
    wid_move_to_pct(wid_rock, x, y);

    if (first) {
        first = 0;
        tree_walk(thing_templates_create_order,
                  wid_intro_demo_buttons_add_tiles, 0 /* arg */);
    }

    if (x > 1.0) {
        wid_destroy_nodelay(&wid_intro);
        wid_destroy_nodelay(&wid_wall_floor);
        wid_intro_init();
        while (demo_player_count-- > 0) {
            wid_destroy_nodelay(&demo_players[demo_player_count--]);
        }
        return (false);
    }

    return (true);
}
