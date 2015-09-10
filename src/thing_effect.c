/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */


#include "main.h"
#include "thing.h"
#include "color.h"
#include "wid_game_map_client.h"
#include "math_util.h"

static void thing_client_effect_hit_miss(thingp t);
static void thing_client_effect_hit_success(thingp t);
static void thing_client_effect_flames(thingp t);
static void thing_client_effect_hit_crit(thingp t);
static void thing_client_effect_power_up(thingp t);

/*
 * Set up an effect on the thing to be sent to the client.
 */
void thing_server_effect (thingp t, int effect)
{
    verify(t);

    if (thing_is_shield(t)) {
        /*
         * Let the shield show the hit
         */
    } else if (thing_is_projectile(t)) {
        /*
         * Do we need these to flash red?
         */
        return;
    } else {
        /*
         * Let the owner show the hit?
         */
        thingp owner = thing_owner(t);
        if (owner) {
            t = owner;
        }
    }

    if (!t->on_server) {
        ERR("Trying to do an effect on a client thing");
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
        ERR("Trying to do an effect on a server thing on client");
    }

    switch (effect) {
    case THING_STATE_EFFECT_IS_HIT_MISS:
        thing_client_effect_hit_miss(t);
        break;
    case THING_STATE_EFFECT_IS_HIT_SUCCESS:
        thing_client_effect_hit_success(t);
        break;
    case THING_STATE_EFFECT_IS_FLAME:
        thing_client_effect_flames(t);
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
         * Shake the screen.
         */
        if (thing_is_player(t)) {
            sdl_joy_rumble(1.0, 2000);
        }

        /*
         * Shake the screen.
         */
        if (thing_is_player(t)) {
            wid_shake_to_pct_in(wid_game_map_client_grid_container, 
                                0.01, 0.9, 100, 5);

            sdl_joy_rumble(1.0, 1000);
        } else if (thing_is_ethereal(t)) {
            /*
             * No shaking jesus
             */
        } else if (thing_is_monst(t)) {
            wid_shake_to_pct_in(w, 0.1, 0.9, 100, 5);
        } else {
            wid_shake_to_pct_in(w, 0.2, 0.9, 100, 5);
        }

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

static void thing_client_effect_flames (thingp t)
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
            sdl_joy_rumble(1.0, 2000);
        }

        level_place_flames(client_level,
                          0, // owner
                          t->x, t->y);
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
            wid_shake_to_pct_in(wid_game_map_client_grid_container, 
                                0.01, 0.9, 100, 5);

            sdl_joy_rumble(1.0, 1000);
        } else if (thing_is_ethereal(t)) {
            /*
             * No shaking jesus
             */
        } else if (thing_is_monst(t)) {
            wid_shake_to_pct_in(w, 0.1, 0.9, 100, 5);
        } else {
            wid_shake_to_pct_in(w, 0.2, 0.9, 100, 5);
        }

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
    double rad;

    for (rad = 0.0; rad < RAD_360; rad += RAD_360 / 360.0) {
        widp w = t->wid;
        double px, py;

        wid_get_pct(w, &px, &py);

        fpoint tl = {0.0, 0.0};
        fpoint br = {0.1, 0.1};

        br.y = 
            ((double)global_config.video_gl_width /
             (double)global_config.video_gl_height) / 10.0;

        widp power_up = wid_new_square_window("sparkle");
        wid_set_tl_br_pct(power_up, tl, br);
        wid_set_tilename(power_up, "powerup1.1");

        int delay = gaussrand(200, 500);
        if (delay < 100) {
            delay = 100;
        }

        double width = 0.05;
        px += fcos(rad) * width;
        py += (fsin(rad) * width) * 
                        ((double)global_config.video_gl_width / 
                         (double)global_config.video_gl_height);

        {
            double width = wid_get_width(w) / 
                            (double)global_config.video_gl_width;
            px -= width / 8.0;

            double height = wid_get_height(w) / 
                            (double)global_config.video_gl_height;
            py += height / 4.0;
        }

        wid_move_to_pct_centered(power_up, px, py);
        px += gaussrand(0.0, width/5.0);
        py += gaussrand(0.0, width/5.0);
        wid_move_to_pct_centered_in(power_up, px, py, delay);

        wid_set_no_shape(power_up);
        wid_set_mode(power_up, WID_MODE_NORMAL);

        wid_set_color(power_up, WID_COLOR_BLIT, WHITE);

        wid_fade_out(power_up, delay);
        wid_destroy_in(power_up, delay);
        wid_scaling_blit_to_pct_in(power_up, 0.1, 1.0, delay * 2, 0);
    }
}
