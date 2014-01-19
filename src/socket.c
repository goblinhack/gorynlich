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
#include "socket.h"
#include "string.h"
#include "slre.h"
#include "command.h"
#include "time.h"
#include "player.h"
#include "wid.h"
#include "wid_button.h"
#include "color.h"
#include "tree.h"

tree_rootp sockets;

typedef struct {
    uint8_t type;
    char name[PLAYER_NAME_LEN_MAX];
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
    char name[PLAYER_NAME_LEN_MAX];
} __attribute__ ((packed)) msg_name;

typedef struct {
    uint8_t type;
    char txt[PLAYER_MSG_MAX];
} __attribute__ ((packed)) msg_shout;

typedef struct {
    uint8_t type;
    char from[PLAYER_NAME_LEN_MAX];
    char to[PLAYER_NAME_LEN_MAX];
    char txt[PLAYER_MSG_MAX];
} __attribute__ ((packed)) msg_tell;

typedef struct {
    char name[PLAYER_NAME_LEN_MAX];
    IPaddress local_ip;
    IPaddress remote_ip;
    uint8_t quality;
    uint16_t avg_latency;
    uint16_t min_latency;
    uint16_t max_latency;
    uint32_t score;
    uint32_t key;
} __attribute__ ((packed)) msg_player;

typedef struct {
    uint8_t type;
    msg_player players[MAX_PLAYERS];
} __attribute__ ((packed)) msg_players;

boolean is_server;
boolean is_client;
boolean is_headless;

IPaddress server_address = {0};
IPaddress no_address = {0};

static void socket_destroy(socketp s);
static boolean sockets_show_all(tokens_t *tokens, void *context);
static boolean sockets_show_summary(tokens_t *tokens, void *context);
static boolean socket_init_done;

boolean socket_init (void)
{
    if (socket_init_done) {
        return (true);
    }

    if (!sockets) {
        sockets = tree_alloc(TREE_KEY_TWO_INTEGER, "TREE ROOT: sockets");
    }

    if (SDLNet_Init() < 0) {
        ERR("cannot init SDL_net");
        return (false);
    }

    command_add(sockets_show_all, "show sockets detail", 
                "show clients and server sockets");

    command_add(sockets_show_summary, "show sockets summary", 
                "show socket quality and latency");

    command_add(debug_socket_ping_enable, "set debug socket ping [01]",
                "debug periodic pings");

    command_add(debug_socket_connect_enable, "set debug socket connect [01]",
                "debug sockets connections");

    command_add(debug_socket_players_enable, "set debug socket update [01]",
                "debug player updates");

    socket_init_done = true;

    return (true);
}

void socket_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (!socket_init_done) {
        return;
    }

    SDLNet_Quit();

    tree_destroy(&sockets, (tree_destroy_func)socket_destroy);

    socket_init_done = false;
}

static socketp socket_create (IPaddress address)
{
    socketp s;

    uint16_t port = address.port;
    if (!port) {
        ERR("Specify a local port to listen on");
        return (0);
    }

    /*
     * Create a new socket.
     */
    s = (typeof(s)) myzalloc(sizeof(*s), "TREE NODE: socket");

    s->tree.key1 = address.host;
    s->tree.key2 = address.port;

    if (!tree_insert(sockets, &s->tree.node)) {
        ERR("failed to add socket");
        return (0);
    }

    s->socklist = SDLNet_AllocSocketSet(1);
    if (!s->socklist) {
        char *tmp = iptodynstr(address);
        ERR("SDLNet_AllocSocketSet %s failed", tmp);
        WARN("  %s", SDLNet_GetError());
        myfree(tmp);

        socket_disconnect(s);
        return (0);
    }

    return (s);
}

socket *socket_listen (IPaddress address)
{
    IPaddress listen_address;

    /*
     * Relisten?
     */
    socketp s = socket_find(address);
    if (s) {
        return (s);
    }

    s = socket_create(address);
    if (!s) {
        return (0);
    }

    uint16_t port = SDLNet_Read16(&address.port);
    s->udp_socket = SDLNet_UDP_Open(port);
    if (!s->udp_socket) {
        char *tmp = iptodynstr(address);
        ERR("SDLNet_UDP_Open %s failed", tmp);
        WARN("  %s", SDLNet_GetError());
        myfree(tmp);

        socket_disconnect(s);
        return (0);
    }

    s->channel = SDLNet_UDP_Bind(s->udp_socket, -1, &listen_address);
    if (s->channel < 0) {
        char *tmp = iptodynstr(address);
        ERR("SDLNet_UDP_Bind %s failed", tmp);
        WARN("  %s", SDLNet_GetError());
        myfree(tmp);

        socket_disconnect(s);
        return (0);
    }

    if (SDLNet_UDP_AddSocket(s->socklist, s->udp_socket) == -1) {
        char *tmp = iptodynstr(address);
        ERR("SDLNet_UDP_AddSocket %s failed", tmp);
        WARN("  %s", SDLNet_GetError());
        myfree(tmp);

        socket_disconnect(s);
        return (0);
    }

    s->local_ip = address;
    s->server = true;

    return (s);
}

socketp socket_find (IPaddress address)
{
    socket findme;
    socket *s;

    memset(&findme, 0, sizeof(findme));
    findme.tree.key1 = address.host;
    findme.tree.key2 = address.port;

    s = (typeof(s)) tree_find(sockets, &findme.tree.node);

    return (s);
}

socket *socket_connect (IPaddress address, boolean server_side_client)
{
    IPaddress connect_address = address;

    /*
     * Reopen?
     */
    socketp s = socket_find(address);
    if (s) {
        return (s);
    }

    s = socket_create(address);
    if (!s) {
        return (0);
    }

    s->udp_socket = SDLNet_UDP_Open(0);
    if (!s->udp_socket) {
        char *tmp = iptodynstr(connect_address);
        ERR("SDLNet_UDP_Open %s failed", tmp);
        WARN("  %s", SDLNet_GetError());
        myfree(tmp);

        socket_disconnect(s);
        return (0);
    }

    s->channel = SDLNet_UDP_Bind(s->udp_socket, -1, &connect_address);
    if (s->channel < 0) {
        char *tmp = iptodynstr(connect_address);
        ERR("SDLNet_UDP_Bind %s failed", tmp);
        WARN("  %s", SDLNet_GetError());
        myfree(tmp);

        socket_disconnect(s);
        return (0);
    }

    if (SDLNet_UDP_AddSocket(s->socklist, s->udp_socket) == -1) {
        char *tmp = iptodynstr(connect_address);
        ERR("SDLNet_UDP_AddSocket %s failed", tmp);
        WARN("  %s", SDLNet_GetError());
        myfree(tmp);

        socket_disconnect(s);
        return (0);
    }

    s->server_side_client = server_side_client;

    if (!server_side_client) {
        s->client = true;
    }

    s->remote_ip = connect_address;
    s->local_ip = *SDLNet_UDP_GetPeerAddress(s->udp_socket, -1);

    LOG("Socket create to %s", socket_get_remote_logname(s));

    if (debug_socket_connect_enabled) {
        LOG("       from      %s", socket_get_local_logname(s));
    }

    return (s);
}

static void socket_destroy (socketp s)
{
    socket_set_connected(s, false);

    LOG("Socket destroy [%s]", socket_get_remote_logname(s));

    if (s->socklist) {
        SDLNet_FreeSocketSet(s->socklist);
    }

    if (s->udp_socket) {
        SDLNet_UDP_Unbind(s->udp_socket, s->channel);

        SDLNet_UDP_Close(s->udp_socket);
    }

    if (s->local_logname) {
        myfree((char *)s->local_logname);
    }

    if (s->remote_logname) {
        myfree((char *)s->remote_logname);
    }

    socket_set_player(s, 0);
}

void socket_disconnect (socketp s)
{
    tree_remove_found_node(sockets, &s->tree.node);

    socket_destroy(s);
}

/*
 * User has entered a command, run it
 */
boolean debug_socket_ping_enable (tokens_t *tokens, void *context)
{
    char *s = tokens->args[4];

    if (!s || (*s == '\0')) {
        debug_socket_ping_enabled = 1;
    } else {
        debug_socket_ping_enabled = strtol(s, 0, 10) ? 1 : 0;
    }

    CON("Debug ping mode set to %u", debug_socket_ping_enabled);

    return (true);
}

/*
 * User has entered a command, run it
 */
boolean debug_socket_connect_enable (tokens_t *tokens, void *context)
{
    char *s = tokens->args[4];

    if (!s || (*s == '\0')) {
        debug_socket_connect_enabled = 1;
    } else {
        debug_socket_connect_enabled = strtol(s, 0, 10) ? 1 : 0;
    }

    CON("Debug socket connect mode set to %u", debug_socket_connect_enabled);

    return (true);
}

/*
 * User has entered a command, run it
 */
boolean debug_socket_players_enable (tokens_t *tokens, void *context)
{
    char *s = tokens->args[4];

    if (!s || (*s == '\0')) {
        debug_socket_players_enabled = 1;
    } else {
        debug_socket_players_enabled = strtol(s, 0, 10) ? 1 : 0;
    }

    CON("Debug socket player mode set to %u", debug_socket_players_enabled);

    return (true);
}

socket *socket_find_local_ip (IPaddress address)
{
    socketp s;
    TREE_WALK(sockets, s) {
        if (cmp_address(&address, &s->local_ip)) {
            return (s);
        }
    }

    return (0);
}

socket *socket_find_remote_ip (IPaddress address)
{
    socketp s;
    TREE_WALK(sockets, s) {
        if (cmp_address(&address, &s->remote_ip)) {
            return (s);
        }
    }

    return (0);
}

char *iptodynstr (IPaddress ip)
{
    uint32_t ipv4 = SDLNet_Read32(&ip.host);
    uint8_t ip1 = (ipv4>>24) & 0xFF;
    uint8_t ip2 = (ipv4>>16) & 0xFF;
    uint8_t ip3 = (ipv4>>8)  & 0xFF;
    uint8_t ip4 = ipv4 & 0xFF;
    const char *hostname;

    uint16_t port = SDLNet_Read16(&ip.port);

    if (cmp_address(&ip, &no_address)) {
        return (dynprintf("-"));
    }

    if (!(hostname = SDLNet_ResolveIP(&ip))) {
        return (dynprintf("IPv4 %u.%u.%u.%u:%u",
                          hostname, ip1, ip2, ip3, ip4, port));

    }

    if (strstr(hostname, "localhost")) {
        hostname = "lhost";
    }

    if ((ip1 == 0) && (ip2 == 0) && (ip3 == 0) && (ip4 == 0)) {
        return (dynprintf("%s::%u",
                           hostname && *hostname ? hostname : "",
                           port));
    }

    if ((ip1 == 127) && (ip2 == 0) && (ip3 == 0) && (ip4 == 1)) {
        return (dynprintf("%s::%u",
                           hostname && *hostname ? hostname : "",
                           port));
    }

    return (dynprintf("%s%s%u.%u.%u.%u:%u",
                      hostname && *hostname ? hostname : "",
                      hostname && *hostname ? "," : "",
                      ip1, ip2, ip3, ip4, port));
}

char *iptodynstr_no_resolve (IPaddress ip)
{
    uint32_t ipv4 = SDLNet_Read32(&ip.host);
    uint8_t ip1 = (ipv4>>24) & 0xFF;
    uint8_t ip2 = (ipv4>>16) & 0xFF;
    uint8_t ip3 = (ipv4>>8)  & 0xFF;
    uint8_t ip4 = ipv4 & 0xFF;

    uint16_t port = SDLNet_Read16(&ip.port);

    return (dynprintf("%u.%u.%u.%u:%u",
                      ip1, ip2, ip3, ip4, port));
}

char *iprawtodynstr (IPaddress ip)
{
    uint32_t ipv4 = SDLNet_Read32(&ip.host);
    uint8_t ip1 = (ipv4>>24) & 0xFF;
    uint8_t ip2 = (ipv4>>16) & 0xFF;
    uint8_t ip3 = (ipv4>>8)  & 0xFF;
    uint8_t ip4 = ipv4 & 0xFF;

    return (dynprintf("%u.%u.%u.%u", ip1, ip2, ip3, ip4));
}

char *iprawporttodynstr (IPaddress ip)
{
    uint16_t port = SDLNet_Read16(&ip.port);

    return (dynprintf("%u", port));
}

static boolean sockets_show_all (tokens_t *tokens, void *context)
{
    uint32_t si = 0;

    socketp s;
    TREE_WALK(sockets, s) {

        si++;

        if (s->server) {
            CON("[%u] Server", si);
        } else if (s->server_side_client) {
            CON("[%u] Server-side-Client", si);
        } else if (s->client) {
            CON("[%u] Client", si);
        } else {
            CON("[%u] Unknown", si);
        }

        CON("  Local IP : %s", socket_get_local_logname(s));

        if (!s->server) {
            CON("  Remote IP: %s", socket_get_remote_logname(s));
        }

        CON("  Stats    : tx %u packets, rx %u packets", s->tx, s->rx);
        CON("  Errors   : tx error %u, rx error %u, bad message %u packets", 
            s->tx_error, s->rx_error, s->rx_bad_msg);

        CON("  Ping           : tx %u, rx %u",
            s->tx_msg[MSG_PING], 
            s->rx_msg[MSG_PING]);

        CON("  Pong           : tx %u, rx %u",
            s->tx_msg[MSG_PONG], 
            s->rx_msg[MSG_PONG]);

        CON("  Name           : tx %u, rx %u",
            s->tx_msg[MSG_NAME], 
            s->rx_msg[MSG_NAME]);

        CON("  Shout          : tx %u, rx %u",
            s->tx_msg[MSG_SHOUT], 
            s->rx_msg[MSG_SHOUT]);

        CON("  Tell           : tx %u, rx %u",
            s->tx_msg[MSG_TELL], 
            s->rx_msg[MSG_TELL]);

        CON("  Join           : tx %u, rx %u",
            s->tx_msg[MSG_CLIENT_JOIN], 
            s->rx_msg[MSG_CLIENT_JOIN]);

        CON("  Leave          : tx %u, rx %u",
            s->tx_msg[MSG_CLIENT_LEAVE], 
            s->rx_msg[MSG_CLIENT_LEAVE]);

        CON("  Close          : tx %u, rx %u",
            s->tx_msg[MSG_CLIENT_CLOSE], 
            s->rx_msg[MSG_CLIENT_CLOSE]);

        CON("  Server Status  : tx %u, rx %u",
            s->tx_msg[MSG_SERVER_STATUS], 
            s->rx_msg[MSG_SERVER_STATUS]);

        CON("  Server Close   : tx %u, rx %u",
            s->tx_msg[MSG_SERVER_CLOSE], 
            s->rx_msg[MSG_SERVER_CLOSE]);

        /*
         * Ping stats.
         */
        uint32_t no_response = 0;
        uint32_t response = 0;
        uint32_t total_attempts = 0;

        /*
         * Latency.
         */
        uint32_t avg_latency = 0;
        uint32_t max_latency = 0;
        uint32_t min_latency = (uint32_t) -1;

        FOR_ALL_IN_ARRAY(latency, s->ping_responses) {
            if (*latency == -1) {
                ++no_response;
                continue;
            }

            if (*latency == 0) {
                continue;
            }

            ++response;

            /*
             * Latency.
             */
            avg_latency += *latency;

            if (*latency > max_latency) {
                max_latency = *latency;
            }

            if (*latency < min_latency) {
                min_latency = *latency;
            }
        }

        total_attempts = no_response + response;

        if (total_attempts) {
            avg_latency /= total_attempts;

            CON("  Quality  : success %2.2f percent, fails %2.2f percent",

                ((float)((float)response / (float)total_attempts)) * 100.0,
                ((float)((float)no_response / (float)total_attempts)) * 100.0);

            CON("  Latency  : max %u ms, min %u ms, average %u ms",
                max_latency, min_latency, avg_latency);
        }
    }

    return (true);
}

static boolean sockets_show_summary (tokens_t *tokens, void *context)
{
    int si = 0;

    CON("Name                 Quality  Latency       Remote IP           Local IP");
    CON("----                 -------  ------- -------------------- ------------------");
        
    socketp s;
    TREE_WALK(sockets, s) {

        si++;

        /*
         * Ping stats.
         */
        uint32_t no_response = 0;
        uint32_t response = 0;
        uint32_t total_attempts = 0;

        /*
         * Latency.
         */
        uint32_t avg_latency = 0;
        uint32_t max_latency = 0;
        uint32_t min_latency = (uint32_t) -1;

        FOR_ALL_IN_ARRAY(latency, s->ping_responses) {
            if (*latency == -1) {
                ++no_response;
                continue;
            }

            if (*latency == 0) {
                continue;
            }

            ++response;

            /*
             * Latency.
             */
            avg_latency += *latency;

            if (*latency > max_latency) {
                max_latency = *latency;
            }

            if (*latency < min_latency) {
                min_latency = *latency;
            }
        }

        total_attempts = no_response + response;

        if (total_attempts) {
            avg_latency /= total_attempts;

            CON("%-20s %3.0f pct %5d ms %-20s %-20s", 
                socket_get_server(s) ? "server" : socket_get_name(s),
                ((float)((float)response / (float)total_attempts)) * 100.0,
                avg_latency,
                socket_get_remote_logname(s),
                socket_get_local_logname(s));
        } else {
            CON("%-20s                  %-20s %-20s", 
                socket_get_server(s) ? "server" : socket_get_name(s),
                socket_get_remote_logname(s),
                socket_get_local_logname(s));
        }
    }

    return (true);
}

boolean sockets_quality_check (void)
{
    socketp s;

    TREE_WALK(sockets, s) {
        /*
         * Ping stats.
         */
        uint32_t no_response = 0;
        uint32_t response = 0;
        uint32_t total_attempts = 0;

        /*
         * Latency.
         */
        uint32_t avg_latency = 0;
        uint32_t max_latency = 0;
        uint32_t min_latency = (uint32_t) -1;

        FOR_ALL_IN_ARRAY(latency, s->ping_responses) {
            if (*latency == -1) {
                ++no_response;
                continue;
            }

            if (*latency == 0) {
                continue;
            }

            ++response;

            /*
             * Latency.
             */
            avg_latency += *latency;

            if (*latency > max_latency) {
                max_latency = *latency;
            }

            if (*latency < min_latency) {
                min_latency = *latency;
            }
        }

        total_attempts = no_response + response;

        if (total_attempts) {
            s->quality = 
                ((float)((float)response / (float)total_attempts)) * 100.0,

            avg_latency /= total_attempts;
            s->avg_latency = avg_latency;
            s->min_latency = min_latency;
            s->max_latency = max_latency;

            aplayer *p = s->player;
            if (p) {
                p->quality = s->quality;
                p->avg_latency = s->avg_latency;
                p->min_latency = s->min_latency;
                p->max_latency = s->max_latency;
            }
        }
    }

    return (true);
}

void sockets_alive_check (void)
{
    sockets_quality_check();

    socketp s;

    TREE_WALK(sockets, s) {
        /*
         * Don't kill off new born connections.
         */
        if (s->tx < 10) {
            continue;
        }

        if (!socket_get_server_side_client(s)) {
            continue;
        }

        if (s->quality < SOCKET_PING_FAIL_THRESHOLD) {
            LOG("Peer down [%s] qual %u percent",
                socket_get_remote_logname(s), s->quality);

            /*
             * Clients try forever. Server clients disconnect.
             */
            if (socket_get_server_side_client(s)) {
                socket_disconnect(s);
            }

            if (socket_get_client(s)) {
                socket_tx_client_leave(s);
            }
        }
    }
}

IPaddress socket_get_local_ip (const socketp s)
{
    return (s->local_ip);
}

IPaddress socket_get_remote_ip (const socketp s)
{
    return (s->remote_ip);
}

const char *socket_get_name (const socketp s)
{
    return (s->name);
}

void socket_set_name (socketp s, const char *name)
{
    if (!name) {
        memset(s->name, 0, sizeof(s->name));
    } else {
        strncpy(s->name, name, sizeof(s->name) - 1);
    }
}

const char * socket_get_local_logname (const socketp s)
{
    if (!s->local_logname) {
        s->local_logname = iptodynstr(s->local_ip);
    }

    return (s->local_logname);
}

const char * socket_get_remote_logname (const socketp s)
{
    if (!s->remote_logname) {
        s->remote_logname = iptodynstr(s->remote_ip);
    }

    return (s->remote_logname);
}

boolean socket_get_server (const socketp s)
{
    return (s->server);
}

boolean socket_get_client (const socketp s)
{
    return (s->client);
}

boolean socket_get_server_side_client (const socketp s)
{
    return (s->server_side_client);
}

void socket_set_channel (socketp s, int c)
{
    s->channel = c;
}

boolean socket_get_channel (const socketp s)
{
    return (s->channel);
}

void socket_set_connected (socketp s, boolean c)
{
    if (c == s->connected) {
        return;
    }

    if (c) {
        LOG("Connected to %s", socket_get_remote_logname(s));
    } else {
        LOG("Disconnected from %s", socket_get_remote_logname(s));
    }

    s->connected = c;

    /*
     * For aliveness checks so they see this as a new connection.
     */
    s->tx = 0;
}

boolean socket_get_connected (const socketp s)
{
    return (s->connected);
}

UDPsocket socket_get_udp_socket (const socketp s)
{
    return (s->udp_socket);
}

SDLNet_SocketSet socket_get_socklist (const socketp s)
{
    return (s->socklist);
}

aplayerp socket_get_player (const socketp s)
{
    return (s->player);
}

void socket_set_player (const socketp s, aplayer *p)
{
    if (s->player) {
        myfree(s->player);
    }

    s->player = p;
}

void socket_count_inc_pak_rx (const socketp s, msg_type type)
{
    if (type < MSG_MAX) {
        s->rx++;
        s->rx_msg[type]++;
    } else {
        socket_count_inc_pak_rx_bad_msg(s);
    }
}

void socket_count_inc_pak_tx (const socketp s)
{
    s->tx++;
}

static void socket_count_inc_pak_rx_error (const socketp s, UDPpacket *packet)
{
    s->rx_error++;

    char *tmp = iptodynstr(read_address(packet));
    LOG("Bad socket message from %s", tmp);
    myfree(tmp);
}

void socket_count_inc_pak_tx_error (const socketp s)
{
    s->tx_error++;
}

void socket_count_inc_pak_rx_bad_msg (const socketp s)
{
    s->rx_bad_msg++;
}

static UDPpacket *socket_alloc_msg (void)
{
    UDPpacket *packet;

    packet = SDLNet_AllocPacket(MAX_PACKET_SIZE);
    if (!packet) {
        DIE("Out of packet space, pak %u", MAX_PACKET_SIZE);
    }

    newptr(packet, "pak");

    return (packet);
}

static void socket_free_msg (UDPpacket *packet)
{
    oldptr(packet);

    SDLNet_FreePacket(packet);
}

static void socket_tx_msg (socketp s, UDPpacket *packet)
{
    msg_type type;

    type = *(packet->data);

    if (SDLNet_UDP_Send(socket_get_udp_socket(s),
                        socket_get_channel(s), packet) < 1) {
        ERR("no UDP packet sent: %s", SDLNet_GetError());

        socket_count_inc_pak_tx_error(s);
    } else {
        socket_count_inc_pak_tx(s);

        s->tx_msg[type]++;
    }
}

void socket_tx_ping (socketp s, uint8_t seq, uint32_t ts)
{
    if (!socket_get_udp_socket(s)) {
        return;
    }

    UDPpacket *packet = socket_alloc_msg();

    uint8_t *data = packet->data;
    uint8_t *odata = data;

    *data++ = MSG_PING;
    *data++ = seq;

    SDLNet_Write32(ts, data);               
    data += sizeof(uint32_t);

    s->ping_responses[seq % ARRAY_SIZE(s->ping_responses)] = (uint32_t) -1;

    if (debug_socket_ping_enabled) {
        LOG("Tx Ping [to %s] seq %u, ts %u", 
            socket_get_remote_logname(s), seq, ts);
    }

    packet->len = data - odata;
    write_address(packet, socket_get_remote_ip(s));

    socket_tx_msg(s, packet);
            
    socket_free_msg(packet);
}

void socket_tx_pong (socketp s, uint8_t seq, uint32_t ts)
{
    if (!socket_get_udp_socket(s)) {
        return;
    }

    UDPpacket *packet = socket_alloc_msg();

    uint8_t *data = packet->data;
    uint8_t *odata = data;

    *data++ = MSG_PONG;
    *data++ = seq;

    SDLNet_Write32(ts, data);               
    data += sizeof(uint32_t);

    packet->len = data - odata;
    write_address(packet, socket_get_remote_ip(s));

    socket_tx_msg(s, packet);
        
    socket_free_msg(packet);
}

void socket_rx_ping (socketp s, UDPpacket *packet, uint8_t *data)
{
    uint8_t seq = *data++;
    uint32_t ts = SDLNet_Read32(data);
    data += sizeof(uint32_t);

    if (debug_socket_ping_enabled) {
        char *tmp = iptodynstr(read_address(packet));
        LOG("Rx Ping from %s seq %u", tmp, seq);
        myfree(tmp);
    }

    socket_tx_pong(s, seq, ts);

    socket_set_connected(s, true);
}

void socket_rx_pong (socketp s, UDPpacket *packet, uint8_t *data)
{
    uint8_t seq = *data++;
    uint32_t ts = SDLNet_Read32(data);
    data += sizeof(uint32_t);

    if (debug_socket_ping_enabled) {
        char *tmp = iptodynstr(read_address(packet));
        LOG("Rx Pong from %s seq %u, elapsed %u",
            tmp, seq, time_get_time_cached() - ts);
        myfree(tmp);
    }

    s->ping_responses[seq % ARRAY_SIZE(s->ping_responses)] = 
                    time_get_time_cached() - ts;
}

void socket_tx_name (socketp s)
{
    if (!socket_get_udp_socket(s)) {
        return;
    }

    /*
     * Refresh the server with our name.
     */
    if (!socket_get_client(s)) {
        return;
    }

    if (!s->connected) {
        return;
    }

    UDPpacket *packet = socket_alloc_msg();

    msg_name msg = {0};
    msg.type = MSG_NAME;
    strncpy(msg.name, s->name, min(sizeof(msg.name) - 1, strlen(s->name))); 

    memcpy(packet->data, &msg, sizeof(msg));

    if (debug_socket_players_enabled) {
        LOG("Tx Player [to %s] \"%s\"", socket_get_remote_logname(s), s->name);
    }

    packet->len = sizeof(msg);
    write_address(packet, socket_get_remote_ip(s));

    socket_tx_msg(s, packet);
        
    socket_free_msg(packet);
}

void socket_rx_name (socketp s, UDPpacket *packet, uint8_t *data)
{
    msg_name msg = {0};

    if (packet->len != sizeof(msg)) {
        socket_count_inc_pak_rx_error(s, packet);
        return;
    }

    memcpy(&msg, packet->data, sizeof(msg));

    if (debug_socket_players_enabled) {
        char *tmp = iptodynstr(read_address(packet));
        LOG("Rx Name from %s \"%s\"", tmp, msg.name);
        myfree(tmp);
    }

    socket_set_name(s, msg.name);

    /*
     * Update the player structure.
     */
    aplayer *p = s->player;
    if (!p) {
        p = myzalloc(sizeof(*p), "player");

        socket_set_player(s, p);
    }

    memcpy(p->name, msg.name, PLAYER_NAME_LEN_MAX);
    p->local_ip = s->local_ip;
    p->remote_ip = s->remote_ip;
}

boolean socket_tx_client_join (socketp s, uint32_t *key)
{
    if (!socket_get_udp_socket(s)) {
        ERR("no socket to join on");
        return (false);
    }

    /*
     * Refresh the server with our name.
     */
    if (!socket_get_client(s)) {
        ERR("not a client, cannot join");
        return (false);
    }

    if (!s->connected) {
        ERR("not connected, cannot join yet");
        return (false);
    }

    UDPpacket *packet = socket_alloc_msg();

    msg_client_join msg = {0};
    msg.type = MSG_CLIENT_JOIN;

    *key = rand() % time(NULL);
    SDLNet_Write32(*key, &msg.key);

    strncpy(msg.name, s->name, min(sizeof(msg.name) - 1, strlen(s->name))); 

    memcpy(packet->data, &msg, sizeof(msg));

    if (debug_socket_players_enabled) {
        LOG("Tx Join [to %s] \"%s\"", socket_get_remote_logname(s), s->name);
    }

    packet->len = sizeof(msg);
    write_address(packet, socket_get_remote_ip(s));

    socket_tx_msg(s, packet);
        
    socket_free_msg(packet);

    return (true);
}

void socket_rx_client_join (socketp s, UDPpacket *packet, uint8_t *data)
{
    msg_client_join msg = {0};

    if (packet->len != sizeof(msg)) {
        socket_count_inc_pak_rx_error(s, packet);
        return;
    }

    memcpy(&msg, packet->data, sizeof(msg));

    if (debug_socket_players_enabled) {
        char *tmp = iptodynstr(read_address(packet));
        LOG("Rx Join from %s \"%s\"", tmp, msg.name);
        myfree(tmp);
    }

    socket_set_name(s, msg.name);

    /*
     * Update the player structure.
     */
    aplayer *p = s->player;
    if (!p) {
        p = myzalloc(sizeof(*p), "player");

        socket_set_player(s, p);
    }

    memcpy(p->name, msg.name, PLAYER_NAME_LEN_MAX);
    p->local_ip = s->local_ip;
    p->remote_ip = s->remote_ip;
    p->key = SDLNet_Read32(&msg.key);
}

void socket_tx_client_leave (socketp s)
{
    if (!socket_get_udp_socket(s)) {
        return;
    }

    /*
     * Refresh the server with our name.
     */
    if (!socket_get_client(s)) {
        return;
    }

    if (!s->connected) {
        return;
    }

    UDPpacket *packet = socket_alloc_msg();

    msg_client_leave msg = {0};
    msg.type = MSG_CLIENT_LEAVE;

    memcpy(packet->data, &msg, sizeof(msg));

    if (debug_socket_players_enabled) {
        LOG("Tx leave [to %s] \"%s\"", socket_get_remote_logname(s), s->name);
    }

    packet->len = sizeof(msg);
    write_address(packet, socket_get_remote_ip(s));

    socket_set_name(s, 0);

    socket_tx_msg(s, packet);
        
    socket_free_msg(packet);
}

void socket_rx_client_leave (socketp s, UDPpacket *packet, uint8_t *data)
{
    msg_client_leave msg = {0};

    if (packet->len != sizeof(msg)) {
        socket_count_inc_pak_rx_error(s, packet);
        return;
    }

    memcpy(&msg, packet->data, sizeof(msg));

    if (debug_socket_players_enabled) {
        char *tmp = iptodynstr(read_address(packet));
        LOG("Rx leave from %s", tmp);
        myfree(tmp);
    }
}

void socket_tx_client_close (socketp s)
{
    if (!socket_get_udp_socket(s)) {
        return;
    }

    /*
     * Refresh the server with our name.
     */
    if (!socket_get_client(s)) {
        return;
    }

    if (!s->connected) {
        return;
    }

    UDPpacket *packet = socket_alloc_msg();

    msg_client_close msg = {0};
    msg.type = MSG_CLIENT_CLOSE;

    memcpy(packet->data, &msg, sizeof(msg));

    LOG("Tx close [to %s]", socket_get_remote_logname(s));

    packet->len = sizeof(msg);
    write_address(packet, socket_get_remote_ip(s));

    socket_tx_msg(s, packet);
        
    socket_free_msg(packet);
}

void socket_rx_client_close (socketp s, UDPpacket *packet, uint8_t *data)
{
    msg_client_close msg = {0};

    if (packet->len != sizeof(msg)) {
        socket_count_inc_pak_rx_error(s, packet);
        return;
    }

    memcpy(&msg, packet->data, sizeof(msg));

    char *tmp = iptodynstr(read_address(packet));
    LOG("Rx close from %s", tmp);
    myfree(tmp);

    socket_disconnect(s);
}

void socket_tx_shout (socketp s, const char *txt)
{
    if (!socket_get_udp_socket(s)) {
        return;
    }

    if (!s->connected) {
        return;
    }

    UDPpacket *packet = socket_alloc_msg();

    msg_shout msg = {0};
    msg.type = MSG_SHOUT;
    strncpy(msg.txt, txt, min(sizeof(msg.txt) - 1, strlen(txt))); 

    memcpy(packet->data, &msg, sizeof(msg));

    if (debug_socket_players_enabled) {
        LOG("Tx Shout [to %s] \"%s\"", socket_get_remote_logname(s), txt);
    }

    packet->len = sizeof(msg);
    write_address(packet, socket_get_remote_ip(s));

    socket_tx_msg(s, packet);
        
    socket_free_msg(packet);
}

void socket_rx_shout (socketp s, UDPpacket *packet, uint8_t *data)
{
    msg_shout msg = {0};

    if (packet->len != sizeof(msg)) {
        socket_count_inc_pak_rx_error(s, packet);
        return;
    }

    memcpy(&msg, packet->data, sizeof(msg));

    char txt[PLAYER_MSG_MAX + 1] = {0};

    memcpy(txt, msg.txt, PLAYER_MSG_MAX);

    LOG("SHOUT: \"%s\"", txt);

    if (debug_socket_players_enabled) {
        char *tmp = iptodynstr(read_address(packet));
        LOG("Rx Shout from %s \"%s\"", tmp, txt);
        myfree(tmp);
    }

    char *tmp = dynprintf("%s: shouts \"%s\"", socket_get_name(s), txt);
    widp w = wid_button_transient(tmp, 0);
    color c = BLACK;
    c.a = 150;
    wid_set_color(w, WID_COLOR_BG, c);
    wid_set_color(w, WID_COLOR_TL, c);
    wid_set_color(w, WID_COLOR_BR, c);
    wid_move_to_pct_centered(w, 0.5, 0.1);
    wid_set_text_outline(w, true);
    myfree(tmp);

    if (socket_get_client(s)) {
        return;
    }

    socketp sp;

    TREE_WALK(sockets, sp) {
        if (sp == s) {
            continue;
        }

        if (!sp->connected) {
            continue;
        }

        if (!sp->server_side_client) {
            continue;
        }

        socket_tx_shout(sp, txt);
    }
}

void socket_tx_tell (socketp s, 
                     const char *from,
                     const char *to,
                     const char *txt)
{
    if (!socket_get_udp_socket(s)) {
        return;
    }

    if (!s->connected) {
        return;
    }

    UDPpacket *packet = socket_alloc_msg();

    msg_tell msg = {0};
    msg.type = MSG_TELL;

    strncpy(msg.from, from, min(sizeof(msg.from) - 1, strlen(from))); 
    strncpy(msg.to, to, min(sizeof(msg.to) - 1, strlen(to))); 
    strncpy(msg.txt, txt, min(sizeof(msg.txt) - 1, strlen(txt))); 

    memcpy(packet->data, &msg, sizeof(msg));

    if (debug_socket_players_enabled) {
        LOG("Tx Tell [to %s] from \"%s\" to \"%s\" msg \"%s\"", 
            socket_get_remote_logname(s), from, to, txt);
    }

    packet->len = sizeof(msg);
    write_address(packet, socket_get_remote_ip(s));

    socket_tx_msg(s, packet);
        
    socket_free_msg(packet);
}

void socket_rx_tell (socketp s, UDPpacket *packet, uint8_t *data)
{
    msg_tell msg = {0};

    if (packet->len != sizeof(msg)) {
        socket_count_inc_pak_rx_error(s, packet);
        return;
    }

    memcpy(&msg, packet->data, sizeof(msg));

    char txt[PLAYER_MSG_MAX + 1] = {0};
    char from[PLAYER_NAME_LEN_MAX + 1] = {0};
    char to[PLAYER_NAME_LEN_MAX + 1] = {0};

    memcpy(txt, msg.txt, PLAYER_MSG_MAX);
    memcpy(from, msg.from, PLAYER_NAME_LEN_MAX);
    memcpy(to, msg.to, PLAYER_NAME_LEN_MAX);

    LOG("TELL: from \"%s\" to \"%s\" msg \"%s\"", from, to, txt);

    if (debug_socket_players_enabled) {
        char *tmp = iptodynstr(read_address(packet));
        LOG("Rx Shout from %s \"%s\"", tmp, txt);
        myfree(tmp);
    }

    if (socket_get_client(s)) {
        return;
    }

    socketp sp;

    TREE_WALK(sockets, sp) {

        if (sp == s) {
            continue;
        }

        if (!sp->connected) {
            continue;
        }

        if (!sp->server_side_client) {
            continue;
        }

        if (!strcmp(socket_get_name(sp), to)) {
            socket_tx_tell(sp, from, to, txt);
        }
    }
}

/*
 * Send an array of all current players to all clients.
 */
void socket_tx_server_status (void)
{
    aplayer players[MAX_PLAYERS];

    memset(&players, 0, sizeof(players));

    msg_players msg = {0};
    msg.type = MSG_SERVER_STATUS;

    socketp s;
    uint32_t si = 0;

    TREE_WALK(sockets, s) {
        if (!s->server_side_client) {
            continue;
        }

        aplayer *p = s->player;
        if (!s->player) {
            continue;
        }

        if (si >= MAX_PLAYERS) {
            ERR("too many players to send all in message");
            continue;
        }

        msg_player *msg_tx = &msg.players[si];

        strncpy(msg_tx->name, p->name, min(sizeof(msg_tx->name), 
                                           strlen(p->name))); 

        SDLNet_Write32(p->local_ip.host, &msg_tx->local_ip.host);
        SDLNet_Write16(p->local_ip.port, &msg_tx->local_ip.port);

        SDLNet_Write32(p->remote_ip.host, &msg_tx->remote_ip.host);
        SDLNet_Write16(p->remote_ip.port, &msg_tx->remote_ip.port);

        msg_tx->quality = s->quality;
        SDLNet_Write16(s->avg_latency, &msg_tx->avg_latency);
        SDLNet_Write16(s->min_latency, &msg_tx->min_latency);
        SDLNet_Write16(s->max_latency, &msg_tx->max_latency);

        SDLNet_Write32(p->score, &msg_tx->score);
        SDLNet_Write32(p->key, &msg_tx->key);

        si++;
    }

    UDPpacket *packet = socket_alloc_msg();

    memcpy(packet->data, &msg, sizeof(msg));

    {
        TREE_WALK(sockets, s) {
            if (!s->connected) {
                continue;
            }

            if (!s->server_side_client) {
                continue;
            }

            if (debug_socket_players_enabled) {
                LOG("Tx All Players [to %s]",
                    socket_get_remote_logname(s));
            }

            packet->len = sizeof(msg);
            write_address(packet, socket_get_remote_ip(s));

            socket_tx_msg(s, packet);
        }
    }
            
    socket_free_msg(packet);
}

/*
 * Receive an array of all current players from the server.
 */
void socket_rx_server_status (socketp s, UDPpacket *packet, uint8_t *data,
                            aplayer *players)
{
    msg_players *msg;

    if (packet->len != sizeof(*msg)) {
        socket_count_inc_pak_rx_error(s, packet);
        return;
    }

    uint32_t pi;

    msg = (typeof(msg)) packet->data;

    for (pi = 0; pi < MAX_PLAYERS; pi++) {
        aplayer *p = &players[pi];
        msg_player *msg_rx = &msg->players[pi];

        memcpy(p->name, msg_rx->name, PLAYER_NAME_LEN_MAX);

        p->local_ip.host = SDLNet_Read32(&msg_rx->local_ip.host);
        p->local_ip.port = SDLNet_Read16(&msg_rx->local_ip.port);

        p->remote_ip.host = SDLNet_Read32(&msg_rx->remote_ip.host);
        p->remote_ip.port = SDLNet_Read16(&msg_rx->remote_ip.port);

        p->quality = msg_rx->quality;

        p->avg_latency = SDLNet_Read16(&msg_rx->avg_latency);
        p->min_latency = SDLNet_Read16(&msg_rx->min_latency);
        p->max_latency = SDLNet_Read16(&msg_rx->max_latency);

        p->score = SDLNet_Read32(&msg_rx->score);
        p->key = SDLNet_Read32(&msg_rx->key);

        if (!p->name[0]) {
            continue;
        }

        if (debug_socket_players_enabled) {
            char *tmp = iptodynstr(read_address(packet));
            LOG("Rx All Players from %s %u:\"%s\"", tmp, pi, p->name);
            myfree(tmp);
        }
    }
}

/*
 * Tell all players the server is down.
 */
void socket_tx_server_close (void)
{
    msg_server_close msg = {0};
    msg.type = MSG_SERVER_CLOSE;

    UDPpacket *packet = socket_alloc_msg();

    memcpy(packet->data, &msg, sizeof(msg));

    socketp s;

    TREE_WALK(sockets, s) {
        if (!s->connected) {
            continue;
        }

        if (!s->server_side_client) {
            continue;
        }

        LOG("Tx Server down [to %s]",
            socket_get_remote_logname(s));

        packet->len = sizeof(msg);
        write_address(packet, socket_get_remote_ip(s));

        socket_tx_msg(s, packet);
    }
        
    socket_free_msg(packet);
}

void socket_rx_server_close (socketp s, UDPpacket *packet, uint8_t *data)
{
    msg_server_close *msg;

    if (packet->len != sizeof(*msg)) {
        socket_count_inc_pak_rx_error(s, packet);
        return;
    }

    char *tmp = iptodynstr(read_address(packet));
    LOG("Rx Server down from %s", tmp);
    myfree(tmp);
}

uint32_t socket_get_quality (socketp s)
{
    return (s->quality);
}

uint32_t socket_get_avg_latency (socketp s)
{
    return (s->avg_latency);
}

uint32_t socket_get_min_latency (socketp s)
{
    return (s->min_latency);
}

uint32_t socket_get_max_latency (socketp s)
{
    return (s->max_latency);
}

uint32_t socket_get_rx (socketp s)
{
    return (s->min_latency);
}

uint32_t socket_get_tx (socketp s)
{
    return (s->tx);
}

uint32_t socket_get_rx_error (socketp s)
{
    return (s->min_latency);
}

uint32_t socket_get_tx_error (socketp s)
{
    return (s->min_latency);
}

uint32_t socket_get_rx_bad_msg (socketp s)
{
    return (s->min_latency);
}
