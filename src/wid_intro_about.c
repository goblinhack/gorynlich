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
#include "wid_intro_extra.h"
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

    wid_intro_extra_visible();
}

void wid_intro_about_visible (void)
{
    wid_intro_about_create();
}

static uint8_t wid_intro_about_key_event (widp w, const SDL_KEYSYM *key)
{
    switch (key->sym) {
        case 'b':
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
          "\n"
          "\n"
          "\n"
          "Version 0.0.1\n"
          "\n"
          "\n"
          "%%fg=green$Copyright Neil McGill\n"
          "%%fg=green$goblinhack@gmail.com\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "Game Music by the talented and amazing. Charlotte McGill\n"
          "\n"
          "Additional Music by the frankly awesome, Deceased Senior Technician, DST. AKA nosoapradio.us\n"
          "\n"
          "Coding by Me\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "This game is free software; you can redistribute it and/or "
          "modify it under the terms of the GNU Library General Public "
          "License as published by the Free Software Foundation; either "
          "version 2 of the License, or (at your option) any later version."
          "\n"
          "\n"
          "This game is distributed in the hope that it will be fun, "
          "but WITHOUT ANY WARRANTY; without even the implied warranty of "
          "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU "
          "Library General Public License for more details."
          "\n"
          "\n"
          "You should have received a copy of the GNU Library General Public "
          "License along with this game; if not, write to the Free "
          "Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA "
          "\n"
          ,
          "%%fg=gold$Credits",      /* title */
          0.5, 0.5,                 /* x,y postition in percent */
          large_font,               /* title font */
          large_font,              /* body font */
          large_font,              /* button font */
          1,                        /* number buttons */
          "%%tile=button_b$Back", wid_intro_about_callback_close);

    wid_set_on_key_down(wid_intro_about, wid_intro_about_key_event);

    wid_set_tex(wid_intro_about, 0, "window_gothic");
    wid_set_square(wid_intro_about);

    wid_move_to_pct_centered(wid_intro_about, 0.5, 0.5 - 1.0);
    wid_move_to_pct_centered_in(wid_intro_about, 0.5, 0.5, 200);
}

static void wid_intro_about_destroy (void)
{
    wid_destroy(&wid_intro_about);
}
