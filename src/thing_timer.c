/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <SDL.h>

#include "main.h"
#include "tree.h"
#include "thing_private.h"
#include "thing.h"
#include "thing_timer.h"
#include "level.h"
#include "timer.h"
#include "wid.h"
#include "color.h"

/*
 * Various thing timers.
 */
tree_rootp thing_timers;

void thing_timers_destroy (thingp thing)
{
    if (thing->timer_spam) {
        action_timer_destroy(&thing_timers, thing->timer_spam);

        thing->timer_spam = 0;
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
