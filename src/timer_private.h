/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */

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
    void *context;

} timer;
