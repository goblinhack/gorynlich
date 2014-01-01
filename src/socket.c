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
    const char *name;
    const char *local_logname;
    const char *remote_logname;
    int channel;
    SDLNet_SocketSet socklist;
    /*
     * Counters.
     */
    uint32_t rx;
    uint32_t tx;
    uint32_t rx_error;
    uint32_t tx_error;
    uint32_t rx_bad_msg;
    uint32_t ping_responses[20];
    uint32_t tx_msg[MSG_TYPE_MAX];
    uint32_t rx_msg[MSG_TYPE_MAX];
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
static boolean sockets_show_clients(tokens_t *tokens, void *context);
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

    command_add(sockets_show_all, "show socket", 
                "clients and server sockets");

    command_add(sockets_show_clients, "show users", 
                "clients only");

    command_add(debug_socket_ping_enable, "debug socket ping [01]",
                "debug periodic pings");

    command_add(debug_socket_connect_enable, "debug socket connect [01]",
                "debug sockets connections");

    command_add(debug_socket_players_enable, "debug socket player [01]",
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
        ERR("No more sockets are available");
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
        WARN("No more sockets are available");
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

    return (s);
}

void socket_disconnect (socketp s)
{
    if (!s->open) {
        return;
    }

    socket_set_connected(s, false);

    LOG("Close peer [%s] %s", socket_get_remote_logname(s),
        socket_get_local_logname(s));

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

    if (s->name) {
        myfree((char *)s->name);
    }

    memset(s, 0, sizeof(*s));
}

/*
 * User has entered a command, run it
 */
boolean debug_socket_ping_enable (tokens_t *tokens, void *context)
{
    char *s = tokens->args[2];

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
    char *s = tokens->args[2];

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
    char *s = tokens->args[2];

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
        return (dynprintf("<no IP address>"));
    }

    if (!(hostname = SDLNet_ResolveIP(&ip))) {
        return (dynprintf("IPv4 %u.%u.%u.%u:%u",
                          hostname, ip1, ip2, ip3, ip4, port));
    } else {
        return (dynprintf("[%s] %u.%u.%u.%u:%u",
                          hostname, ip1, ip2, ip3, ip4, port));
    }
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
        CON("  Players  : tx %u, rx %u",
            s->tx_msg[MSG_TYPE_PLAYERS], s->rx_msg[MSG_TYPE_PLAYERS]);

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

            CON("  Ping     : success %2.2f percent, fails %2.2f percent",
                ((float)((float)response / (float)total_attempts)) * 100.0,
                ((float)((float)no_response / (float)total_attempts)) * 100.0);

            CON("  Latency  : max %u ms, min %u ms, average %u ms",
                max_latency, min_latency, avg_latency);
        }
    }

    return (true);
}

static boolean sockets_show_clients (tokens_t *tokens, void *context)
{
    int si;

    CON("Name                 Quality  Latency       Remote IP");
    CON("----                 -------  ------- ------------------------");
        
    for (si = 0; si < MAX_SOCKETS; si++) {
        const socketp s = &net.sockets[si];

        if (!s->open) {
            continue;
        }

        if (s->server) {
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

            CON("%-20s %3.0f pct %3d ms %-20s", 
                socket_get_name(s),
                ((float)((float)response / (float)total_attempts)) * 100.0,
                avg_latency,
                socket_get_remote_logname(s));
        } else {
            CON("%-20s ---     ---     %-20s", 
                socket_get_name(s),
                socket_get_remote_logname(s));
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

        if (success < 10) {
            LOG("Dead peer [%s] ping success %u percent",
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
    if (s->name) {
        myfree((char *)s->name);
    }

    s->name = name;
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

void socket_count_inc_pak_rx (const socketp s)
{
    s->rx++;
}

void socket_count_inc_pak_tx (const socketp s)
{
    s->tx++;
}

void socket_count_inc_pak_rx_error (const socketp s)
{
    s->rx_error++;
}

void socket_count_inc_pak_tx_error (const socketp s)
{
    s->tx_error++;
}

void socket_count_inc_pak_rx_bad_msg (const socketp s)
{
    s->rx_bad_msg++;
}

void socket_tx_ping (socketp s, uint8_t seq, uint32_t ts)
{
    if (!socket_get_udp_socket(s)) {
        return;
    }

    UDPpacket *packet;      

    packet = SDLNet_AllocPacket(MAX_PACKET_SIZE);
    if (!packet) {
        ERR("Out of packet space, pak %u", MAX_PACKET_SIZE);
        return;
    }

    uint8_t *data = packet->data;
    uint8_t *odata = data;

    packet->address = socket_get_remote_ip(s);

    *data++ = MSG_TYPE_PING;
    *data++ = seq;

    SDLNet_Write32(ts, data);               
    data += sizeof(uint32_t);

    packet->len = data - odata;

    s->ping_responses[seq % ARRAY_SIZE(s->ping_responses)] = (uint32_t) -1;

    if (debug_socket_ping_enabled) {
        LOG("Tx Ping [to %s] seq %u, ts %u", 
            socket_get_remote_logname(s), seq, ts);
    }

    if (SDLNet_UDP_Send(socket_get_udp_socket(s),
                        socket_get_channel(s), packet) < 1) {
        ERR("no UDP packet sent: %s", SDLNet_GetError());

        socket_count_inc_pak_tx_error(s);
    } else {
        socket_count_inc_pak_tx(s);

        s->tx_msg[MSG_TYPE_PING]++;
    }
            
    SDLNet_FreePacket(packet);
}

void socket_tx_pong (socketp s, uint8_t seq, uint32_t ts)
{
    if (!socket_get_udp_socket(s)) {
        return;
    }

    UDPpacket *packet;      

    packet = SDLNet_AllocPacket(MAX_PACKET_SIZE);
    if (!packet) {
        ERR("Out of packet space, pak %u", MAX_PACKET_SIZE);
        return;
    }

    uint8_t *data = packet->data;
    uint8_t *odata = data;

    packet->address = socket_get_remote_ip(s);

    *data++ = MSG_TYPE_PONG;
    *data++ = seq;

    SDLNet_Write32(ts, data);               
    data += sizeof(uint32_t);

    packet->len = data - odata;

    if (SDLNet_UDP_Send(socket_get_udp_socket(s),
                        socket_get_channel(s), packet) < 1) {
        ERR("no UDP packet sent: %s", SDLNet_GetError());

        socket_count_inc_pak_tx_error(s);
    } else {
        socket_count_inc_pak_tx(s);

        s->tx_msg[MSG_TYPE_PONG]++;
    }
        
    SDLNet_FreePacket(packet);
}

void socket_rx_ping (socketp s, UDPpacket *packet, uint8_t *data)
{
    uint8_t seq = *data++;
    uint32_t ts = SDLNet_Read32(data);
    data += sizeof(uint32_t);

    if (debug_socket_ping_enabled) {
        char *tmp = iptodynstr(packet->address);
        LOG("Rx ping [from %s] seq %u", tmp, seq);
        myfree(tmp);
    }

    socket_tx_pong(s, seq, ts);

    socket_set_connected(s, true);

    s->rx_msg[MSG_TYPE_PING]++;
}

void socket_rx_pong (socketp s, UDPpacket *packet, uint8_t *data)
{
    uint8_t seq = *data++;
    uint32_t ts = SDLNet_Read32(data);
    data += sizeof(uint32_t);

    if (debug_socket_ping_enabled) {
        char *tmp = iptodynstr(packet->address);
        LOG("Rx Pong [from %s] seq %u, elapsed %u",
            tmp, seq, time_get_time_cached() - ts);
        myfree(tmp);
    }

    s->ping_responses[seq % ARRAY_SIZE(s->ping_responses)] = 
                    time_get_time_cached() - ts;

    s->rx_msg[MSG_TYPE_PONG]++;
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

    UDPpacket *packet;      

    if (!s->connected) {
        return;
    }

    packet = SDLNet_AllocPacket(MAX_PACKET_SIZE);
    if (!packet) {
        ERR("Out of packet space, pak %u", MAX_PACKET_SIZE);
        return;
    }

    msg_name msg = {0};
    msg.type = MSG_TYPE_NAME;
    strncpy(msg.name, s->name, min(sizeof(msg.name), strlen(s->name))); 

    memcpy(packet->data, &msg, sizeof(msg));
    packet->len = sizeof(msg);
    packet->address = socket_get_remote_ip(s);

    if (debug_socket_players_enabled) {
        LOG("Tx Name [to %s] \"%s\"", socket_get_remote_logname(s), s->name);
    }

    if (SDLNet_UDP_Send(socket_get_udp_socket(s),
                        socket_get_channel(s), packet) < 1) {
        ERR("no UDP packet sent: %s", SDLNet_GetError());

        socket_count_inc_pak_tx_error(s);
    } else {
        socket_count_inc_pak_tx(s);

        s->tx_msg[msg.type]++;
    }
        
    SDLNet_FreePacket(packet);
}

void socket_rx_name (socketp s, UDPpacket *packet, uint8_t *data)
{
    msg_name msg = {0};

    if (packet->len != sizeof(msg)) {
        char *tmp = iptodynstr(packet->address);
        LOG("Bad socket rx name message [%s]", tmp);
        myfree(tmp);

        socket_count_inc_pak_rx_error(s);
        return;
    }

    memcpy(&msg, packet->data, sizeof(msg));

    char *name = mymalloc(sizeof(msg.name) + 1, "client name");
    memcpy(name, msg.name, sizeof(msg.name));

    if (debug_socket_players_enabled) {
        char *tmp = iptodynstr(packet->address);
        LOG("Rx name [from %s] \"%s\"", tmp, name);
        myfree(tmp);
    }

    socket_set_name(s, name);

    s->rx_msg[MSG_TYPE_NAME]++;
}

void socket_tx_players (void)
{
    UDPpacket *packet;      

    packet = SDLNet_AllocPacket(MAX_PACKET_SIZE);
    if (!packet) {
        ERR("Out of packet space, pak %u", MAX_PACKET_SIZE);
        return;
    }

    msg_players msg = {0};
    msg.type = MSG_TYPE_PLAYERS;

    uint32_t si;

    for (si = 0; si < MAX_SOCKETS; si++) {
        socketp s = &net.sockets[si];

        if (!s->connected) {
            continue;
        }

        if (!s->server_side_client) {
            continue;
        }

        if (!s->name) {
            continue;
        }

        strncpy(msg.players[si].name, s->name, 
                min(sizeof(msg.players[si].name), strlen(s->name))); 
    }

    memcpy(packet->data, &msg, sizeof(msg));
    packet->len = sizeof(msg);

    for (si = 0; si < MAX_SOCKETS; si++) {
        socketp s = &net.sockets[si];

        if (!s->connected) {
            continue;
        }

        if (!s->server_side_client) {
            continue;
        }

        if (debug_socket_players_enabled) {
            LOG("Tx Players [to %s]",
                socket_get_remote_logname(s));
        }

        packet->address = socket_get_remote_ip(s);

        if (SDLNet_UDP_Send(socket_get_udp_socket(s),
                            socket_get_channel(s), packet) < 1) {
            ERR("no UDP packet sent: %s", SDLNet_GetError());

            socket_count_inc_pak_tx_error(s);
        } else {
            socket_count_inc_pak_tx(s);

            s->tx_msg[msg.type]++;
        }
    }
        
    SDLNet_FreePacket(packet);
}

void socket_rx_players (socketp s, UDPpacket *packet, uint8_t *data)
{
    msg_players *msg;

    if (packet->len != sizeof(*msg)) {
        char *tmp = iptodynstr(packet->address);
        LOG("Bad socket rx name message [%s]", tmp);
        myfree(tmp);

        socket_count_inc_pak_rx_error(s);
        return;
    }

    uint32_t si;

    msg = (typeof(msg)) packet->data;

    for (si = 0; si < MAX_PLAYERS; si++) {
        aplayer *pp = &players[si];
        msg_player *pm = &msg->players[si];

        memcpy(pp->name, pm->name, PLAYER_NAME_MAX);

        if (!pp->name[0]) {
            continue;
        }

        if (debug_socket_players_enabled) {
            char *tmp = iptodynstr(packet->address);
            LOG("Rx players [from %s] %u:\"%s\"", tmp, si, pp->name);
            myfree(tmp);
        }
    }

    s->rx_msg[MSG_TYPE_PLAYERS]++;
}
