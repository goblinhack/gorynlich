/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 *
 */

#pragma once

#include <SDL_net.h>
#include "tree.h"

#include "player.h"
#include "player_stats.h"

typedef enum {
    MSG_PING,
    MSG_PONG,
    MSG_NAME,
    MSG_TELL,
    MSG_CLIENT_SHOUT,
    MSG_SERVER_SHOUT,
    MSG_CLIENT_JOIN,
    MSG_CLIENT_LEAVE,
    MSG_CLIENT_CLOSE,
    MSG_SERVER_CLOSE,
    MSG_SERVER_STATUS,
    MSG_SERVER_HISCORE,
    MSG_SERVER_MAP_UPDATE,
    MSG_SERVER_PLAYER_UPDATE,
    MSG_CLIENT_PLAYER_MOVE,
    MSG_CLIENT_PLAYER_ACTION,
    MSG_MAX,
    MSG_COMPRESSED = 0xFF,
} msg_type;

typedef struct {
    uint8_t type;
    char name[SMALL_STRING_LEN_MAX];
    char pclass[SMALL_STRING_LEN_MAX];
    player_stats_t player_stats;
    uint32_t key;
} __attribute__ ((packed)) msg_client_join;

typedef struct {
    uint8_t type;
} __attribute__ ((packed)) msg_client_leave;

typedef struct {
    uint8_t type;
} __attribute__ ((packed)) msg_client_close;

typedef struct {
    uint8_t type;
} __attribute__ ((packed)) msg_server_close;

typedef struct {
    uint8_t type;
    char name[SMALL_STRING_LEN_MAX];
    char pclass[SMALL_STRING_LEN_MAX];
    player_stats_t player_stats;
} __attribute__ ((packed)) msg_name;

typedef struct {
    uint8_t type;
    uint8_t level;
    char from[SMALL_STRING_LEN_MAX];
    char txt[PLAYER_MSG_MAX];
} __attribute__ ((packed)) msg_client_shout;

typedef struct {
    uint8_t type;
    uint8_t dir;
    uint16_t x;
    uint16_t y;
} __attribute__ ((packed)) msg_player_move;

enum {
    PLAYER_ACTION_USE,
    PLAYER_ACTION_DROP,
};

typedef struct {
    uint8_t type;
    uint8_t action;
    uint16_t item;
} __attribute__ ((packed)) msg_player_action;

typedef struct {
    uint8_t type;
    uint8_t level;
    char txt[PLAYER_MSG_MAX];
} __attribute__ ((packed)) msg_server_shout;

typedef struct {
    uint8_t type;
    char from[SMALL_STRING_LEN_MAX];
    char to[SMALL_STRING_LEN_MAX];
    char txt[PLAYER_MSG_MAX];
} __attribute__ ((packed)) msg_tell;

typedef struct msg_player_state_ {
    char name[SMALL_STRING_LEN_MAX];
    char pclass[SMALL_STRING_LEN_MAX];
    player_stats_t player_stats;
    IPaddress local_ip;
    IPaddress remote_ip;
    uint8_t quality;
    uint16_t avg_latency;
    uint16_t min_latency;
    uint16_t max_latency;
    uint32_t score;
    uint32_t key;
    uint16_t thing_id;
} __attribute__ ((packed)) msg_player_state;

typedef struct {
    uint8_t type;
    msg_player_state players[MAX_PLAYERS];
    char server_name[SMALL_STRING_LEN_MAX];
    uint8_t server_max_players;
    uint8_t server_current_players;
    uint8_t level_no;
    uint8_t level_hide;
} __attribute__ ((packed)) msg_server_status;

typedef struct msg_player_hiscore_ {
    char player_name[SMALL_STRING_LEN_MAX];
    char death_reason[SMALL_STRING_LEN_MAX];
    uint32_t score;
} __attribute__ ((packed)) msg_player_hiscore;

typedef struct {
    uint8_t type;
    uint8_t rejoin_allowed;
    msg_player_hiscore players[MAX_HISCORES+1];
} __attribute__ ((packed)) msg_server_hiscores;

typedef struct {
    /*
     * Upper 8 bits are state
     */
    uint8_t state;
    uint16_t thing_id;
    uint16_t x;
    uint16_t y;
} __attribute__ ((packed)) msg_thing_update;

typedef struct {
    /*
     * We don't actually send this message, this is just the max size it can 
     * be. We send only what portions of this message are changed.
     */
    uint8_t data;
    uint8_t template_id;        // optional
    uint16_t id;                // optional
    uint8_t x;                  // optional
    uint8_t y;                  // optional
    uint16_t owner_id;          // optional
} __attribute__ ((packed)) msg_map_update;

typedef struct {
    uint8_t type;
    uint16_t thing_id;
    uint16_t weapon_carry_anim_id;
    uint16_t weapon_swing_anim_id;
    player_stats_t stats;
    uint8_t weapon;
} __attribute__ ((packed)) msg_player_update;

typedef struct socket_ {
    tree_key_two_int tree;

    uint8_t server;
    uint8_t client;
    uint8_t server_side_client;
    uint8_t connected;
    UDPsocket udp_socket;
    IPaddress remote_ip;
    IPaddress local_ip;
    SDLNet_SocketSet socklist;
    /*
     * If there is a player using this socket.
     */
    aplayerp player;
    /*
     * Line quality.
     */
    uint8_t quality;
    uint16_t avg_latency;
    uint16_t min_latency;
    uint16_t max_latency;
    /*
     * Counters.
     */
    uint32_t rx;
    uint32_t tx;
    uint32_t rx_error;
    uint32_t tx_error;
    uint32_t rx_bad_msg;
    uint32_t ping_responses[SOCKET_PING_SEQ_NO_RANGE];
    uint32_t tx_msg[MSG_MAX];
    uint32_t rx_msg[MSG_MAX];
    int channel;
    const char *local_logname;
    const char *remote_logname;

    /*
     * Player name.
     */
    char name[SMALL_STRING_LEN_MAX];
    char pclass[SMALL_STRING_LEN_MAX];
    player_stats_t player_stats;

    /*
     * Last status from this server.
     */
    msg_server_status server_status;
} socket;

extern void socket_count_inc_pak_rx(const socketp, msg_type);

extern int socket_test(int32_t argc, char *argv[]);
extern uint8_t socket_init(void);
extern void socket_fini(void);
extern char *iptodynstr(IPaddress ip);
extern char *iprawtodynstr(IPaddress ip);
extern char *iptodynstr_no_resolve(IPaddress ip);
extern char *iprawporttodynstr(IPaddress ip);

extern IPaddress server_address;
extern IPaddress no_address;

extern socketp socket_find(IPaddress address);
extern socketp socket_listen(IPaddress address);
extern socketp socket_connect_from_client(IPaddress address);
extern socketp socket_connect_from_server(IPaddress address);
extern void socket_disconnect(socketp s);
extern void sockets_alive_check(void);

extern socketp socket_find_local_ip(IPaddress address);
extern socketp socket_find_remote_ip(IPaddress address);

extern IPaddress socket_get_local_ip(socketp);
extern IPaddress socket_get_remote_ip(socketp);

extern const char *socket_get_player_name(const socketp);
extern const char *socket_get_player_pclass(const socketp);
extern player_stats_t *socket_get_player_player_stats(const socketp);

extern const char *socket_get_local_logname(const socketp);
extern const char *socket_get_remote_logname(const socketp);

extern void socket_set_connected(const socketp, uint8_t);
extern uint8_t socket_get_connected(const socketp);

extern uint8_t socket_get_server(const socketp);
extern msg_server_status *socket_get_server_status(const socketp);
extern uint8_t socket_get_server_side_client(const socketp s);
extern uint8_t socket_get_client(const socketp);

extern void socket_set_channel(socketp, int);
extern uint8_t socket_get_channel(const socketp);

extern UDPsocket socket_get_udp_socket(const socketp);
extern SDLNet_SocketSet socket_get_socklist(const socketp);

extern aplayerp socket_get_player(const socketp);
extern void socket_set_player(const socketp s, aplayerp);

extern void socket_count_inc_pak_tx(const socketp);
extern void socket_count_inc_pak_tx_error(const socketp);
extern void socket_count_inc_pak_rx_bad_msg(const socketp);

extern const char *socket_get_name(const socketp s);
extern const char *socket_get_pclass(const socketp s);
extern const player_stats_t *socket_get_player_stats(const socketp s);
extern void socket_set_name(socketp s, const char *name);
extern void socket_set_pclass(socketp s, const char *pclass);
extern void socket_set_player_stats(socketp s, const player_stats_t *player_stats);

extern void socket_tx_ping(socketp s, uint8_t seq, uint32_t ts);
extern void socket_tx_pong(socketp s, uint8_t seq, uint32_t ts);
extern void socket_rx_ping(socketp s, UDPpacket *packet, uint8_t *data);
extern void socket_rx_pong(socketp s, UDPpacket *packet, uint8_t *data);
extern void socket_tx_name(socketp s);
extern void socket_rx_name(socketp s, UDPpacket *packet, uint8_t *data);
extern uint8_t socket_tx_client_join(socketp s, uint32_t *key);
extern uint8_t socket_rx_client_join(socketp s, 
                                     UDPpacket *packet, uint8_t *data);
extern void socket_tx_client_leave(socketp s);
extern uint8_t socket_rx_client_leave(socketp s, 
                                      UDPpacket *packet, uint8_t *data);
extern void socket_tx_client_close(socketp s);
extern void socket_rx_client_close(socketp s, 
                                   UDPpacket *packet, uint8_t *data);
extern void socket_tx_server_close(void);
extern void socket_rx_server_close(socketp s, 
                                   UDPpacket *packet, uint8_t *data);
extern void socket_tx_client_shout(socketp s, 
                                   uint32_t level,
                                   const char *shout);
extern void socket_rx_client_shout(socketp s, 
                                   UDPpacket *packet, uint8_t *data);
extern void socket_tx_player_move(socketp s, 
                                  thingp t,
                                  const uint8_t up,
                                  const uint8_t down,
                                  const uint8_t left,
                                  const uint8_t right,
                                  const uint8_t fire);
extern void socket_tx_player_action(socketp s, 
                                    thingp t,
                                    const uint8_t action,
                                    const uint16_t item);
extern void socket_server_rx_player_move(socketp s, UDPpacket *packet, 
                                         uint8_t *data);
extern void socket_server_rx_player_action(socketp s, UDPpacket *packet, 
                                         uint8_t *data);
extern void socket_tx_server_shout(uint32_t level,
                                   const char *shout);
extern void socket_tx_server_shout_except_to(uint32_t level,
                                             const char *shout, socketp s);
extern void socket_tx_server_shout_only_to(uint32_t level,
                                           const char *shout, socketp s);
extern void socket_rx_server_shout(socketp s, UDPpacket *packet, 
                                   uint8_t *data);
extern void socket_tx_tell(socketp s, 
                           const char *from, const char *to, 
                           const char *shout);
extern void socket_rx_tell(socketp s, UDPpacket *packet, uint8_t *data);
extern void socket_tx_server_status(void);
extern void socket_rx_server_status(socketp s, UDPpacket *packet, 
                                    uint8_t *data, msg_server_status *);
extern void socket_tx_server_hiscore(socketp only,
                                     const char *name,
                                     const char *reason,
                                     const uint32_t score);
extern void socket_rx_server_hiscore(socketp s, UDPpacket *packet, 
                                    uint8_t *data, msg_server_hiscores *);
extern void sockets_quality_check(void);
extern uint32_t socket_get_quality(socketp s);
extern uint32_t socket_get_avg_latency(socketp s);
extern uint32_t socket_get_min_latency(socketp s);
extern uint32_t socket_get_max_latency(socketp s);
extern uint32_t socket_get_rx(socketp s);
extern uint32_t socket_get_tx(socketp s);
extern uint32_t socket_get_rx_error(socketp s);
extern uint32_t socket_get_tx_error(socketp s);
extern uint32_t socket_get_rx_bad_msg(socketp s);
extern void socket_server_tx_map_update(socketp, tree_rootp, const char *type);
extern void socket_client_rx_map_update(socketp s, 
                                        UDPpacket *packet, uint8_t *data);
extern void socket_server_tx_player_update(thingp);
extern void socket_client_rx_player_update(socketp s,
                                          UDPpacket *packet, uint8_t *data);


/*
 * Seemingly harmless, but we need this to read the 6 byte packet address
 * into an 8 byte unpacked local address.
 */
static inline IPaddress read_address (UDPpacket *packet)
{
    IPaddress i;

    i.host = packet->address.host;
    i.port = packet->address.port;

    return (i);
}

static inline void write_address (UDPpacket *packet, IPaddress i)
{
    packet->address.host = i.host;
    packet->address.port = i.port;
}

static inline uint8_t cmp_address (const IPaddress *a, const IPaddress *b)
{
    return ((a->host == b->host) && (a->port == b->port));
}

extern tree_rootp sockets;

static inline UDPpacket *socket_alloc_msg (void)
{
    static UDPpacket *packet;

    if (!packet) {
        packet = SDLNet_AllocPacket(MAX_PACKET_SIZE);
        if (!packet) {
            DIE("Out of packet space, pak %u", MAX_PACKET_SIZE);
        }
    }

    newptr(packet, "pak");

    return (packet);
}

static inline void socket_free_msg (UDPpacket *packet)
{
    oldptr(packet);

//    SDLNet_FreePacket(packet);
}

void socket_tx_msg(socketp s, UDPpacket *packet);
