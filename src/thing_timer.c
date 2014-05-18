/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "tree.h"
#include "thing.h"
#include "thing_timer.h"
#include "level.h"
#include "timer.h"
#include "wid.h"
#include "color.h"
#include "wid_game_map_server.h"

/*
 * Various thing timers.
 */
tree_rootp thing_timers;

void thing_timers_destroy (thingp thing)
{
    if (!thing_timers) {
        return;
    }

    if (thing->timer_spam) {
        action_timer_destroy(&thing_timers, thing->timer_spam);

        thing->timer_spam = 0;
    }

    if (thing->timer_dead) {
        action_timer_destroy(&thing_timers, thing->timer_dead);

        thing->timer_dead = 0;
    }
}

void thing_action_timer_callback_spam (void *context)
{
    thingp thing;
    thing = (typeof(thing)) context;
    verify(thing);

    thing->timer_spam = 0;

    widp w = thing_message(thing, "Spam!");
    if (w) {
        wid_set_color(w, WID_COLOR_TEXT, PINK1);
    }

    THING_LOG(thing, "timer spam fired");

    /*
     * While carrying spam, keep on firing the timer.
     */
    if (thing_has_powerup_spam_count(thing)) {
        thing->timer_spam =
                action_timer_create(&thing_timers,
                                    thing_action_timer_callback_spam,
                                    (void*) thing,
                                    "spam",
                                    ONESEC / 2, 0);
    }
}

static void thing_action_timer_callback_dead (void *context)
{
    thing_place_context_t *place;

    place = (typeof(place)) context;

    thingp thing;
    thing = place->thing;
    verify(thing);

    thing->timer_dead = 0;

    thing_dead(thing, 0, "callback");

    myfree(context);
}

void thing_timer_place_and_destroy_callback (void *context)
{
    thing_place_context_t *place;

    place = (typeof(place)) context;

    if (!place->thing_template) {
        DIE("no thing to place");
    }

    widp w = wid_game_map_server_replace_tile(
                                    wid_game_map_server_grid_container,
                                    place->x,
                                    place->y,
                                    place->thing_template);

    /*
     * Just pass the same context along as it has the expire time but add
     * the newborn thing.
     */
    thingp t = place->thing = wid_get_thing(w);

    t->timer_dead = action_timer_create(
            &timers,
            (action_timer_callback)thing_action_timer_callback_dead,
            context,
            "kill thing",
            place->destroy_in,
            0 /* jitter */);
}

void thing_timer_place_callback (void *context)
{
    thing_place_context_t *place;

    place = (typeof(place)) context;

    wid_game_map_server_replace_tile(wid_game_map_server_grid_container,
                                     place->x,
                                     place->y,
                                     place->thing_template);

    myfree(context);
}
