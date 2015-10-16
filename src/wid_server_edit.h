/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#pragma once

#include <SDL_net.h>

uint8_t wid_server_edit_init(const char *host, int port);
void wid_server_edit_fini(void);
void wid_server_edit_hide(void);
void wid_server_edit_visible(void);
void wid_server_edit_redo(void);
uint8_t wid_server_save_local_server(void);
