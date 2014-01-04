/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file.
 */

#include <SDL_net.h>

boolean player_init(void);
void player_fini(void);

thingp player_new(levelp, const char *name);
void player_destroy(void);

/*
 * Individual messages.
 */
#define PLAYER_NAME_MAX 20

typedef struct aplayer_ {
    IPaddress local_ip;
    IPaddress remote_ip;
    char name[PLAYER_NAME_MAX];
} aplayer;
