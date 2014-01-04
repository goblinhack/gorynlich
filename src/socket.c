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

typedef struct socket_ {
    uint8_t index;
    uint8_t open;
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
    aplayer *player;
    /*
     * Counters.
     */
    uint32_t rx;
    uint32_t tx;
    uint32_t rx_error;
    uint32_t tx_error;
    uint32_t rx_bad_msg;
    uint32_t ping_responses[SOCKET_PING_SEQ_NO_RANGE];
    uint32_t tx_msg[MSG_TYPE_MAX];
    uint32_t rx_msg[MSG_TYPE_MAX];
    int channel;
    const char *local_logname;
    const char *remote_logname;
    char name[PLAYER_NAME_MAX];
} socket;

typedef struct network_ {
    socket sockets[MAX_SOCKETS];
} network;

typedef struct {
    uint8_t type;
    char name[PLAYER_NAME_MAX];
} __attribute__ ((packed)) msg_name;

typedef struct {
    char name[PLAYER_NAME_MAX];
    IPaddress local_ip;
    IPaddress remote_ip;
} __attribute__ ((packed)) msg_player;

typedef struct {
    uint8_t type;
    msg_player players[MAX_SOCKETS];
} __attribute__ ((packed)) msg_players;

boolean is_server;
boolean is_client;
boolean is_headless;

IPaddress server_address = {0};
IPaddress no_address = {0};

network net;

static boolean sockets_show_all(tokens_t *tokens, void *context);
static boolean sockets_show_summary(tokens_t *tokens, void *context);
static boolean socket_init_done;

boolean socket_init (void)
{
    if (socket_init_done) {
        return (true);
    }

    if (SDLNet_Init() < 0) {
        ERR("cannot init SDL_net");
        return (false);
    }

    command_add(sockets_show_all, "show sockets detail", 
                "clients and server sockets");

    command_add(sockets_show_summary, "show sockets summary", 
                "one line output");

    command_add(debug_socket_ping_enable, "debug socket ping [01]",
                "debug periodic pings");

    command_add(debug_socket_connect_enable, "debug socket connect [01]",
                "debug sockets connections");

    command_add(debug_socket_players_enable, "debug socket update [01]",
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

    int s;
    for (s = 0; s < MAX_SOCKETS; s++) {
        socket_disconnect(&net.sockets[s]);
    }

    memset(&net, 0, sizeof(net));

    socket_init_done = false;
}

socket *socket_listen (IPaddress address)
{
    IPaddress listen_address = address;
    uint16_t p;

    socket *s = 0;
    uint32_t si;

    /*
     * Find the first free socket.
     */
    for (si = 0; si < MAX_SOCKETS; si++) {
        s = &net.sockets[si];
        if (s->open) {
            continue;
        }

        break;
    }

    if (si == MAX_SOCKETS) {
        ERR("No more sockets are available for listening");
        return (0);
    }

    /*
     * If no address is given, try and grab one from our well known base.
     */
    if (!memcmp(&no_address, &listen_address, sizeof(no_address))) {
        DBG("Resolve host %s port %u", 
            SERVER_DEFAULT_HOST, 
            SERVER_DEFAULT_PORT);

        if ((SDLNet_ResolveHost(&listen_address, 
                                SERVER_DEFAULT_HOST,
                                SERVER_DEFAULT_PORT)) == -1) {
            ERR("Cannot resolve host %s port %u", 
                SERVER_DEFAULT_HOST, 
                SERVER_DEFAULT_PORT);
            return (false);
        }

        if (!memcmp(&no_address, &listen_address, sizeof(no_address))) {
            ERR("Cannot get a local port to listen on");
            return (false);
        }
    }

    /*
     * If given an address, we must listn on that specific address.
     * If not then we can look for the next free.
     */
    uint16_t max_port;

    if (!memcmp(&no_address, &address, sizeof(no_address))) {
        max_port = 1;
    } else {
        max_port = MAX_SOCKETS;
    }

    uint16_t port = SDLNet_Read16(&listen_address.port);

    DBG("Find a server port...");

    for (p = 0; p <= max_port; p++, port++) {

        SDLNet_Write16(port, &listen_address.port);
        port = SDLNet_Read16(&listen_address.port);

        /*
         * Check the port is not in use.
         */
        for (si = 0; si < MAX_SOCKETS; si++) {
            socket *s = &net.sockets[si];

            if (!s->server) {
                continue;
            }

            if (!memcmp(&listen_address, &s->local_ip, sizeof(IPaddress))) {
                break;
            }
        }

        if (si != MAX_SOCKETS) {
            continue;
        }

        s->udp_socket = SDLNet_UDP_Open(port);
        if (!s->udp_socket) {
            char *tmp = iptodynstr(listen_address);
            ERR("SDLNet_UDP_Open %s failed", tmp);
            WARN("  %s", SDLNet_GetError());
            myfree(tmp);
            continue;
        }

        s->channel = SDLNet_UDP_Bind(s->udp_socket, -1, &listen_address);
        if (s->channel < 0) {
            char *tmp = iptodynstr(listen_address);
            ERR("SDLNet_UDP_Bind %s failed", tmp);
            WARN("  %s", SDLNet_GetError());
            myfree(tmp);
            continue;
        }

        s->socklist = SDLNet_AllocSocketSet(MAX_SOCKETS);
        if (!s->socklist) {
            char *tmp = iptodynstr(listen_address);
            ERR("SDLNet_AllocSocketSet %s failed", tmp);
            WARN("  %s", SDLNet_GetError());
            myfree(tmp);
            continue;
        }

        if (SDLNet_UDP_AddSocket(s->socklist, s->udp_socket) == -1) {
            char *tmp = iptodynstr(listen_address);
            ERR("SDLNet_UDP_AddSocket %s failed", tmp);
            WARN("  %s", SDLNet_GetError());
            myfree(tmp);
            continue;
        }

        s->open = true;
        s->local_ip = listen_address;
        s->index = s - net.sockets;
        s->server = true;

        return (s);
    }

    WARN("Failed to listen");
    return (0);
}

socket *socket_connect (IPaddress address, boolean server_side_client)
{
    IPaddress connect_address = address;

    socket *s = 0;
    uint32_t si;

    /*
     * Find the first free socket.
     */
    for (si = 0; si < MAX_SOCKETS; si++) {
        s = &net.sockets[si];
        if (s->open) {
            continue;
        }

        break;
    }

    if (si == MAX_SOCKETS) {
        WARN("No more sockets [%u] are available for connecting to clients",
             MAX_SOCKETS);
        return (0);
    }

    /*
     * If no address is given, try and grab one from our well known base.
     */
    if (!memcmp(&no_address, &connect_address, sizeof(no_address))) {
        DBG("Resolve client host %s port %u", 
            SERVER_DEFAULT_HOST, 
            SERVER_DEFAULT_PORT);

        if ((SDLNet_ResolveHost(&connect_address, 
                                SERVER_DEFAULT_HOST,
                                SERVER_DEFAULT_PORT)) == -1) {
            WARN("Cannot resolve host %s port %u", 
                SERVER_DEFAULT_HOST, 
                SERVER_DEFAULT_PORT);
            return (false);
        }

        if (!memcmp(&no_address, &connect_address, sizeof(no_address))) {
            WARN("Cannot get a local port to connect on");
            return (false);
        }
    }

    /*
     * If given an address, we must listn on that specific address.
     * If not then we can look for the next free.
     */
    uint16_t port = SDLNet_Read16(&connect_address.port);

    SDLNet_Write16(port, &connect_address.port);
    port = SDLNet_Read16(&connect_address.port);

    s->udp_socket = SDLNet_UDP_Open(0);
    if (!s->udp_socket) {
        char *tmp = iptodynstr(connect_address);
        ERR("SDLNet_UDP_Open %s failed", tmp);
        WARN("  %s", SDLNet_GetError());
        myfree(tmp);
        return (false);
    }

    s->channel = SDLNet_UDP_Bind(s->udp_socket, -1, &connect_address);
    if (s->channel < 0) {
        char *tmp = iptodynstr(connect_address);
        ERR("SDLNet_UDP_Bind %s failed", tmp);
        WARN("  %s", SDLNet_GetError());
        myfree(tmp);
        return (false);
    }

    s->socklist = SDLNet_AllocSocketSet(MAX_SOCKETS);
    if (!s->socklist) {
        char *tmp = iptodynstr(connect_address);
        ERR("SDLNet_AllocSocketSet %s failed", tmp);
        WARN("  %s", SDLNet_GetError());
        myfree(tmp);
        return (false);
    }

    if (SDLNet_UDP_AddSocket(s->socklist, s->udp_socket) == -1) {
        char *tmp = iptodynstr(connect_address);
        ERR("SDLNet_UDP_AddSocket %s failed", tmp);
        WARN("  %s", SDLNet_GetError());
        myfree(tmp);
        return (false);
    }

    s->open = true;
    s->server_side_client = server_side_client;

    if (!server_side_client) {
        s->client = true;
    }

    s->remote_ip = connect_address;
    s->local_ip = *SDLNet_UDP_GetPeerAddress(s->udp_socket, -1);
    s->index = s - net.sockets;

    LOG("Peer up [%s]", socket_get_remote_logname(s));

    return (s);
}

void socket_disconnect (socketp s)
{
    if (!s->open) {
        return;
    }

    socket_set_connected(s, false);

    LOG("Peer disc [%s]", socket_get_remote_logname(s));

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

    memset(s, 0, sizeof(*s));
}

/*
 * User has entered a command, run it
 */
boolean debug_socket_ping_enable (tokens_t *tokens, void *context)
{
    char *s = tokens->args[3];

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
    char *s = tokens->args[3];

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
    char *s = tokens->args[3];

    if (!s || (*s == '\0')) {
        debug_socket_players_enabled = 1;
    } else {
        debug_socket_players_enabled = strtol(s, 0, 10) ? 1 : 0;
    }

    CON("Debug socket player mode set to %u", debug_socket_players_enabled);

    return (true);
}

socket *socket_get (uint32_t si)
{
    if (si >= MAX_SOCKETS) {
        return (0);
    }

    return (&net.sockets[si]);
}

socket *socket_find_local_ip (IPaddress address)
{
    uint32_t si;

    for (si = 0; si < MAX_SOCKETS; si++) {
        socket *s = &net.sockets[si];

        if (!memcmp(&address, &s->local_ip, sizeof(IPaddress))) {
            return (s);
        }
    }

    return (0);
}

socket *socket_find_remote_ip (IPaddress address)
{
    uint32_t si;

    for (si = 0; si < MAX_SOCKETS; si++) {
        socket *s = &net.sockets[si];

        if (!memcmp(&address, &s->remote_ip, sizeof(IPaddress))) {
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

    if (!memcmp(&ip, &no_address, sizeof(no_address))) {
        return (dynprintf("-"));
    }

    if (!(hostname = SDLNet_ResolveIP(&ip))) {
        return (dynprintf("IPv4 %u.%u.%u.%u:%u",
                          hostname, ip1, ip2, ip3, ip4, port));

    }

    if (!strcmp(hostname, "0.0.0.0")) {
        hostname = "";
    }

    if (strcmp(hostname, "localhost")) {
        hostname = "local";
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

static boolean sockets_show_all (tokens_t *tokens, void *context)
{
    int si;

    for (si = 0; si < MAX_SOCKETS; si++) {
        const socketp s = &net.sockets[si];

        if (!s->open) {
            continue;
        }

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

        CON("  Ping     : tx %u, rx %u",
            s->tx_msg[MSG_TYPE_PING], s->rx_msg[MSG_TYPE_PING]);
        CON("  Pong     : tx %u, rx %u",
            s->tx_msg[MSG_TYPE_PONG], s->rx_msg[MSG_TYPE_PONG]);
        CON("  Name     : tx %u, rx %u",
            s->tx_msg[MSG_TYPE_NAME], s->rx_msg[MSG_TYPE_NAME]);
        CON("  Updates  : tx %u, rx %u",
            s->tx_msg[MSG_TYPE_PLAYERS_ALL], s->rx_msg[MSG_TYPE_PLAYERS_ALL]);

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
            avg_latency = avg_latency /= total_attempts;

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
    int si;

    CON("Name                 Quality  Latency       Remote IP           Local IP");
    CON("----                 -------  ------- -------------------- ------------------");
        
    for (si = 0; si < MAX_SOCKETS; si++) {
        const socketp s = &net.sockets[si];

        if (!s->open) {
            continue;
        }

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
            avg_latency = avg_latency /= total_attempts;

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

static uint32_t sockets_fail_rate (socketp s)
{
    /*
     * Ping stats.
     */
    uint32_t no_response = 0;
    uint32_t response = 0;
    uint32_t total_attempts = 0;

    FOR_ALL_IN_ARRAY(latency, s->ping_responses) {
        if (*latency == -1) {
            ++no_response;
            continue;
        }

        if (*latency == 0) {
            continue;
        }

        ++response;
    }

    total_attempts = no_response + response;

    if (total_attempts > ARRAY_SIZE(s->ping_responses) / 2) {
        float success = 
                ((float)((float)response / (float)total_attempts)) * 100.0;

        return (success);
    }

    return (100);
}

void sockets_alive_check (void)
{
    uint32_t si;
    for (si = 0; si < MAX_SOCKETS; si++) {
        socketp s = &net.sockets[si];

        if (!s->open) {
            continue;
        }

        uint32_t success = sockets_fail_rate(s);

        if (success < SOCKET_PING_FAIL_THRESHOLD) {
            LOG("Peer down [%s] qual %u percent",
                socket_get_remote_logname(s), success);

            socket_disconnect(s);
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
    strncpy(s->name, name, sizeof(s->name) - 1);
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

boolean socket_get_open (const socketp s)
{
    return (s->open);
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
        if (debug_socket_connect_enabled) {
            LOG("Connected to [%s]", socket_get_remote_logname(s));
            LOG("  Locally    [%s]", socket_get_local_logname(s));
        }

    } else {
        if (debug_socket_connect_enabled) {
            LOG("Disconnected from [%s]", socket_get_remote_logname(s));
        }
    }

    s->connected = c;
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
    if (type < MSG_TYPE_MAX) {
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
    LOG("Bad socket message [from %s]", tmp);
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

    *data++ = MSG_TYPE_PING;
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

    *data++ = MSG_TYPE_PONG;
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
        LOG("Rx Ping [from %s] seq %u", tmp, seq);
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
        LOG("Rx Pong [from %s] seq %u, elapsed %u",
            tmp, seq, time_get_time_cached() - ts);
        myfree(tmp);
    }

    s->ping_responses[seq % ARRAY_SIZE(s->ping_responses)] = 
                    time_get_time_cached() - ts;
}

void socket_tx_player (socketp s)
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
    msg.type = MSG_TYPE_NAME;
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

void socket_rx_player (socketp s, UDPpacket *packet, uint8_t *data)
{
    msg_name msg = {0};

    if (packet->len != sizeof(msg)) {
        socket_count_inc_pak_rx_error(s, packet);
        return;
    }

    memcpy(&msg, packet->data, sizeof(msg));

    if (debug_socket_players_enabled) {
        char *tmp = iptodynstr(read_address(packet));
        LOG("Rx Name [from %s] \"%s\"", tmp, msg.name);
        myfree(tmp);
    }

    socket_set_name(s, msg.name);

    /*
     * Update the player structure.
     */
    aplayer *p = myzalloc(sizeof(*p), "player");

    memcpy(p->name, msg.name, PLAYER_NAME_MAX);
    p->local_ip = s->local_ip;
    p->remote_ip = s->remote_ip;
    s->player = p;
}

/*
 * Send an array of all curent players to all clients.
 */
void socket_tx_players_all (void)
{
    UDPpacket *packet = socket_alloc_msg();
    aplayer players[MAX_SOCKETS];

    memset(&players, 0, sizeof(players));

    msg_players msg = {0};
    msg.type = MSG_TYPE_PLAYERS_ALL;

    uint32_t si;

    for (si = 0; si < MAX_SOCKETS; si++) {
        socketp s = &net.sockets[si];

        if (!s->connected) {
            continue;
        }

        if (!s->server_side_client) {
            continue;
        }

        msg_player *pm = &msg.players[si];
        aplayer *pp = &players[si];

        strncpy(pm->name, pp->name, min(sizeof(pm->name), strlen(pp->name))); 
        memcpy(&pm->local_ip, &pp->local_ip, sizeof(pp->local_ip));
        memcpy(&pm->remote_ip, &pp->remote_ip, sizeof(pp->remote_ip));
    }

    memcpy(packet->data, &msg, sizeof(msg));

    for (si = 0; si < MAX_SOCKETS; si++) {
        socketp s = &net.sockets[si];

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
        
    socket_free_msg(packet);
}

/*
 * Receive an array of all curent players from the server.
 */
void socket_rx_players_all (socketp s, UDPpacket *packet, uint8_t *data,
                            aplayer *players)
{
    msg_players *msg;

    if (packet->len != sizeof(*msg)) {
        socket_count_inc_pak_rx_error(s, packet);
        return;
    }

    uint32_t si;

    msg = (typeof(msg)) packet->data;

    for (si = 0; si < MAX_SOCKETS; si++) {
        aplayer *pp = &players[si];
        msg_player *pm = &msg->players[si];

        memcpy(pp->name, pm->name, PLAYER_NAME_MAX);
        memcpy(&pp->local_ip, &pm->local_ip, sizeof(s->local_ip));
        memcpy(&pp->remote_ip, &pm->remote_ip, sizeof(s->remote_ip));

        if (!pp->name[0]) {
            continue;
        }

        if (debug_socket_players_enabled) {
            char *tmp = iptodynstr(read_address(packet));
            LOG("Rx All Players [from %s] %u:\"%s\"", tmp, si, pp->name);
            myfree(tmp);
        }
    }
}
