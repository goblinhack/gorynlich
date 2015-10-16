/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info.
 */

#pragma once

typedef struct action_timer_ {
    tree_key_two_int tree;

    /*
     * Name for debugging.
     */
    char *logname;

    /*
     * Expires when ?
     */
    uint32_t expires_when;

    /*
     * What to do on callback.
     */
    action_timer_callback callback;

    /*
     * What to do on timer free.
     */
    action_timer_destroy_callback destroy_callback;

    void *context;

} timer;
