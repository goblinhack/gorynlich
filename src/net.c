/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 *
 * A useful hack to help debug stale/free/bad pointers.
 */

#include <SDL.h>
#include <SDL_net.h>

#include "main.h"
#include "net.h"

static boolean net_init_done;

boolean net_init (void)
{
    if (SDLNet_Init() < 0) {
        ERR("cannot init SDL_net");
        return (false);
    }

    net_init_done = true;

    return (true);
}

void net_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (net_init_done) {
        SDLNet_Quit();
        net_init_done = false;
    }
}
