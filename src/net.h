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

extern IPaddress server_address;
extern IPaddress no_address;

extern socketp socket_find_local_ip(IPaddress address);
extern socketp socket_find_remote_ip(IPaddress address);

extern socketp net_listen(IPaddress address);
extern socketp net_connect(IPaddress address);

extern IPaddress socket_get_local_ip(socketp);
extern IPaddress socket_get_remote_ip(socketp);

extern const char * socket_get_local_logname(const socketp);
extern const char * socket_get_remote_logname(const socketp);

extern void socket_set_server(socketp, boolean);
extern boolean socket_get_server(const socketp);

extern void socket_set_client(socketp, boolean);
extern boolean socket_get_client(const socketp);

extern void socket_set_channel(socketp, int);
extern boolean socket_get_channel(const socketp);

extern UDPsocket socket_get_udp_socket(const socketp);
extern SDLNet_SocketSet socket_get_socklist(const socketp);
