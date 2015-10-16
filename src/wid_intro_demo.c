/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
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
#include "wid_tooltip.h"
#include "time_util.h"
#include "music.h"
#include "timer.h"

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

static int done;

static widp wid_text1;
static widp wid_text2;
static widp wid_text3;
static void sdl_intro_demo_tick(widp w);
static void wid_intro_story_destroy(void);

/*************************************************************************************
 * Story part 6
 *************************************************************************************/
static widp wid_intro_story6;

static void wid_intro_make_visible (void *context)
{
    LOG("Begin intro");

    wid_intro_visible();
}

static void wid_intro_story6_destroy_done (widp w)
{
    if (!done) {
        wid_intro_story_destroy();
        sdl_intro_demo_tick(0);
    } else {
        action_timer_create(
            &wid_timers,
            (action_timer_callback)wid_intro_make_visible,
            (action_timer_destroy_callback)0,
            0, /* context */
            "init game",
            500,
            0 /* jitter */);
    }
}

static int wid_intro_story6_create (void)
{
    widp w = wid_intro_story6 = wid_new_window("players");

    float f = (1024.0 / 680.0);
    fpoint tl = { 0.0, 0.0 };
    fpoint br = { 1.0, f };
    wid_set_tl_br_pct(w, tl, br);

    wid_set_tex(w, 0, "players");

    wid_set_mode(w, WID_MODE_NORMAL);
    wid_set_color(w, WID_COLOR_TL, WHITE);
    wid_set_color(w, WID_COLOR_BR, WHITE);
    wid_set_color(w, WID_COLOR_BG, WHITE);

    int duration = 14000;

    wid_destroy_in(w, duration);
    wid_set_on_destroy_begin(w, wid_intro_story6_destroy_done);
    wid_move_delta_pct_in(w, 0.0f, -0.4f, duration);
    wid_fade_in(w, 2000);
    wid_raise(w);

    wid_destroy_delay_ms = 2000;

    {
        widp w;
        wid_text1 = w = wid_tooltip_transient("Many were called to retrieve the %%fg=gold$allcheese", 3 * ONESEC);
        wid_move_to_pct_centered(w, 0.5, -0.1);
        wid_move_to_pct_centered_in(w, 0.5, 0.2, ONESEC * 4);
        wid_destroy_in(w, ONESEC * 4);
    }

    {
        widp w;
        wid_text2 = w = wid_tooltip_transient("Only five answered...", 4 * ONESEC);
        wid_move_to_pct_centered(w, 0.5, -0.4);
        wid_move_to_pct_centered_in(w, 0.5, 0.2, ONESEC * 8);
        wid_destroy_in(w, ONESEC * 7);
    }

    {
        widp w;
        wid_text3 = w = wid_tooltip_transient("They thought it would be easy...!", 5 * ONESEC);
        wid_move_to_pct_centered(w, 0.5, -0.7);
        wid_move_to_pct_centered_in(w, 0.5, 0.2, ONESEC * 11);
        wid_destroy_in(w, ONESEC * 10);
    }

    wid_update(w);

    return (true);
}

/*************************************************************************************
 * Story part 5
 *************************************************************************************/
static widp wid_intro_story5;

static void wid_intro_story5_destroy_done (widp w)
{
    wid_intro_story5 = 0;

    if (!done) {
        wid_intro_story6_create();
    } else {
        wid_intro_visible();
    }
}

static int wid_intro_story5_create (void)
{
    widp w = wid_intro_story5 = wid_new_window("intro5");

    float f = (1024.0 / 680.0);
    fpoint tl = { 0.0, 0.0 };
    fpoint br = { 1.0, f };
    wid_set_tl_br_pct(w, tl, br);

    wid_set_tex(w, 0, "intro5");

    wid_set_mode(w, WID_MODE_NORMAL);
    wid_set_color(w, WID_COLOR_TL, WHITE);
    wid_set_color(w, WID_COLOR_BR, WHITE);
    wid_set_color(w, WID_COLOR_BG, WHITE);

    int duration = 14000;

    wid_destroy_in(w, duration);
    wid_set_on_destroy_begin(w, wid_intro_story5_destroy_done);
    wid_move_delta_pct_in(w, 0.0f, -0.1f, duration);
    wid_fade_in(w, 2000);
    wid_raise(w);

    wid_destroy_delay_ms = 2000;

    {
        widp w;
        wid_text1 = w = wid_tooltip_transient("The town burned...", 3 * ONESEC);
        wid_move_to_pct_centered(w, 0.5, -0.1);
        wid_move_to_pct_centered_in(w, 0.5, 0.2, ONESEC * 4);
        wid_destroy_in(w, ONESEC * 4);
    }

    {
        widp w;
        wid_text2 = w = wid_tooltip_transient("The streets ran with cheese...", 4 * ONESEC);
        wid_move_to_pct_centered(w, 0.5, -0.4);
        wid_move_to_pct_centered_in(w, 0.5, 0.2, ONESEC * 8);
        wid_destroy_in(w, ONESEC * 7);
    }

    {
        widp w;
        wid_text3 = w = wid_tooltip_transient("And the %%fg=gold$allcheese%%fg=reset$ was gone!", 5 * ONESEC);
        wid_move_to_pct_centered(w, 0.5, -0.7);
        wid_move_to_pct_centered_in(w, 0.5, 0.2, ONESEC * 11);
        wid_destroy_in(w, ONESEC * 10);
    }

    wid_update(w);

    return (true);
}

/*************************************************************************************
 * Story part 4
 *************************************************************************************/
static widp wid_intro_story4;

static void wid_intro_story4_destroy_done (widp w)
{
    wid_intro_story4 = 0;

    if (!done) {
        wid_intro_story5_create();
    } else {
        wid_intro_visible();
    }
}

static int wid_intro_story4_create (void)
{
    widp w = wid_intro_story4 = wid_new_window("intro4");

    float f = (1024.0 / 680.0);
    fpoint tl = { 0.0, 0.0 };
    fpoint br = { 1.0, f };
    wid_set_tl_br_pct(w, tl, br);

    wid_set_tex(w, 0, "intro4");

    wid_set_mode(w, WID_MODE_NORMAL);
    wid_set_color(w, WID_COLOR_TL, WHITE);
    wid_set_color(w, WID_COLOR_BR, WHITE);
    wid_set_color(w, WID_COLOR_BG, WHITE);

    int duration = 14000;

    wid_destroy_in(w, duration);
    wid_set_on_destroy_begin(w, wid_intro_story4_destroy_done);
    wid_move_delta_pct_in(w, 0.0f, -0.1f, duration);
    wid_fade_in(w, 2000);
    wid_raise(w);

    wid_destroy_delay_ms = 2000;

    {
        widp w;
        wid_text1 = w = wid_tooltip_transient("But the cheese drew unwanted attention...", 3 * ONESEC);
        wid_move_to_pct_centered(w, 0.5, -0.1);
        wid_move_to_pct_centered_in(w, 0.5, 0.2, ONESEC * 4);
        wid_destroy_in(w, ONESEC * 4);
    }

    {
        widp w;
        wid_text2 = w = wid_tooltip_transient("From those with great hunger", 4 * ONESEC);
        wid_move_to_pct_centered(w, 0.5, -0.4);
        wid_move_to_pct_centered_in(w, 0.5, 0.2, ONESEC * 8);
        wid_destroy_in(w, ONESEC * 7);
    }

    {
        widp w;
        wid_text3 = w = wid_tooltip_transient("%%fg=red$The great dragon gorynlich...!", 5 * ONESEC);
        wid_move_to_pct_centered(w, 0.5, -0.7);
        wid_move_to_pct_centered_in(w, 0.5, 0.2, ONESEC * 11);
        wid_destroy_in(w, ONESEC * 10);
    }

    wid_update(w);

    return (true);
}

/*************************************************************************************
 * Story part 3
 *************************************************************************************/
static widp wid_intro_story3;

static void wid_intro_story3_destroy_done (widp w)
{
    wid_intro_story3 = 0;

    if (!done) {
        wid_intro_story4_create();
    } else {
        wid_intro_visible();
    }
}

static int wid_intro_story3_create (void)
{
    widp w = wid_intro_story3 = wid_new_window("intro3");

    float f = (1024.0 / 680.0);
    fpoint tl = { 0.0, 0.0 };
    fpoint br = { 1.0, f };
    wid_set_tl_br_pct(w, tl, br);

    wid_set_tex(w, 0, "intro3");

    wid_set_mode(w, WID_MODE_NORMAL);
    wid_set_color(w, WID_COLOR_TL, WHITE);
    wid_set_color(w, WID_COLOR_BR, WHITE);
    wid_set_color(w, WID_COLOR_BG, WHITE);

    int duration = 14000;

    wid_destroy_in(w, duration);
    wid_set_on_destroy_begin(w, wid_intro_story3_destroy_done);
    wid_move_delta_pct_in(w, 0.0f, -0.1f, duration);
    wid_fade_in(w, 2000);
    wid_raise(w);

    wid_destroy_delay_ms = 2000;

    {
        widp w;
        wid_text1 = w = wid_tooltip_transient("All went well for a time...", 3 * ONESEC);
        wid_move_to_pct_centered(w, 0.5, -0.1);
        wid_move_to_pct_centered_in(w, 0.5, 0.2, ONESEC * 4);
        wid_destroy_in(w, ONESEC * 4);
    }

    {
        widp w;
        wid_text2 = w = wid_tooltip_transient("The dairy tolerant dwarves ate well", 4 * ONESEC);
        wid_move_to_pct_centered(w, 0.5, -0.4);
        wid_move_to_pct_centered_in(w, 0.5, 0.2, ONESEC * 8);
        wid_destroy_in(w, ONESEC * 7);
    }

    {
        widp w;
        wid_text3 = w = wid_tooltip_transient("The %%fg=gold$allcheese%%fg=reset$ attracted tourists!", 5 * ONESEC);
        wid_move_to_pct_centered(w, 0.5, -0.7);
        wid_move_to_pct_centered_in(w, 0.5, 0.2, ONESEC * 11);
        wid_destroy_in(w, ONESEC * 10);
    }

    wid_update(w);

    return (true);
}

/*************************************************************************************
 * Story part 2
 *************************************************************************************/
static widp wid_intro_story2;

static void wid_intro_story2_destroy_done (widp w)
{
    wid_intro_story2 = 0;

    if (!done) {
        wid_intro_story3_create();
    } else {
        wid_intro_visible();
    }
}

static int wid_intro_story2_create (void)
{
    widp w = wid_intro_story2 = wid_new_window("intro2");

    float f = (1024.0 / 680.0);
    fpoint tl = { 0.0, 0.0 };
    fpoint br = { 1.0, f };
    wid_set_tl_br_pct(w, tl, br);

    wid_set_tex(w, 0, "intro2");

    wid_set_mode(w, WID_MODE_NORMAL);
    wid_set_color(w, WID_COLOR_TL, WHITE);
    wid_set_color(w, WID_COLOR_BR, WHITE);
    wid_set_color(w, WID_COLOR_BG, WHITE);

    int duration = 14000;

    wid_destroy_in(w, duration);
    wid_set_on_destroy_begin(w, wid_intro_story2_destroy_done);
    wid_move_delta_pct_in(w, 0.0f, -0.3f, duration);
    wid_fade_in(w, 2000);
    wid_raise(w);

    wid_destroy_delay_ms = 2000;

    {
        widp w;
        wid_text1 = w = wid_tooltip_transient("Until the greatest of all cheeses,", 3 * ONESEC);
        wid_move_to_pct_centered(w, 0.5, -0.1);
        wid_move_to_pct_centered_in(w, 0.5, 0.2, ONESEC * 4);
        wid_destroy_in(w, ONESEC * 4);
    }

    {
        widp w;
        wid_text2 = w = wid_tooltip_transient("The %%fg=gold$allcheese!%%fg=reset$ was found", 4 * ONESEC);
        wid_move_to_pct_centered(w, 0.5, -0.4);
        wid_move_to_pct_centered_in(w, 0.5, 0.2, ONESEC * 8);
        wid_destroy_in(w, ONESEC * 7);
    }

    {
        widp w;
        wid_text3 = w = wid_tooltip_transient("The cheesy heart of the mountain", 5 * ONESEC);
        wid_move_to_pct_centered(w, 0.5, -0.7);
        wid_move_to_pct_centered_in(w, 0.5, 0.2, ONESEC * 11);
        wid_destroy_in(w, ONESEC * 10);
    }

    wid_update(w);

    return (true);
}

/*************************************************************************************
 * Story part 1
 *************************************************************************************/
static widp wid_intro_story1;

static void wid_intro_story1_destroy_done (widp w)
{
    wid_intro_story1 = 0;

    if (!done) {
        wid_intro_story2_create();
    } else {
        wid_intro_visible();
    }
}

static widp wid_intro_story;

static void wid_intro_story_destroy (void)
{
    done = true;

    if (wid_intro_story1) {
        wid_destroy(&wid_intro_story1);
    }

    if (wid_intro_story2) {
        wid_destroy(&wid_intro_story2);
    }

    if (wid_intro_story) {
        wid_destroy(&wid_intro_story);
    }
}

static uint8_t wid_intro_story_mouse_down (widp w, int32_t x, int32_t y, uint32_t button)
{
    wid_intro_story_destroy();

    wid_gc_all_force();

    return (true);
}

static uint8_t wid_intro_story_key_down (widp w, const SDL_KEYSYM *key)
{
    wid_intro_story_destroy();

    wid_gc_all_force();

    return (true);
}

int wid_intro_story1_create (void)
{
    {
        widp w = wid_intro_story = wid_new_window("dummy");
        wid_set_on_mouse_down(w, wid_intro_story_mouse_down);
        wid_set_on_key_down(w, wid_intro_story_key_down);
    }

    widp w = wid_intro_story1 = wid_new_window("intro1");

    float f = (1024.0 / 680.0);
    fpoint tl = { 0.0, 0.0 };
    fpoint br = { 1.0, f };
    wid_set_tl_br_pct(w, tl, br);

    wid_set_tex(w, 0, "intro1");

    wid_set_mode(w, WID_MODE_NORMAL);
    wid_set_color(w, WID_COLOR_TL, WHITE);
    wid_set_color(w, WID_COLOR_BR, WHITE);
    wid_set_color(w, WID_COLOR_BG, WHITE);
    wid_set_on_destroy_begin(w, wid_intro_story1_destroy_done);

    int duration = 14000;

    wid_destroy_in(w, duration);
    wid_move_delta_pct_in(w, 0.0f, -0.3f, duration);
    wid_fade_in(w, 2000);
    wid_raise(w);

    wid_destroy_delay_ms = 2000;

    music_play_demo();

    {
        widp w;
        wid_text1 = w = wid_tooltip_transient("For millennia, the dwarves of old", 3 * ONESEC);
        wid_move_to_pct_centered(w, 0.5, -0.1);
        wid_move_to_pct_centered_in(w, 0.5, 0.2, ONESEC * 6);
        wid_destroy_in(w, ONESEC * 6);
    }

    {
        widp w;
        wid_text2 = w = wid_tooltip_transient("mined in search of great cheese", 4 * ONESEC);
        wid_move_to_pct_centered(w, 0.5, -0.4);
        wid_move_to_pct_centered_in(w, 0.5, 0.3, ONESEC * 10);
        wid_destroy_in(w, ONESEC * 9);
    }

    {
        widp w;
        wid_text3 = w = wid_tooltip_transient("ever deeper they dug...", 5 * ONESEC);
        wid_move_to_pct_centered(w, 0.5, -0.7);
        wid_move_to_pct_centered_in(w, 0.5, 0.4, ONESEC * 14);
        wid_destroy_in(w, ONESEC * 13);
    }

    wid_update(w);

    return (true);
}

static void wid_intro_demo_buttons_tick (widp wid)
{
    uint32_t index = (uint32_t) (uintptr_t) wid_get_client_context(wid);
    float *x = &demo_player_x[index];

    tpp t = demo_player_tp[index];

    if (!demo_player_dead[index]) {
        const char *tn = tp_short_name(t);

        char tilename[40];

        snprintf(tilename, sizeof(tilename) - 1, "%s-right", tn);

        wid_set_tilename(wid, tilename);

        *x = X + 0.2 + elapsed * 0.000005 * demo_player_speed[index];

        demo_player_x[index] *= 0.85;

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

static void sdl_intro_demo_tick (widp w)
{
    static const double wall_start = 0.25;
    static widp wid_intro;
    static widp wid_rock;
    static widp wid_wall_floor;

    if (!wid_intro) {
        wid_intro = wid_new_window("splash icon");

        fpoint tl = { -2.0, 0.0 };
        fpoint br = { 5.0, 1.0 };

        wid_set_tl_br_pct(wid_intro, tl, br);

        wid_set_no_shape(wid_intro);

        wid_raise(wid_intro);
        wid_update(wid_intro);

        wid_set_on_tick(wid_intro, sdl_intro_demo_tick);
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
                        tpp tp = tp_find("blood2");
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
    }
}
