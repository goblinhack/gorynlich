/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info.
 */
typedef void (*action_timer_callback)(void *);
typedef void (*action_timer_destroy_callback)(void *);
#pragma once

void action_timers_destroy(tree_rootp *root);
void action_timer_destroy(tree_rootp *t, timerp p);
timerp action_timer_create(tree_rootp *root,
                    action_timer_callback,
                    action_timer_destroy_callback,
                    void *context,
                    const char *name,
                    uint32_t duration_ms,
                    uint32_t jitter);
const char *action_timer_logname(timerp t);
void action_timers_tick(tree_rootp *root);

extern tree_rootp client_timers;
extern tree_rootp server_timers;
