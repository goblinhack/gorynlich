/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include "main.h"
#include "thing.h"
#include "player.h"
#include "level.h"

thingp player;

uint8_t player_init (void)
{
    return (true);
}

void player_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    player_destroy();
}

void player_destroy (void)
{
    if (!player) {
        return;
    }

    thing_destroy(player, "player destroy");

    player = 0;
}
