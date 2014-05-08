/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 *
 * A useful hack to help debug stale/free/bad pointers.
 * #pragma once
 *
 */

#include <SDL_net.h>

extern boolean server_init(void);
extern void server_fini(void);
extern void server_tick(void);
extern boolean server_start(IPaddress);
extern void server_stop(void);

extern socketp server_socket;
