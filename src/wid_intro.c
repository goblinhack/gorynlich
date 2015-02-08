/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "wid_intro.h"
#include "wid_popup.h"
#include "wid_intro_extra.h"
#include "wid_intro_about.h"
#include "wid_intro_buttons.h"
#include "wid_intro_settings.h"
#include "wid_menu.h"
#include "wid_game_over.h"
#include "wid_game_map_client.h"
#include "wid_choose_game_type.h"
#include "wid_hiscore.h"
#include "wid_notify.h"
#include "wid_server_join.h"
#include "wid_server_create.h"
#include "thing_template.h"
#include "music.h"
#include "level.h"
#include "timer.h"
#include "socket_util.h"
#include "server.h"
#include "glapi.h"

static widp wid_intro;
static widp wid_intro_background;
static widp wid_intro_title;
static widp wid_intro_man;
static widp wid_intro_treasure_chest;
static widp wid_intro_eyes;

static uint8_t wid_intro_is_hidden;
static uint8_t wid_intro_is_visible;

static void wid_intro_play_selected(void);
static void wid_intro_single_play_selected(void);

static uint8_t wid_intro_init_done;
static void wid_intro_create(void);

static int intro_effect_delay = 500;
static int intro_effect_delay_zoom = 1000;

uint8_t wid_intro_init (void)
{
    if (!wid_intro_init_done) {
        wid_intro_create();
    }

    wid_intro_init_done = true;

    return (true);
}

void wid_intro_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (wid_intro_init_done) {
        wid_intro_init_done = false;

        if (wid_intro) {
            wid_destroy(&wid_intro);
            wid_destroy_in(wid_intro_background, wid_hide_delay * 2);
            wid_destroy_in(wid_intro_title, wid_hide_delay * 2);
            wid_destroy_in(wid_intro_man, wid_hide_delay * 2);
            wid_destroy_in(wid_intro_treasure_chest, wid_hide_delay * 2);
            wid_destroy_in(wid_intro_eyes, wid_hide_delay * 2);

            wid_intro_buttons_hide();
        }
    }
}

void wid_intro_hide (void)
{
    if (wid_intro_is_hidden) {
        return;
    }

    LOG("Client: Intro screen hide");

    wid_intro_is_hidden = true;
    wid_intro_is_visible = false;

    music_halt();

    if (!wid_intro) {
        DIE("no wid intro");
    }

    wid_scaling_to_pct_in(wid_intro_background, 1.0, 10.01, 
                          intro_effect_delay_zoom, 0);
    wid_fade_out(wid_intro_background, intro_effect_delay_zoom);

    wid_scaling_to_pct_in(wid_intro_title, 1.0, 5.01, 
                          intro_effect_delay_zoom, 0);
    wid_fade_out(wid_intro_title, intro_effect_delay_zoom);
    wid_move_delta_pct_in(wid_intro_title, -2.0f, -4.0f,
                                intro_effect_delay_zoom);

    wid_fade_out(wid_intro_man, intro_effect_delay_zoom);
    wid_move_delta_pct_in(wid_intro_man, -2.0f, -0.0f,
                                intro_effect_delay_zoom);

    wid_fade_out(wid_intro_treasure_chest, intro_effect_delay_zoom);
    wid_move_delta_pct_in(wid_intro_treasure_chest, 2.0f, -0.0f,
                                intro_effect_delay_zoom);

    wid_fade_out(wid_intro_eyes, intro_effect_delay_zoom);

    wid_hide(wid_intro, 0);
    wid_raise(wid_intro);
    wid_update(wid_intro);
}

void wid_intro_visible (void)
{
    if (wid_intro_is_visible) {
        return;
    }

    if (wid_intro) {
        wid_destroy(&wid_intro);
        wid_destroy(&wid_intro_background);
        wid_destroy(&wid_intro_title);
        wid_destroy(&wid_intro_man);
        wid_destroy(&wid_intro_treasure_chest);
        wid_destroy(&wid_intro_eyes);
        wid_intro_create();
    }

    wid_notify_flush();

    LOG("Client: Intro screen show");

    wid_intro_is_visible = true;
    wid_intro_is_hidden = false;

    if (!wid_intro) {
        DIE("no wid intro");
    }

    music_play_intro();

    if (global_config.game_over) {
        global_config.game_over = false;
        wid_game_over_visible();
        return;
    }

    wid_visible(wid_intro, 0);
    wid_raise(wid_intro);
    wid_update(wid_intro);

    wid_intro_buttons_visible();

    wid_fade_in(wid_intro_background, intro_effect_delay);
    wid_fade_in(wid_intro_title, intro_effect_delay);
    wid_fade_in(wid_intro_man, intro_effect_delay);
    wid_fade_in(wid_intro_treasure_chest, intro_effect_delay);
    wid_fade_in(wid_intro_eyes, intro_effect_delay);
}

static uint8_t wid_intro_key_event (widp w, const SDL_KEYSYM *key)
{
    switch ((int)key->sym) {
        case ' ':
            wid_intro_play_selected();
            return (true);

        case 's':
            wid_intro_single_play_selected();
            return (true);

        case 'j':
            wid_intro_hide();
            wid_intro_buttons_hide();

            /*
             * Start with some random junk.
             */
            thing_statsp s;
            s = &global_config.stats;
            thing_stats_get_random(s, false /* new_random_name_and_class */);

            wid_server_create_visible();
            wid_server_join_visible();
            return (true);

        case 'q':
        case SDLK_ESCAPE:
            wid_intro_quit_selected();
            return (true);

        case 'x':
            wid_intro_intro_extra_selected();
            return (true);

        case '`':
        case SDLK_LEFT:
        case SDLK_RIGHT:
        case SDLK_TAB:
            break;

        default:
            return (true);
    }

    return (false);
}

static void wid_intro_single_play_selected_cb (void *context)
{
    LOG("Play selected callback");

    on_server = true;

    wid_server_join_hide();
    wid_server_create_hide();

    /*
     * Let the server map load itself when the client connects.
    wid_game_map_server_visible();
     */
    server_start(server_address);

    wid_game_map_client_visible();

    wid_intro_hide();
}

static void wid_intro_single_play_selected (void)
{
    LOG("Play selected, choose random stats");

    /*
     * Start with some random junk.
     */
    thing_statsp s;
    s = &global_config.stats;
    thing_stats_get_random(s, false /* new_random_name_and_class */);

    action_timer_create(
            &wid_timers,
            (action_timer_callback)wid_intro_single_play_selected_cb,
            (action_timer_destroy_callback)0,
            0, /* context */
            "start game",
            intro_effect_delay,
            0 /* jitter */);

    wid_intro_hide();
    wid_intro_buttons_hide();
}

static void wid_intro_play_selected_cb (void *context)
{
    LOG("Intro play selected callback");

    wid_choose_game_type_visible();
    wid_intro_buttons_hide();
}

static void wid_intro_play_selected (void)
{
    LOG("Quick play selected, start game");

    action_timer_create(
            &wid_timers,
            (action_timer_callback)wid_intro_play_selected_cb,
            (action_timer_destroy_callback)0,
            0, /* context */
            "start game",
            intro_effect_delay,
            0 /* jitter */);

    wid_intro_hide();
}

static uint8_t wid_intro_play_mouse_event (widp w, int32_t x, int32_t y,
                                           uint32_t button)
{
    /*
     * Race condition whilst other windows slide in
     */
    if (!wid_intro_buttons) {
        return (false);
    }

    wid_intro_play_selected();

    return (true);
}

static double y;
static double dy;

static void wid_intro_tick_reset (void)
{
    y = -0.20;
    dy = 0.00026;
}

static void wid_intro_tick (widp wid)
{
    wid_move_to_pct_centered(wid_intro_title, 0.5f, y);

    y += dy;

    static const double wall_start = 0.38;
    static const double accell_down = 1.06;
    static const double friction_up = 0.9;
    static const double elasticity = 0.09;

    if (y > wall_start) {
        y = wall_start;
        dy = -dy * elasticity;
        y += dy;

        wid_rotate_to_pct_in(wid, 0.1, 1.2, 300, 1);
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

static void wid_intro_bg_create (void)
{
    if (wid_intro_background) {
        return;
    }

    {
        {
            widp wid = wid_intro_title = wid_new_window("bg");
            fpoint tl = { 0.0, 0.0 };
            fpoint br = { 1.0, 0.76 };

            wid_set_tl_br_pct(wid, tl, br);

            wid_set_tex(wid, 0, "main_title");

            wid_lower(wid);

            color c;
            c = WHITE;
            wid_set_mode(wid, WID_MODE_NORMAL);
            wid_set_color(wid, WID_COLOR_TL, c);
            wid_set_color(wid, WID_COLOR_BR, c);
            wid_set_color(wid, WID_COLOR_BG, c);

            wid_update(wid);
            wid_move_to_pct_centered(wid_intro_title, 0.5f, -4.1f);
            wid_set_on_tick(wid, wid_intro_tick);
            wid_intro_tick_reset();
        }

        {
            widp wid = wid_intro_man = wid_new_window("bg");
            fpoint tl = { 0.0, 0.56 };
            fpoint br = { 0.3, 1.0 };

            wid_set_tl_br_pct(wid, tl, br);

            tpp tp = tp_find("data/things/anim_man");
            wid_set_thing_template(wid, tp);

            wid_raise(wid);

            wid_set_mode(wid, WID_MODE_NORMAL);

            wid_update(wid);
            wid_move_to_pct_centered(wid_intro_man, -0.8f, 0.70f);
            wid_move_to_pct_centered_in(wid_intro_man, 0.08f, 0.70f, 1000);
        }

        {
            widp wid = wid_intro_treasure_chest = wid_new_window("bg");

            fpoint tl = { 0.0, 0.56 };
            fpoint br = { 0.3, 1.0 };

            wid_set_tl_br_pct(wid, tl, br);

            tpp tp = tp_find("data/things/anim_treasure_chest");
            wid_set_thing_template(wid, tp);

            wid_raise(wid);

            wid_set_mode(wid, WID_MODE_NORMAL);

            wid_update(wid);
            wid_move_to_pct_centered(wid_intro_treasure_chest, 0.96f, 0.75f);
        }

        {
            widp wid = wid_intro_eyes = wid_new_window("bg");

            fpoint tl = { 0.0, 0.0 };
            fpoint br = { 0.1, 0.12 };

            wid_set_tl_br_pct(wid, tl, br);

            tpp tp = tp_find("data/things/anim_eyes");
            wid_set_thing_template(wid, tp);

            wid_raise(wid);

            wid_set_mode(wid, WID_MODE_NORMAL);

            wid_update(wid);
            wid_move_to_pct_centered(wid_intro_eyes, 0.53f, 0.74f);
        }

        {
            widp wid = wid_intro_background = wid_new_window("bg");

            float f;

            f = (1024.0 / 680.0);

            fpoint tl = { 0.0, 0.0 };
            fpoint br = { 1.0, f };

            wid_set_tl_br_pct(wid, tl, br);

            wid_set_tex(wid, 0, "title");

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
}

static uint8_t wid_menu_settings_selected (widp w, 
                                           int32_t x, int32_t y,
                                           uint32_t button)
{
CON("%s",__FUNCTION__);
    return (true);
}

static uint8_t wid_menu_level_editor_selected (widp w,
                                               int32_t x, int32_t y,
                                               uint32_t button)
{
CON("%s",__FUNCTION__);
    return (true);
}

static uint8_t wid_menu_play_game_selected (widp w,
                                            int32_t x, int32_t y,
                                            uint32_t button)
{
CON("%s",__FUNCTION__);
    return (true);
}

static uint8_t wid_menu_past_legends_selected (widp w,
                                               int32_t x, int32_t y,
                                               uint32_t button)
{
CON("%s",__FUNCTION__);
    return (true);
}

static uint8_t wid_menu_quit_selected (widp w,
                                       int32_t x, int32_t y,
                                       uint32_t button)
{
CON("%s",__FUNCTION__);
    return (true);
}

static void wid_intro_create (void)
{
    if (wid_intro) {
        return;
    }

    music_play_intro();

    wid_intro = wid_new_window("intro buttons");

    wid_set_no_shape(wid_intro);

    fpoint tl = {0.0f, 0.0f};
    fpoint br = {1.0f, 1.0f};
    wid_set_tl_br_pct(wid_intro, tl, br);
    wid_set_on_key_down(wid_intro, wid_intro_key_event);
    wid_set_on_mouse_down(wid_intro, wid_intro_play_mouse_event);

    color col = BLACK;
    col.a = 0;
    glcolor(col);

    wid_set_mode(wid_intro, WID_MODE_NORMAL);
    wid_set_color(wid_intro, WID_COLOR_TL, col);
    wid_set_color(wid_intro, WID_COLOR_BR, col);
    wid_set_color(wid_intro, WID_COLOR_BG, col);

    wid_intro_bg_create();
    wid_update(wid_intro);

    {
        widp child;

        child = wid_new_square_button(wid_intro, "play");
        wid_set_font(child, vlarge_font);
        wid_set_no_shape(child);

        fpoint tl = {0.0f, 0.60f};
        fpoint br = {1.0f, 0.80f};

        wid_set_tl_br_pct(child, tl, br);
        wid_set_text(child, "Click to play");
        wid_effect_pulse_forever(child);
        wid_fade_in_out(child, 1000, 1000, false /* fade out first */);

        wid_set_color(child, WID_COLOR_TEXT, WHITE);
        color c = WHITE;
        c.a = 200;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_OVER);
        c = RED;
        c.a = 255;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_FOCUS);
        c.a = 225;
        wid_set_color(child, WID_COLOR_TEXT, c);
        wid_set_text_outline(child, true);

        wid_set_on_mouse_down(child, wid_intro_play_mouse_event);
    }

    wid_update(wid_intro);

    wid_intro_buttons_visible();

    wid_move_to_pct_centered(wid_intro, 0.5f, 0.5f);
    wid_fade_in(wid_intro_background, intro_effect_delay*2);
    wid_fade_in(wid_intro_title, intro_effect_delay*2);
    wid_fade_in(wid_intro_man, intro_effect_delay*2);
    wid_fade_in(wid_intro_treasure_chest, intro_effect_delay*2);
    wid_fade_in(wid_intro_eyes, intro_effect_delay*2);

    {
        widp w = 
            wid_menu(wid_intro,
                     vvlarge_font,
                     large_font,
                     0.95, /* padding between buttons */
                     2, /* focus */
                     5, /* items */
                     "Editor",          wid_menu_level_editor_selected,
                     "Settings",        wid_menu_settings_selected,
                     "Play game",       wid_menu_play_game_selected,
                     "Hiscores",        wid_menu_past_legends_selected,
                     "Quit",            wid_menu_quit_selected);

        wid_move_to_pct_centered(w, 0.5, 0.7);
    }
}
