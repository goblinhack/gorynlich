/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 *
 * A useful hack to help debug stale/free/bad pointers.
 *
 */

#pragma once

#include <SDL_net.h>

extern uint8_t server_init(void);
extern void server_fini(void);
extern void server_tick(void);
extern uint8_t server_start(IPaddress);
extern void server_stop(void);

extern gsocketp server_socket;
