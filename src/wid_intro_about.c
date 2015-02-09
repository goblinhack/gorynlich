/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "wid_popup.h"
#include "wid_intro_about.h"
#include "wid_intro.h"
#include "wid_menu.h"
#include "color.h"

static widp wid_intro_about;
static uint8_t wid_intro_about_init_done;
static void wid_intro_about_create(void);
static void wid_intro_about_destroy(void);

uint8_t wid_intro_about_init (void)
{
    if (!wid_intro_about_init_done) {
    }

    wid_intro_about_init_done = true;

    return (true);
}

void wid_intro_about_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (wid_intro_about_init_done) {
        wid_intro_about_init_done = false;

        wid_intro_about_destroy();
    }
}

void wid_intro_about_hide (void)
{
    wid_intro_about_destroy();

    wid_intro_visible();
}

void wid_intro_about_visible (void)
{
    wid_intro_about_create();
}

static void wid_intro_about_callback_close (widp wid)
{
    wid_intro_about_hide();
}

static void wid_intro_about_create (void)
{
    if (wid_intro_about) {
        return;
    }

    wid_intro_about = 
        wid_menu(0,
                vvlarge_font,
                vlarge_font,
                0.95, /* padding between buttons */
                6, /* focus */
                7, /* items */
                "%%fmt=left$"
                "%%fg=gray$Code monkey #1:\t%%fg=white$Neil McGill", 
                (void*) 0,
                "%%fmt=left$"
                "%%fg=gray$Code monkey #2:\t%%fg=red$Richard Franks",
                (void*) 0,
                "%%fmt=left$"
                "%%fg=gray$Music:\t%%fg=purple$Charlotte McGill",
                (void*) 0,
                "%%fmt=left$"
                "%%fg=gray$Music:\t%%fg=cyan$Deceased Senior Technician",
                (void*) 0,
                "%%fmt=left$"
                "%%fg=gray$Music:\t%%fg=cyan$  (nosoapradio.us)",
                (void*) 0,
                "%%fmt=left$"
                "%%fg=gray$Support: %%fg=green$goblinhack@gmail.com",
                (void*) 0,
                "%%fmt=left$"
                "Back",  wid_intro_about_callback_close);

    wid_move_to_pct_centered(wid_intro_about, 0.6, 0.5);
}

static void wid_intro_about_destroy (void)
{
    wid_destroy(&wid_intro_about);
}
