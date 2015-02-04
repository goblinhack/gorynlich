/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "wid.h"
#include "main.h"
#include "thing.h"
#include "level.h"
#include "color.h"
#include "wid_game_map_client.h"
#include "math_util.h"

static void thing_client_effect_hit_miss(thingp t);
static void thing_client_effect_hit_success(thingp t);
static void thing_client_effect_hit_crit(thingp t);
static void thing_client_effect_power_up(thingp t);

/*
 * Set up an effect on the thing to be sent to the client.
 */
void thing_server_effect (thingp t, int effect)
{
    if (!t->on_server) {
        DIE("Trying to do an effect on a client thing");
    }

    if (effect > t->effect) {
        t->effect = effect;

        thing_update(t);
    }
}

/*
 * Invoke an effect on the client that we received from the server.
 */
void thing_client_effect (thingp t, int effect)
{
    if (t->on_server) {
        DIE("Trying to do an effect on a server thing on client");
    }

    switch (effect) {
    case THING_STATE_EFFECT_IS_HIT_MISS:
        thing_client_effect_hit_miss(t);
        break;
    case THING_STATE_EFFECT_IS_HIT_SUCCESS:
        thing_client_effect_hit_success(t);
        break;
    case THING_STATE_EFFECT_IS_POWER_UP:
        thing_client_effect_power_up(t);
        break;
    case THING_STATE_EFFECT_IS_HIT_CRIT:
        thing_client_effect_hit_crit(t);
        break;
    }
}

static void thing_client_effect_hit_miss (thingp t)
{
    verify(t);

    widp w = t->wid;
    if (w) {
        wid_set_mode(w, WID_MODE_ACTIVE);
        level_place_hit_miss(client_level,
                             0, // owner
                             t->x, t->y);
    }
}

static void thing_client_effect_hit_success (thingp t)
{
    verify(t);

    widp w = t->wid;
    if (w) {
        wid_set_mode(w, WID_MODE_ACTIVE);
        wid_set_color(w, WID_COLOR_BLIT, RED);

        /*
         * Shake the player.
         */
        wid_shake_to_pct_in(w, 0.1, 0.9, 100, 5);

        if (thing_is_warm_blooded(t)) {
            level_place_blood(client_level,
                              0, // owner
                              t->x, t->y);
        } else {
            level_place_hit_success(client_level,
                                    0, // owner
                                    t->x, t->y);
        }
    }
}

static void thing_client_effect_hit_crit (thingp t)
{
    verify(t);

    widp w = t->wid;
    if (w) {
        wid_set_mode(w, WID_MODE_ACTIVE);
        wid_set_color(w, WID_COLOR_BLIT, RED);

        /*
         * Shake the screen.
         */
        if (thing_is_player(t)) {
            wid_shake_to_pct_in(wid_game_map_client_grid_container, 0.03, 0.9, 100, 5);
        }

        /*
         * Shake the player.
         */
        wid_shake_to_pct_in(w, 0.3, 0.9, 100, 5);

        if (thing_is_warm_blooded(t)) {
            level_place_blood_crit(client_level,
                                   0, // owner
                                   t->x, t->y);
        } else {
            level_place_hit_success(client_level,
                                    0, // owner
                                    t->x, t->y);
        }
    }
}

static void thing_client_effect_power_up (thingp t)
{
    int count = 20;
    int i = count;
    while (i--) {
        widp power_up =
            wid_new_square_window("wid player_stats container");

        widp w = t->wid;
        double px, py;

        wid_get_pct(w, &px, &py);

        fpoint tl = {0.0, 0.0};
        fpoint br = {0.1, 0.1};

        br.y = 
            ((double)global_config.video_gl_width /
                (double)global_config.video_gl_height) / 10.0;

        wid_set_tl_br_pct(power_up, tl, br);
        wid_set_tilename(power_up, "sparkle");

        px += gaussrand(0.0, 0.03);
        py += gaussrand(0.0, 0.04);

        wid_move_to_pct_centered(power_up, px, py);

        wid_raise(power_up);
        wid_set_mode(power_up, WID_MODE_NORMAL);
        wid_set_color(power_up, WID_COLOR_TEXT, WHITE);
        wid_set_color(power_up, WID_COLOR_BG, WHITE);
        wid_set_color(power_up, WID_COLOR_TL, RED);
        wid_set_color(power_up, WID_COLOR_BR, WHITE);
        wid_set_no_shape(power_up);
        wid_set_do_not_lower(power_up, 1);

        uint32_t delay = gaussrand(1000, 1000);
        if (delay < 100) {
            delay = 100;
        }

        wid_fade_out(power_up, delay);
        wid_destroy_in(power_up, delay);
        wid_scaling_blit_to_pct_in(power_up, 0.5, 0.0, delay, 0);
        wid_set_debug(power_up, 1);
    }
}
