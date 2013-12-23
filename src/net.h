/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 *
 * A useful hack to help debug stale/free/bad pointers.
 */

#include <SDL_net.h>

extern int net_test(int32_t argc, char *argv[]);
extern boolean net_init(void);
extern void net_fini(void);
char *iptodynstr(IPaddress ip);

#define NET_MAX_HOSTS 10

typedef struct host_ {
    IPaddress ip;
    char *logname;
    char *name;
    uint32_t delay_ms;
    boolean inuse;
} host;

const char *host_logname(hostp);
hostp host_add(IPaddress ip, const char *name);
void host_dump(void);

extern boolean is_server;
extern boolean is_client;
extern IPaddress server_address;
extern IPaddress client_address;
