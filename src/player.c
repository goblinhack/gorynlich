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

boolean player_init (void)
{
    return (true);
}

void player_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    player_destroy();
}

/*
 * Create a new player.
 */
thingp player_new (levelp level, const char *name)
{
    if (!player) {
        player = thing_new(level, name);

        thing_set_level_no(player, start_level);
        thing_set_lives(player, start_lives);
    }

    return (player);
}

void player_destroy (void)
{
    if (!player) {
        return;
    }

    thing_destroy(player, "player destroy");

    player = 0;
}
