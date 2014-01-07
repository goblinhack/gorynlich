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
#include "wid.h"
#include "level.h"
#include "map.h"
#include "wid_tooltip.h"
#include "wid_game_map.h"
#include "sound.h"
#include "timer.h"

/*
 * Have we hit anything?
 */
void thing_collision (thingp t, int32_t x, int32_t y)
{
    thing_templatep me;
    thing_templatep it;
    levelp level;
    widp wid_next;
    widp wid_me;
    widp wid_it;
    thingp thing_it;

    verify(t);

    me = thing_get_template(t);

    if (!thing_template_is_player(me) &&
        !thing_template_is_plant(me) &&
        !thing_template_is_seedpod(me) &&
        !thing_template_is_car(me) &&
        !thing_template_is_bomb(me) &&
        !thing_template_is_esnail(me)) {
        return;
    }

    level = thing_level(t);
    verify(level);

    /*
     * Popped from the level?
     */
    wid_me = thing_wid(t);
    if (!wid_me) {
        return;
    }

    verify(wid_me);

    wid_it = wid_grid_find_first(wid_game_map_grid_container, x, y);
    while (wid_it) {
        verify(wid_it);

        wid_next = wid_grid_find_next(wid_game_map_grid_container,
                                      wid_it, x, y);
        if (wid_me == wid_it) {
            wid_it = wid_next;
            continue;
        }

        if (!wids_overlap(wid_me, wid_it)) {
            wid_it = wid_next;
            continue;
        }

        it = wid_get_thing_template(wid_it);
        if (!it) {
            wid_it = wid_next;
            continue;
        }

        thing_it = wid_get_thing(wid_it);
        if (!thing_it) {
            wid_it = wid_next;
            continue;
        }

        verify(thing_it);

        /*
         * Try to ignore the dead!
         */
        if (thing_is_dead(thing_it)) {
            wid_it = wid_next;
            continue;
        }

        wid_it = wid_next;
    }
}
