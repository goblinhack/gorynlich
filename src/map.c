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
#include "socket.h"
#include "wid_game_map_server.h"

typedef uint8_t (*map_is_at_callback)(thing_templatep);

#ifdef DEBUG
FILE *fp = 0;
#endif

static uint8_t map_is_x_at (levelp level,
                            int32_t x, int32_t y, map_is_at_callback callback)
{
    thing_templatep thing_template;
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

                if (thing_is_dead(thing_it)) {
                    w = wid_grid_find_next(grid_wid, w, x, y, z);
                    continue;
                }
            }

            thing_template = wid_get_thing_template(w);
            if (thing_template) {
                if ((*callback)(thing_template)) {
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
    thing_templatep thing_template;
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
            thing_template = wid_get_thing_template(w);
            if (thing_template) {
                if ((*callback)(thing_template)) {
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
    return (map_is_x_at(level, x, y, thing_template_is_player));
}

uint8_t map_is_monst_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_monst));
}

uint8_t map_is_wall_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_wall));
}

uint8_t map_is_key_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_key));
}

uint8_t map_is_xxx3_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_xxx3));
}

uint8_t map_is_xxx4_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_xxx4));
}

uint8_t map_is_weapon_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_weapon));
}

uint8_t map_is_treasure_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_treasure));
}

uint8_t map_is_wearable_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_wearable));
}

uint8_t map_is_fragile_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_fragile));
}

uint8_t map_is_star_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_star));
}

uint8_t map_is_animated_no_dir_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_animated_no_dir));
}

uint8_t map_is_weapon_swing_effect_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_weapon_swing_effect));
}

uint8_t map_is_key2_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_key2));
}

uint8_t map_is_key3_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_key3));
}

uint8_t map_is_fire_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_fire));
}

uint8_t map_is_poison_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_poison));
}

uint8_t map_is_item_unusable_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_item_unusable));
}

uint8_t map_is_valid_for_shortcut_key_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_valid_for_shortcut_key));
}

uint8_t map_is_seedpod_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_seedpod));
}

uint8_t map_is_spam_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_spam));
}

uint8_t map_is_door_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_door));
}

uint8_t map_is_pipe_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_pipe));
}

uint8_t map_is_mob_spawner_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_mob_spawner));
}

uint8_t map_is_scarable_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_scarable));
}

uint8_t map_is_explosion_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_explosion));
}

uint8_t map_is_hidden_from_editor_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_hidden_from_editor));
}

uint8_t map_is_animated_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_animated));
}

uint8_t map_is_combustable_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_combustable));
}

uint8_t map_is_projectile_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_projectile));
}

uint8_t map_is_boring_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_boring));
}

uint8_t map_is_plant_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_plant));
}

uint8_t map_is_food_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_food));
}

uint8_t map_is_floor_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_floor));
}

uint8_t map_is_exit_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_exit));
}

static thingp map_thing_is_x_at (levelp level,
                                 int32_t x, int32_t y,
                                 map_is_at_callback callback)
{
    thing_templatep thing_template;
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

            if (thing_is_dead(thing_it)) {
                w = wid_grid_find_next(grid_wid, w, x, y, z);
                continue;
            }

            thing_template = wid_get_thing_template(w);
            if (thing_template) {
                if ((*callback)(thing_template)) {
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
    return (map_thing_is_x_at(level, x, y, thing_template_is_player));
}

thingp map_thing_is_monst_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_monst));
}

thingp map_thing_is_wall_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_wall));
}

thingp map_thing_is_key_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_key));
}

thingp map_thing_is_xxx3_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_xxx3));
}

thingp map_thing_is_xxx4_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_xxx4));
}

thingp map_thing_is_weapon_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_weapon));
}

thingp map_thing_is_treasure_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_treasure));
}

thingp map_thing_is_wearable_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_wearable));
}

thingp map_thing_is_fragile_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_fragile));
}

thingp map_thing_is_star_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_star));
}

thingp map_thing_is_animated_no_dir_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_animated_no_dir));
}

thingp map_thing_is_weapon_swing_effect_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_weapon_swing_effect));
}

thingp map_thing_is_key2_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_key2));
}

thingp map_thing_is_key3_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_key3));
}

thingp map_thing_is_fire_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_fire));
}

thingp map_thing_is_ring_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_ring));
}

thingp map_thing_is_animation_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_animation));
}

thingp map_thing_is_poison_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_poison));
}

thingp map_thing_is_item_unusable_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_item_unusable));
}

thingp map_thing_is_valid_for_shortcut_key_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_valid_for_shortcut_key));
}

thingp map_thing_is_seedpod_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_seedpod));
}

thingp map_thing_is_spam_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_spam));
}

thingp map_thing_is_door_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_door));
}

thingp map_thing_is_pipe_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_pipe));
}

thingp map_thing_is_mob_spawner_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_mob_spawner));
}

thingp map_thing_is_scarable_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_scarable));
}

thingp map_thing_is_explosion_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_explosion));
}

thingp map_thing_is_hidden_from_editor_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_hidden_from_editor));
}

thingp map_thing_is_animated_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_animated));
}

thingp map_thing_is_combustable_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_combustable));
}

thingp map_thing_is_projectile_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_projectile));
}

thingp map_thing_is_boring_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_boring));
}

thingp map_thing_is_plant_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_plant));
}

thingp map_thing_is_food_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_food));
}

thingp map_thing_is_floor_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_floor));
}

thingp map_thing_is_exit_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_exit));
}

static tree_rootp map_all_things_is_x_at (levelp level,
                                          int32_t x, int32_t y,
                                          map_is_at_callback callback)
{
    thing_templatep thing_template;
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

            if (thing_is_dead(thing_it)) {
                w = wid_grid_find_next(grid_wid, w, x, y, z);
                continue;
            }

            thing_template = wid_get_thing_template(w);
            if (thing_template) {
                if ((*callback)(thing_template)) {
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
    return (map_all_things_is_x_at(level, x, y, thing_template_is_player));
}

tree_rootp map_all_things_is_monst_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_monst));
}

tree_rootp map_all_things_is_wall_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_wall));
}

tree_rootp map_all_things_is_key_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_key));
}

tree_rootp map_all_things_is_xxx3_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_xxx3));
}

tree_rootp map_all_things_is_xxx4_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_xxx4));
}

tree_rootp map_all_things_is_weapon_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_weapon));
}

tree_rootp map_all_things_is_treasure_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_treasure));
}

tree_rootp map_all_things_is_wearable_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_wearable));
}

tree_rootp map_all_things_is_fragile_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_fragile));
}

tree_rootp map_all_things_is_star_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_star));
}

tree_rootp map_all_things_is_animated_no_dir_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_animated_no_dir));
}

tree_rootp map_all_things_is_weapon_swing_effect_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_weapon_swing_effect));
}

tree_rootp map_all_things_is_key2_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_key2));
}

tree_rootp map_all_things_is_key3_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_key3));
}

tree_rootp map_all_things_is_fire_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_fire));
}

tree_rootp map_all_things_is_ring_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_ring));
}

tree_rootp map_all_things_is_animation_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_animation));
}

tree_rootp map_all_things_is_poison_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_poison));
}

tree_rootp map_all_things_is_item_unusable_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_item_unusable));
}

tree_rootp map_all_things_is_valid_for_shortcut_key_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_valid_for_shortcut_key));
}

tree_rootp map_all_things_is_seedpod_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_seedpod));
}

tree_rootp map_all_things_is_spam_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_spam));
}

tree_rootp map_all_things_is_door_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_door));
}

tree_rootp map_all_things_is_pipe_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_pipe));
}

tree_rootp map_all_things_is_mob_spawner_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_mob_spawner));
}

tree_rootp map_all_things_is_scarable_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_scarable));
}

tree_rootp map_all_things_is_explosion_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_explosion));
}

tree_rootp map_all_things_is_hidden_from_editor_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_hidden_from_editor));
}

tree_rootp map_all_things_is_animated_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_animated));
}

tree_rootp map_all_things_is_combustable_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_combustable));
}

tree_rootp map_all_things_is_projectile_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_projectile));
}

tree_rootp map_all_things_is_boring_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_boring));
}

tree_rootp map_all_things_is_plant_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_plant));
}

tree_rootp map_all_things_is_food_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_food));
}

tree_rootp map_all_things_is_floor_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_floor));
}

tree_rootp map_all_things_is_exit_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_exit));
}

static thing_templatep map_find_x_at (levelp level,
                                      int32_t x, int32_t y,
                                      map_is_at_callback callback,
                                      widp *wout)
{
    thing_templatep thing_template;
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
                * Need to filter dead things so map fixup can no longer see wall
                * tiles that are in the process of being destroyed.
                */
                if (!thing_it) {
                    w = wid_grid_find_next(grid_wid, w, x, y, z);
                    continue;
                }

                if (thing_is_dead(thing_it)) {
                    w = wid_grid_find_next(grid_wid, w, x, y, z);
                    continue;
                }
            }

            thing_template = wid_get_thing_template(w);
            if (thing_template) {
                if ((*callback)(thing_template)) {
                    if (wout) {
                        *wout = w;
                    }

                    return (thing_template);
                }
            }

            w = wid_grid_find_next(grid_wid, w, x, y, z);
        }
    }

    return (0);
}

thing_templatep map_find_player_at (levelp level,
                                    int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_player, w));
}

thing_templatep map_find_monst_at (levelp level,
                                   int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_monst, w));
}

thing_templatep map_find_wall_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_wall, w));
}

thing_templatep map_find_key_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_key, w));
}

thing_templatep map_find_xxx3_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_xxx3, w));
}

thing_templatep map_find_xxx4_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_xxx4, w));
}

thing_templatep map_find_xxx5_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_weapon, w));
}

thing_templatep map_find_xxx6_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_treasure, w));
}

thing_templatep map_find_xxx7_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_wearable, w));
}

thing_templatep map_find_is_fragile_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_fragile, w));
}

thing_templatep map_find_is_star_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_star, w));
}

thing_templatep map_find_powerup_spam_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_animated_no_dir, w));
}

thing_templatep map_find_key1_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_weapon_swing_effect, w));
}

thing_templatep map_find_car_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_key2, w));
}

thing_templatep map_find_star_green_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_key3, w));
}

thing_templatep map_find_star_cyan_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_fire, w));
}

thing_templatep map_find_star_black_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_ring, w));
}

thing_templatep map_find_star_purple_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_animation, w));
}

thing_templatep map_find_explosion_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_animation, w));
}

thing_templatep map_find_spikes_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_poison, w));
}

thing_templatep map_find_is_item_unusable_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_item_unusable, w));
}

thing_templatep map_find_is_valid_for_shortcut_key_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_valid_for_shortcut_key, w));
}

thing_templatep map_find_seedpod_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_seedpod, w));
}

thing_templatep map_find_spam_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_spam, w));
}

thing_templatep map_find_door_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_door, w));
}

thing_templatep map_find_pipe_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_pipe, w));
}

thing_templatep map_find_generator_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_mob_spawner, w));
}

thing_templatep map_find_xxx28_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_scarable, w));
}

thing_templatep map_find_is_explosion_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_explosion, w));
}

thing_templatep map_find_hidden_from_editor_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_hidden_from_editor, w));
}

thing_templatep map_find_is_projectile_at (levelp level,
                                   int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_projectile, w));
}

thing_templatep map_find_item_perma_at (levelp level,
                                   int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_boring, w));
}

thing_templatep map_find_is_ring_at (levelp level,
                                   int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_ring, w));
}

thing_templatep map_find_plant_at (levelp level,
                                   int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_plant, w));
}

thing_templatep map_find_food_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_food, w));
}

thing_templatep map_find_floor_at (levelp level,
                                   int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_floor, w));
}

thing_templatep map_find_exit_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_exit, w));
}

void map_fixup (levelp level)
{
    int32_t index;
    tilep tile;
    int32_t x;
    int32_t y;
    int32_t dx;
    int32_t dy;
    thing_templatep nbrs[3][3];
    widp w;
                
#ifdef DEBUG
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
#ifdef DEBUG
if (level != server_level)
                fprintf(fp,"x");
#endif
                mywid = w;
            } else if (map_find_pipe_at(level, x, y, &w)) {
                mywid = w;
            } else if (map_find_door_at(level, x, y, &w)) {
#ifdef DEBUG
if (level != server_level)
                fprintf(fp,"D");
#endif
                mywid = w;
            }

            if (!mywid) {
#ifdef DEBUG
if (level != server_level)
                fprintf(fp," ");
#endif
                continue;
            }

            memset(nbrs, 0, sizeof(nbrs));

            for (dx = -1; dx <= 1; dx++) {
                for (dy = -1; dy <= 1; dy++) {

                    if (map_find_wall_at(level, x, y, &w)) {
                        nbrs[dx + 1][dy + 1] = map_find_wall_at(level,
                                                                x + dx, y + dy,
                                                                0 /* wid */);
                    }

                    if (map_find_pipe_at(level, x, y, &w)) {
                        nbrs[dx + 1][dy + 1] = map_find_pipe_at(level,
                                                                x + dx, y + dy,
                                                                0 /* wid */);
                    }

                    if (map_find_door_at(level, x, y, &w)) {
                        nbrs[dx + 1][dy + 1] = map_find_door_at(level,
                                                                x + dx, y + dy,
                                                                0 /* wid */);
                    }
                }
            }

            thing_templatep a = nbrs[0][0];
            thing_templatep b = nbrs[1][0];
            thing_templatep c = nbrs[2][0];
            thing_templatep d = nbrs[0][1];
            thing_templatep e = nbrs[1][1];
            thing_templatep f = nbrs[2][1];
            thing_templatep g = nbrs[0][2];
            thing_templatep h = nbrs[1][2];
            thing_templatep i = nbrs[2][2];

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

            thing_templatep t = e;

            thing_tilep thing_tile = thing_tile_find(t, index, &tile);
            if (!thing_tile) {
                index = IS_JOIN_BLOCK;

                thing_tile = thing_tile_find(t, index, &tile);
                if (!thing_tile) {
                    index = IS_JOIN_NODE;

                    thing_tile = thing_tile_find(t, index, &tile);
                    if (!thing_tile) {
                        DIE("no joinable tile for %s", thing_template_name(t));
                    }
                }
            }

            if (!tile) {
                DIE("no tile for %s", thing_template_name(t));
            }

            const char *tilename = thing_tile_name(thing_tile);

            if (!tilename) {
                DIE("no tilename for %s", thing_template_name(e));
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
#ifdef DEBUG
        fprintf(fp,"\n");
#endif
    }
#ifdef DEBUG
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

    int32_t exit = rand() % nexits;

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
        this_door[x][y] = (void*)-1;
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
    return (level_count_is_x(level, thing_template_is_player));
}

uint32_t level_count_is_monst (levelp level)
{
    return (level_count_is_x(level, thing_template_is_monst));
}

uint32_t level_count_is_wall (levelp level)
{
    return (level_count_is_x(level, thing_template_is_wall));
}

uint32_t level_count_is_key (levelp level)
{
    return (level_count_is_x(level, thing_template_is_key));
}

uint32_t level_count_is_xxx3 (levelp level)
{
    return (level_count_is_x(level, thing_template_is_xxx3));
}

uint32_t level_count_is_xxx4 (levelp level)
{
    return (level_count_is_x(level, thing_template_is_xxx4));
}

uint32_t level_count_is_weapon (levelp level)
{
    return (level_count_is_x(level, thing_template_is_weapon));
}

uint32_t level_count_is_treasure (levelp level)
{
    return (level_count_is_x(level, thing_template_is_treasure));
}

uint32_t level_count_is_wearable (levelp level)
{
    return (level_count_is_x(level, thing_template_is_wearable));
}

uint32_t level_count_is_fragile (levelp level)
{
    return (level_count_is_x(level, thing_template_is_fragile));
}

uint32_t level_count_is_star (levelp level)
{
    return (level_count_is_x(level, thing_template_is_star));
}

uint32_t level_count_is_animated_no_dir (levelp level)
{
    return (level_count_is_x(level, thing_template_is_animated_no_dir));
}

uint32_t level_count_is_weapon_swing_effect (levelp level)
{
    return (level_count_is_x(level, thing_template_is_weapon_swing_effect));
}

uint32_t level_count_is_key2 (levelp level)
{
    return (level_count_is_x(level, thing_template_is_key2));
}

uint32_t level_count_is_key3 (levelp level)
{
    return (level_count_is_x(level, thing_template_is_key3));
}

uint32_t level_count_is_fire (levelp level)
{
    return (level_count_is_x(level, thing_template_is_fire));
}

uint32_t level_count_is_ring (levelp level)
{
    return (level_count_is_x(level, thing_template_is_ring));
}

uint32_t level_count_is_animation (levelp level)
{
    return (level_count_is_x(level, thing_template_is_animation));
}

uint32_t level_count_is_poison (levelp level)
{
    return (level_count_is_x(level, thing_template_is_poison));
}

uint32_t level_count_is_item_unusable (levelp level)
{
    return (level_count_is_x(level, thing_template_is_item_unusable));
}

uint32_t level_count_is_valid_for_shortcut_key (levelp level)
{
    return (level_count_is_x(level, thing_template_is_valid_for_shortcut_key));
}

uint32_t level_count_is_seedpod (levelp level)
{
    return (level_count_is_x(level, thing_template_is_seedpod));
}

uint32_t level_count_is_spam (levelp level)
{
    return (level_count_is_x(level, thing_template_is_spam));
}

uint32_t level_count_is_door (levelp level)
{
    return (level_count_is_x(level, thing_template_is_door));
}

uint32_t level_count_is_pipe (levelp level)
{
    return (level_count_is_x(level, thing_template_is_pipe));
}

uint32_t level_count_is_mob_spawner (levelp level)
{
    return (level_count_is_x(level, thing_template_is_mob_spawner));
}

uint32_t level_count_is_scarable (levelp level)
{
    return (level_count_is_x(level, thing_template_is_scarable));
}

uint32_t level_count_is_explosion (levelp level)
{
    return (level_count_is_x(level, thing_template_is_explosion));
}

uint32_t level_count_is_hidden_from_editor (levelp level)
{
    return (level_count_is_x(level, thing_template_is_hidden_from_editor));
}

uint32_t level_count_is_combustable (levelp level)
{
    return (level_count_is_x(level, thing_template_is_combustable));
}

uint32_t level_count_is_projectile (levelp level)
{
    return (level_count_is_x(level, thing_template_is_projectile));
}

uint32_t level_count_is_boring (levelp level)
{
    return (level_count_is_x(level, thing_template_is_boring));
}

uint32_t level_count_is_item_is_hidden (levelp level)
{
    return (level_count_is_x(level, thing_template_is_ring));
}

uint32_t level_count_is_plant (levelp level)
{
    return (level_count_is_x(level, thing_template_is_plant));
}

uint32_t level_count_is_food (levelp level)
{
    return (level_count_is_x(level, thing_template_is_food));
}

uint32_t level_count_is_floor (levelp level)
{
    return (level_count_is_x(level, thing_template_is_floor));
}

uint32_t level_count_is_exit (levelp level)
{
    return (level_count_is_x(level, thing_template_is_exit));
}

static tree_rootp map_all_things_is_x (levelp level,
                                       map_is_at_callback callback)
{
    thing_templatep thing_template;
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

                    if (thing_is_dead(thing_it)) {
                        w = wid_grid_find_next(grid_wid, w, x, y, z);
                        continue;
                    }

                    thing_template = wid_get_thing_template(w);
                    if (thing_template) {
                        if ((*callback)(thing_template)) {
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
    return (map_all_things_is_x(level, thing_template_is_player));
}

tree_rootp map_all_things_is_monst (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_monst));
}

tree_rootp map_all_things_is_wall (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_wall));
}

tree_rootp map_all_things_is_key (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_key));
}

tree_rootp map_all_things_is_xxx3 (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_xxx3));
}

tree_rootp map_all_things_is_xxx4 (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_xxx4));
}

tree_rootp map_all_things_is_weapon (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_weapon));
}

tree_rootp map_all_things_is_treasure (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_treasure));
}

tree_rootp map_all_things_is_wearable (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_wearable));
}

tree_rootp map_all_things_is_fragile (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_fragile));
}

tree_rootp map_all_things_is_star (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_star));
}

tree_rootp map_all_things_is_animated_no_dir (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_animated_no_dir));
}

tree_rootp map_all_things_is_weapon_swing_effect (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_weapon_swing_effect));
}

tree_rootp map_all_things_is_key2 (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_key2));
}

tree_rootp map_all_things_is_key3 (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_key3));
}

tree_rootp map_all_things_is_fire (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_fire));
}

tree_rootp map_all_things_is_ring (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_ring));
}

tree_rootp map_all_things_is_animation (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_animation));
}

tree_rootp map_all_things_is_poison (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_poison));
}

tree_rootp map_all_things_is_item_unusable (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_item_unusable));
}

tree_rootp map_all_things_is_valid_for_shortcut_key (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_valid_for_shortcut_key));
}

tree_rootp map_all_things_is_seedpod (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_seedpod));
}

tree_rootp map_all_things_is_spam (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_spam));
}

tree_rootp map_all_things_is_door (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_door));
}

tree_rootp map_all_things_is_pipe (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_pipe));
}

tree_rootp map_all_things_is_mob_spawner (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_mob_spawner));
}

tree_rootp map_all_things_is_scarable (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_scarable));
}

tree_rootp map_all_things_is_explosion (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_explosion));
}

tree_rootp map_all_things_is_hidden_from_editor (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_hidden_from_editor));
}

tree_rootp map_all_things_is_animated (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_animated));
}

tree_rootp map_all_things_is_combustable (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_combustable));
}

tree_rootp map_all_things_is_projectile (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_projectile));
}

tree_rootp map_all_things_is_boring (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_boring));
}

tree_rootp map_all_things_is_plant (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_plant));
}

tree_rootp map_all_things_is_food (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_food));
}

tree_rootp map_all_things_is_floor (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_floor));
}

tree_rootp map_all_things_is_exit (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_exit));
}
