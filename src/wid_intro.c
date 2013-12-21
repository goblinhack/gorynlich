/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "tex.h"
#include "color.h"
#include "wid_intro.h"
#include "wid_popup.h"
#include "wid_editor.h"
#include "wid_editor_map.h"
#include "wid_game_map.h"
#include "wid_intro_about.h"
#include "wid_intro_settings.h"
#include "wid_intro_help.h"
#include "wid_intro_guide.h"
#include "wid_game_over.h"
#include "wid_hiscore.h"
#include "music.h"
#include "sdl.h"
#include "level.h"

static widp wid_intro;
static widp wid_intro_title;
static widp wid_intro_background;

static boolean wid_intro_quit_selected(void);
static void wid_intro_editor_selected(void);
static void wid_intro_play_selected(void);
static void wid_intro_about_selected(void);
static void wid_intro_settings_selected(void);
static void wid_intro_hiscore_selected(void);
static void wid_intro_help_selected(void);
static void wid_intro_guide_selected(void);

static boolean wid_intro_init_done;
static void wid_intro_create(void);

boolean wid_intro_init (void)
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
            wid_destroy_in(wid_intro_title, wid_hide_delay * 2);
            wid_destroy_in(wid_intro_background, wid_hide_delay * 2);
        }
    }
}

static boolean wid_intro_is_hidden;
static boolean wid_intro_is_visible;

void wid_intro_hide (void)
{
    if (wid_intro_is_hidden) {
        return;
    }

    wid_intro_is_hidden = true;
    wid_intro_is_visible = false;

    music_halt();

    if (!wid_intro) {
        DIE("no wid intro");
    }

    wid_move_end(wid_intro);
    wid_move_end(wid_intro_background);
    wid_move_end(wid_intro_title);

    wid_hide(wid_intro, wid_swipe_delay);
    wid_hide(wid_intro_title, wid_swipe_delay);
    wid_hide(wid_intro_background, wid_swipe_delay);

    wid_lower(wid_intro_title);
    wid_lower(wid_intro_background);
    wid_raise(wid_intro);
    wid_update(wid_intro);
}

void wid_intro_visible (void)
{
    if (wid_intro_is_visible) {
        return;
    }

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
    wid_visible(wid_intro_background, 0);
    wid_visible(wid_intro_title, 0);

    wid_move_end(wid_intro);
    wid_move_end(wid_intro_background);
    wid_move_end(wid_intro_title);

    wid_lower(wid_intro_title);
    wid_lower(wid_intro_background);
    wid_raise(wid_intro);
    wid_update(wid_intro);
}

static boolean wid_intro_key_event (widp w, const SDL_KEYSYM *key)
{
    switch (key->sym) {
        case ' ':
        case SDLK_RETURN:
            wid_intro_play_selected();
            return (true);

        case 'q':
        case SDLK_ESCAPE:
            wid_intro_quit_selected();
            return (true);

        case 'a':
            wid_intro_about_selected();
            return (true);

        case 's':
            wid_intro_settings_selected();
            return (true);

        case 'c':
            wid_intro_hiscore_selected();
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

static
boolean wid_intro_wid_noop (widp w, int32_t x, int32_t y, uint32_t button)
{
    return (true);
}

static void wid_intro_play_selected (void)
{
    wid_game_visible();

    wid_intro_hide();
}

static boolean wid_intro_play_mouse_event (widp w, int32_t x, int32_t y,
                                           uint32_t button)
{
    wid_intro_play_selected();

    return (true);
}

static boolean wid_intro_play_key_event (widp w, const SDL_KEYSYM *key)
{
    switch (key->sym) {
        case ' ':
        case SDLK_RETURN:
            wid_intro_play_selected();
            return (true);

        default:
            break;
    }

    return (false);
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

static void wid_intro_help_selected (void)
{
    wid_intro_help_visible();
}

static void wid_intro_guide_selected (void)
{
    wid_intro_guide_visible();

    wid_intro_hide();
}

static boolean wid_intro_help_mouse_event (widp w, int32_t x, int32_t y,
                                           uint32_t button)
{
    wid_intro_guide_selected();

    return (true);
}

static boolean wid_intro_help_key_event (widp w, const SDL_KEYSYM *key)
{
    switch (key->sym) {
        case SDLK_RETURN:
            wid_intro_guide_selected();
            return (true);

        default:
            break;
    }

    return (false);
}

static boolean wid_intro_about_mouse_event (widp w, int32_t x, int32_t y,
                                            uint32_t button)
{
    wid_intro_about_selected();

    return (true);
}

static boolean wid_intro_about_key_event (widp w, const SDL_KEYSYM *key)
{
    switch (key->sym) {
        case SDLK_RETURN:
            wid_intro_about_selected();
            return (true);

        default:
            break;
    }

    return (false);
}

static boolean wid_intro_settings_mouse_event (widp w, int32_t x, int32_t y,
                                            uint32_t button)
{
    wid_intro_settings_selected();

    return (true);
}

static boolean wid_intro_settings_key_event (widp w, const SDL_KEYSYM *key)
{
    switch (key->sym) {
        case SDLK_RETURN:
            wid_intro_settings_selected();
            return (true);

        default:
            break;
    }

    return (false);
}

static boolean wid_intro_hiscore_mouse_event (widp w, int32_t x, int32_t y,
                                            uint32_t button)
{
    wid_intro_hiscore_selected();

    return (true);
}

static boolean wid_intro_hiscore_key_event (widp w, const SDL_KEYSYM *key)
{
    switch (key->sym) {
        case SDLK_RETURN:
            wid_intro_hiscore_selected();
            return (true);

        default:
            break;
    }

    return (false);
}

static void wid_intro_editor_selected (void)
{
    wid_editor_visible();

    wid_intro_hide();
}

static boolean wid_intro_editor_mouse_event (widp w, int32_t x, int32_t y,
                                             uint32_t button)
{
    wid_intro_editor_selected();

    return (true);
}

static boolean wid_intro_editor_key_event (widp w, const SDL_KEYSYM *key)
{
    switch (key->sym) {
        case SDLK_RETURN:
            wid_intro_editor_selected();
            return (true);

        default:
            break;
    }

    return (false);
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

static boolean wid_intro_quit_selected (void)
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
static boolean wid_intro_quit_receive_mouse_up (widp w,
                                                int32_t x, int32_t y,
                                                uint32_t button)
{
    return (wid_intro_quit_selected());
}

static boolean wid_intro_quit_key_event (widp w, const SDL_KEYSYM *key)
{
    switch (key->sym) {
        case SDLK_RETURN:
            return (wid_intro_quit_selected());

        default:
            break;
    }

    return (false);
}

static void wid_intro_bg_create (void)
{
    widp wid;
    texp tex;
    uint32_t tw;
    uint32_t th;

    if (wid_intro_title) {
        return;
    }

    {
        wid = wid_intro_background = wid_new_window("bg");

        tex = tex_find("gorynlich");
        tw = tex_get_width(tex);
        th = tex_get_height(tex);

        fpoint tl = { 0, 0 };
        fpoint br = { (double) tw, (double) th };

        wid_set_tl_br(wid, tl, br);

        wid_set_tex(wid, 0, "gorynlich");

        wid_lower(wid);

        color c;
        wid_set_mode(wid, WID_MODE_NORMAL);
        c = WHITE;
        c.a = 100;

        wid_set_color(wid, WID_COLOR_TL, c);
        wid_set_color(wid, WID_COLOR_BR, c);
        wid_set_color(wid, WID_COLOR_BG, c);
    }

    {
        wid = wid_intro_title = wid_new_window("title");

        tex = tex_find("title");
        tw = tex_get_width(tex);
        th = tex_get_height(tex);

        fpoint tl = { 0, 0 };
        fpoint br = { (double) tw, (double) th };

        wid_set_tl_br(wid, tl, br);

        wid_set_tex(wid, 0, "title");

        wid_raise(wid);

        wid_set_mode(wid, WID_MODE_NORMAL);
        wid_set_color(wid, WID_COLOR_TL, WHITE);
        wid_set_color(wid, WID_COLOR_BR, WHITE);
        wid_set_color(wid, WID_COLOR_BG, WHITE);
    }
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
        c.a = 100;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_OVER);
        c.a = 200;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_FOCUS);
        c.a = 100;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_NORMAL);

        wid_set_on_mouse_down(child, wid_intro_wid_noop);
        wid_set_on_mouse_up(child, wid_intro_editor_mouse_event);
        wid_set_on_key_down(child, wid_intro_editor_key_event);
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
        c.a = 100;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_OVER);
        c.a = 200;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_FOCUS);
        c.a = 100;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_NORMAL);

        wid_set_on_mouse_down(child, wid_intro_wid_noop);
        wid_set_on_mouse_up(child, wid_intro_about_mouse_event);
        wid_set_on_key_down(child, wid_intro_about_key_event);
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
        c.a = 100;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_OVER);
        c.a = 200;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_FOCUS);
        c.a = 100;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_NORMAL);

        wid_set_on_mouse_down(child, wid_intro_wid_noop);
        wid_set_on_mouse_up(child, wid_intro_settings_mouse_event);
        wid_set_on_key_down(child, wid_intro_settings_key_event);
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
        c.a = 100;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_OVER);
        c.a = 200;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_FOCUS);
        c.a = 100;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_NORMAL);

        wid_set_on_mouse_down(child, wid_intro_wid_noop);
        wid_set_on_mouse_up(child, wid_intro_hiscore_mouse_event);
        wid_set_on_key_down(child, wid_intro_hiscore_key_event);
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
        c.a = 100;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_OVER);
        c.a = 200;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_FOCUS);
        c.a = 100;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_on_mouse_down(child, wid_intro_wid_noop);
        wid_set_on_mouse_up(child, wid_intro_help_mouse_event);
        wid_set_on_key_down(child, wid_intro_help_key_event);
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
        c.a = 100;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_OVER);
        c.a = 200;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_FOCUS);
        c.a = 100;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_on_mouse_down(child, wid_intro_wid_noop);
        wid_set_on_mouse_up(child, wid_intro_quit_receive_mouse_up);
        wid_set_on_key_down(child, wid_intro_quit_key_event);
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

        wid_set_on_mouse_down(child, wid_intro_wid_noop);
        wid_set_on_mouse_up(child, wid_intro_play_mouse_event);
        wid_set_on_key_down(child, wid_intro_play_key_event);
    }

    wid_intro_bg_create();
    wid_update(wid_intro);

    wid_move_to_pct_centered(wid_intro, 0.5f, 0.5f);
    wid_move_to_pct_centered(wid_intro_background, 0.5f, 0.5f);
    wid_move_to_pct_centered(wid_intro_title, 0.5f, 0.4f);
}
