/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */


#include "main.h"
#include "socket_util.h"
#include "tile.h"
#include "thing_shop.h"

uint8_t thing_server_move (thingp t,
                           double x,
                           double y,
                           const uint8_t up,
                           const uint8_t down,
                           const uint8_t left,
                           const uint8_t right,
                           const uint8_t fire,
                           const uint8_t magic)
{
    if (thing_is_dead_or_dying(t)) {
        return (false);
    }

    if (thing_is_player(t)) {
        level_place_light(server_level, t->x, t->y);
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

    thingp shield_carry_anim = thing_shield_carry_anim(t);
    if (shield_carry_anim) {
        thing_move_set_dir(shield_carry_anim, &x, &y, up, down, left, right);
    }

    thingp magic_anim = thing_magic_anim(t);
    if (magic_anim) {
        thing_move_set_dir(magic_anim, &x, &y, up, down, left, right);
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

            if (!fire && !magic) {
                return (false);
            }
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

            if (!fire && !magic) {
                return (false);
            }
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

    if (shield_carry_anim) {
        thing_move_set_dir(shield_carry_anim, &x, &y, up, down, left, right);
    }

    if (magic_anim) {
        thing_move_set_dir(magic_anim, &x, &y, up, down, left, right);
    }

    if (fire) {
        thing_server_fire(t, up, down, left, right);
    }

    if (magic) {
        thing_server_magic_powerup(t);
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
                          uint32_t action_bar_index,
                          int change_selection_only)
{
    switch (action) {
        case PLAYER_ACTION_PAUSE_GAME: 
            level_set_is_paused(server_level, true);
            return;

        case PLAYER_ACTION_RESUME_GAME: 
            level_set_is_paused(server_level, false);
            return;
    }

    if (action_bar_index >= THING_ACTION_BAR_MAX) {
        ERR("invalid action bar slot %u", action_bar_index);
        return;
    }

    itemp item = &t->stats.action_bar[action_bar_index];
    if (!item->id) {
        MSG_SERVER_SHOUT_AT(WARNING, t, 0, 0, "No item in that slot to use");
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
        if (change_selection_only) {
            thing_stats_set_action_bar_index(t, action_bar_index);
            return;
        }

        if (tp_is_weapon(tp)) {
            thing_wield(t, tp);

            thing_stats_set_action_bar_index(t, action_bar_index);
            return;
        }

        if (item->id == THING_POTION_JESUS) {
            thing_jesus_spawn();
        } else if 
            ((item->id == THING_POTION_FIRE) ||
             (item->id == THING_POTION_MONSTICIDE) ||
             (item->id == THING_POTION_CLOUDKILL)) {
            level_place_explosion(server_level,
                                  t,
                                  tp,
                                  t->x, t->y,
                                  t->x, t->y);
            break;
        } else if (item->id == THING_POTION_LIFE) {
            thing_server_effect(t, THING_STATE_EFFECT_IS_POWER_UP);
            break;
        } else if (item->id == THING_POTION_SHIELD) {
            thing_wield_shield(t, tp_find("data/things/shield1"));
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
            MSG_SERVER_SHOUT_AT(INFO, t, 0, 0, "%s", message);
            break;
        }

        /*
         * Failed to use.
         */
        MSG_SERVER_SHOUT_AT(WARNING, t, 0, 0, "Failed to use the %s", 
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
    {
        widp w;

        w = thing_place(t, tp, item);
        if (!w) {
            /*
             * Urk!
             */
            MSG_SERVER_SHOUT_AT(INFO, t, 0, 0, "Drop failed");
            return;
        }

        /*
         * If in a shop, try to sell it.
         */
        if (t->in_shop_owned_by_thing_id) {
            thingp newt;
            newt = wid_get_thing(w);
            shop_deposit_message(t, newt);
        }

        MSG_SERVER_SHOUT_AT(SOUND, t, t->x, t->y, "drop");
    } break;

    case PLAYER_ACTION_PAY:

        if (t->in_shop_owned_by_thing_id) {
            shop_pay_for_items(t);
        } else {
            MSG_SERVER_SHOUT_AT(INFO, t, 0, 0, "I'm not in a shop");
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
        if (!change_selection_only) {
            thing_used(t, tp);
        }
        break;

    case PLAYER_ACTION_DROP:
        thing_drop(t, tp);
        break;
    }
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

#if 0
    /*
     * Stretch vertically a bit and then recented. Don't do this. Overlapping 
     * wall pixels with shadows mess up.
     */
    tl.y -= tile_height / 8.0;
    tl.y += tile_height / 16.0;
    br.y += tile_height / 16.0;
#endif

    if (is_new || 
        thing_is_player(t) ||
        thing_is_weapon(t) ||
        thing_is_weapon_swing_effect(t)) {
        wid_set_tl_br(t->wid, tl, br);
    } else {
        if (t->want_to_jump) {
            t->want_to_jump = false;
            t->is_jumping = true;
        }

        if (t->is_jumping) {
            wid_move_end(t->wid);
        }

        wid_move_to_abs_in(t->wid, tl.x, tl.y, 5000.0 / thing_speed(t));
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
    thingp shield_carry_anim = thing_shield_carry_anim(t);
    if (shield_carry_anim) {
        thing_server_wid_move(shield_carry_anim, x, y, is_new);
    }

    /*
     * Make the magic follow the thing.
     */
    thingp magic_anim = thing_magic_anim(t);
    if (magic_anim) {
        thing_server_wid_move(magic_anim, x, y, is_new);
    }
}
