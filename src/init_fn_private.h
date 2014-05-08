/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
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
