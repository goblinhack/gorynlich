/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */

#pragma once

#include <SDL_net.h>

#ifndef _PLAYER_H_
#define _PLAYER_H_

uint8_t player_init(void);
void player_fini(void);

void player_destroy(void);

/*
 * Individual messages.
 */
typedef struct aplayer_ {
    socketp socket;
    IPaddress local_ip;
    IPaddress remote_ip;
    uint32_t key;
    thing_stats stats_from_client;
    thingp thing;
} aplayer;

#endif
