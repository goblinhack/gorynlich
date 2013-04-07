/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <SDL.h>
#include "glapi.h"

#include "main.h"
#include "gl.h"
#include "color.h"
#include "tex.h"
#include "wid.h"
#include "ttf.h"
#include "time.h"
#include "map.h"

widp wid_map;

/*
 * wid_map_key_event
 */
static boolean wid_map_key_event (widp w, const SDL_keysym *key)
{
    static uint32_t accel = 1;
    static uint32_t last;

    if (time_have_x_tenths_passed_since(1, last)) {
        accel = 1;
    }

    last = time_get_time_cached();

#if SDL_MAJOR_VERSION == 1 && SDL_MINOR_VERSION == 2 /* { */
    uint8_t *state = SDL_GetKeyState(0);

    boolean right = state[SDLK_RIGHT];
    boolean left  = state[SDLK_LEFT];
    boolean up    = state[SDLK_UP];
    boolean down  = state[SDLK_DOWN];
#else /* } { */
    uint8_t *state = SDL_GetKeyboardState(0);

    boolean right = state[SDL_SCANCODE_RIGHT];
    boolean left  = state[SDL_SCANCODE_LEFT];
    boolean up    = state[SDL_SCANCODE_UP];
    boolean down  = state[SDL_SCANCODE_DOWN];
#endif /* } */

    if (!left && !right && !up && !down) {
        accel = 1;
    } else {
        if (left) {
            map_move_delta_pixels(-accel, 0);
        } else if (right) {
            map_move_delta_pixels(accel, 0);
        }

        if (up) {
            map_move_delta_pixels(0, -accel);
        } else if (down) {
            map_move_delta_pixels(0, accel);
        }

        accel++;
    }

    return (false);
}

/*
 * map_display_wid_init
 *
 * Event widget for all in game events.
 */
void map_display_wid_init (void)
{
    if (wid_map) {
        return;
    }

    wid_map = wid_new_window("wid map");

    wid_set_no_shape(wid_map);

    fpoint tl = {0.0f, 0.0f};
    fpoint br = {1.0f, 1.0f};
    wid_set_tl_br_pct(wid_map, tl, br);
    wid_set_on_key_down(wid_map, wid_map_key_event);

    color col = BLACK;
    col.a = 0;
    glcolor(col);

    wid_set_mode(wid_map, WID_MODE_NORMAL);
    wid_set_color(wid_map, WID_COLOR_TL, col);
    wid_set_color(wid_map, WID_COLOR_BR, col);
    wid_set_color(wid_map, WID_COLOR_BG, col);

    wid_set_on_key_down(wid_map, wid_map_key_event);

    wid_update(wid_map);
}

/*
 * map_display_wid_fini
 */
void map_display_wid_fini (void)
{
    if (wid_map) {
        wid_destroy(&wid_map);
    }
}
