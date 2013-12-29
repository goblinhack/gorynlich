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
extern char *iptodynstr(IPaddress ip);

extern IPaddress listen_address;
extern IPaddress connect_address;

typedef struct {
    boolean open:1;
    boolean server:1;
    boolean client:1;
    UDPsocket udp_socket;
    IPaddress remote_ip;
    IPaddress local_ip;
    char *local_logname;
    char *remote_logname;
    int channel;
    SDLNet_SocketSet socklist;
} socket;

typedef struct {
    socket sockets[MAX_SOCKETS];
} network;

extern socket *net_listen(IPaddress address);
extern socket *net_connect(IPaddress address);
extern network net;
extern IPaddress no_address;
