/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
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
#include "time_util.h"

#define MAX_DEMO_PLAYERS 20
static int demo_player_count;
static widp demo_players[MAX_DEMO_PLAYERS];
static float demo_player_x[MAX_DEMO_PLAYERS];
static int demo_player_dead[MAX_DEMO_PLAYERS];
static tpp demo_player_tp[MAX_DEMO_PLAYERS];
static double demo_player_speed[MAX_DEMO_PLAYERS];

static double x;
static double X;
static double y;
static double elapsed;
static const double wall_start = 0.25;

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

        *x = X + 0.2 + elapsed * 0.000005 * demo_player_speed[index];

        demo_player_x[index] *= 0.95;

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
                myrand() % (tp_get_speed(tp) * 8);

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

    if (opt_quickstart || !global_config.intro_screen) {
        wid_intro_init();
        return (false);
    }

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

        tree_walk(thing_templates_create_order,
                  wid_intro_demo_buttons_add_tiles, 0 /* arg */);
    }

    {
        static double ts_start;
        if (!ts_start) {
            ts_start = time_get_time_ms();
        }

        elapsed = time_get_time_ms() - ts_start;
        x = (elapsed / 7000.0);

        static double y = 0;;
#define gravity 0.03
        static double speed = 0.001;
        static double decel = 1.0 - gravity;
        static double accel = 1.0 + gravity;
        static double speed_loss_on_impact = 0.7;
        static double dir = 1;

        y += speed * dir;

        if (dir < 0) {
            speed *= decel;
        } else {
            speed *= accel;
        }

        if (speed < 0.001) {
            dir = -dir;
            speed = 0.001;
            y += speed * dir;
        }

        X = (x - 0.2) * 3.5;

        wid_move_to_pct(wid_rock, X, y);

        if (y > wall_start) {
            y = wall_start;

            speed *= speed_loss_on_impact;

            dir = -dir;

            int i;
            for (i = 0; i < demo_player_count; i++) {
                widp wid = demo_players[i];

                if (wid) {
                    if ((demo_player_x[i] > X - 0.1) && (demo_player_x[i] < X + 0.2)) {
                        tpp tp = tp_find("data/things/blood2");
                        wid_set_thing_template(wid, tp);
                        demo_player_dead[i] = true;
                        wid_set_animate(wid, true);
                        wid_destroy(&demo_players[i]);
                    }
                }
            }
        }
    }
   
    if (opt_quickstart || !global_config.intro_screen) {
        global_config.intro_screen = 0;
        x = 1.0;
    }

    if (x > 0.6) {
        wid_destroy_nodelay(&wid_intro);
        wid_destroy_nodelay(&wid_wall_floor);
        wid_intro_init();

        int i;
        for (i = 0; i < demo_player_count; i++) {
            widp wid = demo_players[i];
            if (wid) {
                wid_destroy(&demo_players[i]);
            }
        }
        return (false);
    }

    return (true);
}
