/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */


#include "main.h"
#include "thing.h"
#include "thing_timer.h"
#include "timer.h"
#include "wid_game_map_server.h"
#include "wid_game_map_client.h"

/*
 * Various thing timers.
 */
tree_rootp thing_timers;

void thing_timers_destroy (thingp thing)
{
    if (!thing->timer_dead_tree) {
        if (thing->timer_dead) {
            ERR("dead timer still running but no tree?");
            thing->timer_dead = 0;
            thing->timer_dead_tree = 0;
            return;
        }
    }

    if (thing->timer_dead) {
        if (!thing->timer_dead_tree) {
            ERR("timer dead is set but has no tree");
            thing->timer_dead = 0;
            thing->timer_dead_tree = 0;
            return;
        }

        action_timer_destroy(&thing->timer_dead_tree, thing->timer_dead);
        thing->timer_dead = 0;
        thing->timer_dead_tree = 0;
    }
}

static void thing_action_timer_callback_dead (void *context)
{
    thing_place_context_t *place;

    place = (typeof(place)) context;

    thingp thing;
    if (place->on_server) {
        thing = thing_server_find(place->thing_id);
    } else {
        thing = thing_client_find(place->thing_id);
    }

    if (thing) {
        verify(thing);
        thing->timer_dead = 0;
        thing->timer_dead_tree = 0;

        thing_dead(thing, 0, "callback");
    }

    place->thing_id = 0;
}

static void thing_action_timer_destroy_callback_dead (void *context)
{
    thing_place_context_t *place;

    place = (typeof(place)) context;

    if (place->thing_id) {
        thingp thing;

        if (place->on_server) {
            thing = thing_server_find(place->thing_id);
            if (!thing) {
                ERR("timer cleanup, server thing ID %u is gone", place->thing_id);
                return;
            }
        } else{
            thing = thing_client_find(place->thing_id);
            if (!thing) {
                ERR("timer cleanup, client thing ID %u is gone", place->thing_id);
                return;
            }
        }

        verify(thing);
        thing->timer_dead = 0;
        thing->timer_dead_tree = 0;

        thing_dead(thing, 0, "timer aborted");
    }

    myfree(context);
}

void thing_timer_place_and_destroy_callback (void *context)
{
    thing_place_context_t *place;

    place = (typeof(place)) context;

    if (!place->tp) {
        ERR("no thing to place");
    }

    widp w;

    if (place->on_server) {
        w = wid_game_map_server_replace_tile(
                                        wid_game_map_server_grid_container,
                                        place->x,
                                        place->y,
                                        0, /* thing */
                                        place->tp,
                                        0, /* tpp data */
                                        0 /* item */,
                                        0 /* stats */);
    } else {
        thingp t = thing_client_local_new(place->tp);

        w = wid_game_map_client_replace_tile(
                                        wid_game_map_client_grid_container,
                                        place->x,
                                        place->y,
                                        t,
                                        0 /* tpp */);
    }

    /*
     * Some things like projectiles can go offscrenn.
     */
    if (!w) {
        return;
    }

    /*
     * Just pass the same context along as it has the expire time but add
     * the newborn thing.
     */
    thingp t = wid_get_thing(w);

    if (place->is_epicenter) {
        t->is_epicenter = 1;
    }

    place->thing_id = t->thing_id;

    /*
     * Save the owner of this new thing. This could be who cast a spell.
     */
    thing_set_owner_id(t, place->owner_id);

    /*
     * Make a copy of the original context that is used in the next timer.
     */
    thing_place_context_t *context2;

    context2 = (typeof(context2)) myzalloc(sizeof(*context2), 
                                           "place and destroy thing");
    memcpy(context2, context, sizeof(*context2));

    if (place->owner_id) {
        thingp owner = thing_server_id(place->owner_id);
        if (!owner) {
            ERR("no owner ID %u for explosion", place->owner_id);
        }

        t->timer_dead = action_timer_create(
                &owner->timers,
                (action_timer_callback)thing_action_timer_callback_dead,
                (action_timer_callback)thing_action_timer_destroy_callback_dead,
                context2,
                "kill thing",
                place->destroy_in,
                0 /* jitter */);

        t->timer_dead_tree = owner->timers;
    } else {
        t->timer_dead = action_timer_create(
                &thing_timers,
                (action_timer_callback)thing_action_timer_callback_dead,
                (action_timer_callback)thing_action_timer_destroy_callback_dead,
                context2,
                "kill thing",
                place->destroy_in,
                0 /* jitter */);

        t->timer_dead_tree = thing_timers;
    }
}

void thing_timer_destroy (thingp t, uint32_t destroy_in)
{
    thing_place_context_t *context;

    context = (typeof(context)) myzalloc(sizeof(*context), "destroy thing");
    memcpy(context, context, sizeof(*context));

    context->thing_id = t->thing_id;
    context->on_server = t->on_server ? 1 : 0;

    action_timer_create(
            &t->timers,
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

    widp w;

    if (place->on_server) {
        w = wid_game_map_server_replace_tile(
                                        wid_game_map_server_grid_container,
                                        place->x,
                                        place->y,
                                        0, /* thing */
                                        place->tp,
                                        0, /* tpp data */
                                        0 /* item */,
                                        0 /* stats */);
    } else {
        thingp t = thing_client_local_new(place->tp);

        w = wid_game_map_client_replace_tile(
                                        wid_game_map_client_grid_container,
                                        place->x,
                                        place->y,
                                        t,
                                        0 /* tpp */);
    }

    /*
     * Some things like projectiles can go offscrenn.
     */
    if (!w) {
        return;
    }

    thingp t = wid_get_thing(w);

    /*
     * Save the owner of this new thing. This could be who cast a spell.
     */
    thing_set_owner_id(t, place->owner_id);
}

void thing_timer_place_destroy_callback (void *context)
{
    myfree(context);
}
