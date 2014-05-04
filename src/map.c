/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "map.h"
#include "tree.h"
#include "wid.h"
#include "thing.h"
#include "thing_tile.h"
#include "level.h"
#include "level_private.h"
#include "bits.h"
#include "socket.h"

typedef boolean (*map_is_at_callback)(thing_templatep);

static boolean map_is_x_at (levelp level,
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
    w = wid_grid_find_first(grid_wid, x, y);
    while (w) {
        thingp thing_it = wid_get_thing(w);

        if (thing_it) {
            if (thing_is_dead(thing_it)) {
                w = wid_grid_find_next(grid_wid, w, x, y);
                continue;
            }
        }

        thing_template = wid_get_thing_template(w);
        if (thing_template) {
            if ((*callback)(thing_template)) {
                return (true);
            }
        }

        w = wid_grid_find_next(grid_wid, w, x, y);
    }

    return (false);
}

static boolean map_count_x_at (levelp level,
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
    w = wid_grid_find_first(grid_wid, x, y);
    while (w) {
        thing_template = wid_get_thing_template(w);
        if (thing_template) {
            if ((*callback)(thing_template)) {
                count++;
            }
        }

        w = wid_grid_find_next(grid_wid, w, x, y);
    }

    return (count);
}

boolean map_is_player_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_player));
}

boolean map_is_monst_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_monst));
}

boolean map_is_wall_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_wall));
}

boolean map_is_key_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_key));
}

boolean map_is_collision_map_small_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_collision_map_small));
}

boolean map_is_xxx3_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_xxx3));
}

boolean map_is_xxx4_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_xxx4));
}

boolean map_is_xxx5_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_xxx5));
}

boolean map_is_xxx6_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_xxx6));
}

boolean map_is_xxx7_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_xxx7));
}

boolean map_is_xxx8_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_xxx8));
}

boolean map_is_star_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_star));
}

boolean map_is_key0_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_key0));
}

boolean map_is_key1_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_key1));
}

boolean map_is_key2_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_key2));
}

boolean map_is_key3_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_key3));
}

boolean map_is_key4_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_key4));
}

boolean map_is_key5_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_key5));
}

boolean map_is_key6_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_key6));
}

boolean map_is_key7_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_key7));
}

boolean map_is_key8_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_key8));
}

boolean map_is_xxx20_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_xxx20));
}

boolean map_is_xxx21_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_xxx21));
}

boolean map_is_seedpod_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_seedpod));
}

boolean map_is_bomb_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_bomb));
}

boolean map_is_spam_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_spam));
}

boolean map_is_door_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_door));
}

boolean map_is_pipe_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_pipe));
}

boolean map_is_generator_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_generator));
}

boolean map_is_scarable_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_scarable));
}

boolean map_is_xxx29_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_xxx29));
}

boolean map_is_hidden_from_editor_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_hidden_from_editor));
}

boolean map_is_animated_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_animated));
}

boolean map_is_xxx32_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_xxx32));
}

boolean map_is_xxx33_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_xxx33));
}

boolean map_is_projectile_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_projectile));
}

boolean map_is_boring_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_boring));
}

boolean map_is_item_hidden_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_item_hidden));
}

boolean map_is_plant_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_plant));
}

boolean map_is_food_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_food));
}

boolean map_is_floor_at (levelp level, int32_t x, int32_t y)
{
    return (map_is_x_at(level, x, y, thing_template_is_floor));
}

boolean map_is_exit_at (levelp level, int32_t x, int32_t y)
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
    w = wid_grid_find_first(grid_wid, x, y);
    while (w) {
        thingp thing_it = wid_get_thing(w);

        if (!thing_it) {
            w = wid_grid_find_next(grid_wid, w, x, y);
            continue;
        }

        if (thing_is_dead(thing_it)) {
            w = wid_grid_find_next(grid_wid, w, x, y);
            continue;
        }

        thing_template = wid_get_thing_template(w);
        if (thing_template) {
            if ((*callback)(thing_template)) {
                return (thing_it);
            }
        }

        w = wid_grid_find_next(grid_wid, w, x, y);
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

thingp map_thing_is_collision_map_small_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_collision_map_small));
}

thingp map_thing_is_xxx3_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_xxx3));
}

thingp map_thing_is_xxx4_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_xxx4));
}

thingp map_thing_is_xxx5_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_xxx5));
}

thingp map_thing_is_xxx6_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_xxx6));
}

thingp map_thing_is_xxx7_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_xxx7));
}

thingp map_thing_is_xxx8_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_xxx8));
}

thingp map_thing_is_star_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_star));
}

thingp map_thing_is_key0_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_key0));
}

thingp map_thing_is_key1_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_key1));
}

thingp map_thing_is_key2_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_key2));
}

thingp map_thing_is_key3_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_key3));
}

thingp map_thing_is_key4_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_key4));
}

thingp map_thing_is_key5_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_key5));
}

thingp map_thing_is_key6_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_key6));
}

thingp map_thing_is_key7_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_key7));
}

thingp map_thing_is_key8_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_key8));
}

thingp map_thing_is_xxx20_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_xxx20));
}

thingp map_thing_is_xxx21_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_xxx21));
}

thingp map_thing_is_seedpod_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_seedpod));
}

thingp map_thing_is_bomb_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_bomb));
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

thingp map_thing_is_generator_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_generator));
}

thingp map_thing_is_scarable_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_scarable));
}

thingp map_thing_is_xxx29_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_xxx29));
}

thingp map_thing_is_hidden_from_editor_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_hidden_from_editor));
}

thingp map_thing_is_animated_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_animated));
}

thingp map_thing_is_xxx32_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_xxx32));
}

thingp map_thing_is_xxx33_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_xxx33));
}

thingp map_thing_is_projectile_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_projectile));
}

thingp map_thing_is_boring_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_boring));
}

thingp map_thing_is_item_hidden_at (levelp level, int32_t x, int32_t y)
{
    return (map_thing_is_x_at(level, x, y, thing_template_is_item_hidden));
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
    w = wid_grid_find_first(grid_wid, x, y);
    while (w) {
        thingp thing_it = wid_get_thing(w);

        if (!thing_it) {
            w = wid_grid_find_next(grid_wid, w, x, y);
            continue;
        }

        if (thing_is_dead(thing_it)) {
            w = wid_grid_find_next(grid_wid, w, x, y);
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

        w = wid_grid_find_next(grid_wid, w, x, y);
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

tree_rootp map_all_things_is_collision_map_small_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_collision_map_small));
}

tree_rootp map_all_things_is_xxx3_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_xxx3));
}

tree_rootp map_all_things_is_xxx4_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_xxx4));
}

tree_rootp map_all_things_is_xxx5_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_xxx5));
}

tree_rootp map_all_things_is_xxx6_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_xxx6));
}

tree_rootp map_all_things_is_xxx7_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_xxx7));
}

tree_rootp map_all_things_is_xxx8_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_xxx8));
}

tree_rootp map_all_things_is_star_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_star));
}

tree_rootp map_all_things_is_key0_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_key0));
}

tree_rootp map_all_things_is_key1_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_key1));
}

tree_rootp map_all_things_is_key2_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_key2));
}

tree_rootp map_all_things_is_key3_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_key3));
}

tree_rootp map_all_things_is_key4_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_key4));
}

tree_rootp map_all_things_is_key5_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_key5));
}

tree_rootp map_all_things_is_key6_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_key6));
}

tree_rootp map_all_things_is_key7_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_key7));
}

tree_rootp map_all_things_is_key8_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_key8));
}

tree_rootp map_all_things_is_xxx20_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_xxx20));
}

tree_rootp map_all_things_is_xxx21_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_xxx21));
}

tree_rootp map_all_things_is_seedpod_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_seedpod));
}

tree_rootp map_all_things_is_bomb_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_bomb));
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

tree_rootp map_all_things_is_generator_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_generator));
}

tree_rootp map_all_things_is_scarable_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_scarable));
}

tree_rootp map_all_things_is_xxx29_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_xxx29));
}

tree_rootp map_all_things_is_hidden_from_editor_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_hidden_from_editor));
}

tree_rootp map_all_things_is_animated_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_animated));
}

tree_rootp map_all_things_is_xxx32_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_xxx32));
}

tree_rootp map_all_things_is_xxx33_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_xxx33));
}

tree_rootp map_all_things_is_projectile_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_projectile));
}

tree_rootp map_all_things_is_boring_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_boring));
}

tree_rootp map_all_things_is_item_hidden_at (levelp level, int32_t x, int32_t y)
{
    return (map_all_things_is_x_at(level, x, y, thing_template_is_item_hidden));
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

    w = wid_grid_find_first(grid_wid, x, y);
    while (w) {
        thing_template = wid_get_thing_template(w);
        if (thing_template) {
            if ((*callback)(thing_template)) {
                if (wout) {
                    *wout = w;
                }

                return (thing_template);
            }
        }

        w = wid_grid_find_next(grid_wid, w, x, y);
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

thing_templatep map_find_xxx2_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_collision_map_small, w));
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
    return (map_find_x_at(level, x, y, thing_template_is_xxx5, w));
}

thing_templatep map_find_xxx6_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_xxx6, w));
}

thing_templatep map_find_xxx7_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_xxx7, w));
}

thing_templatep map_find_xxx8_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_xxx8, w));
}

thing_templatep map_find_is_star_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_star, w));
}

thing_templatep map_find_powerup_spam_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_key0, w));
}

thing_templatep map_find_key1_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_key1, w));
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
    return (map_find_x_at(level, x, y, thing_template_is_key4, w));
}

thing_templatep map_find_star_black_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_key5, w));
}

thing_templatep map_find_star_purple_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_key6, w));
}

thing_templatep map_find_explosion_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_key7, w));
}

thing_templatep map_find_spikes_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_key8, w));
}

thing_templatep map_find_is_xxx20_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_xxx20, w));
}

thing_templatep map_find_is_xxx21_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_xxx21, w));
}

thing_templatep map_find_seedpod_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_seedpod, w));
}

thing_templatep map_find_bomb_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_bomb, w));
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
    return (map_find_x_at(level, x, y, thing_template_is_generator, w));
}

thing_templatep map_find_xxx28_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_scarable, w));
}

thing_templatep map_find_is_xxx29_at (levelp level,
                                  int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_xxx29, w));
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

thing_templatep map_find_is_item_hidden_at (levelp level,
                                   int32_t x, int32_t y, widp *w)
{
    return (map_find_x_at(level, x, y, thing_template_is_item_hidden, w));
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

    for (x = 0; x < TILES_MAP_WIDTH; x++) {
        for (y = 0; y < TILES_MAP_HEIGHT; y++) {

            if (!map_find_wall_at(level, x, y, &w) &&
                !map_find_pipe_at(level, x, y, &w) &&
                !map_find_door_at(level, x, y, &w)) {
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
                if (map_find_door_at(level, x, y, &w)) {
                    if ( map_find_wall_at(level, x - 1, y, 0) &&
                         map_find_wall_at(level, x + 1, y, 0) &&
                        !map_find_wall_at(level, x, y - 1, 0) &&
                        !map_find_wall_at(level, x, y + 1, 0)) {
                        index = IS_JOIN_HORIZ2;
                    }
                }
            }

            if (index == IS_JOIN_NODE) {
                if (map_find_door_at(level, x, y, &w)) {
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

            wid_set_tilename(w, tilename);
            wid_set_font(w, small_font);
        }
    }
}

static uint32_t level_count_is_x (levelp level, map_is_at_callback callback)
{
    uint32_t count;
    int32_t x;
    int32_t y;

    count = 0;

    for (x = 0; x < TILES_MAP_WIDTH; x++) {
        for (y = 0; y < TILES_MAP_HEIGHT; y++) {
            count += map_count_x_at(level, x, y, callback);
        }
    }

    return (count);
}

static char this_pipe[TILES_MAP_WIDTH][TILES_MAP_HEIGHT];

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

boolean level_pipe_find_exit (levelp level,
                              int32_t ix, int32_t iy,
                              int32_t *exit_x, int32_t *exit_y)
{
    int32_t exits_x[TILES_MAP_WIDTH];
    int32_t exits_y[TILES_MAP_WIDTH];
    int32_t nexits;
    int32_t x;
    int32_t y;

    memset(this_pipe, ' ', sizeof(this_pipe));
    memset(exits_x, 0, sizeof(exits_x));
    memset(exits_y, 0, sizeof(exits_y));
    nexits = 0;

    pipe_flood(level, ix, iy);

    for (x = 1; x < TILES_MAP_WIDTH-1; x++) {
        for (y = 1; y < TILES_MAP_HEIGHT-1; y++) {

            if ((x == ix) && (y == iy)) {
                continue;
            }

            if (this_pipe[x][y] != '+') {
                continue;
            }

            if (level->end_pipe[x][y] != ' ') {
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

static thingp this_door[TILES_MAP_WIDTH][TILES_MAP_HEIGHT];

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

    for (x = 0; x < TILES_MAP_WIDTH; x++) {
        for (y = 0; y < TILES_MAP_HEIGHT; y++) {
            if (!this_door[x][y]) {
                continue;
            }

            if (this_door[x][y] == (void*)-1) {
                continue;
            }

            thing_dead(this_door[x][y], 0 /* killer */, "open");
        }
    }

    level_set_monst_walls(level);

    /*
     * Send the update quickly to the client. Don't wait for the things to
     * tick. The doors should be on the active list now anyway as they were
     * killed above.
     */
    socket_server_tx_map_update(0 /* all clients */, server_active_things);
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

uint32_t level_count_is_collision_map_small (levelp level)
{
    return (level_count_is_x(level, thing_template_is_collision_map_small));
}

uint32_t level_count_is_xxx3 (levelp level)
{
    return (level_count_is_x(level, thing_template_is_xxx3));
}

uint32_t level_count_is_xxx4 (levelp level)
{
    return (level_count_is_x(level, thing_template_is_xxx4));
}

uint32_t level_count_is_xxx5 (levelp level)
{
    return (level_count_is_x(level, thing_template_is_xxx5));
}

uint32_t level_count_is_xxx6 (levelp level)
{
    return (level_count_is_x(level, thing_template_is_xxx6));
}

uint32_t level_count_is_xxx7 (levelp level)
{
    return (level_count_is_x(level, thing_template_is_xxx7));
}

uint32_t level_count_is_xxx8 (levelp level)
{
    return (level_count_is_x(level, thing_template_is_xxx8));
}

uint32_t level_count_is_star (levelp level)
{
    return (level_count_is_x(level, thing_template_is_star));
}

uint32_t level_count_is_key0 (levelp level)
{
    return (level_count_is_x(level, thing_template_is_key0));
}

uint32_t level_count_is_key1 (levelp level)
{
    return (level_count_is_x(level, thing_template_is_key1));
}

uint32_t level_count_is_key2 (levelp level)
{
    return (level_count_is_x(level, thing_template_is_key2));
}

uint32_t level_count_is_key3 (levelp level)
{
    return (level_count_is_x(level, thing_template_is_key3));
}

uint32_t level_count_is_key4 (levelp level)
{
    return (level_count_is_x(level, thing_template_is_key4));
}

uint32_t level_count_is_key5 (levelp level)
{
    return (level_count_is_x(level, thing_template_is_key5));
}

uint32_t level_count_is_key6 (levelp level)
{
    return (level_count_is_x(level, thing_template_is_key6));
}

uint32_t level_count_is_key7 (levelp level)
{
    return (level_count_is_x(level, thing_template_is_key7));
}

uint32_t level_count_is_key8 (levelp level)
{
    return (level_count_is_x(level, thing_template_is_key8));
}

uint32_t level_count_is_xxx20 (levelp level)
{
    return (level_count_is_x(level, thing_template_is_xxx20));
}

uint32_t level_count_is_xxx21 (levelp level)
{
    return (level_count_is_x(level, thing_template_is_xxx21));
}

uint32_t level_count_is_seedpod (levelp level)
{
    return (level_count_is_x(level, thing_template_is_seedpod));
}

uint32_t level_count_is_bomb (levelp level)
{
    return (level_count_is_x(level, thing_template_is_bomb));
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

uint32_t level_count_is_generator (levelp level)
{
    return (level_count_is_x(level, thing_template_is_generator));
}

uint32_t level_count_is_scarable (levelp level)
{
    return (level_count_is_x(level, thing_template_is_scarable));
}

uint32_t level_count_is_xxx29 (levelp level)
{
    return (level_count_is_x(level, thing_template_is_xxx29));
}

uint32_t level_count_is_hidden_from_editor (levelp level)
{
    return (level_count_is_x(level, thing_template_is_hidden_from_editor));
}

uint32_t level_count_is_xxx32 (levelp level)
{
    return (level_count_is_x(level, thing_template_is_xxx32));
}

uint32_t level_count_is_xxx33 (levelp level)
{
    return (level_count_is_x(level, thing_template_is_xxx33));
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
    return (level_count_is_x(level, thing_template_is_item_hidden));
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

    for (y = 0; y < TILES_MAP_HEIGHT; y++) {
        for (x = 0; x < TILES_MAP_WIDTH; x++) {

            /*
             * Look for a floor tile where we can place stuff.
             */
            w = wid_grid_find_first(grid_wid, x, y);
            while (w) {
                thingp thing_it = wid_get_thing(w);

                if (!thing_it) {
                    w = wid_grid_find_next(grid_wid, w, x, y);
                    continue;
                }

                if (thing_is_dead(thing_it)) {
                    w = wid_grid_find_next(grid_wid, w, x, y);
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

                w = wid_grid_find_next(grid_wid, w, x, y);
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

tree_rootp map_all_things_is_collision_map_small (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_collision_map_small));
}

tree_rootp map_all_things_is_xxx3 (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_xxx3));
}

tree_rootp map_all_things_is_xxx4 (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_xxx4));
}

tree_rootp map_all_things_is_xxx5 (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_xxx5));
}

tree_rootp map_all_things_is_xxx6 (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_xxx6));
}

tree_rootp map_all_things_is_xxx7 (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_xxx7));
}

tree_rootp map_all_things_is_xxx8 (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_xxx8));
}

tree_rootp map_all_things_is_star (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_star));
}

tree_rootp map_all_things_is_key0 (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_key0));
}

tree_rootp map_all_things_is_key1 (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_key1));
}

tree_rootp map_all_things_is_key2 (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_key2));
}

tree_rootp map_all_things_is_key3 (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_key3));
}

tree_rootp map_all_things_is_key4 (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_key4));
}

tree_rootp map_all_things_is_key5 (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_key5));
}

tree_rootp map_all_things_is_key6 (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_key6));
}

tree_rootp map_all_things_is_key7 (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_key7));
}

tree_rootp map_all_things_is_key8 (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_key8));
}

tree_rootp map_all_things_is_xxx20 (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_xxx20));
}

tree_rootp map_all_things_is_xxx21 (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_xxx21));
}

tree_rootp map_all_things_is_seedpod (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_seedpod));
}

tree_rootp map_all_things_is_bomb (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_bomb));
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

tree_rootp map_all_things_is_generator (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_generator));
}

tree_rootp map_all_things_is_scarable (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_scarable));
}

tree_rootp map_all_things_is_xxx29 (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_xxx29));
}

tree_rootp map_all_things_is_hidden_from_editor (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_hidden_from_editor));
}

tree_rootp map_all_things_is_animated (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_animated));
}

tree_rootp map_all_things_is_xxx32 (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_xxx32));
}

tree_rootp map_all_things_is_xxx33 (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_xxx33));
}

tree_rootp map_all_things_is_projectile (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_projectile));
}

tree_rootp map_all_things_is_boring (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_boring));
}

tree_rootp map_all_things_is_item_hidden (levelp level)
{
    return (map_all_things_is_x(level, thing_template_is_item_hidden));
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
