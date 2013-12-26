/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 *
 * A useful hack to help debug stale/free/bad pointers.
 */


#include "SDL_thread.h"
#include "SDL_timer.h"

#include "main.h"
#include "net.h"
#include "server.h"

static boolean server_init_done;
static int server_thread(void *ptr);
static SDL_Thread *thread;

boolean server_init (void)
{
    server_init_done = true;

    thread = SDL_CreateThread(server_thread, "server_thread", (void *)NULL);

    if (!thread) {
        DIE("SDL_CreateThread failed: %s", SDL_GetError());
    }

    return (true);
}

void server_fini (void)
{
    int threadReturnValue;

    FINI_LOG("%s", __FUNCTION__);

    if (server_init_done) {
        server_init_done = false;

        SDL_WaitThread(thread, &threadReturnValue);
    }
}

static int server_thread (void *ptr)
{
    while (!quitting) {
//        CON("threead");
        SDL_Delay(1000);
    }

    return (0);
}
