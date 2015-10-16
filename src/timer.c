/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#include <SDL.h>

#include "main.h"
#include "tree.h"
#include "timer.h"
#include "timer_private.h"
#include "string_util.h"
#include "time_util.h"

/*
 * Various global timers.
 */
tree_rootp client_timers;
tree_rootp server_timers;

void action_timers_destroy (tree_rootp *root)
{
    if (!*root) {
        return;
    }

    verify(*root);

    timerp p;

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
    verify(p);

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
    verify(p);

    if (!t) {
        return;
    }

    verify(*t);

    tree_rootp tree = *t;

    TIMER_LOG(p, "destroy");

    if (!tree) {
        ERR("no timer tree");
    }

    if (!tree_remove(tree, &p->tree.node)) {
        ERR("timer remove [%s] failed", action_timer_logname(p));
    }

    TIMER_LOG(p, "destroyed");

    action_timer_free(p);

    if (!tree_root_size(tree)) {
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

    verify(*root);

    t = (typeof(t)) myzalloc(sizeof(*t), "TREE NODE: timer");

    t->expires_when = time_get_time_ms() + duration_ms;
    if (jitter) {
        t->expires_when += myrand() % jitter;
    }

    t->context = context;
    t->callback = callback;
    t->destroy_callback = destroy_callback;
    t->tree.key2 = t->expires_when;
    t->tree.key3 = tiebreak++;

    if (!tree_insert(*root, &t->tree.node)) {
        ERR("collect timer [%s] failed", action_timer_logname(t));
    }

    t->logname = dynprintf("%s [timer:%p] context:%p", name, t, context);

    TIMER_LOG(t, "created, fires in %3.2f secs",
              ((float)(t->expires_when - time_get_time_ms())) /
              (float)ONESEC);

    return (t);
}

const char *action_timer_logname (timerp t)
{
    verify(t);

    return (t->logname);
}

void action_timers_tick (tree_rootp *root)
{
    timer *t;

    if (!*root) {
        return;
    }

    for (;;) {
        verify(*root);

        t = (typeof(t)) tree_root_first(*root);
        if (!t) {
            return;
        }

        verify(t);

        if (t->expires_when > time_get_time_ms()) {
            return;
        }

        TIMER_LOG(t, "fired");

        (t->callback)(t->context);

        if (!*root) {
            /*
             * Destroyed in callback?
             */
            break;
        }

        verify(t);
        verify(*root);

        tree_remove_found_node(*root, &t->tree.node);

        action_timer_free(t);
    }
}
