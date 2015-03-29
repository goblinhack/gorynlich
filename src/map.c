/*
 * Copyright (C) 2011 Neil McGill
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

typedef uint8_t (*map_is_at_callback)(tpp);

#ifdef GORY_DEBUG
FILE *fp = 0;
#endif

static uint8_t map_is_x_at (levelp level,
                            int32_t x, int32_t y, map_is_at_callback callback)
{
    tpp tp;
    widp grid_wid;
    widp w;

    grid_wid = level_get_map(level);
    if (!grid_wid) {
        DIE("no grid wid");
    }

    /*
     * Look for a floor tile where we can place stuff.
     */
    uint8_t z;

    for (z = 0; z < MAP_DEPTH; z++) {
        w = wid_grid_find_first(grid_wid, x, y, z);
        while (w) {
            thingp thing_it = wid_get_thing(w);

            /*
             * No things on level editor, just templates.
             */
            if (!level_is_editor(level)) {
                if (!thing_it) {
                    w = wid_grid_find_next(grid_wid, w, x, y, z);
                    continue;
                }

                if (thing_is_dead_or_dying(thing_it)) {
                    w = wid_grid_find_next(grid_wid, w, x, y, z);
                    continue;
                }
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

static uint8_t map_count_x_at (levelp level,
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
        DIE("no grid wid");
    }

    /*
     * Look for a floor tile where we can place stuff.
     */
    uint8_t z;

    for (z = 0; z < MAP_DEPTH; z++) {
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

uint8_t map_is_key_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_key));
}

uint8_t map_is_rock_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_rock));
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

uint8_t map_is_poison_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_poison));
}

uint8_t map_is_item_unusable_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_item_unusable));
}

uint8_t map_is_valid_for_action_bar_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_valid_for_action_bar));
}

uint8_t map_is_seedpod_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_seedpod));
}

uint8_t map_is_spam_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_spam));
}

uint8_t map_is_door_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_door));
}

uint8_t map_is_pipe_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_pipe));
}

uint8_t map_is_mob_spawner_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_mob_spawner));
}

uint8_t map_is_rrr1_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_rrr1));
}

uint8_t map_is_rrr2_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_rrr2));
}

uint8_t map_is_rrr3_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_rrr3));
}

uint8_t map_is_rrr4_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_rrr4));
}

uint8_t map_is_rrr5_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_rrr5));
}

uint8_t map_is_rrr6_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_rrr6));
}

uint8_t map_is_rrr7_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_rrr7));
}

uint8_t map_is_rrr8_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_rrr8));
}

uint8_t map_is_rrr9_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_rrr9));
}

uint8_t map_is_rrr10_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_rrr10));
}

uint8_t map_is_rrr11_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_rrr11));
}

uint8_t map_is_rrr12_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_rrr12));
}

uint8_t map_is_rrr13_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_rrr13));
}

uint8_t map_is_rrr14_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_rrr14));
}

uint8_t map_is_rrr15_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_rrr15));
}

uint8_t map_is_rrr16_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_rrr16));
}

uint8_t map_is_rrr17_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_rrr17));
}

uint8_t map_is_rrr18_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_rrr18));
}

uint8_t map_is_rrr19_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_rrr19));
}

uint8_t map_is_rrr20_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_rrr20));
}

uint8_t map_is_rrr21_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_rrr21));
}

uint8_t map_is_rrr22_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_rrr22));
}

uint8_t map_is_rrr23_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_rrr23));
}

uint8_t map_is_rrr24_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_rrr24));
}

uint8_t map_is_action_bridge_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_action_bridge));
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

uint8_t map_is_floor_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, tp_is_floor));
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
        DIE("no grid wid");
    }

    /*
     * Look for a floor tile where we can place stuff.
     */
    uint8_t z;

    for (z = 0; z < MAP_DEPTH; z++) {
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

thingp map_thing_is_poison_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_poison));
}

thingp map_thing_is_item_unusable_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_item_unusable));
}

thingp map_thing_is_valid_for_action_bar_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_valid_for_action_bar));
}

thingp map_thing_is_seedpod_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_seedpod));
}

thingp map_thing_is_spam_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_spam));
}

thingp map_thing_is_door_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_door));
}

thingp map_thing_is_pipe_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_pipe));
}

thingp map_thing_is_mob_spawner_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_mob_spawner));
}

thingp map_thing_is_rrr1_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_rrr1));
}

thingp map_thing_is_rrr2_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_rrr2));
}

thingp map_thing_is_rrr3_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_rrr3));
}

thingp map_thing_is_rrr4_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_rrr4));
}

thingp map_thing_is_rrr5_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_rrr5));
}

thingp map_thing_is_rrr6_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_rrr6));
}

thingp map_thing_is_rrr7_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_rrr7));
}

thingp map_thing_is_rrr8_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_rrr8));
}

thingp map_thing_is_rrr9_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_rrr9));
}

thingp map_thing_is_rrr10_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_rrr10));
}

thingp map_thing_is_rrr11_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_rrr11));
}

thingp map_thing_is_rrr12_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_rrr12));
}

thingp map_thing_is_rrr13_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_rrr13));
}

thingp map_thing_is_rrr14_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_rrr14));
}

thingp map_thing_is_rrr15_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_rrr15));
}

thingp map_thing_is_rrr16_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_rrr16));
}

thingp map_thing_is_rrr17_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_rrr17));
}

thingp map_thing_is_rrr18_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_rrr18));
}

thingp map_thing_is_rrr19_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_rrr19));
}

thingp map_thing_is_rrr20_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_rrr20));
}

thingp map_thing_is_rrr21_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_rrr21));
}

thingp map_thing_is_rrr22_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_rrr22));
}

thingp map_thing_is_rrr23_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_rrr23));
}

thingp map_thing_is_rrr24_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_rrr24));
}

thingp map_thing_is_action_bridge_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_action_bridge));
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

thingp map_thing_is_floor_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, tp_is_floor));
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
        DIE("no grid wid");
    }

    /*
     * Look for a floor tile where we can place stuff.
     */
    uint8_t z;

    for (z = 0; z < MAP_DEPTH; z++) {
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
                        DIE("insert thingp %p", thing_it);
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

tree_rootp map_all_things_is_poison_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_poison));
}

tree_rootp map_all_things_is_item_unusable_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_item_unusable));
}

tree_rootp map_all_things_is_valid_for_action_bar_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_valid_for_action_bar));
}

tree_rootp map_all_things_is_seedpod_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_seedpod));
}

tree_rootp map_all_things_is_spam_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_spam));
}

tree_rootp map_all_things_is_door_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_door));
}

tree_rootp map_all_things_is_pipe_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_pipe));
}

tree_rootp map_all_things_is_mob_spawner_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_mob_spawner));
}

tree_rootp map_all_things_is_rrr1_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_rrr1));
}

tree_rootp map_all_things_is_rrr2_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_rrr2));
}

tree_rootp map_all_things_is_rrr3_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_rrr3));
}

tree_rootp map_all_things_is_rrr4_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_rrr4));
}

tree_rootp map_all_things_is_rrr5_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_rrr5));
}

tree_rootp map_all_things_is_rrr6_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_rrr6));
}

tree_rootp map_all_things_is_rrr7_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_rrr7));
}

tree_rootp map_all_things_is_rrr8_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_rrr8));
}

tree_rootp map_all_things_is_rrr9_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_rrr9));
}

tree_rootp map_all_things_is_rrr10_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_rrr10));
}

tree_rootp map_all_things_is_rrr11_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_rrr11));
}

tree_rootp map_all_things_is_rrr12_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_rrr12));
}

tree_rootp map_all_things_is_rrr13_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_rrr13));
}

tree_rootp map_all_things_is_rrr14_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_rrr14));
}

tree_rootp map_all_things_is_rrr15_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_rrr15));
}

tree_rootp map_all_things_is_rrr16_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_rrr16));
}

tree_rootp map_all_things_is_rrr17_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_rrr17));
}

tree_rootp map_all_things_is_rrr18_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_rrr18));
}

tree_rootp map_all_things_is_rrr19_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_rrr19));
}

tree_rootp map_all_things_is_rrr20_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_rrr20));
}

tree_rootp map_all_things_is_rrr21_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_rrr21));
}

tree_rootp map_all_things_is_rrr22_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_rrr22));
}

tree_rootp map_all_things_is_rrr23_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_rrr23));
}

tree_rootp map_all_things_is_rrr24_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_rrr24));
}

tree_rootp map_all_things_is_action_bridge_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_action_bridge));
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

tree_rootp map_all_things_is_floor_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_floor));
}

tree_rootp map_all_things_is_exit_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, tp_is_exit));
}

static tpp map_find_x_at_depth (levelp level,
                                int32_t x, int32_t y,
                                map_is_at_callback callback,
                                uint8_t z,
                                widp *wout)
{
    tpp tp;
    widp grid_wid;
    widp w;

    grid_wid = level_get_map(level);
    if (!grid_wid) {
        DIE("no grid wid");
    }

    w = wid_grid_find_first(grid_wid, x, y, z);
    while (w) {
        thingp thing_it = wid_get_thing(w);

        /*
         * No things on level editor, just templates.
         */
        if (!level_is_editor(level)) {
            /*
             * Need to filter dead things so map fixup can no longer see wall 
             * tiles that are in the process of being destroyed.
             */
            if (!thing_it) {
                w = wid_grid_find_next(grid_wid, w, x, y, z);
                continue;
            }

            if (thing_is_dead_or_dying(thing_it)) {
                w = wid_grid_find_next(grid_wid, w, x, y, z);
                continue;
            }
        }

        tp = wid_get_thing_template(w);
        if (tp) {
            if ((*callback)(tp)) {
                if (wout) {
                    *wout = w;
                }

                return (tp);
            }
        }

        w = wid_grid_find_next(grid_wid, w, x, y, z);
    }

    return (0);
}

static tpp map_find_x_at (levelp level,
                                      int32_t x, int32_t y,
                                      map_is_at_callback callback,
                                      widp *wout)
{
    tpp tp;
    widp grid_wid;
    widp w;

    grid_wid = level_get_map(level);
    if (!grid_wid) {
        DIE("no grid wid");
    }

    uint8_t z;

    for (z = 0; z < MAP_DEPTH; z++) {
        w = wid_grid_find_first(grid_wid, x, y, z);
        while (w) {
            thingp thing_it = wid_get_thing(w);

            /*
             * No things on level editor, just templates.
             */
            if (!level_is_editor(level)) {
                /*
                 * Need to filter dead things so map fixup can no longer see 
                 * wall tiles that are in the process of being destroyed.
                 */
                if (!thing_it) {
                    w = wid_grid_find_next(grid_wid, w, x, y, z);
                    continue;
                }

                if (thing_is_dead_or_dying(thing_it)) {
                    w = wid_grid_find_next(grid_wid, w, x, y, z);
                    continue;
                }
            }

            tp = wid_get_thing_template(w);
            if (tp) {
                if ((*callback)(tp)) {
                    if (wout) {
                        *wout = w;
                    }

                    return (tp);
                }
            }

            w = wid_grid_find_next(grid_wid, w, x, y, z);
        }
    }

    return (0);
}

tpp map_find_player_at (levelp level,
                                    int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_player, w));
}

tpp map_find_monst_at (levelp level,
                                   int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_monst, w));
}

tpp map_find_wall_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at_depth(level, x, y, 
                                tp_is_wall, 
                                MAP_DEPTH_WALL, w));
}

tpp map_find_rock_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at_depth(level, x, y, 
                                tp_is_rock, 
                                MAP_DEPTH_WALL, w));
}

tpp map_find_key_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_key, w));
}

tpp map_find_xxx3_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_rock, w));
}

tpp map_find_blocks_light_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_shadow_caster, w));
}

tpp map_find_xxx5_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_weapon, w));
}

tpp map_find_xxx6_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_treasure, w));
}

tpp map_find_xxx7_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_wearable, w));
}

tpp map_find_is_fragile_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_fragile, w));
}

tpp map_find_is_star_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_star, w));
}

tpp map_find_key1_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_weapon_swing_effect, w));
}

tpp map_find_light_source_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_light_source, w));
}

tpp map_find_candle_light_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_candle_light, w));
}

tpp map_find_star_green_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_cats_eyes, w));
}

tpp map_find_star_cyan_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_fire, w));
}

tpp map_find_star_black_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_ring, w));
}

tpp map_find_star_purple_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_animation, w));
}

tpp map_find_explosion_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_animation, w));
}

tpp map_find_spikes_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_poison, w));
}

tpp map_find_is_item_unusable_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_item_unusable, w));
}

tpp map_find_is_valid_for_action_bar_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_valid_for_action_bar, w));
}

tpp map_find_seedpod_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_seedpod, w));
}

tpp map_find_spam_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_spam, w));
}

tpp map_find_door_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_door, w));
}

tpp map_find_pipe_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_pipe, w));
}

tpp map_find_generator_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_mob_spawner, w));
}

tpp map_find_rrr1_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_rrr1, w));
}

tpp map_find_rrr2_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_rrr2, w));
}

tpp map_find_rrr3_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_rrr3, w));
}

tpp map_find_rrr4_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_rrr4, w));
}

tpp map_find_rrr5_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_rrr5, w));
}

tpp map_find_rrr6_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_rrr6, w));
}

tpp map_find_rrr7_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_rrr7, w));
}

tpp map_find_rrr8_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_rrr8, w));
}

tpp map_find_rrr9_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_rrr9, w));
}

tpp map_find_rrr10_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_rrr10, w));
}

tpp map_find_rrr11_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_rrr11, w));
}

tpp map_find_rrr12_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_rrr12, w));
}

tpp map_find_rrr13_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_rrr13, w));
}

tpp map_find_rrr14_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_rrr14, w));
}

tpp map_find_rrr15_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_rrr15, w));
}

tpp map_find_rrr16_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_rrr16, w));
}

tpp map_find_rrr17_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_rrr17, w));
}

tpp map_find_rrr18_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_rrr18, w));
}

tpp map_find_rrr19_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_rrr19, w));
}

tpp map_find_rrr20_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_rrr20, w));
}

tpp map_find_rrr21_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_rrr21, w));
}

tpp map_find_rrr22_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_rrr22, w));
}

tpp map_find_rrr23_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_rrr23, w));
}

tpp map_find_rrr24_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_rrr24, w));
}

tpp map_find_action_bridge_at (levelp level, int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_action_bridge, w));
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

tpp map_find_can_walk_through_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_can_walk_through, w));
}

tpp map_find_weapon_carry_anim_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_weapon_carry_anim, w));
}

tpp map_find_spell_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_spell, w));
}

tpp map_find_given_randomly_at_start_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_given_randomly_at_start, w));
}

tpp map_find_magical_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_magical, w));
}

tpp map_find_degradable_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_degradable, w));
}

tpp map_find_cursed_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_cursed, w));
}

tpp map_find_animate_only_when_moving_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_animate_only_when_moving, w));
}

tpp map_find_torch_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_torch, w));
}

tpp map_find_is_explosion_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_explosion, w));
}

tpp map_find_hidden_from_editor_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_hidden_from_editor, w));
}

tpp map_find_is_projectile_at (levelp level,
                                   int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_projectile, w));
}

tpp map_find_item_perma_at (levelp level,
                                   int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_inactive, w));
}

tpp map_find_is_ring_at (levelp level,
                                   int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_ring, w));
}

tpp map_find_plant_at (levelp level,
                                   int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_plant, w));
}

tpp map_find_food_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_food, w));
}

tpp map_find_floor_at (levelp level,
                                   int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, tp_is_floor, w));
}

tpp map_find_exit_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
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

            if (map_find_wall_at(level, x, y, &w)) {
#ifdef GORY_DEBUG
if (level != server_level)
                fprintf(fp,"x");
#endif
                mywid = w;
            } else if (map_find_pipe_at(level, x, y, &w)) {
                mywid = w;
            } else if (map_find_door_at(level, x, y, &w)) {
#ifdef GORY_DEBUG
if (level != server_level)
                fprintf(fp,"D");
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

            if (wid_is_moving(mywid)) {
                continue;
            }

            memset(nbrs, 0, sizeof(nbrs));

            for (dx = -1; dx <= 1; dx++) {
                for (dy = -1; dy <= 1; dy++) {
                    widp w;
                    tpp tp;

                    if (map_find_wall_at(level, x, y, &w)) {
                        tp = map_find_wall_at(level, x + dx, y + dy, &w);
                        if (wid_is_moving(w)) {
                            continue;
                        }
                        nbrs[dx + 1][dy + 1] = tp;

                    }

                    if (map_find_pipe_at(level, x, y, &w)) {
                        tp = map_find_pipe_at(level, x + dx, y + dy, &w);
                        if (wid_is_moving(w)) {
                            continue;
                        }

                        nbrs[dx + 1][dy + 1] = tp;
                    }

                    if (map_find_door_at(level, x, y, &w)) {
                        tp = map_find_door_at(level, x + dx, y + dy, &w);
                        if (wid_is_moving(w)) {
                            continue;
                        }

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
                DIE("%u%u%u %u%u%u %u%u%u not handled",
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

            thing_tilep thing_tile = thing_tile_find(t, index, &tile);
            if (!thing_tile) {
                index = IS_JOIN_BLOCK;

                thing_tile = thing_tile_find(t, index, &tile);
                if (!thing_tile) {
                    index = IS_JOIN_NODE;

                    thing_tile = thing_tile_find(t, index, &tile);
                    if (!thing_tile) {
                        DIE("no joinable tile for %s", tp_name(t));
                    }
                }
            }

            if (!tile) {
                DIE("no tile for %s", tp_name(t));
            }

            const char *tilename = thing_tile_name(thing_tile);

            if (!tilename) {
                DIE("no tilename for %s", tp_name(e));
            }

            /*
             * If an existing thing, make sure and send an update if say
             * a door or wall is destroyed.
             */
            {
                tilep existing_tile = wid_get_tile(mywid);

                if (existing_tile && (existing_tile != tile)) {
                    thingp t = wid_get_thing(mywid);
                    if (t && t->on_server) {
                        thing_update(t);
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

static uint32_t level_count_is_x (levelp level, map_is_at_callback callback)
{
    uint32_t count;
    int32_t x;
    int32_t y;

    count = 0;

    for (x = 0; x < MAP_WIDTH; x++) {
        for (y = 0; y < MAP_HEIGHT; y++) {
            count += map_count_x_at(level, x, y, callback);
        }
    }

    return (count);
}

static char this_pipe[MAP_WIDTH][MAP_HEIGHT];

static void pipe_flood (levelp level, int32_t x, int32_t y)
{
    if (!map_thing_is_pipe_at(level, x, y)) {
        return;
    }

    if (this_pipe[x][y] == '+') {
        return;
    }

    this_pipe[x][y] = '+';

    pipe_flood(level, x-1, y);
    pipe_flood(level, x+1, y);
    pipe_flood(level, x, y-1);
    pipe_flood(level, x, y+1);
}

uint8_t level_pipe_find_exit (levelp level,
                              int32_t ix, int32_t iy,
                              int32_t *exit_x, int32_t *exit_y)
{
    int32_t exits_x[MAP_WIDTH];
    int32_t exits_y[MAP_WIDTH];
    int32_t nexits;
    int32_t x;
    int32_t y;

    memset(this_pipe, ' ', sizeof(this_pipe));
    memset(exits_x, 0, sizeof(exits_x));
    memset(exits_y, 0, sizeof(exits_y));
    nexits = 0;

    pipe_flood(level, ix, iy);

    for (x = 1; x < MAP_WIDTH-1; x++) {
        for (y = 1; y < MAP_HEIGHT-1; y++) {

            if ((x == ix) && (y == iy)) {
                continue;
            }

            if (this_pipe[x][y] != '+') {
                continue;
            }

            if (level->end_pipe.walls[x][y] != ' ') {
                exits_x[nexits] = x;
                exits_y[nexits] = y;
                nexits++;
            }
        }
    }

    if (!nexits) {
        return (false);
    }

    int32_t exit = myrand() % nexits;

    *exit_x = exits_x[exit];
    *exit_y = exits_y[exit];

    return (true);
}

static thingp this_door[MAP_WIDTH][MAP_HEIGHT];

static void door_flood (levelp level, int32_t x, int32_t y)
{
    if (this_door[x][y]) {
        return;
    }

    if (!(this_door[x][y] = map_thing_is_door_at(level, x, y))) {
        this_door[x][y] = (thingp) (void*)-1;
        return;
    }

    door_flood(level, x-1, y);
    door_flood(level, x+1, y);
    door_flood(level, x, y-1);
    door_flood(level, x, y+1);
}

void level_open_door (levelp level, int32_t ix, int32_t iy)
{
    int32_t x;
    int32_t y;

    memset(this_door, 0, sizeof(this_door));

    door_flood(level, ix, iy);

    for (x = 0; x < MAP_WIDTH; x++) {
        for (y = 0; y < MAP_HEIGHT; y++) {
            if (!this_door[x][y]) {
                continue;
            }

            if (this_door[x][y] == (void*)-1) {
                continue;
            }

            thing_dead(this_door[x][y], 0 /* killer */, "open");
        }
    }

    level_set_monst_map_treat_doors_as_passable(level);
    level_set_monst_map_treat_doors_as_walls(level);
    level_set_player_map_treat_doors_as_walls(level);

    /*
     * Send the update quickly to the client. Don't wait for the things to
     * tick. The doors should be on the active list now anyway as they were
     * killed above.
     */
    socket_server_tx_map_update(0 /* all clients */, server_active_things,
                                "open door");
}

uint32_t level_count_is_player (levelp level)
{
    return (level_count_is_x(level, tp_is_player));
}

uint32_t level_count_is_monst (levelp level)
{
    return (level_count_is_x(level, tp_is_monst));
}

uint32_t level_count_is_wall (levelp level)
{
    return (level_count_is_x(level, tp_is_wall));
}

uint32_t level_count_is_key (levelp level)
{
    return (level_count_is_x(level, tp_is_key));
}

uint32_t level_count_is_rock (levelp level)
{
    return (level_count_is_x(level, tp_is_rock));
}

uint32_t level_count_is_shadow_caster (levelp level)
{
    return (level_count_is_x(level, tp_is_shadow_caster));
}

uint32_t level_count_is_weapon (levelp level)
{
    return (level_count_is_x(level, tp_is_weapon));
}

uint32_t level_count_is_treasure (levelp level)
{
    return (level_count_is_x(level, tp_is_treasure));
}

uint32_t level_count_is_wearable (levelp level)
{
    return (level_count_is_x(level, tp_is_wearable));
}

uint32_t level_count_is_fragile (levelp level)
{
    return (level_count_is_x(level, tp_is_fragile));
}

uint32_t level_count_is_star (levelp level)
{
    return (level_count_is_x(level, tp_is_star));
}

uint32_t level_count_is_animated_no_dir (levelp level)
{
    return (level_count_is_x(level, tp_is_animated_no_dir));
}

uint32_t level_count_is_weapon_swing_effect (levelp level)
{
    return (level_count_is_x(level, tp_is_weapon_swing_effect));
}

uint32_t level_count_is_light_source (levelp level)
{
    return (level_count_is_x(level, tp_is_light_source));
}

uint32_t level_count_is_candle_light (levelp level)
{
    return (level_count_is_x(level, tp_is_candle_light));
}

uint32_t level_count_is_cats_eyes (levelp level)
{
    return (level_count_is_x(level, tp_is_cats_eyes));
}

uint32_t level_count_is_fire (levelp level)
{
    return (level_count_is_x(level, tp_is_fire));
}

uint32_t level_count_is_ring (levelp level)
{
    return (level_count_is_x(level, tp_is_ring));
}

uint32_t level_count_is_animation (levelp level)
{
    return (level_count_is_x(level, tp_is_animation));
}

uint32_t level_count_is_poison (levelp level)
{
    return (level_count_is_x(level, tp_is_poison));
}

uint32_t level_count_is_item_unusable (levelp level)
{
    return (level_count_is_x(level, tp_is_item_unusable));
}

uint32_t level_count_is_valid_for_action_bar (levelp level)
{
    return (level_count_is_x(level, tp_is_valid_for_action_bar));
}

uint32_t level_count_is_seedpod (levelp level)
{
    return (level_count_is_x(level, tp_is_seedpod));
}

uint32_t level_count_is_spam (levelp level)
{
    return (level_count_is_x(level, tp_is_spam));
}

uint32_t level_count_is_door (levelp level)
{
    return (level_count_is_x(level, tp_is_door));
}

uint32_t level_count_is_pipe (levelp level)
{
    return (level_count_is_x(level, tp_is_pipe));
}

uint32_t level_count_is_mob_spawner (levelp level)
{
    return (level_count_is_x(level, tp_is_mob_spawner));
}

uint32_t level_count_is_rrr1 (levelp level)
{
    return (level_count_is_x(level, tp_is_rrr1));
}

uint32_t level_count_is_rrr2 (levelp level)
{
    return (level_count_is_x(level, tp_is_rrr2));
}

uint32_t level_count_is_rrr3 (levelp level)
{
    return (level_count_is_x(level, tp_is_rrr3));
}

uint32_t level_count_is_rrr4 (levelp level)
{
    return (level_count_is_x(level, tp_is_rrr4));
}

uint32_t level_count_is_rrr5 (levelp level)
{
    return (level_count_is_x(level, tp_is_rrr5));
}

uint32_t level_count_is_rrr6 (levelp level)
{
    return (level_count_is_x(level, tp_is_rrr6));
}

uint32_t level_count_is_rrr7 (levelp level)
{
    return (level_count_is_x(level, tp_is_rrr7));
}

uint32_t level_count_is_rrr8 (levelp level)
{
    return (level_count_is_x(level, tp_is_rrr8));
}

uint32_t level_count_is_rrr9 (levelp level)
{
    return (level_count_is_x(level, tp_is_rrr9));
}

uint32_t level_count_is_rrr10 (levelp level)
{
    return (level_count_is_x(level, tp_is_rrr10));
}

uint32_t level_count_is_rrr11 (levelp level)
{
    return (level_count_is_x(level, tp_is_rrr11));
}

uint32_t level_count_is_rrr12 (levelp level)
{
    return (level_count_is_x(level, tp_is_rrr12));
}

uint32_t level_count_is_rrr13 (levelp level)
{
    return (level_count_is_x(level, tp_is_rrr13));
}

uint32_t level_count_is_rrr14 (levelp level)
{
    return (level_count_is_x(level, tp_is_rrr14));
}

uint32_t level_count_is_rrr15 (levelp level)
{
    return (level_count_is_x(level, tp_is_rrr15));
}

uint32_t level_count_is_rrr16 (levelp level)
{
    return (level_count_is_x(level, tp_is_rrr16));
}

uint32_t level_count_is_rrr17 (levelp level)
{
    return (level_count_is_x(level, tp_is_rrr17));
}

uint32_t level_count_is_rrr18 (levelp level)
{
    return (level_count_is_x(level, tp_is_rrr18));
}

uint32_t level_count_is_rrr19 (levelp level)
{
    return (level_count_is_x(level, tp_is_rrr19));
}

uint32_t level_count_is_rrr20 (levelp level)
{
    return (level_count_is_x(level, tp_is_rrr20));
}

uint32_t level_count_is_rrr21 (levelp level)
{
    return (level_count_is_x(level, tp_is_rrr21));
}

uint32_t level_count_is_rrr22 (levelp level)
{
    return (level_count_is_x(level, tp_is_rrr22));
}

uint32_t level_count_is_rrr23 (levelp level)
{
    return (level_count_is_x(level, tp_is_rrr23));
}

uint32_t level_count_is_rrr24 (levelp level)
{
    return (level_count_is_x(level, tp_is_rrr24));
}

uint32_t level_count_is_rrr25 (levelp level)
{
    return (level_count_is_x(level, tp_is_rrr25));
}

uint32_t level_count_is_rrr26 (levelp level)
{
    return (level_count_is_x(level, tp_is_rrr26));
}

uint32_t level_count_is_rrr27 (levelp level)
{
    return (level_count_is_x(level, tp_is_rrr27));
}

uint32_t level_count_is_rrr28 (levelp level)
{
    return (level_count_is_x(level, tp_is_rrr28));
}

uint32_t level_count_is_rrr29 (levelp level)
{
    return (level_count_is_x(level, tp_is_rrr29));
}

uint32_t level_count_is_action (levelp level)
{
    return (level_count_is_x(level, tp_is_action));
}

uint32_t level_count_can_walk_through (levelp level)
{
    return (level_count_is_x(level, tp_can_walk_through));
}

uint32_t level_count_is_weapon_carry_anim (levelp level)
{
    return (level_count_is_x(level, tp_is_weapon_carry_anim));
}

uint32_t level_count_is_spell (levelp level)
{
    return (level_count_is_x(level, tp_is_spell));
}

uint32_t level_count_is_hand_item (levelp level)
{
    return (level_count_is_x(level, tp_is_hand_item));
}

uint32_t level_count_is_boots (levelp level)
{
    return (level_count_is_x(level, tp_is_boots));
}

uint32_t level_count_is_helmet (levelp level)
{
    return (level_count_is_x(level, tp_is_helmet));
}

uint32_t level_count_is_armor (levelp level)
{
    return (level_count_is_x(level, tp_is_armor));
}

uint32_t level_count_is_given_randomly_at_start (levelp level)
{
    return (level_count_is_x(level, tp_is_given_randomly_at_start));
}

uint32_t level_count_is_magical (levelp level)
{
    return (level_count_is_x(level, tp_is_magical));
}

uint32_t level_count_is_degradable (levelp level)
{
    return (level_count_is_x(level, tp_is_degradable));
}

uint32_t level_count_is_cursed (levelp level)
{
    return (level_count_is_x(level, tp_is_cursed));
}

uint32_t level_count_is_animate_only_when_moving (levelp level)
{
    return (level_count_is_x(level, tp_is_animate_only_when_moving));
}

uint32_t level_count_is_warm_blooded (levelp level)
{
    return (level_count_is_x(level, tp_is_warm_blooded));
}

uint32_t level_count_can_be_enchanted (levelp level)
{
    return (level_count_is_x(level, tp_can_be_enchanted));
}

uint32_t level_count_is_stackable (levelp level)
{
    return (level_count_is_x(level, tp_is_stackable));
}

uint32_t level_count_is_torch (levelp level)
{
    return (level_count_is_x(level, tp_is_torch));
}

uint32_t level_count_is_explosion (levelp level)
{
    return (level_count_is_x(level, tp_is_explosion));
}

uint32_t level_count_is_hidden_from_editor (levelp level)
{
    return (level_count_is_x(level, tp_is_hidden_from_editor));
}

uint32_t level_count_is_combustable (levelp level)
{
    return (level_count_is_x(level, tp_is_combustable));
}

uint32_t level_count_is_projectile (levelp level)
{
    return (level_count_is_x(level, tp_is_projectile));
}

uint32_t level_count_is_inactive (levelp level)
{
    return (level_count_is_x(level, tp_is_inactive));
}

uint32_t level_count_is_item_is_hidden (levelp level)
{
    return (level_count_is_x(level, tp_is_ring));
}

uint32_t level_count_is_plant (levelp level)
{
    return (level_count_is_x(level, tp_is_plant));
}

uint32_t level_count_is_food (levelp level)
{
    return (level_count_is_x(level, tp_is_food));
}

uint32_t level_count_is_floor (levelp level)
{
    return (level_count_is_x(level, tp_is_floor));
}

uint32_t level_count_is_exit (levelp level)
{
    return (level_count_is_x(level, tp_is_exit));
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
        DIE("no grid wid");
    }

    uint8_t z;

    for (z = 0; z < MAP_DEPTH; z++) {
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
                                DIE("insert thingp %p", thing_it);
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

tree_rootp map_all_things_is_poison (levelp level)
{
    return (map_all_things_is_x(level, tp_is_poison));
}

tree_rootp map_all_things_is_item_unusable (levelp level)
{
    return (map_all_things_is_x(level, tp_is_item_unusable));
}

tree_rootp map_all_things_is_valid_for_action_bar (levelp level)
{
    return (map_all_things_is_x(level, tp_is_valid_for_action_bar));
}

tree_rootp map_all_things_is_seedpod (levelp level)
{
    return (map_all_things_is_x(level, tp_is_seedpod));
}

tree_rootp map_all_things_is_spam (levelp level)
{
    return (map_all_things_is_x(level, tp_is_spam));
}

tree_rootp map_all_things_is_door (levelp level)
{
    return (map_all_things_is_x(level, tp_is_door));
}

tree_rootp map_all_things_is_pipe (levelp level)
{
    return (map_all_things_is_x(level, tp_is_pipe));
}

tree_rootp map_all_things_is_mob_spawner (levelp level)
{
    return (map_all_things_is_x(level, tp_is_mob_spawner));
}

tree_rootp map_all_things_is_rrr1 (levelp level)
{
    return (map_all_things_is_x(level, tp_is_rrr1));
}

tree_rootp map_all_things_is_rrr2 (levelp level)
{
    return (map_all_things_is_x(level, tp_is_rrr2));
}

tree_rootp map_all_things_is_rrr3 (levelp level)
{
    return (map_all_things_is_x(level, tp_is_rrr3));
}

tree_rootp map_all_things_is_rrr4 (levelp level)
{
    return (map_all_things_is_x(level, tp_is_rrr4));
}

tree_rootp map_all_things_is_rrr5 (levelp level)
{
    return (map_all_things_is_x(level, tp_is_rrr5));
}

tree_rootp map_all_things_is_rrr6 (levelp level)
{
    return (map_all_things_is_x(level, tp_is_rrr6));
}

tree_rootp map_all_things_is_rrr7 (levelp level)
{
    return (map_all_things_is_x(level, tp_is_rrr7));
}

tree_rootp map_all_things_is_rrr8 (levelp level)
{
    return (map_all_things_is_x(level, tp_is_rrr8));
}

tree_rootp map_all_things_is_rrr9 (levelp level)
{
    return (map_all_things_is_x(level, tp_is_rrr9));
}

tree_rootp map_all_things_is_rrr10 (levelp level)
{
    return (map_all_things_is_x(level, tp_is_rrr10));
}

tree_rootp map_all_things_is_rrr11 (levelp level)
{
    return (map_all_things_is_x(level, tp_is_rrr11));
}

tree_rootp map_all_things_is_rrr12 (levelp level)
{
    return (map_all_things_is_x(level, tp_is_rrr12));
}

tree_rootp map_all_things_is_rrr13 (levelp level)
{
    return (map_all_things_is_x(level, tp_is_rrr13));
}

tree_rootp map_all_things_is_rrr14 (levelp level)
{
    return (map_all_things_is_x(level, tp_is_rrr14));
}

tree_rootp map_all_things_is_rrr15 (levelp level)
{
    return (map_all_things_is_x(level, tp_is_rrr15));
}

tree_rootp map_all_things_is_rrr16 (levelp level)
{
    return (map_all_things_is_x(level, tp_is_rrr16));
}

tree_rootp map_all_things_is_rrr17 (levelp level)
{
    return (map_all_things_is_x(level, tp_is_rrr17));
}

tree_rootp map_all_things_is_rrr18 (levelp level)
{
    return (map_all_things_is_x(level, tp_is_rrr18));
}

tree_rootp map_all_things_is_rrr19 (levelp level)
{
    return (map_all_things_is_x(level, tp_is_rrr19));
}

tree_rootp map_all_things_is_rrr20 (levelp level)
{
    return (map_all_things_is_x(level, tp_is_rrr20));
}

tree_rootp map_all_things_is_rrr21 (levelp level)
{
    return (map_all_things_is_x(level, tp_is_rrr21));
}

tree_rootp map_all_things_is_rrr22 (levelp level)
{
    return (map_all_things_is_x(level, tp_is_rrr22));
}

tree_rootp map_all_things_is_rrr23 (levelp level)
{
    return (map_all_things_is_x(level, tp_is_rrr23));
}

tree_rootp map_all_things_is_rrr24 (levelp level)
{
    return (map_all_things_is_x(level, tp_is_rrr24));
}

tree_rootp map_all_things_is_action_bridge (levelp level)
{
    return (map_all_things_is_x(level, tp_is_action_bridge));
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

tree_rootp map_all_things_is_floor (levelp level)
{
    return (map_all_things_is_x(level, tp_is_floor));
}

tree_rootp map_all_things_is_exit (levelp level)
{
    return (map_all_things_is_x(level, tp_is_exit));
}
