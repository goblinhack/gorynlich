/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#pragma once

#include <SDL_net.h>

boolean wid_server_join_init(void);
void wid_server_join_fini(void);
void wid_server_join_hide(void);
void wid_server_join_visible(void);
void wid_server_join_redo(boolean soft_refresh);
boolean wid_server_save_remote_server_list(void);
