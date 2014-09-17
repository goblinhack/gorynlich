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
    uint8_t quality;
    uint16_t avg_latency;
    uint16_t min_latency;
    uint16_t max_latency;
    uint32_t key;
    char name[SMALL_STRING_LEN_MAX + 1];
    char pclass[SMALL_STRING_LEN_MAX + 1];
    player_stats_t player_stats;
    thingp thing;
} aplayer;

#endif
