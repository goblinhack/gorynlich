/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "thing.h"
#include "thing_timer.h"
#include "wid.h"
#include "wid_game_map_client.h"
#include "wid_game_map_server.h"
#include "map.h"
#include "level.h"
#include "time_util.h"
#include "string_util.h"
#include "wid_textbox.h"
#include "sound.h"
#include "socket_util.h"
#include "client.h"
#include "timer.h"
#include "tile.h"
#include "math_util.h"
#include "wid_hiscore.h"
#include "wid_player_stats.h"
#include "wid_player_info.h"
#include "wid_player_inventory.h"
#include "wid_player_action.h"
#include "string_ext.h"

/*
 * This is movement on the client of the player initiated by the player.
 */
void thing_client_move (thingp t,
                        double x,
                        double y,
                        const uint8_t up,
                        const uint8_t down,
                        const uint8_t left,
                        const uint8_t right,
                        const uint8_t fire)
{
    if (thing_is_dead_or_dying(t)) {
        return;
    }

    widp grid = wid_game_map_client_grid_container;

    if (t->wid) {
        if (thing_hit_solid_obstacle(grid, t, x, y)) {
            if (!thing_hit_solid_obstacle(grid, t, x, t->y)) {
                y = t->y;
            } else if (!thing_hit_solid_obstacle(grid, t, t->x, y)) {
                x = t->x;
            } else {
                x = t->x;
                y = t->y;
            }
        }
    }

    thing_move_set_dir(t, &x, &y, up, down, left, right);

    /*
     * Move the weapon too.
     */
    thingp weapon_carry_anim = thing_weapon_carry_anim(t);
    if (weapon_carry_anim) {
        thing_move_set_dir(weapon_carry_anim, &x, &y, up, down, left, right);
    }

    thingp weapon_swing_anim = thing_weapon_swing_anim(t);
    if (weapon_swing_anim) {
        thing_move_set_dir(weapon_swing_anim, &x, &y, up, down, left, right);
    }

    thingp shield_anim = thing_shield_anim(t);
    if (shield_anim) {
        thing_move_set_dir(shield_anim, &x, &y, up, down, left, right);
    }

    /*
     * If no widget yet then this can be a dummy move during thing creation
     * just to set the weapon anim correctly.
     */
    if (!t->wid) {
        return;
    }

    /*
     * Oddly doing smooth moving makes it more jumpy when scrolling.
     *
     * Don't send an update if just firing as it looks like we're moving
     * and the player will hop up and down.
     */
    if (up || down || left || right) {
        thing_client_wid_update(t, x, y, false, false /* is new */);
    }

    socket_tx_player_move(client_joined_server, t, up, down, left, right, 
                          fire);
}

static void thing_client_wid_move (thingp t, 
                                   double x, 
                                   double y, 
                                   uint8_t smooth)
{
    double dist = DISTANCE(t->x, t->y, x, y);

    if (smooth) {
        if (dist == 0.0) {
            return;
        }
    }

    thing_move(t, x, y);

    x *= client_tile_width;
    y *= client_tile_height;

    x += client_tile_width / 2;
    y += client_tile_height / 2;

    fpoint tl = { x, y };
    fpoint br = { x, y };

    double base_tile_width =
            ((1.0f / ((double)TILES_SCREEN_WIDTH)) *
                (double)global_config.video_gl_width);

    double base_tile_height =
            ((1.0f / ((double)TILES_SCREEN_HEIGHT)) *
                (double)global_config.video_gl_height);

    tilep tile = wid_get_tile(t->wid);
    double tw = tile_get_width(tile);
    double th = tile_get_height(tile);
    double scale_x = tw / TILE_WIDTH; 
    double scale_y = th / TILE_HEIGHT; 

    if (scale_x > 1) {
        base_tile_width *= scale_x;
        base_tile_height *= scale_y;
    }

    br.x += base_tile_width / 2.0;
    br.y += base_tile_height / 2.0;
    tl.x -= base_tile_width / 2.0;
    tl.y -= base_tile_height / 2.0;

    /*
     * Now the tile itself has a shadow that is 1/4 of the pixels.
     * The center is 24x24 and with shadow it is 32x32. We need to
     * stretch the tile so it overlaps so the inner 24x24 if seamless.
     */
    double tile_width = ((br.x - tl.x) / 
                         (double)TILE_PIX_WIDTH) * 
                            (double)TILE_PIX_WITH_SHADOW_WIDTH;

    double tile_height = ((br.y - tl.y) / 
                         (double)TILE_PIX_HEIGHT) * 
                            (double)TILE_PIX_WITH_SHADOW_HEIGHT;

    if (scale_x == 1) {
        tl.y -= tile_height / 4.0;
        br.x += tile_width / 4.0;
    }

    tl.y -= tile_height / 8.0;

    /*
     * Off the map? Perhaps between levels.
     */
    if (!t->wid) {
        return;
    }

    /*
     * Make the weapon follow the thing.
     */
    if (smooth) {
        double time_step = dist;
        double speed = thing_speed(t);

        /*
         * If a weapon is being carried, it has no speed. Move at the same 
         * speed as the carrier.
         */
        if (!speed) {
            thingp owner = thing_owner(t);
            if (owner) {
                speed = thing_speed(owner);
            }
        }

        if (!speed) {
            speed = 1;
        }

        double ms = (5000.0 / speed) / (1.0 / time_step);

        wid_move_to_abs_in(t->wid, tl.x, tl.y, ms);

    } else {
        wid_set_tl_br(t->wid, tl, br);
    }

    /*
     * Make the player bounce about as they walk
     */
    if (thing_is_player(t)) {
        if (!t->wid->bouncing) {
            wid_bounce_to_pct_in(t->wid, 0.1, 0.9, 200, 0);
        }

        /*
         * And their little weapon too.
         */
        widp weapon_wid = thing_get_weapon_carry_anim_wid(t);
        if (weapon_wid) {
            wid_bounce_to_pct_in(weapon_wid, 0.15, 0.9, 150, 0);
        }
    }
}

void thing_client_wid_update (thingp t, double x, double y, 
                              uint8_t smooth,
                              uint8_t is_new)
{
    thing_client_wid_move(t, x, y, smooth);

    /*
     * Update the weapon being carried.
     */
    thingp weapon_carry_anim = thing_weapon_carry_anim(t);
    if (weapon_carry_anim) {
        weapon_carry_anim->dir = t->dir;
        thing_client_wid_move(weapon_carry_anim, x, y, smooth);
    }

    /*
     * Update the weapon being swung.
     */
    thingp weapon_swing_anim = thing_weapon_swing_anim(t);
    if (weapon_swing_anim) {
        double dx = 0;
        double dy = 0;

        weapon_swing_anim->dir = t->dir;
        thing_weapon_swing_offset(t, &dx, &dy);
        thing_client_wid_move(weapon_swing_anim, x + dx, y + dy, smooth);
    }

    /*
     * Update the weapon being carried.
     */
    thingp shield_anim = thing_shield_anim(t);
    if (shield_anim) {
        shield_anim->dir = t->dir;
        thing_client_wid_move(shield_anim, x, y + 0.5, smooth);
    }
}
