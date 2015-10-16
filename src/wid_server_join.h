/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#pragma once

#include <SDL_net.h>

uint8_t wid_server_join_init(void);
void wid_server_join_fini(void);
void wid_server_join_hide(void);
void wid_server_join_visible(void);
void wid_server_join_redo(uint8_t soft_refresh);
uint8_t wid_server_save_remote_server_list(void);
uint8_t wid_server_join(char *host, int port);
uint8_t wid_server_replace(char *host, int port);
