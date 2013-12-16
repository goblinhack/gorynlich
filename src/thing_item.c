/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "tree.h"
#include "thing.h"
#include "thing_template.h"
#include "thing_private.h"
#include "item.h"
#include "wid_game_map.h"
#include "map.h"
#include "wid.h"
#include "level.h"
#include "sound.h"

static itemp
thing_item_collect_internal (thingp owner, thingp thing_item,
                             boolean auto_use_allowed)
{
    tree_rootp *items;
    levelp level;

    verify(owner);
    verify(thing_item);

    THING_LOG(owner, "collect: %s", thing_logname(thing_item));

    items = thing_carried_itemsp(owner);
    itemp item = item_collect(items, thing_item, owner);
    if (!item) {
        DIE("%s failed to collect: %s", thing_logname(owner),
            thing_logname(thing_item));
    }

    /*
     * Record the owner.
     */
    thing_set_item_owner(thing_item, owner);

    /*
     * Bounty for the collector?
     */
    uint32_t score = thing_template_get_score_on_collect(
                                            thing_get_template(thing_item));

    if (score) {
        thing_inc_score_pump(owner, score);
    }

    boolean need_update = true;

    level = thing_level(owner);
    if (level) {
        /*
         * If the last letter, open the exit.
         */
        if (thing_is_letter(thing_item)) {
            if (level_count_is_letter(level) == 0) {
                tree_thing_node *node;
                tree_rootp root = map_all_things_is_exit(level);

                /*
                 * Record this thing opened the exit.
                 */
                thing_set_opened_exit(owner, true);

                if (root) {
                    TREE_WALK_UNSAFE(root, node) {
                        thingp t = (typeof(t)) node->tree.key;

                        thing_set_is_open(t, true);
                    }

                    tree_destroy(&root, 0 /* func */);
                }

                level_set_is_exit_open(level, true);

                sound_play_letter();
            }
        }

        /*
         * Use items immediately?
         */
        if (thing_is_star(thing_item)) {
            if (thing_is_esnail(owner)) {
                THING_LOG(owner, "auto use: %s", thing_logname(thing_item));

                if (auto_use_allowed) {
                    thing_item_use(owner, thing_item);
                    need_update = false;
                }
            }
        }

        /*
         * Go faster, collected a rocket.
         */
        if (thing_is_powerup_rocket(thing_item)) {
            thing_inc_powerup_rocket_count(owner, 1);
        }

        /*
         * Go slower, collected spam powerup.
         */
        if (thing_is_powerup_spam(thing_item)) {
            thing_inc_powerup_spam_count(owner, 1);
        }

        /*
         * Go slower, collected spam.
         */
        if (thing_is_spam(thing_item)) {
            THING_LOG(owner, "auto use: %s", thing_logname(thing_item));

            if (auto_use_allowed) {
                thing_item_use(owner, thing_item);
                need_update = false;
            }
        }
    }

    if (owner == player) {
        if (need_update) {
            wid_game_map_item_update(level_game);
        }
    }

    return (item);
}

itemp thing_item_collect (thingp owner, thingp thing_item)
{
    return (thing_item_collect_internal(owner, thing_item,
                                        true /* auto use allowed */));
}

itemp thing_item_collect_no_auto_use (thingp owner, thingp thing_item)
{
    return (thing_item_collect_internal(owner, thing_item,
                                        false /* auto use allowed */));
}

boolean thing_item_use (thingp owner, thingp thing_item)
{
    levelp level;

    verify(owner);
    verify(thing_item);

    level = thing_level(owner);
    verify(level);

    THING_LOG(owner, "use: %s", thing_logname(thing_item));

    if (thing_is_star_yellow(thing_item)) {
        thing_item_drop(owner, thing_item, true /* destroy */, "used");

        /*
         * Place a rocket over the thing.
         */
fprintf(stderr,"\nZZZ %s %s %d ",__FILE__,__FUNCTION__,__LINE__);
        widp w = wid_game_map_replace_tile(
                        wid_game_map_grid_container,
                        thing_grid_x(owner),
                        thing_grid_y(owner),
                        0,
                        thing_template_find("data/things/powerup_rocket"));

        thing_item = wid_get_thing(w);
        thing_item_collect(owner, thing_item);

        if (owner == player) {
            wid_game_map_item_update(level_game);
        }

        return (true);
    }

    if (thing_is_powerup_spam(thing_item) ||
        thing_is_powerup_rocket(thing_item)) {

        thing_item_drop(owner, thing_item, true /* destroy */, "used");

        if (owner == player) {
            wid_game_map_item_update(level_game);
        }

        return (true);
    }

    /*
     * Start the bomb ticking.
     */
    if (thing_is_bomb(thing_item)) {
        thing_set_is_open(thing_item, true);
    }

    if (thing_is_spam(thing_item)) {
        thing_item_drop(owner, thing_item, true /* destroy */, "used");

        /*
         * Make some spam.
         */
        thing_item = thing_new(level, "data/things/powerup_spam");

        thing_item_collect(owner, thing_item);

        if (owner == player) {
            wid_game_map_item_update(level_game);
        }

        return (true);
    }

    if (owner == player) {
        wid_game_map_item_update(level_game);
    }

    return (false);
}

boolean thing_item_drop (thingp owner, thingp thing_item,
                         boolean destroy_on_drop,
                         const char *why)
{
    tree_rootp *items;
    boolean drop;

    verify(owner);
    verify(thing_item);

    items = thing_carried_itemsp(owner);
    if (!items) {
        DIE("%s is not carrying (any items) %s", thing_logname(owner),
            thing_logname(thing_item));
        return (true);
    }

    itemp item = item_find(*items, thing_item);
    if (!item) {
        THING_LOG(owner, "is not carrying %s", thing_logname(thing_item));

        DIE("%s is not carrying %s", thing_logname(owner),
            thing_logname(thing_item));

        return (false);
    }

    thing_set_item_owner(thing_item, 0);

    THING_LOG(owner, "drop: %s (%s)", thing_logname(thing_item), why);

    /*
     * Remove powerups.
     */
    if (thing_is_powerup_rocket(thing_item)) {
        thing_dec_powerup_rocket_count(owner, 1);
    }

    if (thing_is_powerup_spam(thing_item)) {
        thing_dec_powerup_spam_count(owner, 1);
    }

    /*
     * Recalculate maze searches.
     */
    if (thing_is_food(thing_item)) {
        thing_set_redo_maze_search(owner, true);
    }

    drop = item_drop(items, thing_item);
    if (!drop) {
        THING_LOG(owner, "failed to drop: %s", thing_logname(thing_item));

        DIE("%s failed to drop %s", thing_logname(owner),
            thing_logname(thing_item));

        return (false);
    }

    if (destroy_on_drop) {
        thing_dead(thing_item, 0, "dropped");
    }

    return (true);
}

boolean thing_item_drop_all (thingp owner)
{
    tree_rootp items;
    thingp thing;
    itemp item;

    items = thing_carried_items(owner);
    if (!items) {
        return (true);
    }

    item = item_get_first(items);
    if (!item) {
        return (true);
    }

    THING_LOG(owner, "drop all items");

    while (item) {
        thing = item_thing(item);

        item = item_get_next(items, item);

        if (!thing_item_drop(owner, thing, true /* destroy */, "drop all")) {
            THING_LOG(owner, "failed to drop %s", thing_logname(thing));
        }
    }

    return (true);
}

boolean thing_item_pop_all (thingp owner)
{
    tree_rootp items;
    thingp thing;
    itemp item;

    items = thing_carried_items(owner);
    if (!items) {
        return (true);
    }

    item = item_get_first(items);
    if (!item) {
        return (true);
    }

    THING_LOG(owner, "pop all items");

    while (item) {
        thing = item_thing(item);

        item = item_get_next(items, item);

        if (thing_wid(thing)) {
            thing_set_wid(thing, 0);
        }
    }

    return (true);
}

boolean thing_item_steal (thingp owner, thingp thief,
                          thing_template_is_callback loot)
{
    tree_rootp items;
    thingp thing;
    itemp item;

    items = thing_carried_items(owner);
    if (!items) {
        return (true);
    }

    item = item_get_first(items);
    if (!item) {
        return (true);
    }

    boolean aligned_x;
    boolean aligned_y;

    /*
     * Only steal once per move.
     */
    wid_get_grid_coord(thief->wid, &thief->grid_x, &thief->grid_y,
                       &aligned_x, &aligned_y);

    if (!aligned_x || !aligned_y) {
        return (false);
    }

    THING_LOG(thief, "stealing items from %s", thing_logname(owner));

    uint32_t stole = 0;

    while (item) {
        thing = item_thing(item);

        item = item_get_next(items, item);

        if (!(*loot)(thing_get_template(thing))) {
            continue;
        }

        THING_LOG(owner, "steal %s", thing_logname(thing));

        if (!thing_item_drop(owner, thing, false /* destroy */, "stole")) {
            THING_LOG(owner, "failed to drop %s", thing_logname(thing));
        }

        if (!thing_item_collect(thief, thing)) {
            THING_LOG(thief, "failed to steal %s", thing_logname(thing));
        }

        THING_LOG(thief, "stole one item");

        stole++;
    }

    if (stole) {
        thing_message(owner, "You thief!");

        sound_play_thief();
    }

    if ((owner == player) || (thief == player)) {
        wid_game_map_item_update(level_game);
    }

    THING_LOG(thief, "done stealing");

    return (true);
}

boolean thing_item_drop_end_of_life (thingp owner)
{
    tree_rootp items;
    thingp thing;
    itemp item;

    items = thing_carried_items(owner);
    if (!items) {
        return (true);
    }

    item = item_get_first(items);
    if (!item) {
        return (true);
    }

    THING_LOG(owner, "drop end of life items");

    while (item) {
        thing = item_thing(item);

        item = item_get_next(items, item);

        if (thing_is_powerup_rocket(thing)) {
            continue;
        }

        if (!thing_item_drop(owner, thing, true /* destroy */, "end of life")) {
            THING_LOG(owner, "failed to drop %s", thing_logname(thing));
        }
    }

    return (true);
}

boolean thing_item_drop_end_of_level (thingp owner)
{
    tree_rootp items;
    thingp thing;
    itemp item;

    items = thing_carried_items(owner);
    if (!items) {
        return (true);
    }

    item = item_get_first(items);
    if (!item) {
        return (true);
    }

    THING_LOG(owner, "drop end of level items");

    while (item) {
        thing = item_thing(item);

        item = item_get_next(items, item);

        if (!thing_is_item_removed_at_level_end(thing)) {
            continue;
        }

        if (!thing_item_drop(owner, thing, true /* destroy */, "end of level")) {
            THING_LOG(owner, "failed to drop %s", thing_logname(thing));
        }
    }

    return (true);
}
