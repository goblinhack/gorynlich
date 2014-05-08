/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */
typedef boolean (*action_init_fn_callback)(void *);
#pragma once


void action_init_fns_destroy(tree_rootp *root);
void action_init_fn_destroy(tree_rootp *t, init_fnp p);
init_fnp action_init_fn_create(tree_rootp *root,
                               action_init_fn_callback callback,
                               void *context,
                               const char *name);
const char *action_init_fn_logname(init_fnp t);
boolean action_init_fns_tick(tree_rootp *root);

extern tree_rootp init_fns;
