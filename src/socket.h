/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 *
 * A useful hack to help debug stale/free/bad pointers.
 */

#include <SDL_net.h>

extern int socket_test(int32_t argc, char *argv[]);
extern boolean socket_init(void);
extern void socket_fini(void);
extern char *iptodynstr(IPaddress ip);

extern IPaddress server_address;
extern IPaddress no_address;

extern socketp socket_listen(IPaddress address);
extern socketp socket_connect(IPaddress address, boolean server);
extern void socket_disconnect(socketp s);
extern void sockets_alive_check(void);

extern socketp socket_get(uint32_t si);
extern socketp socket_find_local_ip(IPaddress address);
extern socketp socket_find_remote_ip(IPaddress address);

extern IPaddress socket_get_local_ip(socketp);
extern IPaddress socket_get_remote_ip(socketp);

extern const char *socket_get_player_name(const socketp);

extern const char *socket_get_local_logname(const socketp);
extern const char *socket_get_remote_logname(const socketp);

extern boolean socket_get_open(const socketp);

extern void socket_set_connected(const socketp, boolean);
extern boolean socket_get_connected(const socketp);

extern boolean socket_get_server(const socketp);
extern boolean socket_get_server_side_client(const socketp s);
extern boolean socket_get_client(const socketp);

extern void socket_set_channel(socketp, int);
extern boolean socket_get_channel(const socketp);

extern UDPsocket socket_get_udp_socket(const socketp);
extern SDLNet_SocketSet socket_get_socklist(const socketp);

extern void socket_count_inc_pak_rx(const socketp);
extern void socket_count_inc_pak_rx_error(const socketp);
extern void socket_count_inc_pak_tx(const socketp);
extern void socket_count_inc_pak_tx_error(const socketp);
extern void socket_count_inc_pak_rx_bad_msg(const socketp);

extern const char *socket_get_name(const socketp s);
extern void socket_set_name(socketp s, const char *name);

typedef enum {
    MSG_TYPE_PING,
    MSG_TYPE_PONG,
    MSG_TYPE_NAME,
    MSG_TYPE_PLAYERS,
    MSG_TYPE_MAX,
} msg_type;

extern void socket_tx_ping(socketp s, uint8_t seq, uint32_t ts);
extern void socket_tx_pong(socketp s, uint8_t seq, uint32_t ts);
extern void socket_rx_ping(socketp s, UDPpacket *packet, uint8_t *data);
extern void socket_rx_pong(socketp s, UDPpacket *packet, uint8_t *data);
extern void socket_tx_name(socketp s, const char *name);
extern void socket_rx_name(socketp s, UDPpacket *packet, uint8_t *data);
extern void socket_tx_players(void);
