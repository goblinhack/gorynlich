/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "wid_popup.h"
#include "wid_map_dialog.h"
#include "wid_intro.h"
#include "wid_menu.h"
#include "wid_map.h"
#include "color.h"

static widp wid_map_dialog;
static uint8_t wid_map_dialog_init_done;
static void wid_map_dialog_create(const char *title);
static void wid_map_dialog_destroy(void);

uint8_t wid_map_dialog_init (void)
{
    if (!wid_map_dialog_init_done) {
    }

    wid_map_dialog_init_done = true;

    return (true);
}

void wid_map_dialog_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (wid_map_dialog_init_done) {
        wid_map_dialog_init_done = false;

        wid_map_dialog_destroy();
    }
}

void wid_map_dialog_hide (void)
{
    wid_map_dialog_destroy();
}

void wid_map_dialog_visible (const char *title)
{
    wid_map_dialog_create(title);
}

static void wid_map_dialog_callback_play (widp wid)
{
    wid_map_dialog_hide();
    wid_map_cell_play();
}

static void wid_map_dialog_callback_load (widp wid)
{
    wid_map_dialog_hide();
    wid_map_cell_load();
}

static void wid_map_dialog_callback_close (widp wid)
{
    wid_map_dialog_hide();
    wid_map_visible();
}

static void wid_map_dialog_create (const char *title)
{
    if (wid_map_dialog) {
        return;
    }

    wid_map_dialog = 
        wid_menu(0,
                vvlarge_font,
                vlarge_font,
                0.55, /* x */
                0.5, /* y */
                1, /* columns */
                2, /* focus */
                4, /* items */

                (int) '0', 
                title ? title : "Unnamed level",
                (void*) 0,

                (int) '0', 
                "Test level",
                (void*) wid_map_dialog_callback_play,

                (int) '0', 
                "Edit level",
                (void*) wid_map_dialog_callback_load,

                (int) 'b', 
                "Back", 
                wid_map_dialog_callback_close);

    wid_set_on_display_top_level(wid_map_dialog, wid_map_preview_thumbnail);
}

static void wid_map_dialog_destroy (void)
{
    wid_destroy(&wid_map_dialog);
}
