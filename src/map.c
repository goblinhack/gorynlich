/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "map.h"
#include "wid.h"
#include "thing.h"
#include "thing_tile.h"
#include "level.h"
#include "bits.h"
#include "socket_util.h"
#include "./wid_game_map_server.h"
#include "./wid_game_map_client.h"

#ifdef GORY_DEBUG
FILE *fp = 0;
#endif

thingp map_is_tp_at (levelp level, int32_t x, int32_t y, tpp tp)
{
    widp grid_wid;
    widp w;

    grid_wid = level_get_map(level);
    if (!grid_wid) {
        ERR("no grid wid");
    }

    /*
     * Look for a floor tile where we can place stuff.
     */
    uint8_t z;

    for (z = 0; z < MAP_DEPTH_MAX; z++) {
        w = wid_grid_find_first(grid_wid, x, y, z);
        while (w) {
            thingp thing_it = wid_get_thing(w);

            if (!thing_it) {
                w = wid_grid_find_next(grid_wid, w, x, y, z);
                continue;
            }

            if (thing_is_dead_or_dying(thing_it)) {
                w = wid_grid_find_next(grid_wid, w, x, y, z);
                continue;
            }

            if (tp == wid_get_thing_template(w)) {
                return (thing_it);
            }

            w = wid_grid_find_next(grid_wid, w, x, y, z);
        }
    }

    return (0);
}

static uint8_t map_is_x_at (levelp level,
                            int32_t x, int32_t y, map_is_at_callback callback)
{
    tpp tp;
    widp grid_wid;
    widp w;

    grid_wid = level_get_map(level);
    if (!grid_wid) {
        ERR("no grid wid");
    }

    uint8_t z;

    for (z = 0; z < MAP_DEPTH_MAX; z++) {
        w = wid_grid_find_first(grid_wid, x, y, z);
        while (w) {
            thingp thing_it = wid_get_thing(w);

            if (!thing_it) {
                w = wid_grid_find_next(grid_wid, w, x, y, z);
                continue;
            }

            if (thing_is_dead_or_dying(thing_it)) {
                w = wid_grid_find_next(grid_wid, w, x, y, z);
                continue;
            }

            tp = wid_get_thing_template(w);
            if (tp) {
                if ((*callback)(tp)) {
                    return (true);
                }
            }

            w = wid_grid_find_next(grid_wid, w, x, y, z);
        }
    }

    return (false);
}

uint8_t map_count_x_at (levelp level,
                        int32_t x, int32_t y,
                        map_is_at_callback callback)
{
    tpp tp;
    widp grid_wid;
    widp w;
    uint32_t count;

    count = 0;

    grid_wid = level_get_map(level);
    if (!grid_wid) {
        ERR("no grid wid");
    }

    /*
     * Look for a floor tile where we can place stuff.
     */
    uint8_t z;

    for (z = 0; z < MAP_DEPTH_MAX; z++) {
        w = wid_grid_find_first(grid_wid, x, y, z);
        while (w) {
            tp = wid_get_thing_template(w);
            if (tp) {
                if ((*callback)(tp)) {
                    count++;
                }
            }

            w = wid_grid_find_next(grid_wid, w, x, y, z);
        }
    }

    return (count);
}

uint8_t map_is_player_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_player));
}

uint8_t map_is_monst_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_monst));
}

uint8_t map_is_wall_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_wall));
}

uint8_t map_is_shop_floor_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_shop_floor));
}

uint8_t map_is_key_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_key));
}

uint8_t map_is_rock_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_rock));
}

uint8_t map_is_crystal_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_crystal));
}

uint8_t map_is_shadow_caster_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_shadow_caster));
}

uint8_t map_is_shadow_caster_soft_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_shadow_caster_soft));
}

uint8_t map_is_weapon_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_weapon));
}

uint8_t map_is_treasure_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_treasure));
}

uint8_t map_is_wearable_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_wearable));
}

uint8_t map_is_fragile_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_fragile));
}

uint8_t map_is_star_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_star));
}

uint8_t map_is_animated_no_dir_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_animated_no_dir));
}

uint8_t map_is_weapon_swing_effect_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_weapon_swing_effect));
}

uint8_t map_is_light_source_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_light_source));
}

uint8_t map_is_candle_light_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_candle_light));
}

uint8_t map_is_cats_eyes_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_cats_eyes));
}

uint8_t map_is_fire_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_fire));
}

uint8_t map_is_non_explosive_gas_cloud_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_non_explosive_gas_cloud));
}

uint8_t map_is_item_unusable_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_item_unusable));
}

uint8_t map_is_valid_for_action_bar_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_valid_for_action_bar));
}

uint8_t map_is_door_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_door));
}

uint8_t map_is_mob_spawner_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_mob_spawner));
}

uint8_t map_is_acid_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_acid));
}

uint8_t map_is_water_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_water));
}

uint8_t map_is_lava_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_lava));
}

uint8_t map_is_teleport_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_teleport));
}

uint8_t map_is_potion_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_potion));
}

uint8_t map_is_shield_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_shield));
}

uint8_t map_is_death_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_death));
}

uint8_t map_is_cobweb_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_cobweb));
}

uint8_t map_is_stickyslime_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_stickyslime));
}

uint8_t map_is_ethereal_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_ethereal));
}

uint8_t map_is_variable_size_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_variable_size));
}

uint8_t map_is_magical_weapon_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_magical_weapon));
}

uint8_t map_is_ranged_weapon_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_ranged_weapon));
}

uint8_t map_is_melee_weapon_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_melee_weapon));
}

uint8_t map_is_poison_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_poison));
}

uint8_t map_is_cloud_effect_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_cloud_effect));
}

uint8_t map_is_powerup_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_powerup));
}

uint8_t map_is_damaged_on_firing_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_damaged_on_firing));
}

uint8_t map_is_hard_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_hard));
}

uint8_t map_is_sleeping_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_sleeping));
}

uint8_t map_is_bomb_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_bomb));
}

uint8_t map_is_sawblade_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_sawblade));
}

uint8_t map_is_action_text_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_action_text));
}

uint8_t map_is_visible_on_debug_only_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_visible_on_debug_only));
}

uint8_t map_is_action_zap_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_action_zap));
}

uint8_t map_is_action_sleep_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_action_sleep));
}

uint8_t map_is_action_trigger_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_action_trigger));
}

uint8_t map_is_action_down_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_action_down));
}

uint8_t map_is_action_up_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_action_up));
}

uint8_t map_is_action_left_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_action_left));
}

uint8_t map_is_action_right_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_action_right));
}

uint8_t map_can_walk_through_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_can_walk_through));
}

uint8_t map_is_weapon_carry_anim_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_weapon_carry_anim));
}

uint8_t map_is_spell_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_spell));
}

uint8_t map_is_hand_item_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_hand_item));
}

uint8_t map_is_boots_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_boots));
}

uint8_t map_is_helmet_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_helmet));
}

uint8_t map_is_armor_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_armor));
}

uint8_t map_is_given_randomly_at_start_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_given_randomly_at_start));
}

uint8_t map_is_magical_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_magical));
}

uint8_t map_is_degradable_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_degradable));
}

uint8_t map_is_cursed_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_cursed));
}

uint8_t map_is_animate_only_when_moving_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_animate_only_when_moving));
}

uint8_t map_is_warm_blooded_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_warm_blooded));
}

uint8_t map_can_be_enchanted_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_can_be_enchanted));
}

uint8_t map_is_stackable_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_stackable));
}

uint8_t map_is_torch_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_torch));
}

uint8_t map_is_explosion_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_explosion));
}

uint8_t map_is_hidden_from_editor_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_hidden_from_editor));
}

uint8_t map_is_animated_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_animated));
}

uint8_t map_is_combustable_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_combustable));
}

uint8_t map_is_projectile_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_projectile));
}

uint8_t map_is_inactive_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_inactive));
}

uint8_t map_is_plant_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_plant));
}

uint8_t map_is_food_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_food));
}

uint8_t map_is_dungeon_floor_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_dungeon_floor));
}

uint8_t map_is_exit_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_exit));
}

static thingp map_thing_is_x_at (levelp level,
                                 int32_t x, int32_t y,
                                 map_is_at_callback callback)
{
    tpp tp;
    widp grid_wid;
    widp w;

    grid_wid = level_get_map(level);
    if (!grid_wid) {
        ERR("no grid wid");
    }

    /*
     * Look for a floor tile where we can place stuff.
     */
    uint8_t z;

    for (z = 0; z < MAP_DEPTH_MAX; z++) {
        w = wid_grid_find_first(grid_wid, x, y, z);
        while (w) {
            thingp thing_it = wid_get_thing(w);

            if (!thing_it) {
                w = wid_grid_find_next(grid_wid, w, x, y, z);
                continue;
            }

            if (thing_is_dead_or_dying(thing_it)) {
                w = wid_grid_find_next(grid_wid, w, x, y, z);
                continue;
            }

            tp = wid_get_thing_template(w);
            if (tp) {
                if ((*callback)(tp)) {
                    return (thing_it);
                }
            }

            w = wid_grid_find_next(grid_wid, w, x, y, z);
        }
    }

    return (0);
}

thingp map_thing_is_player_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_player));
}

thingp map_thing_is_monst_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_monst));
}

thingp map_thing_is_wall_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_wall));
}

thingp map_thing_is_rock_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_rock));
}

thingp map_thing_is_crystal_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_crystal));
}

thingp map_thing_is_key_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_key));
}

thingp map_thing_is_shadow_caster_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_shadow_caster));
}

thingp map_thing_is_weapon_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_weapon));
}

thingp map_thing_is_treasure_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_treasure));
}

thingp map_thing_is_wearable_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_wearable));
}

thingp map_thing_is_fragile_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_fragile));
}

thingp map_thing_is_star_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_star));
}

thingp map_thing_is_animated_no_dir_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_animated_no_dir));
}

thingp map_thing_is_weapon_swing_effect_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_weapon_swing_effect));
}

thingp map_thing_is_light_source_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_light_source));
}

thingp map_thing_is_candle_light_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_candle_light));
}

thingp map_thing_is_cats_eyes_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_cats_eyes));
}

thingp map_thing_is_fire_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_fire));
}

thingp map_thing_is_ring_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_ring));
}

thingp map_thing_is_animation_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_animation));
}

thingp map_thing_is_non_explosive_gas_cloud_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_non_explosive_gas_cloud));
}

thingp map_thing_is_item_unusable_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_item_unusable));
}

thingp map_thing_is_valid_for_action_bar_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_valid_for_action_bar));
}

thingp map_thing_is_door_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_door));
}

thingp map_thing_is_mob_spawner_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_mob_spawner));
}

thingp map_thing_is_acid_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_acid));
}

thingp map_thing_is_lava_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_lava));
}

thingp map_thing_is_teleport_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_teleport));
}

thingp map_thing_is_potion_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_potion));
}

thingp map_thing_is_shield_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_shield));
}

thingp map_thing_is_death_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_death));
}

thingp map_thing_is_cobweb_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_cobweb));
}

thingp map_thing_is_ethereal_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_ethereal));
}

thingp map_thing_is_variable_size_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_variable_size));
}

thingp map_thing_is_magical_weapon_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_magical_weapon));
}

thingp map_thing_is_ranged_weapon_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_ranged_weapon));
}

thingp map_thing_is_melee_weapon_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_melee_weapon));
}

thingp map_thing_is_poison_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_poison));
}

thingp map_thing_is_cloud_effect_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_cloud_effect));
}

thingp map_thing_is_powerup_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_powerup));
}

thingp map_thing_is_damaged_on_firing_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_damaged_on_firing));
}

thingp map_thing_is_hard_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_hard));
}

thingp map_thing_is_sleeping_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_sleeping));
}

thingp map_thing_is_bomb_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_bomb));
}

thingp map_thing_is_sawblade_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_sawblade));
}

thingp map_thing_is_action_text_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_action_text));
}

thingp map_thing_is_visible_on_debug_only_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_visible_on_debug_only));
}

thingp map_thing_is_action_zap_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_action_zap));
}

thingp map_thing_is_action_sleep_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_action_sleep));
}

thingp map_thing_is_action_trigger_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_action_trigger));
}

thingp map_thing_is_action_down_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_action_down));
}

thingp map_thing_is_action_up_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_action_up));
}

thingp map_thing_is_action_left_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_action_left));
}

thingp map_thing_is_action_right_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_action_right));
}

thingp map_thing_can_walk_through_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_can_walk_through));
}

thingp map_thing_is_weapon_carry_anim_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_weapon_carry_anim));
}

thingp map_thing_is_spell_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_spell));
}

thingp map_thing_is_hand_item_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_hand_item));
}

thingp map_thing_is_boots_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_boots));
}

thingp map_thing_is_helmet_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_helmet));
}

thingp map_thing_is_armor_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_armor));
}

thingp map_thing_is_given_randomly_at_start_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_given_randomly_at_start));
}

thingp map_thing_is_magical_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_magical));
}

thingp map_thing_is_degradable_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_degradable));
}

thingp map_thing_is_cursed_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_cursed));
}

thingp map_thing_is_animate_only_when_moving_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_animate_only_when_moving));
}

thingp map_thing_is_warm_blooded_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_warm_blooded));
}

thingp map_thing_can_be_enchanted_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_can_be_enchanted));
}

thingp map_thing_is_stackable_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_stackable));
}

thingp map_thing_is_torch_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_torch));
}

thingp map_thing_is_explosion_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_explosion));
}

thingp map_thing_is_hidden_from_editor_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_hidden_from_editor));
}

thingp map_thing_is_animated_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_animated));
}

thingp map_thing_is_combustable_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_combustable));
}

thingp map_thing_is_projectile_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_projectile));
}

thingp map_thing_is_inactive_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_inactive));
}

thingp map_thing_is_plant_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_plant));
}

thingp map_thing_is_food_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_food));
}

thingp map_thing_is_dungeon_floor_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_dungeon_floor));
}

thingp map_thing_is_exit_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_exit));
}

static tree_rootp map_all_things_is_x_at (levelp level,
                                          int32_t x, int32_t y,
                                          map_is_at_callback callback)
{
    tpp tp;
    tree_thing_node *node;
    tree_rootp root;
    widp grid_wid;
    widp w;

    root = 0;

    grid_wid = level_get_map(level);
    if (!grid_wid) {
        ERR("no grid wid");
    }

    /*
     * Look for a floor tile where we can place stuff.
     */
    uint8_t z;

    for (z = 0; z < MAP_DEPTH_MAX; z++) {
        w = wid_grid_find_first(grid_wid, x, y, z);
        while (w) {
            thingp thing_it = wid_get_thing(w);

            if (!thing_it) {
                w = wid_grid_find_next(grid_wid, w, x, y, z);
                continue;
            }

            if (thing_is_dead_or_dying(thing_it)) {
                w = wid_grid_find_next(grid_wid, w, x, y, z);
                continue;
            }

            tp = wid_get_thing_template(w);
            if (tp) {
                if ((*callback)(tp)) {
                    if (!root) {
                        root = tree_alloc(TREE_KEY_POINTER,
                                        "TREE ROOT: map find things");
                    }

                    node = (typeof(node))
                        myzalloc(sizeof(*node), "TREE NODE: map find thing");

                    node->tree.key = (void*)thing_it;

                    if (!tree_insert(root, &node->tree.node)) {
                        ERR("insert thingp %p", thing_it);
                    }
                }
            }

            w = wid_grid_find_next(grid_wid, w, x, y, z);
        }
    }

    return (root);
}

tree_rootp map_all_things_is_player_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_player));
}

tree_rootp map_all_things_is_monst_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_monst));
}

tree_rootp map_all_things_is_wall_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_wall));
}

tree_rootp map_all_things_is_key_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_key));
}

tree_rootp map_all_things_is_rock_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_rock));
}

tree_rootp map_all_things_is_crystal_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_crystal));
}

tree_rootp map_all_things_is_shadow_caster_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_shadow_caster));
}

tree_rootp map_all_things_is_weapon_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_weapon));
}

tree_rootp map_all_things_is_treasure_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_treasure));
}

tree_rootp map_all_things_is_wearable_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_wearable));
}

tree_rootp map_all_things_is_fragile_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_fragile));
}

tree_rootp map_all_things_is_star_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_star));
}

tree_rootp map_all_things_is_animated_no_dir_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_animated_no_dir));
}

tree_rootp map_all_things_is_weapon_swing_effect_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_weapon_swing_effect));
}

tree_rootp map_all_things_is_light_source_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_light_source));
}

tree_rootp map_all_things_is_candle_light_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_candle_light));
}

tree_rootp map_all_things_is_cats_eyes_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_cats_eyes));
}

tree_rootp map_all_things_is_fire_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_fire));
}

tree_rootp map_all_things_is_ring_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_ring));
}

tree_rootp map_all_things_is_animation_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_animation));
}

tree_rootp map_all_things_is_non_explosive_gas_cloud_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_non_explosive_gas_cloud));
}

tree_rootp map_all_things_is_item_unusable_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_item_unusable));
}

tree_rootp map_all_things_is_valid_for_action_bar_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_valid_for_action_bar));
}

tree_rootp map_all_things_is_door_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_door));
}

tree_rootp map_all_things_is_mob_spawner_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_mob_spawner));
}

tree_rootp map_all_things_is_acid_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_acid));
}

tree_rootp map_all_things_is_lava_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_lava));
}

tree_rootp map_all_things_is_teleport_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_teleport));
}

tree_rootp map_all_things_is_potion_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_potion));
}

tree_rootp map_all_things_is_shield_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_shield));
}

tree_rootp map_all_things_is_death_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_death));
}

tree_rootp map_all_things_is_cobweb_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_cobweb));
}

tree_rootp map_all_things_is_ethereal_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_ethereal));
}

tree_rootp map_all_things_is_variable_size_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_variable_size));
}

tree_rootp map_all_things_is_magical_weapon_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_magical_weapon));
}

tree_rootp map_all_things_is_ranged_weapon_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_ranged_weapon));
}

tree_rootp map_all_things_is_melee_weapon_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_melee_weapon));
}

tree_rootp map_all_things_is_poison_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_poison));
}

tree_rootp map_all_things_is_cloud_effect_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_cloud_effect));
}

tree_rootp map_all_things_is_powerup_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_powerup));
}

tree_rootp map_all_things_is_damaged_on_firing_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_damaged_on_firing));
}

tree_rootp map_all_things_is_hard_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_hard));
}

tree_rootp map_all_things_is_sleeping_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_sleeping));
}

tree_rootp map_all_things_is_bomb_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_bomb));
}

tree_rootp map_all_things_is_sawblade_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_sawblade));
}

tree_rootp map_all_things_is_action_text_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_action_text));
}

tree_rootp map_all_things_is_visible_on_debug_only_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_visible_on_debug_only));
}

tree_rootp map_all_things_is_action_sleep_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_action_sleep));
}

tree_rootp map_all_things_is_action_zap_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_action_zap));
}

tree_rootp map_all_things_is_action_trigger_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_action_trigger));
}

tree_rootp map_all_things_is_action_down_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_action_down));
}

tree_rootp map_all_things_is_action_up_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_action_up));
}

tree_rootp map_all_things_is_action_left_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_action_left));
}

tree_rootp map_all_things_is_action_right_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_action_right));
}

tree_rootp map_all_things_can_walk_through_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_can_walk_through));
}

tree_rootp map_all_things_is_weapon_carry_anim_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_weapon_carry_anim));
}

tree_rootp map_all_things_is_spell_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_spell));
}

tree_rootp map_all_things_is_hand_item_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_hand_item));
}

tree_rootp map_all_things_is_boots_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_boots));
}

tree_rootp map_all_things_is_helmet_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_helmet));
}

tree_rootp map_all_things_is_armor_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_armor));
}

tree_rootp map_all_things_is_given_randomly_at_start_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_given_randomly_at_start));
}

tree_rootp map_all_things_is_magical_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_magical));
}

tree_rootp map_all_things_is_degradable_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_degradable));
}

tree_rootp map_all_things_is_cursed_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_cursed));
}

tree_rootp map_all_things_is_animate_only_when_moving_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_animate_only_when_moving));
}

tree_rootp map_all_things_is_warm_blooded_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_warm_blooded));
}

tree_rootp map_all_things_can_be_enchanted_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_can_be_enchanted));
}

tree_rootp map_all_things_is_stackable_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_stackable));
}

tree_rootp map_all_things_is_torch_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_torch));
}

tree_rootp map_all_things_is_explosion_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_explosion));
}

tree_rootp map_all_things_is_hidden_from_editor_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_hidden_from_editor));
}

tree_rootp map_all_things_is_animated_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_animated));
}

tree_rootp map_all_things_is_combustable_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_combustable));
}

tree_rootp map_all_things_is_projectile_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_projectile));
}

tree_rootp map_all_things_is_inactive_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_inactive));
}

tree_rootp map_all_things_is_plant_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_plant));
}

tree_rootp map_all_things_is_food_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_food));
}

tree_rootp map_all_things_is_dungeon_floor_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_dungeon_floor));
}

tree_rootp map_all_things_is_exit_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_exit));
}

static tpp map_find_x_at (levelp level,
                          int32_t x, int32_t y,
                          map_is_at_callback callback,
                          widp *wout)
{
    if ((x >= MAP_WIDTH) || (x < 0) || (y >= MAP_HEIGHT) || (y < 0)) {
        return (0);
    }

    thing_map *map = level_get_thing_map(level);
    thing_map_cell *cell = &map->cells[x][y];

    uint32_t i;
    for (i = 0; i < cell->count; i++) {
        thingp thing_it;
            
        if (level == server_level) {
            thing_it = thing_server_id(cell->id[i]);
        } else {
            thing_it = thing_client_id(cell->id[i]);
        }

        if (thing_is_dead_or_dying(thing_it)) {
            continue;
        }

        if ((*callback)(thing_it->tp)) {
            if (wout) {
                *wout = thing_it->wid;
            }

            return (thing_it->tp);
        }
    }

    return (0);
}

tpp map_find_player_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_player, w));
}

tpp map_find_monst_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_monst, w));
}

tpp map_find_wall_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_wall, w));
}

tpp map_find_rock_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_rock, w));
}

tpp map_find_crystal_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_crystal, w));
}

tpp map_find_key_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_key, w));
}

tpp map_find_xxx3_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_rock, w));
}

tpp map_find_blocks_light_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_shadow_caster, w));
}

tpp map_find_xxx5_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_weapon, w));
}

tpp map_find_xxx6_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_treasure, w));
}

tpp map_find_xxx7_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_wearable, w));
}

tpp map_find_is_fragile_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_fragile, w));
}

tpp map_find_is_star_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_star, w));
}

tpp map_find_key1_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_weapon_swing_effect, w));
}

tpp map_find_light_source_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_light_source, w));
}

tpp map_find_candle_light_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_candle_light, w));
}

tpp map_find_star_green_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_cats_eyes, w));
}

tpp map_find_star_cyan_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_fire, w));
}

tpp map_find_star_black_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_ring, w));
}

tpp map_find_star_purple_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_animation, w));
}

tpp map_find_explosion_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_animation, w));
}

tpp map_find_spikes_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_non_explosive_gas_cloud, w));
}

tpp map_find_is_item_unusable_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_item_unusable, w));
}

tpp map_find_is_valid_for_action_bar_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_valid_for_action_bar, w));
}

tpp map_find_door_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_door, w));
}

tpp map_find_generator_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_mob_spawner, w));
}

tpp map_find_acid_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_acid, w));
}

tpp map_find_lava_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_lava, w));
}

tpp map_find_teleport_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_teleport, w));
}

tpp map_find_potion_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_potion, w));
}

tpp map_find_shield_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_shield, w));
}

tpp map_find_death_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_death, w));
}

tpp map_find_cobweb_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_cobweb, w));
}

tpp map_find_ethereal_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_ethereal, w));
}

tpp map_find_variable_size_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_variable_size, w));
}

tpp map_find_magical_weapon_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_magical_weapon, w));
}

tpp map_find_ranged_weapon_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_ranged_weapon, w));
}

tpp map_find_melee_weapon_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_melee_weapon, w));
}

tpp map_find_is_poison_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_poison, w));
}

tpp map_find_cloud_effect_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_cloud_effect, w));
}

tpp map_find_damaged_on_firing_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_damaged_on_firing, w));
}

tpp map_find_hard_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_hard, w));
}

tpp map_find_sleeping_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_sleeping, w));
}

tpp map_find_bomb_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_bomb, w));
}

tpp map_find_saw_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_sawblade, w));
}

tpp map_find_action_text_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_action_text, w));
}

tpp map_find_visible_on_debug_only_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_visible_on_debug_only, w));
}

tpp map_find_action_zap_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_action_zap, w));
}

tpp map_find_action_sleep_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_action_sleep, w));
}

tpp map_find_action_trigger_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_action_trigger, w));
}

tpp map_find_action_down_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_action_down, w));
}

tpp map_find_action_up_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_action_up, w));
}

tpp map_find_action_left_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_action_left, w));
}

tpp map_find_action_right_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_action_right, w));
}

tpp map_find_can_walk_through_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_can_walk_through, w));
}

tpp map_find_weapon_carry_anim_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_weapon_carry_anim, w));
}

tpp map_find_spell_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_spell, w));
}

tpp map_find_given_randomly_at_start_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_given_randomly_at_start, w));
}

tpp map_find_magical_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_magical, w));
}

tpp map_find_degradable_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_degradable, w));
}

tpp map_find_cursed_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_cursed, w));
}

tpp map_find_animate_only_when_moving_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_animate_only_when_moving, w));
}

tpp map_find_torch_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_torch, w));
}

tpp map_find_is_explosion_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_explosion, w));
}

tpp map_find_hidden_from_editor_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_hidden_from_editor, w));
}

tpp map_find_is_projectile_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_projectile, w));
}

tpp map_find_item_perma_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_inactive, w));
}

tpp map_find_is_ring_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_ring, w));
}

tpp map_find_plant_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_plant, w));
}

tpp map_find_food_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_food, w));
}

tpp map_find_floor_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_dungeon_floor, w));
}

tpp map_find_exit_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_exit, w));
}

void map_fixup (levelp level)
{
    int32_t index;
    tilep tile;
    int32_t x;
    int32_t y;
    int32_t dx;
    int32_t dy;
    tpp nbrs[3][3];
    widp w;
                
#ifdef GORY_DEBUG
    if (!fp) {
        fp = fopen("map.txt","w");
    }

    if (level == server_level) {
        fprintf(fp,"update server level %p\n", level);
    } else {
        fprintf(fp,"update client level %p\n",level);
    }
#endif

    for (y = 0; y < MAP_HEIGHT; y++) {
        for (x = 0; x < MAP_WIDTH; x++) {

            widp mywid = 0;

            tpp tp = 0;

            if ((tp = map_find_wall_at(level, x, y, &w))) {
#ifdef GORY_DEBUG
if (level != server_level)
                fprintf(fp,"x");
#endif
                mywid = w;
            } else if ((tp = map_find_door_at(level, x, y, &w))) {
#ifdef GORY_DEBUG
if (level != server_level)
                fprintf(fp,"D");
#endif
                mywid = w;

            } else if ((tp = map_find_lava_at(level, x, y, &w))) {
#ifdef GORY_DEBUG
if (level != server_level)
                fprintf(fp,"L");
#endif
                mywid = w;

            } else if ((tp = map_find_acid_at(level, x, y, &w))) {
#ifdef GORY_DEBUG
if (level != server_level)
                fprintf(fp,"A");
#endif
                mywid = w;
            }

            if (!mywid) {
#ifdef GORY_DEBUG
if (level != server_level)
                fprintf(fp," ");
#endif
                continue;
            }

            memset(nbrs, 0, sizeof(nbrs));

            for (dx = -1; dx <= 1; dx++) {
                for (dy = -1; dy <= 1; dy++) {
                    widp w;
                    tpp tp;

                    if (map_find_wall_at(level, x, y, &w)) {
                        if (thing_wid_is_active(w)) {
                            continue;
                        }

                        tp = map_find_wall_at(level, x + dx, y + dy, &w);
                        if (thing_wid_is_active(w)) {
                            continue;
                        }

                        nbrs[dx + 1][dy + 1] = tp;

                    } else if (map_find_door_at(level, x, y, &w)) {

                        tp = map_find_door_at(level, x + dx, y + dy, &w);

                        nbrs[dx + 1][dy + 1] = tp;

                    } else if (map_find_acid_at(level, x, y, &w)) {

                        tp = map_find_acid_at(level, x + dx, y + dy, &w);

                        nbrs[dx + 1][dy + 1] = tp;

                    } else if (map_find_lava_at(level, x, y, &w)) {

                        tp = map_find_lava_at(level, x + dx, y + dy, &w);

                        nbrs[dx + 1][dy + 1] = tp;
                    }
                }
            }

            tpp a = nbrs[0][0];
            tpp b = nbrs[1][0];
            tpp c = nbrs[2][0];
            tpp d = nbrs[0][1];
            tpp e = nbrs[1][1];
            tpp f = nbrs[2][1];
            tpp g = nbrs[0][2];
            tpp h = nbrs[1][2];
            tpp i = nbrs[2][2];

            uint8_t A = (a != 0) ? 1 : 0;
            uint8_t B = (b != 0) ? 1 : 0;
            uint8_t C = (c != 0) ? 1 : 0;
            uint8_t D = (d != 0) ? 1 : 0;
            uint8_t E = (e != 0) ? 1 : 0;
            uint8_t F = (f != 0) ? 1 : 0;
            uint8_t G = (g != 0) ? 1 : 0;
            uint8_t H = (h != 0) ? 1 : 0;
            uint8_t I = (i != 0) ? 1 : 0;

            const uint16_t omask =
                (I << 8) | (H << 7) | (G << 6) | (F << 5) |
                (E << 4) | (D << 3) | (C << 2) | (B << 1) |
                (A << 0);

            uint8_t score;
            uint8_t best = 0;

            index = -1;

            uint16_t mask;

#define BLOCK(a,b,c,d,e,f,g,h,i, _index_)                               \
            mask =                                                      \
                (i << 8) | (h << 7) | (g << 6) | (f << 5) |             \
                (e << 4) | (d << 3) | (c << 2) | (b << 1) |             \
                (a << 0);                                               \
                                                                        \
            if ((mask & omask) == mask) {                               \
                uint32_t difference = mask ^ omask;                     \
                BITCOUNT(difference);                                   \
                score = 32 - difference;                                \
                if (score > best) {                                     \
                    best = score;                                       \
                    index = _index_;                                    \
                }                                                       \
            }                                                           \

            BLOCK(1,1,1,1,1,1,1,1,1,IS_JOIN_BLOCK)
            BLOCK(0,0,0,0,1,0,0,0,0,IS_JOIN_NODE)
            BLOCK(0,0,0,0,0,0,0,0,0,IS_JOIN_NODE) // moving blocks
            BLOCK(0,0,0,0,1,1,0,0,0,IS_JOIN_LEFT)
            BLOCK(0,0,0,0,1,0,0,1,0,IS_JOIN_TOP)
            BLOCK(0,0,0,1,1,0,0,0,0,IS_JOIN_RIGHT)
            BLOCK(0,1,0,0,1,0,0,0,0,IS_JOIN_BOT)
            BLOCK(0,0,0,1,1,1,0,0,0,IS_JOIN_HORIZ)
            BLOCK(0,1,0,0,1,0,0,1,0,IS_JOIN_VERT)
            BLOCK(0,0,0,0,1,1,0,1,1,IS_JOIN_TL2)
            BLOCK(0,1,1,0,1,1,0,0,0,IS_JOIN_BL2)
            BLOCK(1,1,0,1,1,0,0,0,0,IS_JOIN_BR2)
            BLOCK(0,0,0,1,1,0,1,1,0,IS_JOIN_TR2)
            BLOCK(0,0,0,0,1,1,0,1,0,IS_JOIN_TL)
            BLOCK(0,1,0,0,1,1,0,0,0,IS_JOIN_BL)
            BLOCK(0,1,0,1,1,0,0,0,0,IS_JOIN_BR)
            BLOCK(0,0,0,1,1,0,0,1,0,IS_JOIN_TR)
            BLOCK(1,1,0,1,1,0,1,1,0,IS_JOIN_T90_3)
            BLOCK(1,1,1,1,1,1,0,0,0,IS_JOIN_T180_3)
            BLOCK(0,1,1,0,1,1,0,1,1,IS_JOIN_T270_3)
            BLOCK(0,0,0,1,1,1,1,1,1,IS_JOIN_T_3)
            BLOCK(0,1,0,0,1,1,0,1,0,IS_JOIN_T270)
            BLOCK(0,1,0,1,1,1,0,0,0,IS_JOIN_T180)
            BLOCK(0,1,0,1,1,0,0,1,0,IS_JOIN_T90)
            BLOCK(0,0,0,1,1,1,0,1,0,IS_JOIN_T)
            BLOCK(0,1,1,0,1,1,0,1,0,IS_JOIN_T270_2)
            BLOCK(1,1,0,1,1,1,0,0,0,IS_JOIN_T180_2)
            BLOCK(0,1,0,1,1,0,1,1,0,IS_JOIN_T90_2)
            BLOCK(0,0,0,1,1,1,0,1,1,IS_JOIN_T_2)
            BLOCK(0,1,0,0,1,1,0,1,1,IS_JOIN_T270_1)
            BLOCK(0,1,1,1,1,1,0,0,0,IS_JOIN_T180_1)
            BLOCK(1,1,0,1,1,0,0,1,0,IS_JOIN_T90_1)
            BLOCK(0,0,0,1,1,1,1,1,0,IS_JOIN_T_1)
            BLOCK(0,1,0,1,1,1,0,1,0,IS_JOIN_X)
            BLOCK(0,1,0,1,1,1,0,1,1,IS_JOIN_X1)
            BLOCK(0,1,1,1,1,1,0,1,0,IS_JOIN_X1_270)
            BLOCK(1,1,0,1,1,1,0,1,0,IS_JOIN_X1_180)
            BLOCK(0,1,0,1,1,1,1,1,0,IS_JOIN_X1_90)
            BLOCK(0,1,0,1,1,1,1,1,1,IS_JOIN_X2)
            BLOCK(0,1,1,1,1,1,0,1,1,IS_JOIN_X2_270)
            BLOCK(1,1,1,1,1,1,0,1,0,IS_JOIN_X2_180)
            BLOCK(1,1,0,1,1,1,1,1,0,IS_JOIN_X2_90)
            BLOCK(0,1,1,1,1,1,1,1,0,IS_JOIN_X3)
            BLOCK(1,1,0,1,1,1,0,1,1,IS_JOIN_X3_180)
            BLOCK(0,1,1,1,1,1,1,1,1,IS_JOIN_X4)
            BLOCK(1,1,1,1,1,1,0,1,1,IS_JOIN_X4_270)
            BLOCK(1,1,1,1,1,1,1,1,0,IS_JOIN_X4_180)
            BLOCK(1,1,0,1,1,1,1,1,1,IS_JOIN_X4_90)

            /*
             * Single node doors need to join onto walls.
             */
            if (index == IS_JOIN_NODE) {
                if (map_find_door_at(level, x, y, &mywid)) {
                    if ( map_find_wall_at(level, x - 1, y, 0) &&
                         map_find_wall_at(level, x + 1, y, 0) &&
                        !map_find_wall_at(level, x, y - 1, 0) &&
                        !map_find_wall_at(level, x, y + 1, 0)) {
                        index = IS_JOIN_HORIZ2;
                    }
                }
            }

            if (index == IS_JOIN_NODE) {
                if (map_find_door_at(level, x, y, &mywid)) {
                    if (!map_find_wall_at(level, x - 1, y, 0) &&
                        !map_find_wall_at(level, x + 1, y, 0) &&
                         map_find_wall_at(level, x, y - 1, 0) &&
                         map_find_wall_at(level, x, y + 1, 0)) {
                        index = IS_JOIN_VERT2;
                    }
                }
            }

            if (index == -1) {
                ERR("%u%u%u %u%u%u %u%u%u not handled",
                    a ? 1 : 0,
                    b ? 1 : 0,
                    c ? 1 : 0,
                    d ? 1 : 0,
                    e ? 1 : 0,
                    f ? 1 : 0,
                    g ? 1 : 0,
                    h ? 1 : 0,
                    i ? 1 : 0);
            }

            tpp t = e;
            if (!e) {
                t = e = tp;
            }

            thing_tilep thing_tile = thing_tile_find(t, index, &tile);
            if (!thing_tile) {
                index = IS_JOIN_BLOCK;

                thing_tile = thing_tile_find(t, index, &tile);
                if (!thing_tile) {
                    index = IS_JOIN_NODE;

                    thing_tile = thing_tile_find(t, index, &tile);
                    if (!thing_tile) {
                        ERR("no joinable tile for %s", tp_name(t));
                    }
                }
            }

            if (!tile) {
                ERR("no tile for %s", tp_name(t));
            }

            const char *tilename = thing_tile_name(thing_tile);

            if (!tilename) {
                ERR("no tilename for %s", tp_name(e));
            }

            /*
             * If an existing thing, make sure and send an update if say
             * a door or wall is destroyed.
             */
            {
                tilep existing_tile = wid_get_tile(mywid);

                if (existing_tile && (existing_tile != tile)) {
                    thingp t = wid_get_thing(mywid);
                    if (t) {
                        t->join_index = index;

                        if (t && t->on_server) {
                            thing_update(t);
                        }
                    }
                }
            }

            wid_set_tilename(mywid, tilename);
            wid_set_font(mywid, small_font);
        }
#ifdef GORY_DEBUG
        fprintf(fp,"\n");
#endif
    }
#ifdef GORY_DEBUG
    fprintf(fp,"\n");
    fprintf(fp,"\n");
#endif
}

static tree_rootp map_all_things_is_x (levelp level,
                                       map_is_at_callback callback)
{
    tpp tp;
    tree_thing_node *node;
    tree_rootp root;
    widp grid_wid;
    widp w;
    int32_t x;
    int32_t y;

    root = 0;

    grid_wid = level_get_map(level);
    if (!grid_wid) {
        ERR("no grid wid");
    }

    uint8_t z;

    for (z = 0; z < MAP_DEPTH_MAX; z++) {
        for (y = 0; y < MAP_HEIGHT; y++) {
            for (x = 0; x < MAP_WIDTH; x++) {

                /*
                * Look for a floor tile where we can place stuff.
                */
                w = wid_grid_find_first(grid_wid, x, y, z);
                while (w) {
                    thingp thing_it = wid_get_thing(w);

                    if (!thing_it) {
                        w = wid_grid_find_next(grid_wid, w, x, y, z);
                        continue;
                    }

                    if (thing_is_dead_or_dying(thing_it)) {
                        w = wid_grid_find_next(grid_wid, w, x, y, z);
                        continue;
                    }

                    tp = wid_get_thing_template(w);
                    if (tp) {
                        if ((*callback)(tp)) {
                            if (!root) {
                                root = tree_alloc(TREE_KEY_POINTER,
                                                "TREE ROOT: map find things");
                            }

                            node = (typeof(node))
                                myzalloc(sizeof(*node), "TREE NODE: map find thing");

                            node->tree.key = (void*)thing_it;

                            if (!tree_insert(root, &node->tree.node)) {
                                ERR("insert thingp %p", thing_it);
                            }
                        }
                    }

                    w = wid_grid_find_next(grid_wid, w, x, y, z);
                }
            }
        }
    }

    return (root);
}

tree_rootp map_all_things_is_player (levelp level)
{
    return (map_all_things_is_x(level, tp_is_player));
}

tree_rootp map_all_things_is_monst (levelp level)
{
    return (map_all_things_is_x(level, tp_is_monst));
}

tree_rootp map_all_things_is_wall (levelp level)
{
    return (map_all_things_is_x(level, tp_is_wall));
}

tree_rootp map_all_things_is_key (levelp level)
{
    return (map_all_things_is_x(level, tp_is_key));
}

tree_rootp map_all_things_is_rock (levelp level)
{
    return (map_all_things_is_x(level, tp_is_rock));
}

tree_rootp map_all_things_is_crystal (levelp level)
{
    return (map_all_things_is_x(level, tp_is_crystal));
}

tree_rootp map_all_things_is_shadow_caster (levelp level)
{
    return (map_all_things_is_x(level, tp_is_shadow_caster));
}

tree_rootp map_all_things_is_weapon (levelp level)
{
    return (map_all_things_is_x(level, tp_is_weapon));
}

tree_rootp map_all_things_is_treasure (levelp level)
{
    return (map_all_things_is_x(level, tp_is_treasure));
}

tree_rootp map_all_things_is_wearable (levelp level)
{
    return (map_all_things_is_x(level, tp_is_wearable));
}

tree_rootp map_all_things_is_fragile (levelp level)
{
    return (map_all_things_is_x(level, tp_is_fragile));
}

tree_rootp map_all_things_is_star (levelp level)
{
    return (map_all_things_is_x(level, tp_is_star));
}

tree_rootp map_all_things_is_animated_no_dir (levelp level)
{
    return (map_all_things_is_x(level, tp_is_animated_no_dir));
}

tree_rootp map_all_things_is_weapon_swing_effect (levelp level)
{
    return (map_all_things_is_x(level, tp_is_weapon_swing_effect));
}

tree_rootp map_all_things_is_light_source (levelp level)
{
    return (map_all_things_is_x(level, tp_is_light_source));
}

tree_rootp map_all_things_is_candle_light (levelp level)
{
    return (map_all_things_is_x(level, tp_is_candle_light));
}

tree_rootp map_all_things_is_cats_eyes (levelp level)
{
    return (map_all_things_is_x(level, tp_is_cats_eyes));
}

tree_rootp map_all_things_is_fire (levelp level)
{
    return (map_all_things_is_x(level, tp_is_fire));
}

tree_rootp map_all_things_is_ring (levelp level)
{
    return (map_all_things_is_x(level, tp_is_ring));
}

tree_rootp map_all_things_is_animation (levelp level)
{
    return (map_all_things_is_x(level, tp_is_animation));
}

tree_rootp map_all_things_is_non_explosive_gas_cloud (levelp level)
{
    return (map_all_things_is_x(level, tp_is_non_explosive_gas_cloud));
}

tree_rootp map_all_things_is_item_unusable (levelp level)
{
    return (map_all_things_is_x(level, tp_is_item_unusable));
}

tree_rootp map_all_things_is_valid_for_action_bar (levelp level)
{
    return (map_all_things_is_x(level, tp_is_valid_for_action_bar));
}

tree_rootp map_all_things_is_door (levelp level)
{
    return (map_all_things_is_x(level, tp_is_door));
}

tree_rootp map_all_things_is_mob_spawner (levelp level)
{
    return (map_all_things_is_x(level, tp_is_mob_spawner));
}

tree_rootp map_all_things_is_acid (levelp level)
{
    return (map_all_things_is_x(level, tp_is_acid));
}

tree_rootp map_all_things_is_lava (levelp level)
{
    return (map_all_things_is_x(level, tp_is_lava));
}

tree_rootp map_all_things_is_teleport (levelp level)
{
    return (map_all_things_is_x(level, tp_is_teleport));
}

tree_rootp map_all_things_is_potion (levelp level)
{
    return (map_all_things_is_x(level, tp_is_potion));
}

tree_rootp map_all_things_is_shield (levelp level)
{
    return (map_all_things_is_x(level, tp_is_shield));
}

tree_rootp map_all_things_is_death (levelp level)
{
    return (map_all_things_is_x(level, tp_is_death));
}

tree_rootp map_all_things_is_cobweb (levelp level)
{
    return (map_all_things_is_x(level, tp_is_cobweb));
}

tree_rootp map_all_things_is_ethereal (levelp level)
{
    return (map_all_things_is_x(level, tp_is_ethereal));
}

tree_rootp map_all_things_is_variable_size (levelp level)
{
    return (map_all_things_is_x(level, tp_is_variable_size));
}

tree_rootp map_all_things_is_magical_weapon (levelp level)
{
    return (map_all_things_is_x(level, tp_is_magical_weapon));
}

tree_rootp map_all_things_is_ranged_weapon (levelp level)
{
    return (map_all_things_is_x(level, tp_is_ranged_weapon));
}

tree_rootp map_all_things_is_melee_weapon (levelp level)
{
    return (map_all_things_is_x(level, tp_is_melee_weapon));
}

tree_rootp map_all_things_is_poison (levelp level)
{
    return (map_all_things_is_x(level, tp_is_poison));
}

tree_rootp map_all_things_is_cloud_effect (levelp level)
{
    return (map_all_things_is_x(level, tp_is_cloud_effect));
}

tree_rootp map_all_things_is_powerup (levelp level)
{
    return (map_all_things_is_x(level, tp_is_powerup));
}

tree_rootp map_all_things_is_damaged_on_firing (levelp level)
{
    return (map_all_things_is_x(level, tp_is_damaged_on_firing));
}

tree_rootp map_all_things_is_hard (levelp level)
{
    return (map_all_things_is_x(level, tp_is_hard));
}

tree_rootp map_all_things_is_sleeping (levelp level)
{
    return (map_all_things_is_x(level, tp_is_sleeping));
}

tree_rootp map_all_things_is_bomb (levelp level)
{
    return (map_all_things_is_x(level, tp_is_bomb));
}

tree_rootp map_all_things_is_sawblade (levelp level)
{
    return (map_all_things_is_x(level, tp_is_sawblade));
}

tree_rootp map_all_things_is_action_text (levelp level)
{
    return (map_all_things_is_x(level, tp_is_action_text));
}

tree_rootp map_all_things_is_visible_on_debug_only (levelp level)
{
    return (map_all_things_is_x(level, tp_is_visible_on_debug_only));
}

tree_rootp map_all_things_is_action_zap (levelp level)
{
    return (map_all_things_is_x(level, tp_is_action_zap));
}

tree_rootp map_all_things_is_action_sleep (levelp level)
{
    return (map_all_things_is_x(level, tp_is_action_sleep));
}

tree_rootp map_all_things_is_action_trigger (levelp level)
{
    return (map_all_things_is_x(level, tp_is_action_trigger));
}

tree_rootp map_all_things_is_action_down (levelp level)
{
    return (map_all_things_is_x(level, tp_is_action_down));
}

tree_rootp map_all_things_is_action_up (levelp level)
{
    return (map_all_things_is_x(level, tp_is_action_up));
}

tree_rootp map_all_things_is_action_left (levelp level)
{
    return (map_all_things_is_x(level, tp_is_action_left));
}

tree_rootp map_all_things_is_action_right (levelp level)
{
    return (map_all_things_is_x(level, tp_is_action_right));
}

tree_rootp map_all_things_can_walk_through (levelp level)
{
    return (map_all_things_is_x(level, tp_can_walk_through));
}

tree_rootp map_all_things_is_weapon_carry_anim (levelp level)
{
    return (map_all_things_is_x(level, tp_is_weapon_carry_anim));
}

tree_rootp map_all_things_is_spell (levelp level)
{
    return (map_all_things_is_x(level, tp_is_spell));
}

tree_rootp map_all_things_is_hand_item (levelp level)
{
    return (map_all_things_is_x(level, tp_is_hand_item));
}

tree_rootp map_all_things_is_boots (levelp level)
{
    return (map_all_things_is_x(level, tp_is_boots));
}

tree_rootp map_all_things_is_helmet (levelp level)
{
    return (map_all_things_is_x(level, tp_is_helmet));
}

tree_rootp map_all_things_is_armor (levelp level)
{
    return (map_all_things_is_x(level, tp_is_armor));
}

tree_rootp map_all_things_is_given_randomly_at_start (levelp level)
{
    return (map_all_things_is_x(level, tp_is_given_randomly_at_start));
}

tree_rootp map_all_things_is_magical (levelp level)
{
    return (map_all_things_is_x(level, tp_is_magical));
}

tree_rootp map_all_things_is_degradable (levelp level)
{
    return (map_all_things_is_x(level, tp_is_degradable));
}

tree_rootp map_all_things_is_cursed (levelp level)
{
    return (map_all_things_is_x(level, tp_is_cursed));
}

tree_rootp map_all_things_is_animate_only_when_moving (levelp level)
{
    return (map_all_things_is_x(level, tp_is_animate_only_when_moving));
}

tree_rootp map_all_things_is_warm_blooded (levelp level)
{
    return (map_all_things_is_x(level, tp_is_warm_blooded));
}

tree_rootp map_all_things_can_be_enchanted (levelp level)
{
    return (map_all_things_is_x(level, tp_can_be_enchanted));
}

tree_rootp map_all_things_is_stackable (levelp level)
{
    return (map_all_things_is_x(level, tp_is_stackable));
}

tree_rootp map_all_things_is_torch (levelp level)
{
    return (map_all_things_is_x(level, tp_is_torch));
}

tree_rootp map_all_things_is_explosion (levelp level)
{
    return (map_all_things_is_x(level, tp_is_explosion));
}

tree_rootp map_all_things_is_hidden_from_editor (levelp level)
{
    return (map_all_things_is_x(level, tp_is_hidden_from_editor));
}

tree_rootp map_all_things_is_animated (levelp level)
{
    return (map_all_things_is_x(level, tp_is_animated));
}

tree_rootp map_all_things_is_combustable (levelp level)
{
    return (map_all_things_is_x(level, tp_is_combustable));
}

tree_rootp map_all_things_is_projectile (levelp level)
{
    return (map_all_things_is_x(level, tp_is_projectile));
}

tree_rootp map_all_things_is_inactive (levelp level)
{
    return (map_all_things_is_x(level, tp_is_inactive));
}

tree_rootp map_all_things_is_plant (levelp level)
{
    return (map_all_things_is_x(level, tp_is_plant));
}

tree_rootp map_all_things_is_food (levelp level)
{
    return (map_all_things_is_x(level, tp_is_food));
}

tree_rootp map_all_things_is_dungeon_floor (levelp level)
{
    return (map_all_things_is_x(level, tp_is_dungeon_floor));
}

tree_rootp map_all_things_is_exit (levelp level)
{
    return (map_all_things_is_x(level, tp_is_exit));
}
