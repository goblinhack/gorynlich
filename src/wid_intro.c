/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "wid_intro.h"
#include "wid_intro2.h"
#include "wid_popup.h"
#include "wid_editor.h"
#include "wid_intro_about.h"
#include "wid_intro_settings.h"
#include "wid_intro_help.h"
#include "wid_intro_guide.h"
#include "wid_game_over.h"
#include "wid_game_map_client.h"
#include "wid_hiscore.h"
#include "wid_server_join.h"
#include "wid_server_create.h"
#include "music.h"
#include "level.h"
#include "timer.h"
#include "wid_notify.h"
#include "socket.h"
#include "server.h"
#include "glapi.h"

static widp wid_intro;
static widp wid_intro_background;
static widp wid_intro_title;
static widp wid_intro_man;

static uint8_t wid_intro_quit_selected(void);
static void wid_intro_editor_selected(void);
static void wid_intro_play_selected(void);
static void wid_intro_single_play_selected(void);
static void wid_intro_about_selected(void);
static void wid_intro_settings_selected(void);
static void wid_intro_hiscore_selected(void);
static void wid_server_join_selected(void);
static void wid_server_create_selected(void);
static void wid_intro_help_selected(void);
static void wid_intro_guide_selected(void);

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
        }
    }
}

static uint8_t wid_intro_is_hidden;
static uint8_t wid_intro_is_visible;

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

    wid_hide(wid_intro, 0);
    wid_raise(wid_intro);
    wid_update(wid_intro);
}

void wid_intro_visible (void)
{
    if (wid_intro_is_visible) {
        return;
    }

    wid_notify_flush();

    LOG("Client: Intro screen show");

    wid_intro_is_visible = true;
    wid_intro_is_hidden = false;

    if (!wid_intro) {
        DIE("no wid intro");
    }

    music_play_intro();

    if (game_over) {
        game_over = false;
        wid_game_over_visible();
        return;
    }

    wid_visible(wid_intro, 0);
    wid_raise(wid_intro);
    wid_update(wid_intro);

    wid_fade_in(wid_intro_background, intro_effect_delay);
    wid_fade_in(wid_intro_title, intro_effect_delay);
    wid_fade_in(wid_intro_man, intro_effect_delay);
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

        case 'q':
        case SDLK_ESCAPE:
            wid_intro_quit_selected();
            return (true);

        case 'c':
            wid_intro_settings_selected();
            return (true);

        case 'H':
            wid_intro_hiscore_selected();
            return (true);

        case 'm':
            wid_server_create_selected();
            wid_server_join_selected();
            return (true);

        case 'e':
            wid_intro_editor_selected();
            return (true);

        case 'h':
            wid_intro_guide_selected();
            return (true);

        case '`':
        case SDLK_LEFT:
        case SDLK_RIGHT:
        case SDLK_TAB:
            break;

        default:
            wid_intro_help_selected();
            return (true);
    }

    return (false);
}

static void wid_intro_single_play_selected_cb (void *context)
{
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
    action_timer_create(
            &wid_timers,
            (action_timer_callback)wid_intro_single_play_selected_cb,
            (action_timer_destroy_callback)0,
            0, /* context */
            "start game",
            intro_effect_delay,
            0 /* jitter */);

    wid_intro_hide();
}

static void wid_intro_play_selected_cb (void *context)
{
    wid_intro2_visible();
}

static void wid_intro_play_selected (void)
{
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
    wid_intro_play_selected();

    return (true);
}

static void wid_intro_about_selected (void)
{
    wid_intro_about_visible();
}

static void wid_intro_settings_selected (void)
{
    wid_intro_settings_visible();
}

static void wid_intro_hiscore_selected (void)
{
    wid_hiscore_visible();
}

static void wid_server_join_selected (void)
{
    wid_server_join_visible();
}

static void wid_server_create_selected (void)
{
    wid_server_create_visible();
}

static void wid_intro_help_selected (void)
{
    wid_intro_help_visible();
}

static void wid_intro_guide_selected_cb (void *context)
{
    wid_intro_guide_visible();
}

static void wid_intro_guide_selected (void)
{
    action_timer_create(
            &wid_timers,
            (action_timer_callback)wid_intro_guide_selected_cb,
            (action_timer_destroy_callback)0,
            0, /* context */
            "start guide",
            intro_effect_delay,
            0 /* jitter */);

    wid_intro_hide();
}

static uint8_t wid_intro_help_mouse_event (widp w, int32_t x, int32_t y,
                                           uint32_t button)
{
    wid_intro_guide_selected();

    return (true);
}

static uint8_t wid_intro_about_mouse_event (widp w, int32_t x, int32_t y,
                                            uint32_t button)
{
    wid_intro_about_selected();

    return (true);
}

static uint8_t wid_intro_settings_mouse_event (widp w, int32_t x, int32_t y,
                                            uint32_t button)
{
    wid_intro_settings_selected();

    return (true);
}

static uint8_t wid_intro_hiscore_mouse_event (widp w, int32_t x, int32_t y,
                                            uint32_t button)
{
    wid_intro_hiscore_selected();

    return (true);
}

static uint8_t wid_server_join_mouse_event (widp w, int32_t x, int32_t y,
                                            uint32_t button)
{
    wid_server_join_selected();

    return (true);
}

static void wid_intro_editor_selected_cb (void *context)
{
    wid_editor_visible();
}

static void wid_intro_editor_selected (void)
{
    action_timer_create(
            &wid_timers,
            (action_timer_callback)wid_intro_editor_selected_cb,
            (action_timer_destroy_callback)0,
            0, /* context */
            "start editor",
            intro_effect_delay,
            0 /* jitter */);

    wid_intro_hide();
}

static uint8_t wid_intro_editor_mouse_event (widp w, int32_t x, int32_t y,
                                             uint32_t button)
{
    wid_intro_editor_selected();

    return (true);
}

static widp wid_intro_quit_popup;

static void wid_intro_quit_callback_yes (widp wid)
{
    wid_destroy(&wid_intro_quit_popup);

    FINI_LOG("quit yes selected");
    sdl_exit();
}

static void wid_intro_quit_callback_no (widp wid)
{
    wid_destroy(&wid_intro_quit_popup);
}

static uint8_t wid_intro_quit_selected (void)
{
    if (wid_intro_quit_popup) {
        return (false);
    }

    wid_intro_quit_popup =
        wid_popup("%%fg=red$Quit?",
                  0                 /* title */,
                  0.5f, 0.5f,       /* x,y postition in percent */
                  large_font,       /* title font */
                  large_font,         /* body font */
                  large_font,         /* button font */
                  2,                /* number buttons */
                  "Yes", wid_intro_quit_callback_yes,
                  "No", wid_intro_quit_callback_no);

    return (true);
}

/*
 * Mouse up etc...
 */
static uint8_t wid_intro_quit_receive_mouse_down (widp w,
                                                int32_t x, int32_t y,
                                                uint32_t button)
{
    return (wid_intro_quit_selected());
}

static void wid_intro_bg_create (void)
{
    if (wid_intro_background) {
        return;
    }

    {
        {
            widp wid = wid_intro_title = wid_new_window("bg");
            float f;

            f = (512.0 / 200.0);

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
            wid_move_to_pct_centered_in(wid_intro_title, 0.5f, 0.38f, 800);
        }

        {
            widp wid = wid_intro_man = wid_new_window("bg");
            float f;

            f = (512.0 / 200.0);

            fpoint tl = { 0.0, 0.56 };
            fpoint br = { 0.3, 1.0 };

            wid_set_tl_br_pct(wid, tl, br);

            wid_set_tex(wid, 0, "man");

            wid_raise(wid);

            color c;
            c = WHITE;
            wid_set_mode(wid, WID_MODE_NORMAL);
            wid_set_color(wid, WID_COLOR_TL, c);
            wid_set_color(wid, WID_COLOR_BR, c);
            wid_set_color(wid, WID_COLOR_BG, c);

            wid_update(wid);
            wid_move_to_pct_centered(wid_intro_man, -0.5f, 0.7f);
            wid_move_to_pct_centered_in(wid_intro_man, 0.1f, 0.7f, 8000);
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

static void wid_intro_create (void)
{
    if (wid_intro) {
        return;
    }

    wid_notify(INFO, "Press h for help");
    wid_notify(INFO, "Press s for quick start single player");

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

    {
        widp child;

        child = wid_new_square_button(wid_intro, "Editor");
        wid_set_font(child, med_font);
        wid_set_no_shape(child);

        fpoint tl = {0.1f, 0.0f};
        fpoint br = {0.3f, 0.05};

        wid_set_tl_br_pct(child, tl, br);
        wid_set_text(child, "Editor");

        wid_set_color(child, WID_COLOR_TEXT, WHITE);
        color c = ORANGE;
        c.a = 50;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_OVER);
        c.a = 200;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_FOCUS);
        c.a = 100;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_NORMAL);

        wid_set_on_mouse_down(child, wid_intro_editor_mouse_event);
    }

    {
        widp child;

        child = wid_new_square_button(wid_intro, "About");
        wid_set_font(child, med_font);
        wid_set_no_shape(child);

        fpoint tl = {0.3f, 0.0f};
        fpoint br = {0.5f, 0.05};

        wid_set_tl_br_pct(child, tl, br);
        wid_set_text(child, "About");

        wid_set_color(child, WID_COLOR_TEXT, WHITE);
        color c = ORANGE;
        c.a = 50;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_OVER);
        c.a = 200;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_FOCUS);
        c.a = 100;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_NORMAL);

        wid_set_on_mouse_down(child, wid_intro_about_mouse_event);
    }

    {
        widp child;

        child = wid_new_square_button(wid_intro, "Settings");
        wid_set_font(child, med_font);
        wid_set_no_shape(child);

        fpoint tl = {0.5f, 0.0f};
        fpoint br = {0.7f, 0.05};

        wid_set_tl_br_pct(child, tl, br);
        wid_set_text(child, "Settings");

        wid_set_color(child, WID_COLOR_TEXT, WHITE);
        color c = ORANGE;
        c.a = 50;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_OVER);
        c.a = 200;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_FOCUS);
        c.a = 100;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_NORMAL);

        wid_set_on_mouse_down(child, wid_intro_settings_mouse_event);
    }

    {
        widp child;

        child = wid_new_square_button(wid_intro, "Hiscore");
        wid_set_font(child, med_font);
        wid_set_no_shape(child);

        fpoint tl = {0.7f, 0.0f};
        fpoint br = {0.9f, 0.05};

        wid_set_tl_br_pct(child, tl, br);
        wid_set_text(child, "Hiscore");

        wid_set_color(child, WID_COLOR_TEXT, WHITE);
        color c = ORANGE;
        c.a = 50;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_OVER);
        c.a = 200;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_FOCUS);
        c.a = 100;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_NORMAL);

        wid_set_on_mouse_down(child, wid_intro_hiscore_mouse_event);
    }

    {
        widp child;

        child = wid_new_square_button(wid_intro, "servers");
        wid_set_font(child, med_font);
        wid_set_no_shape(child);

        fpoint tl = {0.7f, 0.2f};
        fpoint br = {0.9f, 0.05};

        wid_set_tl_br_pct(child, tl, br);
        wid_set_text(child, "server");

        wid_set_color(child, WID_COLOR_TEXT, WHITE);
        color c = ORANGE;
        c.a = 50;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_OVER);
        c.a = 200;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_FOCUS);
        c.a = 100;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_NORMAL);

        wid_set_on_mouse_down(child, wid_server_join_mouse_event);
    }

    {
        widp child;

        child = wid_new_square_button(wid_intro, "help");
        wid_set_font(child, large_font);
        wid_set_no_shape(child);

        fpoint tl = {0.00f, 0.87f};
        fpoint br = {0.33f, 1.00f};

        wid_set_tl_br_pct(child, tl, br);
        wid_set_text(child, "Help");

        wid_set_color(child, WID_COLOR_TEXT, WHITE);
        color c = ORANGE;
        c.a = 50;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_OVER);
        c.a = 200;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_FOCUS);
        c.a = 100;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_on_mouse_down(child, wid_intro_help_mouse_event);
    }

    {
        widp child;

        child = wid_new_square_button(wid_intro, "quit");
        wid_set_font(child, large_font);
        wid_set_no_shape(child);

        fpoint tl = {0.66f, 0.87f};
        fpoint br = {1.00f, 1.00f};

        wid_set_tl_br_pct(child, tl, br);
        wid_set_text(child, "Quit");

        wid_set_color(child, WID_COLOR_TEXT, WHITE);
        color c = ORANGE;
        c.a = 50;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_OVER);
        c.a = 200;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_FOCUS);
        c.a = 100;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_on_mouse_down(child, wid_intro_quit_receive_mouse_down);
    }

    {
        widp child;

        child = wid_new_square_button(wid_intro, "play");
        wid_set_font(child, med_font);
        wid_set_no_shape(child);

        fpoint tl = {0.2f, 0.70f};
        fpoint br = {0.8f, 0.90f};

        wid_set_tl_br_pct(child, tl, br);
        wid_set_text(child, "Press SPACE or click to play");
        wid_fade_in_out(child, 1000, 1000, false /* fade out first */);

        wid_set_color(child, WID_COLOR_TEXT, WHITE);
        color c = WHITE;
        c.a = 200;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_OVER);
        c.a = 200;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_FOCUS);
        c.a = 100;
        wid_set_color(child, WID_COLOR_TEXT, c);
        wid_set_text_outline(child, true);

        wid_set_on_mouse_down(child, wid_intro_play_mouse_event);
    }

    wid_intro_bg_create();
    wid_update(wid_intro);

    wid_move_to_pct_centered(wid_intro, 0.5f, 0.5f);
    wid_fade_in(wid_intro_background, intro_effect_delay*2);
    wid_fade_in(wid_intro_title, intro_effect_delay*2);
    wid_fade_in(wid_intro_man, intro_effect_delay*2);
}
