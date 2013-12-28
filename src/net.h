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

extern IPaddress server_address;
extern IPaddress client_address;

typedef struct {
    boolean open;
    boolean server;
    UDPsocket udp_socket;
    IPaddress client;
    char *logname;
} socket;

typedef struct {
    socket sockets[MAX_SOCKETS];
    SDLNet_SocketSet socklist;
} network;

extern network net;
