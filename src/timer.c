/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <SDL.h>

#include "main.h"
#include "tree.h"
#include "timer.h"
#include "timer_private.h"
#include "string.h"
#include "time.h"

void action_timers_destroy (tree_rootp *root)
{
    timerp p;

    if (!root) {
        DIE("no timers to destroy");
    }

    TREE_WALK((*root), p) {
        action_timer_destroy(root, p);
        if (!*root) {
            return;
        }
    }

    if (*root) {
        tree_destroy(root, 0);
    }
}

static void action_timer_free (timerp p)
{
    if (p->destroy_callback) {
        (p->destroy_callback)(p->context);
    }

    if (p->logname) {
        myfree(p->logname);
    }

    myfree(p);
}

void action_timer_destroy (tree_rootp *t, timerp p)
{
    if (!t) {
        return;
    }

    TIMER_LOG(p, "destroy");

    if (!*t) {
        DIE("no timer tree");
    }

    if (!tree_remove(*t, &p->tree.node)) {
        DIE("timer remove [%s] failed", action_timer_logname(p));
    }

    TIMER_LOG(p, "destroyed");

    action_timer_free(p);

    if (!tree_root_size(*t)) {
        tree_destroy(t, 0);
    }
}

timerp action_timer_create (tree_rootp *root,
                            action_timer_callback callback,
                            action_timer_destroy_callback destroy_callback,
                            void *context,
                            const char *name,
                            uint32_t duration_ms,
                            uint32_t jitter)
{
    static uint32_t tiebreak;
    timer *t;

    if (!*root) {
        *root = tree_alloc(TREE_KEY_TWO_INTEGER, "TREE ROOT: timers");
    }

    t = (typeof(t)) myzalloc(sizeof(*t), "TREE NODE: timer");

    t->expires_when = time_get_time_cached() + duration_ms;
    if (jitter) {
        t->expires_when += rand() % jitter;
    }

    t->context = context;
    t->callback = callback;
    t->destroy_callback = destroy_callback;
    t->tree.key2 = t->expires_when;
    t->tree.key3 = tiebreak++;

    if (!tree_insert(*root, &t->tree.node)) {
        DIE("collect timer [%s] failed", action_timer_logname(t));
    }

    t->logname = dynprintf("%s [timer:%p] context:%p", name, t, context);

    TIMER_LOG(t, "created, fires in %3.2f secs",
              ((float)(t->expires_when - time_get_time_cached())) /
              (float)ONESEC);

    return (t);
}

const char *action_timer_logname (timerp t)
{
    verify(t);

    return (t->logname);
}

void action_timers_tick (tree_rootp root)
{
    timer *t;

    if (!root) {
        return;
    }

    for (;;) {
        t = (typeof(t)) tree_root_first(root);
        if (!t) {
            return;
        }

        if (t->expires_when > time_get_time_cached()) {
            return;
        }

        TIMER_LOG(t, "fired");

        (t->callback)(t->context);

        tree_remove_found_node(root, &t->tree.node);

        action_timer_free(t);
    }
}
