/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <SDL_net.h>

boolean wid_server_init(void);
void wid_server_fini(void);
void wid_server_hide(void);
void wid_server_visible(void);
void wid_server_redo(boolean soft_refresh);
boolean server_save(void);
boolean server_load(void);
