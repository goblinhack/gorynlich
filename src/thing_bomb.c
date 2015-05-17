/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */


#include "main.h"
#include "thing.h"

thingp level_place_bomb (levelp level, 
                         thingp owner,
                         double x, double y)
{
    widp w = thing_place_behind_or_under(owner,
                                         id_to_tp(THING_BOMB),
                                         0 /* itemp */);
    if (!w) {
        ERR("could not place bomb");
        return (0);
    }

    thingp t = wid_get_thing(w);

    /*
     * Set no owner. Bombs should kill their owner too!
     *
    thing_set_owner(t, owner);
     */

    thing_wake(t);

    return (t);
}
