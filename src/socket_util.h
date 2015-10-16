/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#pragma once

#include <SDL_net.h>
#include "tree.h"

#include "player.h"
#include "thing_stats.h"

typedef enum {
    MSG_PING,
    MSG_PONG,
    MSG_TELL,
    MSG_CLIENT_SHOUT,
    MSG_SERVER_SHOUT,
    MSG_CLIENT_JOIN,
    MSG_CLIENT_LEAVE,
    MSG_CLIENT_CLOSE,
    MSG_SERVER_CLOSE,
    MSG_CLIENT_STATUS,
    MSG_SERVER_STATUS,
    MSG_SERVER_HISCORE,
    MSG_SERVER_MAP_UPDATE,
    MSG_CLIENT_PLAYER_MOVE,
    MSG_CLIENT_PLAYER_ACTION,
    MSG_SUPER_PACKET,
    MSG_MAX,
    MSG_COMPRESSED = 0xFF,
} msg_type;

enum {
    SOCKET_LISTEN,
    SOCKET_CONNECT,
    SOCKET_CONNECT_SERVER_SIDE,
};

typedef struct {
    uint8_t type;
    uint16_t len;
} __attribute__ ((packed)) msg_super_packet;

typedef struct {
    uint8_t type;
    thing_stats stats;
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
    thing_stats stats;
} __attribute__ ((packed)) msg_client_status;

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
    PLAYER_ACTION_PAUSE_GAME,
    PLAYER_ACTION_RESUME_GAME,
    PLAYER_ACTION_DROP,
    PLAYER_ACTION_PAY,
    PLAYER_ACTION_STOP_USE,
};

typedef struct {
    uint8_t type;
    uint8_t action;
    uint8_t action_bar_index;
    uint8_t change_selection_only;
} __attribute__ ((packed)) msg_player_action;

typedef struct {
    uint8_t type;
    uint8_t level;
    uint8_t x;
    uint8_t y;
    /*
     * Used to place a message above a thing
     */
    uint16_t thing_id;
    char txt[PLAYER_MSG_MAX];
} __attribute__ ((packed)) msg_server_shout;

typedef struct {
    uint8_t type;
    char from[SMALL_STRING_LEN_MAX];
    char to[SMALL_STRING_LEN_MAX];
    char txt[PLAYER_MSG_MAX];
} __attribute__ ((packed)) msg_tell;

typedef struct msg_player_state_ {
    thing_stats stats;
    IPaddress local_ip;
    IPaddress remote_ip;
} __attribute__ ((packed)) msg_player_state;

typedef struct {
    uint8_t type;
    msg_player_state player;
    /*
     * This indicates there is a player on this server currently.
     */
    uint8_t you_are_playing_on_this_server;
    uint8_t level_hide;
    uint8_t server_current_players;
    level_pos_t level_pos;
    uint32_t seed;
} __attribute__ ((packed)) msg_server_status;

typedef struct {
    uint8_t type;
    uint8_t seq;
    uint32_t ts;
    char server_name[SMALL_STRING_LEN_MAX];
    char player_name[MAX_PLAYERS][SMALL_STRING_LEN_MAX];
    char level_name[SMALL_STRING_LEN_MAX];
    uint8_t server_max_players;
    uint8_t server_current_players;
} __attribute__ ((packed)) msg_pong;

typedef struct msg_player_hiscore_ {
    char player_name[SMALL_STRING_LEN_MAX];
    char death_reason[SMALL_STRING_LEN_MAX];
    uint32_t score;
} __attribute__ ((packed)) msg_player_hiscore;

typedef struct msg_server_hiscores_ {
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
    uint8_t ext;
    uint16_t template_id;       // optional
    uint16_t x;                 // optional
    uint16_t y;                 // optional
    uint16_t id;                // optional

    /*
     * THING_STATE_BIT_SHIFT_EXT1_WEAPON_ID_PRESENT
     */
    uint16_t weapon_id;

    /*
     * THING_STATE_BIT_SHIFT_EXT1_SHIELD_ID_PRESENT
     */
    uint16_t shield_id;

    /*
     * THING_STATE_BIT_SHIFT_EXT2_MAGIC_ID_PRESENT
     */
    uint16_t magic_id;

    /*
     * THING_STATE_BIT_SHIFT_EXT1_EFFECT_PRESENT
     */
    uint8_t effect;

    /*
     * THING_STATE_BIT_SHIFT_EXT2_TORCH_LIGHT_RADIUS
     */
    uint8_t torch_light;

    /*
     * THING_STATE_BIT_SHIFT_EXT2_COLOR
     */
    uint8_t r, g, b;

    /*
     * THING_STATE_BIT_SHIFT_EXT2_SCALE
     */
    uint8_t scale;

} __attribute__ ((packed)) msg_map_update;

/*
 * The number of packets we allow to buffer up on a socket during sends.
 */
#define MAX_SOCKET_QUEUE_SIZE       256

typedef struct socket_ {
    tree_key_three_int tree;

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
     * Ping sequence number
     */
    uint8_t tx_ping_seq;

    /*
     * Line quality.
     */
    uint8_t quality;
    uint16_t avg_latency_rtt;
    uint16_t min_latency_rtt;
    uint16_t max_latency_rtt;

    /*
     * Counters.
     */
    uint32_t rx;
    uint32_t tx;
    uint32_t tx_last_ping;
    uint32_t rx_error;
    uint32_t tx_error;
    uint32_t rx_bad_msg;
    uint32_t tx_msg[MSG_MAX];
    uint32_t rx_msg[MSG_MAX];
    int channel;
    const char *local_logname;
    const char *remote_logname;
    uint32_t latency_rtt[SOCKET_PING_SEQ_NO_RANGE];

    /*
     * Player name.
     */
    thing_stats stats;

    /*
     * Last status from this server.
     */
    msg_server_status server_status;

    /*
     * Server name and players on this server.
     */
    char server_name[SMALL_STRING_LEN_MAX];
    char player_name[MAX_PLAYERS][SMALL_STRING_LEN_MAX];
    uint8_t server_max_players;
    uint8_t server_current_players;

    UDPpacket *tx_queue[MAX_SOCKET_QUEUE_SIZE];
    uint8_t tx_queue_head;
    uint8_t tx_queue_tail;
    uint8_t tx_queue_size;

    UDPpacket *rx_queue[MAX_SOCKET_QUEUE_SIZE];
    uint8_t rx_queue_head;
    uint8_t rx_queue_tail;
    uint8_t rx_queue_size;

    /*
     * Only used with ENABLE_PAK_EXTRA_HEADER
     */
    uint8_t tx_seq;
    uint8_t rx_seq;
    uint8_t rx_seq_valid;

} gsocket;

extern void socket_count_inc_pak_rx(const gsocketp, msg_type);

extern int socket_test(int32_t argc, char *argv[]);
extern uint8_t socket_init(void);
extern void socket_fini(void);
extern char *iptodynstr(IPaddress ip);
extern char *iprawtodynstr(IPaddress ip);
extern char *iptodynstr_no_resolve(IPaddress ip);
extern char *iprawporttodynstr(IPaddress ip);

extern IPaddress server_address;
extern IPaddress no_address;

extern gsocketp socket_find(IPaddress address, int);
extern gsocketp socket_listen(IPaddress address);
extern gsocketp socket_connect_from_client(IPaddress address);
extern gsocketp socket_connect_from_server(IPaddress address);
extern void socket_disconnect(gsocketp s);
extern void sockets_alive_check(void);

extern gsocketp socket_find_local_ip(IPaddress address);
extern gsocketp socket_find_remote_ip(IPaddress address);
extern gsocketp socket_find_server_side_remote_ip(IPaddress address);

extern IPaddress socket_get_local_ip(gsocketp);
extern IPaddress socket_get_remote_ip(gsocketp);

extern const char *socket_get_player_name(const gsocketp);
extern const char *socket_get_player_pclass(const gsocketp);
extern thing_statsp socket_get_player_player_stats(const gsocketp);

extern const char *socket_get_local_logname(const gsocketp);
extern const char *socket_get_remote_logname(const gsocketp);

extern void socket_set_connected(const gsocketp, uint8_t);
extern uint8_t socket_get_connected(const gsocketp);

extern uint8_t socket_get_server(const gsocketp);
extern msg_server_status *socket_get_server_status(const gsocketp);
extern uint8_t socket_get_server_side_client(const gsocketp s);
extern uint8_t socket_get_client(const gsocketp);

extern void socket_set_channel(gsocketp, int);
extern uint8_t socket_get_channel(const gsocketp);

extern UDPsocket socket_get_udp_socket(const gsocketp);
extern SDLNet_SocketSet socket_get_socklist(const gsocketp);

extern aplayerp socket_get_player(const gsocketp);
extern void socket_set_player(const gsocketp s, aplayerp);

extern void socket_count_inc_pak_tx(const gsocketp);
extern void socket_count_inc_pak_tx_error(const gsocketp);
extern void socket_count_inc_pak_rx_bad_msg(const gsocketp);

extern const char *socket_get_name(const gsocketp s);
extern const char *socket_get_pclass(const gsocketp s);
extern const thing_statsp socket_get_player_stats(const gsocketp s);
extern void socket_set_name(gsocketp s, const char *name);
extern void socket_set_pclass(gsocketp s, const char *pclass);
extern int socket_set_player_stats(gsocketp s, const thing_statsp stats);

extern void socket_tx_ping(gsocketp s, uint8_t *seq, uint32_t ts);
extern void socket_tx_pong(gsocketp s, uint8_t seq, uint32_t ts);
extern void socket_rx_ping(gsocketp s, UDPpacket *packet, uint8_t *data);
extern void socket_rx_pong(gsocketp s, UDPpacket *packet, uint8_t *data);
extern void socket_tx_client_status(gsocketp s);
extern void socket_rx_client_status(gsocketp s, UDPpacket *packet, uint8_t *data);
extern uint8_t socket_tx_client_join(gsocketp s, uint32_t *key);
extern uint8_t socket_rx_client_join(gsocketp s, 
                                     UDPpacket *packet, uint8_t *data);
extern void socket_tx_client_leave(gsocketp s);
extern uint8_t socket_rx_client_leave(gsocketp s, 
                                      UDPpacket *packet, uint8_t *data);
extern void socket_tx_client_close(gsocketp s);
extern void socket_rx_client_close(gsocketp s, 
                                   UDPpacket *packet, uint8_t *data);
extern void socket_tx_server_close(void);
extern void socket_rx_server_close(gsocketp s, 
                                   UDPpacket *packet, uint8_t *data);
extern void socket_tx_client_shout(gsocketp s, 
                                   uint32_t level,
                                   const char *shout);
extern void socket_rx_client_shout(gsocketp s, 
                                   UDPpacket *packet, uint8_t *data);

extern void socket_tx_player_move(gsocketp s, 
                                  thingp t,
                                  const uint8_t up,
                                  const uint8_t down,
                                  const uint8_t left,
                                  const uint8_t right,
                                  const uint8_t fire,
                                  const uint8_t magic);

extern void socket_tx_player_action(gsocketp s, 
                                    thingp t,
                                    const uint8_t action,
                                    const uint32_t action_bar_index,
                                    const int change_selection_only);

extern void socket_server_rx_player_move(gsocketp s, 
                                         UDPpacket *packet, 
                                         uint8_t *data);

extern void socket_server_rx_player_action(gsocketp s, 
                                           UDPpacket *packet,
                                           uint8_t *data);

extern void socket_tx_server_shout_at_all_players(uint32_t level,
                                                  double x,
                                                  double y,
                                                  const char *shout);

extern void socket_tx_server_shout_at_all_players_except(gsocketp,
                                                         uint32_t level,
                                                         double x,
                                                         double y,
                                                         const char *shout);

extern void socket_tx_server_shout_only_to(gsocketp,
                                           uint32_t level,
                                           double x,
                                           double y,
                                           const char *shout);

extern void socket_tx_server_shout_over(uint32_t level,
                                        uint32_t thing_id,
                                        double x,
                                        double y,
                                        const char *txt);

extern void socket_rx_server_shout(gsocketp s, UDPpacket *packet, 
                                   uint8_t *data);

extern void socket_tx_tell(gsocketp s, 
                           const char *from, const char *to, 
                           const char *shout);

extern void socket_rx_tell(gsocketp s, UDPpacket *packet, uint8_t *data);
extern void socket_tx_server_status(gsocketp s);
extern void socket_rx_server_status(gsocketp s, UDPpacket *packet, 
                                    uint8_t *data, msg_server_status *);
extern void socket_tx_server_hiscore(gsocketp only,
                                     const char *name,
                                     const char *reason,
                                     const uint32_t score);
extern void socket_rx_server_hiscore(gsocketp s, UDPpacket *packet, 
                                    uint8_t *data, msg_server_hiscores *);
extern void sockets_quality_check(void);
extern uint32_t socket_get_quality(gsocketp s);
extern const char *socket_get_server_name(gsocketp s);
extern const char *socket_get_other_player_name(gsocketp s, const uint32_t p);
extern uint32_t socket_get_avg_latency_rtt(gsocketp s);
extern uint32_t socket_get_current_players(gsocketp s);
extern uint32_t socket_get_max_players(gsocketp s);
extern uint32_t socket_get_min_latency_rtt(gsocketp s);
extern uint32_t socket_get_max_latency_rtt(gsocketp s);
extern uint32_t socket_get_rx(gsocketp s);
extern uint32_t socket_get_tx(gsocketp s);
extern uint32_t socket_get_rx_error(gsocketp s);
extern uint32_t socket_get_tx_error(gsocketp s);
extern uint32_t socket_get_rx_bad_msg(gsocketp s);
extern void socket_server_tx_map_update(gsocketp, 
                                        tree_rootp, const char *type);
extern void socket_client_rx_map_update(gsocketp s, 
                                        UDPpacket *packet, uint8_t *data);
extern void socket_server_tx_player_update(thingp);
extern void socket_client_rx_player_update(gsocketp s,
                                           UDPpacket *packet, uint8_t *data);
extern void socket_tx_tick(void);
extern void socket_rx_tick(void);

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

/*
 * address_resolve
 *
 * Avpid calling SDLNet_ResolveHost unless we have to as DNS look ups can 
 * block for a long time (seconds).
 */
static inline int address_resolve (IPaddress *address,
                                   const char *host,
                                   const uint16_t port)
{
    unsigned int n1;
    unsigned int n2;
    unsigned int n3;
    unsigned int n4;
    uint32_t addr;

    if (!host || !strcasecmp(host, "localhost")) {
        n1 = 0;
        n2 = 0;
        n3 = 0;
        n4 = 0;

        addr = (n1 << 24) | (n2 << 16) | (n3 << 8) | n4;

        SDLNet_Write32(addr, &address->host);
    } else if (sscanf(host, "%u.%u.%u.%u", &n1, &n2, &n3, &n4) == 4) {
        addr = (n1 << 24) | (n2 << 16) | (n3 << 8) | n4;

        if ((n1 > 255) || (n2 > 255) || (n3 > 255) || (n4 > 255)) {
            return (-1);
        }

        SDLNet_Write32(addr, &address->host);
    } else {
        LOG("Resolving address [%s]...", host);

        if ((SDLNet_ResolveHost(address, host, port)) == -1) {
            ERR("Cannot resolve host [%s] port %u", host, port);

            return (-1);
        }
    }

    SDLNet_Write16(port, &address->port);

    return (0);
}

/*
 * is_ip_address
 *
 * Is the string in a.b.c.d format?
 */
static inline int is_ip_address (const char *host)
{
    unsigned int n1;
    unsigned int n2;
    unsigned int n3;
    unsigned int n4;

    if (!host) {
        return (0);
    }

    if (sscanf(host, "%u.%u.%u.%u", &n1, &n2, &n3, &n4) == 4) {
        if ((n1 > 255) || (n2 > 255) || (n3 > 255) || (n4 > 255)) {
            return (0);
        }

        return (1);
    }

    return (0);
}

extern tree_rootp sockets;

UDPpacket *packet_alloc(void);
UDPpacket *packet_dup(const UDPpacket *packet);

uint8_t *packet_decompress(UDPpacket *packet, uint8_t *uncompressed);
void packet_compress(UDPpacket *packet);

void packet_free(UDPpacket *packet);
UDPpacket *packet_definalize(gsocketp s, UDPpacket *packet);

void socket_tx_enqueue(gsocketp s, UDPpacket **packet);
UDPpacket *socket_rx_dequeue(gsocketp s);
void socket_dequeue_packet(gsocketp s, UDPpacket **packet);
