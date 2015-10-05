/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
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
                vlarge_font,
                large_font,
                0, // on_update
                0.55, /* x */
                0.5, /* y */
                2, /* columns */
                8, /* focus */
                9, /* items */

                /*
                 * Column widths
                 */
                (double) 0.25, (double) 0.75,

                (int) '0', 
                "%%fmt=left$%%fg=gray$Code monkey #1",
                "%%fg=white$Neil McGill", 
                (void*) 0,

                (int) '0', 
                "%%fmt=left$%%fg=gray$Code monkey #2",
                "%%fg=red$Alexandra Franks",
                (void*) 0,

                (int) '0', 
                "%%fmt=left$%%fg=gray$Music and Art",
                "%%fg=purple$Charlotte McGill",
                (void*) 0,

                (int) '0', 
                "%%fmt=left$%%fg=gray$Art",
                "%%fg=purple$neoguid@hotmail.com",
                (void*) 0,

                (int) '0', 
                "%%fmt=left$%%fg=gray$Music",
                "%%fg=cyan$Deceased Senior Technician",
                (void*) 0,

                (int) '0',
                "%%fmt=left$%%fg=gray$Music",
                "%%fg=cyan$  (nosoapradio.us)",
                (void*) 0,

                (int) '0', 
                "%%fmt=left$%%fg=gray$Level design",
                "%%fg=cyan$Ben McGill",
                (void*) 0,

                (int) '0', 
                "%%fmt=left$%%fg=gray$Support",
                "%%fg=green$goblinhack@gmail.com",
                (void*) 0,

                (int) 'b', 
                "%%fmt=left$Back", 
                (char*) 0,
                wid_intro_about_callback_close);
}

static void wid_intro_about_destroy (void)
{
    wid_destroy(&wid_intro_about);
}
