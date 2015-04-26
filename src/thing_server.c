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

uint8_t thing_server_move (thingp t,
                           double x,
                           double y,
                           const uint8_t up,
                           const uint8_t down,
                           const uint8_t left,
                           const uint8_t right,
                           const uint8_t fire)
{
    if (thing_is_dead_or_dying(t)) {
        return (false);
    }

    widp grid = wid_game_map_server_grid_container;

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
     * A thing can move diagonally and may not have a collision at the
     * destination, but it might half way through the move; so check for that 
     * and this prevents ghosts taking shortcuts around corners.
     */
    if (thing_hit_solid_obstacle(grid, t, x, y) ||
        thing_hit_solid_obstacle(grid, t, 
                                 (t->x + x) / 2.0, 
                                 (t->y + y) / 2.0)) {
        if ((x != t->x) &&
            !thing_hit_solid_obstacle(grid, t, x, t->y)) {
            y = t->y;
        } else if ((y != t->y) &&
                   !thing_hit_solid_obstacle(grid, t, t->x, y)) {
            x = t->x;
        } else {
            if (thing_is_player(t)) {
                THING_LOG(t, "client collision, ignore move");
                THING_LOG(t, "  server %f %f", t->x, t->y);
                THING_LOG(t, "  client %f %f", x, y);
            }

            return (false);
        }
    }

    if (thing_is_player(t)) {
        if ((fabs(x - t->x) > THING_MAX_SERVER_DISCREPANCY) ||
            (fabs(y - t->y) > THING_MAX_SERVER_DISCREPANCY)) {
            /*
             * Client is cheating?
             */
            THING_LOG(t, "client moved too much, ignore move");
            THING_LOG(t, "  server %f %f", t->x, t->y);
            THING_LOG(t, "  client %f %f", x, y);

            thing_update(t);
            t->needs_tx_refresh_xy_and_template_id = 1;

            return (false);
        }
    }

    thing_move_set_dir(t, &x, &y, up, down, left, right);

    /*
     * Move the weapon too.
     */
    if (weapon_carry_anim) {
        thing_move_set_dir(weapon_carry_anim, &x, &y, up, down, left, right);
    }

    if (weapon_swing_anim) {
        thing_move_set_dir(weapon_swing_anim, &x, &y, up, down, left, right);
    }

    if (shield_anim) {
        thing_move_set_dir(shield_anim, &x, &y, up, down, left, right);
    }

    if (fire) {
        thing_server_fire(t, up, down, left, right);
    }

    thing_server_wid_update(t, x, y, false /* is_new */);
    thing_update(t);

    thing_handle_collisions(wid_game_map_server_grid_container, t);

    if (thing_is_projectile(t)) {
        socket_server_tx_map_update(0, server_active_things,
                                    "rx client join active things");
    }

    return (true);
}

void thing_server_action (thingp t,
                          uint8_t action,
                          uint32_t action_bar_index)
{
    if (action_bar_index >= THING_ACTION_BAR_MAX) {
        ERR("invalid action bar slot %u", action_bar_index);
        return;
    }

    itemp item = &t->stats.action_bar[action_bar_index];
    if (!item->id) {
        MSG_SERVER_SHOUT_AT_PLAYER(WARNING, t, "No item in that slot to use");
        return;
    }

    tpp tp = id_to_tp(item->id);
    if (!tp) {
        ERR("Unkown item use request, id %u", item->id);
        return;
    }

    if (!t->player) {
        ERR("no player to handle action");
        return;
    }

    gsocketp s = t->player->socket;
    if (!s) {
        ERR("no player socket to handle action");
        return;
    }

    if (thing_is_dead_or_dying(t)) {
        /*
         * Future use when dead?
         */
        return;
    }

    switch (action) {
    case PLAYER_ACTION_USE: {
        if (tp_is_weapon(tp)) {
            thing_wield(t, tp);

            thing_stats_set_action_bar_index(t, action_bar_index);
            return;
        }

        if (item->id == THING_POTION_FIRE) {
            level_place_fireball(server_level, t, t->x, t->y);
            break;
        } else if (item->id == THING_POTION_MONSTICIDE) {
            level_place_poison(server_level, t, t->x, t->y);
            break;
        } else if (item->id == THING_POTION_LIFE) {
            thing_server_effect(t, THING_STATE_EFFECT_IS_POWER_UP);
            break;
        } else if (item->id == THING_POTION_SHIELD) {
            thing_wield_shield(t, tp);
            break;
        } else if (item->id == THING_POTION_CLOUDKILL) {
            level_place_cloudkill(server_level, t, t->x, t->y);
            break;
        } else if (item->id == THING_BOMB) {
            if (level_place_bomb(server_level, t, t->x + 1, t->y)) {
                break;
            }
        } else if (item->id == THING_TORCH) {
            if (thing_place_behind(t, tp, item)) {
                break;
            }
        }

        const char *message = tp_message_on_use(tp);
        if (message) {
            MSG_SERVER_SHOUT_AT_PLAYER(INFO, t, "%s", message);
            break;
        }

        /*
         * Failed to use.
         */
        MSG_SERVER_SHOUT_AT_PLAYER(WARNING, t, "Failed to use the %s", 
                                   tp_short_name(tp));
        return;
    }

    case PLAYER_ACTION_STOP_USE: {
        if (tp_is_weapon(tp)) {
            thing_unwield(t);
            return;
        }

        return;
    }

    case PLAYER_ACTION_DROP:

        if (!thing_place(t, tp, item)) {
            /*
             * Urk!
             */
            MSG_SERVER_SHOUT_AT_PLAYER(INFO, t, "Drop failed");
            return;
        }
        break;

    default:
        ERR("Unkown player action %u on action bar item %u", 
            action, 
            action_bar_index);
        return;
    }

    switch (action) {
    case PLAYER_ACTION_USE:
        thing_used(t, tp);
        break;

    case PLAYER_ACTION_DROP:
        thing_drop(t, tp);
        break;
    }
}

void thing_server_fire (thingp t,
                        const uint8_t up,
                        const uint8_t down,
                        const uint8_t left,
                        const uint8_t right)
{
    /*
     * Cannot fire until we're on a level.
     */
    if (!t->wid) {
        THING_LOG(t, "cannot fire yet, not on the level");
        return;
    }

    /*
     * Use the currently wielded weapon.
     */
    tpp weapon = thing_weapon(t);
    if (!weapon) {
        THING_LOG(t, "has no weapon, cannot fire");
        return;
    }

    /*
     * Does it potentially break each use?
     */
    thing_weapon_check_for_damage_on_firing(t, weapon);

    double dx, dy;
    double dist_from_player = 0.1;

    /*
     * Try current direction.
     */
    dx = 0.0;
    dy = 0.0;

    /*
     * If the player is moving too then the weapon has a bit more
     * speed than if thrown when stationary.
     */
    if (down) {
        dy = dist_from_player;
    }

    if (up) {
        dy = -dist_from_player;
    }

    if (right) {
        dx = dist_from_player;
    }

    if (left) {
        dx = -dist_from_player;
    }

    /*
     * If no dir, then try the last thing dir.
     */
    if ((dx == 0) && (dy == 0)) {
        if (thing_is_dir_down(t)) {
            dy = dist_from_player;
        }

        if (thing_is_dir_up(t)) {
            dy = -dist_from_player;
        }

        if (thing_is_dir_right(t)) {
            dx = dist_from_player;
        }

        if (thing_is_dir_left(t)) {
            dx = -dist_from_player;
        }

        if (thing_is_dir_tl(t)) {
            dx = -dist_from_player;
            dy = -dist_from_player;
        }

        if (thing_is_dir_tr(t)) {
            dx = dist_from_player;
            dy = -dist_from_player;
        }

        if (thing_is_dir_bl(t)) {
            dx = -dist_from_player;
            dy = dist_from_player;
        }

        if (thing_is_dir_br(t)) {
            dx = dist_from_player;
            dy = dist_from_player;
        }
    }

    /*
     * Fire from the player position plus the initial delta so it looks like 
     * it comes from outside of the body.
     */
    double x = t->x;
    double y = t->y;

    x += dx;
    y += dy;

    tpp projectile = tp_fires(weapon);
    if (!projectile) {
        /*
         * Might be a sword.
         */
        thing_swing(t);
        return;
    }

    widp w = wid_game_map_server_replace_tile(
                                    wid_game_map_server_grid_container,
                                    x,
                                    y,
                                    0, /* thing */
                                    projectile,
                                    0, /* tpp data */
                                    0, /* item */
                                    0 /* stats */);

    thingp p = wid_get_thing(w);

    /*
     * Make sure we keep track of who fired so we can award scores.
     */
    thing_set_owner(p, t);

    /*
     * Round up say -0.7 to -1.0
     */
    dx *= 10.0;
    dy *= 10.0;
    dx /= (dist_from_player * 10.0);
    dy /= (dist_from_player * 10.0);

    p->dx = dx;
    p->dy = dy;
    p->dir = t->dir;

    /*
     * Check for immediate collision with a wall
     */
    thing_handle_collisions(wid_game_map_server_grid_container, p);
    if (thing_is_dead_or_dying(p)) {
        return;
    }

    double fnexthop_x = p->x + p->dx;
    double fnexthop_y = p->y + p->dy;

    thing_server_move(p,
            fnexthop_x,
            fnexthop_y,
            fnexthop_y < p->y,
            fnexthop_y > p->y,
            fnexthop_x < p->x,
            fnexthop_x > p->x,
            false);
}

static void thing_server_wid_move (thingp t, double x, double y, uint8_t is_new)
{
    thing_move(t, x, y);

    /*
     * Off the map? Perhaps between levels.
     */
    if (!t->wid) {
        return;
    }

    x *= server_tile_width;
    y *= server_tile_height;

    x += server_tile_width / 2;
    y += server_tile_height / 2;

    fpoint tl = { x, y };
    fpoint br = { x, y };

    double base_tile_width =
            ((1.0f / ((double)TILES_SCREEN_WIDTH)) *
                (double)global_config.video_gl_width);

    double base_tile_height =
            ((1.0f / ((double)TILES_SCREEN_HEIGHT)) *
                (double)global_config.video_gl_height);

    /*
     * Work out the tile size in a percentage of the screen.
     */
    br.x += base_tile_width;
    br.y += base_tile_height;

    /*
     * Now center the tile.
     */
    tl.x -= base_tile_width / 2.0;
    tl.y -= base_tile_height / 2.0;

    br.x -= base_tile_width / 2.0;
    br.y -= base_tile_height / 2.0;

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

    tl.y -= tile_height / 4.0;
    br.x += tile_width / 4.0;

    if (is_new || 
        thing_is_player(t) ||
        thing_is_weapon(t) ||
        thing_is_weapon_swing_effect(t)) {
        wid_set_tl_br(t->wid, tl, br);
    } else {
        wid_move_to_abs_in(t->wid, tl.x, tl.y, 1000.0 / thing_speed(t));
    }
}

void thing_server_wid_update (thingp t, double x, double y, uint8_t is_new)
{
    thing_server_wid_move(t, x, y, is_new);

    /*
     * Make the weapon follow the thing.
     */
    thingp weapon_carry_anim = thing_weapon_carry_anim(t);
    if (weapon_carry_anim) {
        thing_server_wid_move(weapon_carry_anim, x, y, is_new);
    }

    /*
     * Make the weapon being swung follow the thing.
     */
    thingp weapon_swing_anim = thing_weapon_swing_anim(t);
    if (weapon_swing_anim) {
        double dx = 0;
        double dy = 0;

        thing_weapon_swing_offset(t, &dx, &dy);
        thing_server_wid_move(weapon_swing_anim, x + dx, y + dy, is_new);
    }

    /*
     * Make the shield follow the thing.
     */
    thingp shield_anim = thing_shield_anim(t);
    if (shield_anim) {
        thing_server_wid_move(shield_anim, x, y, is_new);
    }
}
