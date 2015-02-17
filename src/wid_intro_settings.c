/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "color.h"
#include "wid_popup.h"
#include "wid_intro_settings.h"
#include "wid_intro.h"
#include "wid_menu.h"
#include "string_util.h"
#include "music.h"

static widp wid_intro_settings;
static widp wid_intro_menu;
static uint8_t wid_intro_settings_init_done;
static uint8_t wid_intro_settings_restart_needed;
static struct config old_global_config;

static void wid_intro_settings_create(void);
static void wid_intro_settings_destroy(void);
static void wid_intro_settings_save(void);
static uint8_t wid_intro_restart_selected(void);
static void wid_intro_settings_read(void);
static uint8_t wid_intro_settings_increment_mouse_event(widp w,
                                                   int32_t x, int32_t y,
                                                   uint32_t button);
static uint8_t wid_intro_settings_decrement_mouse_event(widp w,
                                                   int32_t x, int32_t y,
                                                   uint32_t button);
static uint8_t wid_intro_settings_toggle_mouse_event(widp w,
                                                   int32_t x, int32_t y,
                                                   uint32_t button);
static int saved_focus;

#define WID_INTRO_MAX_SETTINGS  6
#define WID_INTRO_MAX_VAL      30 

enum {
    WID_INTRO_SETTINGS_ROW_WINDOW,
    WID_INTRO_SETTINGS_ROW_SOUND,
    WID_INTRO_SETTINGS_ROW_MUSIC,
    WID_INTRO_SETTINGS_ROW_INTRO_SCREEN,
    WID_INTRO_SETTINGS_ROW_DISPLAY_SYNC,
    WID_INTRO_SETTINGS_ROW_FPS_COUNTER,
};

static const char *wid_intro_button_name[WID_INTRO_MAX_SETTINGS] = {
    "Window",
    "Sound",
    "Music",
    "Intro screen",
    "Display sync",
    "FPS counter",
};

static const char *wid_intro_button_value_string
                        [WID_INTRO_MAX_SETTINGS][WID_INTRO_MAX_VAL] = {
    { 
        "640x480",
        "800x600",
        "1008x672",
        "1024x768",
        "1152x864",
        "1512x1008",
        "1280x960",
        "1280x1024",
        "1370x856",
        "1400x1050",
        "1600x800",
        "2880x1800",
        0 
    },
    { "Off", "Min", "Normal", "Max", 0 },
    { "Off", "Min", "Normal", "Max", 0 },
    { "Off", "On", 0 },
    { "Off", "On", 0 },
    { "Off", "On", 0 },
};

static int wid_intro_button_value_toggle[WID_INTRO_MAX_SETTINGS] = {
    0,
    0,
    0,
    1,
    1,
    1,
};

static uint32_t wid_intro_button_val[WID_INTRO_MAX_SETTINGS];

uint8_t wid_intro_settings_init (void)
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

    if (!wid_intro_settings_restart_needed) {
        wid_intro_visible();
    }
}

void wid_intro_settings_visible (void)
{
    wid_intro_settings_read();

    wid_intro_settings_create();
}

static uint8_t wid_intro_settings_back_mouse_event (widp w, 
                                                    int32_t x, int32_t y,
                                                    uint32_t button)
{
    wid_intro_settings_save();

    wid_intro_settings_hide();

    wid_intro_restart_selected();

    return (true);
}

static uint8_t wid_intro_settings_mouse_event (widp w,
                                               int32_t x, int32_t y,
                                               uint32_t button)
{
    if (button == SDLK_LEFT) {
        wid_intro_settings_decrement_mouse_event(w, x, y, SDL_BUTTON_LEFT);
    } else if (button == SDLK_RIGHT) {
        wid_intro_settings_increment_mouse_event(w, x, y, SDL_BUTTON_LEFT);
    } else {
        wid_intro_settings_toggle_mouse_event(w, x, y, SDL_BUTTON_LEFT);
    }

    return (true);
}

static uint8_t wid_intro_settings_increment_mouse_event (widp w,
                                                         int32_t x, int32_t y,
                                                         uint32_t button)
{
    /*
     * Increment.
     */
    wid_menu_ctx *ctx = (typeof(ctx)) wid_get_client_context(w);
    verify(ctx);

    int32_t row = (typeof(row)) (intptr_t) wid_get_client_context2(w);
    int32_t val = wid_intro_button_val[row];

    if (!wid_intro_button_value_string[row][val+1]) {
        wid_intro_button_val[row] = 0;
    } else {
        wid_intro_button_val[row]++;
    }

    wid_intro_settings_destroy();
    wid_intro_settings_create();
    wid_intro_settings_save();

    return (true);
}

static uint8_t wid_intro_settings_decrement_mouse_event (widp w,
                                                         int32_t x, int32_t y,
                                                         uint32_t button)
{
    /*
     * Decrement.
     */
    wid_menu_ctx *ctx = (typeof(ctx)) wid_get_client_context(w);
    verify(ctx);

    int32_t row = (typeof(row)) (intptr_t) wid_get_client_context2(w);
    int32_t val = wid_intro_button_val[row];

    if (!val) {
        return (true);
    }

    wid_intro_button_val[row]--;

    wid_intro_settings_destroy();
    wid_intro_settings_create();
    wid_intro_settings_save();

    return (true);
}

static uint8_t wid_intro_settings_toggle_mouse_event (widp w,
                                                    int32_t x, int32_t y,
                                                    uint32_t button)
{
    /*
     * Invert.
     */
    wid_menu_ctx *ctx = (typeof(ctx)) wid_get_client_context(w);
    verify(ctx);

    int32_t row = (typeof(row)) (intptr_t) wid_get_client_context2(w);

    /*
     * If this is not the kind of value you toggle, then just increment.
     */
    if (!wid_intro_button_value_toggle[row]) {
        return (wid_intro_settings_increment_mouse_event(w, x, y, button));
    }

    wid_intro_button_val[row] = !wid_intro_button_val[row];

    wid_intro_settings_destroy();
    wid_intro_settings_create();
    wid_intro_settings_save();

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
        cmp_str = wid_intro_button_value_string
                        [WID_INTRO_SETTINGS_ROW_WINDOW][val];
        if (!cmp_str) {
            continue;
        }

        if (!strcasecmp(val_str, cmp_str)) {
            break;
        }
    }

    if (val == WID_INTRO_MAX_VAL) {
        MSG_BOX("Window size %s was not found in known list", val_str);
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
        !wid_intro_button_value_string[WID_INTRO_SETTINGS_ROW_SOUND][val]) {

        wid_intro_button_val[WID_INTRO_SETTINGS_ROW_SOUND] = 0;

        MSG_BOX("Sound volume size %d was not found in known list", val);
    }

    /*
     * music_volume.
     */
    val = wid_intro_button_val[WID_INTRO_SETTINGS_ROW_MUSIC] =
        global_config.music_volume;

    if ((val >= WID_INTRO_MAX_VAL) || (val < 0) ||
        !wid_intro_button_value_string[WID_INTRO_SETTINGS_ROW_MUSIC][val]) {

        wid_intro_button_val[WID_INTRO_SETTINGS_ROW_MUSIC] = 0;

        MSG_BOX("Music volume %d was not found in known list", val);
    }

    /*
     * display_sync.
     */
    val = wid_intro_button_val[WID_INTRO_SETTINGS_ROW_DISPLAY_SYNC] =
        global_config.display_sync;

    if ((val >= WID_INTRO_MAX_VAL) || (val < 0) ||
        !wid_intro_button_value_string[WID_INTRO_SETTINGS_ROW_DISPLAY_SYNC][val]) {

        wid_intro_button_val[WID_INTRO_SETTINGS_ROW_DISPLAY_SYNC] = 0;

        MSG_BOX("Display sync value %d was not found in known list", val);
    }

    /*
     * fps_counter.
     */
    val = wid_intro_button_val[WID_INTRO_SETTINGS_ROW_FPS_COUNTER] =
        global_config.fps_counter;

    if ((val >= WID_INTRO_MAX_VAL) || (val < 0) ||
        !wid_intro_button_value_string[WID_INTRO_SETTINGS_ROW_FPS_COUNTER][val]) {

        wid_intro_button_val[WID_INTRO_SETTINGS_ROW_FPS_COUNTER] = 0;

        MSG_BOX("FPS counter value %d was not found in known list", val);
    }

    /*
     * intro_screen.
     */
    val = wid_intro_button_val[WID_INTRO_SETTINGS_ROW_INTRO_SCREEN] =
        global_config.intro_screen;

    if ((val >= WID_INTRO_MAX_VAL) || (val < 0) ||
        !wid_intro_button_value_string[WID_INTRO_SETTINGS_ROW_INTRO_SCREEN][val]) {

        wid_intro_button_val[WID_INTRO_SETTINGS_ROW_INTRO_SCREEN] = 0;

        MSG_BOX("Intro screen value %d was not found in known list", val);
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

    wid_intro_visible();
}

static uint8_t wid_intro_restart_selected (void)
{
    LOG("Restart selected");

    if (wid_intro_restart_popup) {
        return (false);
    }

    if (!wid_intro_settings_restart_needed) {
        return (false);
    }

    wid_intro_restart_popup = 
        wid_menu(0,
                 vvlarge_font,
                 large_font,
                 0.5, /* x */
                 0.5, /* y */
                 0.95, /* padding between buttons */
                 2, /* focus */
                 3, /* items */
                 (int) 0, "Settings changed, restart game?", (void*)0,

                 (int) 'y', "Yes", wid_intro_restart_callback_yes,

                 (int) 'n', "No",  wid_intro_restart_callback_no);

    return (true);
}

static void wid_intro_settings_save (void)
{
    wid_intro_settings_restart_needed = false;

    /*
     * window.
     */
    sscanf(wid_intro_button_value_string[WID_INTRO_SETTINGS_ROW_WINDOW][
            wid_intro_button_val[WID_INTRO_SETTINGS_ROW_WINDOW]],
            "%dx%d",
            &global_config.video_pix_width,
            &global_config.video_pix_height);

    global_config.music_volume = 
        wid_intro_button_val[WID_INTRO_SETTINGS_ROW_MUSIC];

    global_config.sound_volume =
        wid_intro_button_val[WID_INTRO_SETTINGS_ROW_SOUND];

    global_config.display_sync =
        wid_intro_button_val[WID_INTRO_SETTINGS_ROW_DISPLAY_SYNC];

#if SDL_MAJOR_VERSION == 1 && SDL_MINOR_VERSION == 2 /* { */

    if (global_config.display_sync) {
        SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
    } else {
        SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 0);
    }

#else /* } { */

    if (global_config.display_sync) {
        SDL_GL_SetSwapInterval(1);
    } else {
        SDL_GL_SetSwapInterval(0);
    }

#endif /* } */

    global_config.fps_counter =
        wid_intro_button_val[WID_INTRO_SETTINGS_ROW_FPS_COUNTER];

    global_config.intro_screen =
        wid_intro_button_val[WID_INTRO_SETTINGS_ROW_INTRO_SCREEN];

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

    if (wid_intro_settings) {
        return;
    }

    widp w = wid_intro_settings = wid_new_rounded_window("wid settings");

    fpoint tl = {0.0, 0.0};
    fpoint br = {1.0, 1.0};

    wid_set_tl_br_pct(w, tl, br);
    wid_set_no_shape(w);

    char *values[WID_INTRO_MAX_SETTINGS];
    int i;
    for (i = WID_INTRO_SETTINGS_ROW_WINDOW; i < WID_INTRO_MAX_SETTINGS; i++) {
        values[i] = dynprintf("%s:%s",
                wid_intro_button_name[i],
                wid_intro_button_value_string[i][wid_intro_button_val[i]]);
    }

    i = WID_INTRO_SETTINGS_ROW_WINDOW;
    wid_intro_menu = wid_menu(wid_intro_settings,
                vvlarge_font,
                large_font,
                0.5, /* x */
                0.7, /* y */
                0.95, /* padding between buttons */
                saved_focus, /* focus */
                7, /* items */

                (int) '1', values[i], wid_intro_settings_mouse_event,

                (int) '2', values[i + 1], wid_intro_settings_mouse_event,

                (int) '3', values[i + 2], wid_intro_settings_mouse_event,

                (int) '4', values[i + 3], wid_intro_settings_mouse_event,

                (int) '5', values[i + 4], wid_intro_settings_mouse_event,

                (int) '6', values[i + 5], wid_intro_settings_mouse_event,

                (int) 'b', "Back", wid_intro_settings_back_mouse_event);

    for (i = WID_INTRO_SETTINGS_ROW_WINDOW; i < WID_INTRO_MAX_SETTINGS; i++) {
        myfree(values[i]);
    }

    wid_raise(wid_intro_settings);
    wid_update(wid_intro_settings);
}

static void wid_intro_settings_destroy (void)
{
    if (!wid_intro_settings) {
        return;
    }

    wid_menu_ctx *ctx = 
                    (typeof(ctx)) wid_get_client_context(wid_intro_menu);
    verify(ctx);

    /*
     * Save the focus so when we remake the menu we are at the same entry.
     */
    saved_focus = ctx->focus;

    wid_destroy_nodelay(&wid_intro_settings);
}
