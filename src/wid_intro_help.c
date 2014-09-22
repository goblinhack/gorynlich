/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "wid_popup.h"
#include "wid_intro_help.h"
#include "color.h"

static widp wid_intro_help;
static uint8_t wid_intro_help_init_done;
static void wid_intro_help_create(void);
static void wid_intro_help_destroy(void);

uint8_t wid_intro_help_init (void)
{
    if (!wid_intro_help_init_done) {
    }

    wid_intro_help_init_done = true;

    return (true);
}

void wid_intro_help_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (wid_intro_help_init_done) {
        wid_intro_help_init_done = false;

        wid_intro_help_destroy();
    }
}

void wid_intro_help_hide (void)
{
    wid_intro_help_destroy();
}

void wid_intro_help_visible (void)
{
    wid_intro_help_create();
}

static uint8_t wid_intro_help_key_event (widp w, const SDL_KEYSYM *key)
{
    switch (key->sym) {
        case 'q':
        case SDLK_ESCAPE:
            wid_intro_help_hide();
            return (true);

        default:
            break;
    }

    return (false);
}

static void wid_intro_help_callback_close (widp wid)
{
    wid_intro_help_hide();
}

static void wid_intro_help_create (void)
{
    if (wid_intro_help) {
        return;
    }

    wid_intro_help =
        wid_popup("%%fmt=left$%%fg=red$space\t%%fg=green$Play game\n"
                  "%%fmt=left$%%fg=red$s\t\t%%fg=green$Single player\n"
                  "%%fmt=left$%%fg=red$m\t\t%%fg=green$Multi player\n"
                  "%%fmt=left$%%fg=red$e\t\t%%fg=green$Level editor\n"
                  "%%fmt=left$%%fg=red$c\t\t%%fg=green$config cettings\n"
                  "%%fmt=left$%%fg=red$i\t\t%%fg=green$Hi-scores\n"
                  "%%fmt=left$%%fg=red$q\t\t%%fg=green$Quit\n"
                  ,
                  "Keys",         /* title */
                  0.5f, 0.5f,
                  med_font,       /* title font */
                  med_font,       /* body font */
                  med_font,       /* button font */
                  1,              /* number args */
                  "Close", wid_intro_help_callback_close);

    wid_set_on_key_down(wid_intro_help, wid_intro_help_key_event);

    wid_set_tex(wid_intro_help, 0, "gothic_wide");
    color c = WHITE;
    wid_set_color(wid_intro_help, WID_COLOR_BG, c);
    wid_set_color(wid_intro_help, WID_COLOR_TL, c);
    wid_set_color(wid_intro_help, WID_COLOR_BR, c);
    wid_set_square(wid_intro_help);

}

static void wid_intro_help_destroy (void)
{
    wid_destroy(&wid_intro_help);
}
