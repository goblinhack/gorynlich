/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info.
 */

#pragma once

typedef struct action_init_fn_ {
    tree_key_int tree;

    /*
     * Name for debugging.
     */
    char *logname;

    /*
     * What to do on callback.
     */
    action_init_fn_callback callback;
    void *context;

} init;
