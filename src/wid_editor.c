/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <libgen.h>
#include <SDL.h>
#include <errno.h>

#include "main.h"
#include "wid.h"
#include "color.h"
#include "wid_editor_buttons.h"
#include "wid_editor_map.h"
#include "wid_editor.h"
#include "wid_popup.h"
#include "wid_tooltip.h"
#include "wid_dirlist.h"
#include "wid_intro.h"
#include "dir.h"
#include "file.h"
#include "string.h"
#include "marshal.h"
#include "wid_text_input.h"
#include "wid_textbox.h"
#include "level.h"

static boolean wid_editor_init_done;

widp wid_editor_clear_popup;
widp wid_editor_help_popup;
widp wid_editor_save_popup;
widp wid_editor_title_popup;
widp wid_editor_load_popup;
widp wid_editor_filename_and_title;

boolean wid_editor_mode_eraser;
boolean wid_editor_mode_inc;
boolean wid_editor_mode_dec;
boolean wid_editor_mode_draw;
boolean wid_editor_mode_line;
boolean wid_editor_mode_fill;

boolean wid_editor_init (void)
{
    wid_editor_init_done = true;

    return (true);
}

void wid_editor_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (wid_editor_init_done) {
        wid_editor_init_done = false;

        wid_editor_map_wid_destroy();
        wid_editor_buttons_wid_destroy();
        wid_destroy(&wid_editor_filename_and_title);
    }
}

void wid_editor_hide (void)
{
    if (!wid_editor_map_window) {
        return;
    }

    wid_move_end(wid_editor_buttons_window);
    wid_move_end(wid_editor_map_window);

    wid_hide(wid_editor_map_window, wid_fade_delay);
    wid_hide(wid_editor_buttons_window, wid_fade_delay);

    wid_intro_visible();
}

void wid_editor_visible (void)
{
    wid_editor_map_wid_create();
    wid_editor_buttons_wid_create();

    wid_editor_draw();

    wid_move_end(wid_editor_buttons_window);
    wid_move_end(wid_editor_map_window);

    wid_move_to_pct_in(wid_editor_buttons_window, 0.70, 0.0, 0);

    wid_visible(wid_editor_map_window, wid_fade_delay);
    wid_visible(wid_editor_buttons_window, wid_fade_delay);
}

static boolean wid_editor_any_popup (void)
{
    if (wid_editor_help_popup ||
        wid_editor_clear_popup ||
        wid_editor_save_popup ||
        wid_editor_title_popup ||
        wid_editor_load_popup) {
        return (true);
    }

    return (false);
}

static void wid_editor_save_ok (widp w)
{
    widp top;

    /*
     * We're given the ok or cancel button, so must save the text box.
     */
    const char *dir_and_file = wid_get_text(w);

    /*
     * Destroy the save dialog.
     */
    top = wid_get_top_parent(w);
    wid_destroy(&top);
    wid_editor_save_popup = 0;

    LOG("Saving: %s", dir_and_file);

    /*
     * Write the file.
     */
    marshal_p ctx;
    ctx = marshal(dir_and_file);
    wid_editor_marshal(ctx);

    if (marshal_fini(ctx) < 0) {
        /*
         * Fail
         */
        char *popup_str = dynprintf("Failed to save %s: %s", dir_and_file,
                                  strerror(errno));

        (void) wid_popup_error(popup_str);
        myfree(popup_str);
    } else {
        /*
         * Success
         */
        char *popup_str = dynprintf("Saved %s", dir_and_file);

        widp popup = wid_tooltip_simple(popup_str);
        wid_destroy_in(popup, ONESEC);
        myfree(popup_str);

        LOG("Saved: %s", dir_and_file);
    }
}

static void wid_editor_save_cancel (widp w)
{
    widp top;

    top = wid_get_top_parent(w);
    wid_destroy(&top);
    wid_editor_save_popup = 0;
}

static void wid_editor_title_ok (widp w)
{
    widp top;

    /*
     * We're given the ok or cancel button, so must title the text box.
     */
    const char *title = wid_get_text(w);

    level_set_title(level_ed, title);

    /*
     * Destroy the title dialog.
     */
    top = wid_get_top_parent(w);
    wid_destroy(&top);
    wid_editor_title_popup = 0;

    wid_destroy(&wid_editor_filename_and_title);

   if (level_get_title(level_ed) &&
        strcmp(level_get_title(level_ed), "(null)")) {

        LOG("Level titled as: %s", title);

        char *name = dynprintf("%s", level_get_title(level_ed));

        wid_editor_filename_and_title = wid_textbox(
                    wid_editor_map_window,
                    0, /* textbox */
                    name,
                    0.5, 0.07, med_font);

        myfree(name);
    } else {
        wid_editor_filename_and_title = wid_textbox(
                    wid_editor_map_window,
                    0, /* textbox */
                    "Level is unnamed",
                    0.5, 0.07, med_font);
    }

    wid_set_no_shape(wid_editor_filename_and_title);
    wid_raise(wid_editor_filename_and_title);
    wid_set_do_not_lower(wid_editor_filename_and_title, true);
}

static void wid_editor_title_cancel (widp w)
{
    widp top;

    top = wid_get_top_parent(w);
    wid_destroy(&top);
    wid_editor_title_popup = 0;
}

static void wid_editor_load_ok (widp w)
{
    char *dir_and_file;
    widp top;

    /*
     * We're given the ok or cancel button, so must load the text box.
     */
    dir_and_file = dupstr(wid_get_text(wid_editor_load_popup), __FUNCTION__);

    if (!file_exists(dir_and_file)) {
        char *tmp = dupstr(dir_and_file, "strip dir");
        char *tmp2 = dupstr(basename(tmp), "strip dir");
        myfree(dir_and_file);
        dir_and_file = tmp2;
        myfree(tmp);

        if (!file_exists(dir_and_file)) {
            tmp = dynprintf("data/levels/%s", dir_and_file);
            myfree(dir_and_file);
            dir_and_file = tmp;
        }
    }

    /*
     * Destroy the load dialog.
     */
    top = wid_get_top_parent(w);
    wid_destroy(&top);
    wid_editor_load_popup = 0;

    demarshal_p ctx;

    LOG("Loading: %s", dir_and_file);

    if (!(ctx = demarshal(dir_and_file))) {
        /*
         * Fail
         */
        char *popup_str = dynprintf("Failed to load %s: %s", dir_and_file,
                                    strerror(errno));

        (void) wid_popup_error(popup_str);
        myfree(popup_str);
    } else {
        /*
         * Success
         */
        char *popup_str = dynprintf("Loaded %s", dir_and_file);

        widp popup = wid_tooltip_simple(popup_str);
        wid_destroy_in(popup, ONESEC);
        myfree(popup_str);

        LOG("Loaded: %s", dir_and_file);

        if (!wid_editor_demarshal(ctx)) {
            char *popup_str = dynprintf("There were some errors "
                                        "while loading %s: %s", dir_and_file,
                                        strerror(errno));

            (void) wid_popup_error(popup_str);
            myfree(popup_str);
        }

        demarshal_fini(ctx);
    }

    /*
     * Print the level title.
     */
    verify(level_ed);

    wid_destroy(&wid_editor_filename_and_title);

    if (level_get_title(level_ed) &&
            strcmp(level_get_title(level_ed), "(null)")) {

        char *title = dynprintf("%s", level_get_title(level_ed));

        wid_editor_filename_and_title = wid_textbox(
                    wid_editor_map_window,
                    0,
                    title,
                    0.5, 0.07, med_font);
        myfree(title);
    } else {
        wid_editor_filename_and_title = wid_textbox(
                    wid_editor_map_window,
                    0,
                    basename(dir_and_file),
                    0.5, 0.07, med_font);
    }

    wid_set_no_shape(wid_editor_filename_and_title);
    wid_raise(wid_editor_filename_and_title);
    wid_set_do_not_lower(wid_editor_filename_and_title, true);

    myfree(dir_and_file);
}

static void wid_editor_load_cancel (widp w)
{
    widp top;

    top = wid_get_top_parent(w);
    wid_destroy(&top);
    wid_editor_load_popup = 0;
}

void wid_editor_save (void)
{
    if (wid_editor_any_popup()) {
        return;
    }

    wid_editor_save_popup = wid_dirlist(
          LEVELS_PATH,              /* dir */
          0,                        /* include */
          0,                        /* exclude */
          "%%fg=red$Save level",    /* title */
          0.5, 0.5,                 /* position */
          2,                        /* buttons */
          "Ok", wid_editor_save_ok,
          "Cancel", wid_editor_save_cancel);
}

void wid_editor_title (void)
{
    if (wid_editor_any_popup()) {
        return;
    }

    wid_editor_title_popup = wid_text_input(
          "Level title?",
          0.5, 0.5,                 /* position */
          2,                        /* buttons */
          "Ok", wid_editor_title_ok,
          "Cancel", wid_editor_title_cancel);
}

void wid_editor_load (void)
{
    if (wid_editor_any_popup()) {
        return;
    }

    /*
     * The button returned is the text box.
     */
    wid_editor_load_popup = wid_dirlist(
          LEVELS_PATH,              /* dir */
          0,                        /* include */
          0,                        /* exclude */
          "%%fg=red$Load level",    /* title */
          0.5, 0.5,                 /* position */
          2,                        /* buttons */
          "Ok", wid_editor_load_ok,
          "Cancel", wid_editor_load_cancel);
}

static void wid_editor_clear_callback_yes (widp wid)
{
    wid_destroy(&wid_editor_clear_popup);

    wid_empty_grid(wid_editor_map_grid_container);

    wid_editor_add_grid();

    wid_destroy(&wid_editor_filename_and_title);

    wid_raise(wid_editor_buttons_window);
}

static void wid_editor_clear_callback_no (widp wid)
{
    wid_destroy(&wid_editor_clear_popup);
}

void wid_editor_clear (void)
{
    if (wid_editor_any_popup()) {
        return;
    }

    wid_editor_clear_popup =
        wid_popup("%%fg=red$Clear level?",
                  0                 /* title */,
                  0.5f, 0.5f,       /* x,y postition in percent */
                  large_font,       /* title font */
                  large_font,       /* body font */
                  large_font,       /* button font */
                  2,                /* number buttons */
                  "Yes", wid_editor_clear_callback_yes,
                  "No", wid_editor_clear_callback_no);
}

void wid_editor_reset_buttons (void)
{
    wid_editor_mode_eraser = false;
    wid_editor_mode_draw = false;
    wid_editor_mode_line = false;
    wid_editor_mode_fill = false;

    wid_set_color(wid_editor_wid_eraser, WID_COLOR_BG, BLACK);
    wid_set_color(wid_editor_wid_draw, WID_COLOR_BG, BLACK);
    wid_set_color(wid_editor_wid_line, WID_COLOR_BG, BLACK);
    wid_set_color(wid_editor_wid_fill, WID_COLOR_BG, BLACK);
}

void wid_editor_draw (void)
{
    wid_editor_reset_buttons();
    wid_editor_mode_draw = true;
    wid_set_mode(wid_editor_wid_draw, WID_MODE_NORMAL);
    wid_set_color(wid_editor_wid_draw, WID_COLOR_BG, STEELBLUE);
}

void wid_editor_line (void)
{
    wid_editor_reset_buttons();
    wid_editor_mode_line = true;
    wid_set_mode(wid_editor_wid_line, WID_MODE_NORMAL);
    wid_set_color(wid_editor_wid_line, WID_COLOR_BG, STEELBLUE);
}

void wid_editor_erase (void)
{
    wid_editor_reset_buttons();
    wid_editor_mode_eraser = true;
    wid_set_mode(wid_editor_wid_eraser, WID_MODE_NORMAL);
    wid_set_color(wid_editor_wid_eraser, WID_COLOR_BG, STEELBLUE);
}

void wid_editor_fill (void)
{
    wid_editor_reset_buttons();
    wid_editor_mode_fill = true;
    wid_set_mode(wid_editor_wid_fill, WID_MODE_NORMAL);
    wid_set_color(wid_editor_wid_fill, WID_COLOR_BG, STEELBLUE);
}
