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
#include "wid_intro_settings.h"
#include "wid_popup.h"
#include "wid_intro_settings.h"
#include "sdl.h"
#include "config.h"
#include "string.h"
#include "gl.h"
#include "music.h"

static widp wid_intro_settings;
static widp wid_intro_settings_container;
static boolean wid_intro_settings_init_done;
static boolean wid_intro_settings_restart_needed;
static struct config old_global_config;

static void wid_intro_settings_create(void);
static void wid_intro_settings_destroy(void);
static void wid_intro_settings_save(void);
static boolean wid_intro_restart_selected(void);

#define WID_INTRO_MAX_SETTINGS  3
#define WID_INTRO_MAX_VAL      30 

enum {
    WID_INTRO_SETTINGS_ROW_WINDOW,
    WID_INTRO_SETTINGS_ROW_SOUND,
    WID_INTRO_SETTINGS_ROW_MUSIC,
};

static const char *wid_intro_button_col1[WID_INTRO_MAX_SETTINGS] = {
    "Window",
    "Sound",
    "Music",
};

static const char *wid_intro_button_col2[WID_INTRO_MAX_SETTINGS] = {
    "+",
    "+",
    "+",
};

static const char *wid_intro_button_col3[WID_INTRO_MAX_SETTINGS] = {
    "-",
    "-",
    "-",
};

static const char *wid_intro_button_col4
                        [WID_INTRO_MAX_SETTINGS][WID_INTRO_MAX_VAL] = {
    { 
        "640x480",
        "800x600",
        "1080x720",
        "1024x600",
        "1024x768",
        "1093x614",
        "1152x864",
        "1280x1024",
        "1280x720",
        "1280x768",
        "1280x800",
        "1280x960",
        "1360x768",
        "1366x768",
        "1400x1050",
        "1440x900",
        "1600x1200",
        "1600x900",
        "1680x1050",
        "1920x1080",
        "1920x1200",
        "2048x1152",
        "2560x1440",
        "2560x1600",
        0 
    },
    { "Off", "Min", "Normal", "Max", 0 },
    { "Off", "Min", "Normal", "Max", 0 },
};

static uint32_t wid_intro_button_val[WID_INTRO_MAX_SETTINGS];

boolean wid_intro_settings_init (void)
{
    if (!wid_intro_settings_init_done) {
    }

    wid_intro_settings_init_done = true;

    return (true);
}

void wid_intro_settings_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (wid_intro_settings_init_done) {
        wid_intro_settings_init_done = false;

        wid_intro_settings_destroy();
    }
}

void wid_intro_settings_hide (void)
{
    wid_intro_settings_destroy();
}

void wid_intro_settings_visible (void)
{
    wid_intro_settings_create();
}

static boolean wid_intro_settings_mouse_event (widp w, int32_t x, int32_t y,
                                               uint32_t button)
{
    wid_intro_settings_save();

    wid_intro_settings_hide();

    wid_intro_restart_selected();

    return (true);
}

static boolean wid_intro_settings_col1_mouse_event (widp w,
                                                    int32_t x, int32_t y,
                                                    uint32_t button)
{
    return (true);
}

static boolean wid_intro_settings_col2_mouse_event (widp w,
                                                    int32_t x, int32_t y,
                                                    uint32_t button)
{
    /*
     * Increment.
     */
    int32_t row = (typeof(row)) (intptr_t) wid_get_client_context(w);
    int32_t val = wid_intro_button_val[row];

    if (!wid_intro_button_col4[row][val+1]) {
        return (true);
    }

    wid_intro_button_val[row]++;

    wid_destroy_nodelay(&wid_intro_settings_container);
    wid_intro_settings_create();

    wid_intro_settings_save();

    return (true);
}

static boolean wid_intro_settings_col3_mouse_event (widp w,
                                                    int32_t x, int32_t y,
                                                    uint32_t button)
{
    /*
     * Decrement.
     */
    int32_t row = (typeof(row)) (intptr_t) wid_get_client_context(w);
    int32_t val = wid_intro_button_val[row];

    if (!val) {
        return (true);
    }

    wid_intro_button_val[row]--;

    wid_destroy_nodelay(&wid_intro_settings_container);
    wid_intro_settings_create();

    wid_intro_settings_save();

    return (true);
}

static boolean wid_intro_settings_col4_mouse_event (widp w,
                                                    int32_t x, int32_t y,
                                                    uint32_t button)
{
    return (true);
}

static boolean wid_intro_settings_key_event (widp w, const SDL_KEYSYM *key)
{
    switch (key->sym) {
        case 'q':
        case SDLK_ESCAPE:
            wid_intro_settings_hide();
            return (true);

        default:
            break;
    }

    return (false);
}

static boolean wid_intro_settings_receive_mouse_motion (
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

static void wid_intro_settings_read (void)
{
    memcpy(&old_global_config, &global_config, sizeof(old_global_config));

    const char *cmp_str;
    char *val_str;
    int32_t val;

    /*
     * window.
     */
    val_str = dynprintf("%dx%d",
                        global_config.video_pix_width, global_config.video_pix_height);

    for (val = 0; val < WID_INTRO_MAX_VAL; val++) {
        cmp_str = wid_intro_button_col4[WID_INTRO_SETTINGS_ROW_WINDOW][val];
        if (!cmp_str) {
            continue;
        }

        if (!strcmp(val_str, cmp_str)) {
            break;
        }
    }

    if (val == WID_INTRO_MAX_VAL) {
        ERR_TB("Window size %s was not found in known list", val_str);
        val = 0;
    }

    wid_intro_button_val[WID_INTRO_SETTINGS_ROW_WINDOW] = val;

    myfree(val_str);

    /*
     * sound_volume.
     */
    val = wid_intro_button_val[WID_INTRO_SETTINGS_ROW_SOUND] =
        global_config.sound_volume;

    if ((val >= WID_INTRO_MAX_VAL) || (val < 0) ||
        !wid_intro_button_col4[WID_INTRO_SETTINGS_ROW_SOUND][val]) {

        wid_intro_button_val[WID_INTRO_SETTINGS_ROW_SOUND] = 0;

        ERR_TB("Sound volume size %d was not found in known list", val);
    }

    /*
     * music_volume.
     */
    val = wid_intro_button_val[WID_INTRO_SETTINGS_ROW_MUSIC] =
        global_config.music_volume;

    if ((val >= WID_INTRO_MAX_VAL) || (val < 0) ||
        !wid_intro_button_col4[WID_INTRO_SETTINGS_ROW_MUSIC][val]) {

        wid_intro_button_val[WID_INTRO_SETTINGS_ROW_MUSIC] = 0;

        ERR_TB("Music volume %d was not found in known list", val);
    }

}

static widp wid_intro_restart_popup;

static void wid_intro_restart_callback_yes (widp wid)
{
    wid_destroy(&wid_intro_restart_popup);

    FINI_LOG("restart yes selected");

    restart();
}

static void wid_intro_restart_callback_no (widp wid)
{
    wid_destroy(&wid_intro_restart_popup);
}

static boolean wid_intro_restart_selected (void)
{
    if (wid_intro_restart_popup) {
        return (false);
    }

    if (!wid_intro_settings_restart_needed) {
        return (false);
    }

    wid_intro_restart_popup =
        wid_popup("%%fg=red$Restart game?",
                  0                 /* title */,
                  0.5f, 0.5f,       /* x,y postition in percent */
                  large_font,       /* title font */
                  large_font,         /* body font */
                  large_font,         /* button font */
                  2,                /* number buttons */
                  "Yes", wid_intro_restart_callback_yes,
                  "No", wid_intro_restart_callback_no);

    return (true);
}

static void wid_intro_settings_save (void)
{
    wid_intro_settings_restart_needed = false;

    /*
     * window.
     */
    sscanf(wid_intro_button_col4[WID_INTRO_SETTINGS_ROW_WINDOW][
            wid_intro_button_val[WID_INTRO_SETTINGS_ROW_WINDOW]],
            "%dx%d",
            &global_config.video_pix_width,
            &global_config.video_pix_height);

    /*
     * sound_volume.
     */
    global_config.sound_volume =
        wid_intro_button_val[WID_INTRO_SETTINGS_ROW_SOUND];

    /*
     * music_volume.
     */
    global_config.music_volume = 
        wid_intro_button_val[WID_INTRO_SETTINGS_ROW_MUSIC];

    music_update_volume();

    if (!memcmp(&old_global_config,
                &global_config, sizeof(old_global_config))) {
        LOG("No config change");
        return;
    }

    config_save();

    if ((old_global_config.video_pix_width != global_config.video_pix_width) ||
        (old_global_config.video_pix_height != global_config.video_pix_height)) {

        global_config.video_pix_width = old_global_config.video_pix_width;
        global_config.video_pix_height = old_global_config.video_pix_height;

        LOG("Restart needed");

        wid_intro_settings_restart_needed = true;
    }
}

static void wid_intro_settings_create (void)
{
    wid_intro_settings_restart_needed = false;

    if (!wid_intro_settings) {
        wid_intro_settings_read();

        widp w = wid_intro_settings = wid_new_rounded_window("wid settings");

        fpoint tl = {0.1, 0.2};
        fpoint br = {0.9, 0.8};

        wid_set_tl_br_pct(w, tl, br);
        wid_set_font(w, med_font);

        wid_set_color(w, WID_COLOR_TEXT, WHITE);

        color c = BLACK;
        c.a = 200;
        wid_set_color(w, WID_COLOR_BG, c);

        c = STEELBLUE;
        c.a = 150;
        wid_set_color(w, WID_COLOR_TL, c);
        wid_set_color(w, WID_COLOR_BR, c);
        wid_set_bevel(w, 4);

        wid_set_on_mouse_motion(w, wid_intro_settings_receive_mouse_motion);
    }

    {
        widp w = wid_intro_settings_container =
            wid_new_container(wid_intro_settings, "wid settings container");

        fpoint tl = {0.0, 0.0};
        fpoint br = {1.0, 1.0};

        wid_set_tl_br_pct(w, tl, br);
    }

    {
        fpoint tl = {0.0, 0.0};
        fpoint br = {1.0, 0.2};

        widp w = wid_new_container(wid_intro_settings, "wid settings title");

        wid_set_tl_br_pct(w, tl, br);

        wid_set_text(w, "Settings");
        wid_set_font(w, large_font);
        wid_set_color(w, WID_COLOR_TEXT, STEELBLUE);

        wid_set_text_outline(w, true);
    }

    {
        uint32_t i;

        for (i=0; i<ARRAY_SIZE(wid_intro_button_col1); i++)
        {
            widp w = wid_new_square_button(wid_intro_settings_container,
                                           wid_intro_button_col1[i]);

            fpoint tl = {0.05, 0.2};
            fpoint br = {0.48, 0.3};

            double height = 0.12;

            br.y += (double)i * height;
            tl.y += (double)i * height;

            wid_set_tl_br_pct(w, tl, br);
            wid_set_text(w, wid_intro_button_col1[i]);
            wid_set_font(w, med_font);

            color c = BLACK;

            c.a = 100;
            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_OVER);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_NORMAL);

            wid_set_on_mouse_up(w, wid_intro_settings_col1_mouse_event);
            wid_set_client_context(w, (void*)(uintptr_t)i);
            wid_set_bevel(w,0);
        }
    }

    {
        uint32_t i;

        for (i=0; i<ARRAY_SIZE(wid_intro_button_col1); i++) {

            if (!wid_intro_button_col2[i]) {
                continue;
            }

            widp w = wid_new_square_button(wid_intro_settings_container,
                                           wid_intro_button_col2[i]);

            fpoint tl = {0.49, 0.2};
            fpoint br = {0.595, 0.3};

            double height = 0.12;

            br.y += (double)i * height;
            tl.y += (double)i * height;

            wid_set_tl_br_pct(w, tl, br);
            wid_set_text(w, wid_intro_button_col2[i]);
            wid_set_font(w, med_font);

            color c = LIME;

            c.a = 150;
            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_color(w, WID_COLOR_BG, c);

            c.a = 250;
            wid_set_mode(w, WID_MODE_OVER);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_NORMAL);

            wid_set_on_mouse_up(w, wid_intro_settings_col2_mouse_event);
            wid_set_client_context(w, (void*)(uintptr_t)i);
            wid_set_bevel(w,0);
        }
    }

    {
        uint32_t i;

        for (i=0; i<ARRAY_SIZE(wid_intro_button_col1); i++) {

            if (!wid_intro_button_col3[i]) {
                continue;
            }

            widp w = wid_new_square_button(wid_intro_settings_container,
                                           wid_intro_button_col3[i]);

            fpoint tl = {0.605, 0.2};
            fpoint br = {0.71, 0.3};

            double height = 0.12;

            br.y += (double)i * height;
            tl.y += (double)i * height;

            wid_set_tl_br_pct(w, tl, br);
            wid_set_text(w, wid_intro_button_col3[i]);
            wid_set_font(w, med_font);

            color c = STEELBLUE;

            c.a = 150;
            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_color(w, WID_COLOR_BG, c);

            c.a = 250;
            wid_set_mode(w, WID_MODE_OVER);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_NORMAL);

            wid_set_on_mouse_up(w, wid_intro_settings_col3_mouse_event);
            wid_set_client_context(w, (void*)(uintptr_t)i);
            wid_set_bevel(w,0);
        }
    }

    {
        uint32_t i;

        for (i=0; i<ARRAY_SIZE(wid_intro_button_col1); i++) {

            if (!wid_intro_button_col4[i]) {
                continue;
            }

            widp w = wid_new_square_button(wid_intro_settings_container,
                                           wid_intro_button_col3[i]);

            fpoint tl = {0.72, 0.2};
            fpoint br = {0.95, 0.3};

            double height = 0.12;

            br.y += (double)i * height;
            tl.y += (double)i * height;

            wid_set_tl_br_pct(w, tl, br);
            wid_set_text(w,
                         wid_intro_button_col4[i][wid_intro_button_val[i]]);
            wid_set_font(w, med_font);

            color c = BLUE;

            c.a = 100;
            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_OVER);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_NORMAL);

            wid_set_on_mouse_up(w, wid_intro_settings_col4_mouse_event);
            wid_set_client_context(w, (void*)(uintptr_t)i);
            wid_set_bevel(w,0);
        }
    }

    {
        const char *button_name = "All Done";

        widp w = wid_new_rounded_small_button(wid_intro_settings_container,
                                              button_name);

        fpoint tl = {0.7, 0.85};
        fpoint br = {0.95, 0.98};

        wid_set_tl_br_pct(w, tl, br);
        wid_set_text(w, button_name);
        wid_set_font(w, med_font);

        color c = STEELBLUE;

        c.a = 100;
        wid_set_mode(w, WID_MODE_NORMAL);
        wid_set_color(w, WID_COLOR_BG, c);

        c.a = 250;
        wid_set_mode(w, WID_MODE_OVER);
        wid_set_color(w, WID_COLOR_BG, c);

        wid_set_mode(w, WID_MODE_FOCUS);

        wid_set_mode(w, WID_MODE_NORMAL);

        wid_set_on_mouse_up(w, wid_intro_settings_mouse_event);
        wid_set_on_key_down(w, wid_intro_settings_key_event);
    }

    wid_raise(wid_intro_settings);

    wid_update(wid_intro_settings);
}

static void wid_intro_settings_destroy (void)
{
    wid_destroy(&wid_intro_settings);
}
