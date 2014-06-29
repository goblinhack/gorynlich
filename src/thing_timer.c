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
        if (thing->timer_dead) {
            DIE("dead timer still running but no tree?");
        }

        return;
    }

    if (thing->timer_dead) {
        action_timer_destroy(&thing_timers, thing->timer_dead);

        thing->timer_dead = 0;

        if (!thing_timers) {
            return;
        }
    }
}

static void thing_action_timer_callback_dead (void *context)
{
    thing_place_context_t *place;

    place = (typeof(place)) context;

    thingp thing = thing_server_find(place->thing_id);
    if (thing) {
        verify(thing);
        thing->timer_dead = 0;
        thing_dead(thing, 0, "callback");
    }

    place->thing_id = 0;
}

static void thing_action_timer_destroy_callback_dead (void *context)
{
    thing_place_context_t *place;

    place = (typeof(place)) context;

    if (place->thing_id) {
        thingp thing = thing_server_find(place->thing_id);
        if (thing) {
            verify(thing);
            thing->timer_dead = 0;
        }
    }

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
                                    0, /* thing */
                                    place->thing_template);

    /*
     * Just pass the same context along as it has the expire time but add
     * the newborn thing.
     */
    thingp t = wid_get_thing(w);

    place->thing_id = t->thing_id;

    /*
     * Save the owner of this new thing. This could be who cast a spell.
     */
    t->owner_id = place->owner_id;

    /*
     * Make a copy of the original context that is used in the next timer.
     */
    thing_place_context_t *context2;

    context2 = (typeof(context2)) myzalloc(sizeof(*context2), 
                                           "place and destroy thing");
    memcpy(context2, context, sizeof(*context2));

    t->timer_dead = action_timer_create(
            &thing_timers,
            (action_timer_callback)thing_action_timer_callback_dead,
            (action_timer_callback)thing_action_timer_destroy_callback_dead,
            context2,
            "kill thing",
            place->destroy_in,
            0 /* jitter */);
}

void thing_timer_destroy (thingp t, uint32_t destroy_in)
{
    thing_place_context_t *context;

    context = (typeof(context)) myzalloc(sizeof(*context), "destroy thing");
    memcpy(context, context, sizeof(*context));

    context->thing_id = t->thing_id;

    t->timer_dead = action_timer_create(
            &thing_timers,
            (action_timer_callback)thing_action_timer_callback_dead,
            (action_timer_callback)thing_action_timer_destroy_callback_dead,
            context,
            "kill thing",
            destroy_in,
            0 /* jitter */);
}

void thing_timer_place_and_destroy_destroy_callback (void *context)
{
    thing_place_context_t *place;

    place = (typeof(place)) context;

    myfree(place);
}

void thing_timer_place_callback (void *context)
{
    thing_place_context_t *place;

    place = (typeof(place)) context;

    widp w = wid_game_map_server_replace_tile(
                                    wid_game_map_server_grid_container,
                                    place->x,
                                    place->y,
                                    0, /* thing */
                                    place->thing_template);

    thingp t = wid_get_thing(w);

    /*
     * Save the owner of this new thing. This could be who cast a spell.
     */
    t->owner_id = place->owner_id;
}

void thing_timer_place_destroy_callback (void *context)
{
    thing_place_context_t *place;

    place = (typeof(place)) context;

    myfree(context);
}
