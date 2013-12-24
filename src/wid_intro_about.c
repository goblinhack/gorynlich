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
#include "wid_intro_about.h"
#include "wid_popup.h"
#include "wid_intro_about.h"
#include "sdl.h"
#include "wid_hiscore.h"

static widp wid_intro_about;
static boolean wid_intro_about_init_done;
static void wid_intro_about_create(void);
static void wid_intro_about_destroy(void);

boolean wid_intro_about_init (void)
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
}

void wid_intro_about_visible (void)
{
    wid_intro_about_create();
}

static boolean wid_intro_about_key_event (widp w, const SDL_KEYSYM *key)
{
    switch (key->sym) {
        case 'q':
        case SDLK_ESCAPE:
            wid_intro_about_hide();
            return (true);

        default:
            break;
    }

    return (false);
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

    wid_intro_about = wid_popup(
          "\n"
          "\n"
          "Version 0.0.0\n"
          "\n"
          "\n"
          "%%fg=green$Copyright Neil McGill\n"
          "%%fg=green$goblinhack@gmail.com\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "For my little minions.\n"
          "\n"
          "Game Music: Charlotte McGill\n"
          "\n"
          "Intro Music: www.nosoapradio.us\n"
          "\n"
          "Sound effects: Psychentist\n"
          "\n"
          "Server console: https://github.com/antirez/linenoise\n"
          "\n"
          "\n"
          "\n"
          "This game is free software; you can redistribute it and/or "
          "modify it under the terms of the GNU Library General Public "
          "License as published by the Free Software Foundation; either "
          "version 2 of the License, or (at your option) any later version."
          "\n"
          "This game is distributed in the hope that it will be fun, "
          "but WITHOUT ANY WARRANTY; without even the implied warranty of "
          "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU "
          "Library General Public License for more details."
          "\n"
          "You should have received a copy of the GNU Library General Public "
          "License along with this game; if not, write to the Free "
          "Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA "
          "\n"
          ,
          "%%fg=red$Licenses and stuffs. Srsly.",   /* title */
          0.5, 0.5,                 /* x,y postition in percent */
          med_font,               /* title font */
          vsmall_font,               /* body font */
          med_font,                 /* button font */
          1,                        /* number buttons */
          "Close", wid_intro_about_callback_close);

    wid_set_on_key_down(wid_intro_about, wid_intro_about_key_event);
}

static void wid_intro_about_destroy (void)
{
    wid_destroy(&wid_intro_about);
}
