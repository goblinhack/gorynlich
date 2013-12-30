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

typedef struct socket_ {
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
    /*
     * Counters.
     */
    uint32_t rx;
    uint32_t tx;
    uint32_t rx_error;
    uint32_t tx_error;
    uint32_t rx_bad_msg;
} socket;

typedef struct network_ {
    socket sockets[MAX_SOCKETS];
} network;

boolean is_server;
boolean is_client;
boolean is_headless;

IPaddress server_address = {0};
IPaddress no_address = {0};

network net;

static boolean sockets_show(tokens_t *tokens, void *context);
static boolean net_init_done;

boolean net_init (void)
{
    if (net_init_done) {
        return (true);
    }

    if (SDLNet_Init() < 0) {
        ERR("cannot init SDL_net");
        return (false);
    }

    net_init_done = true;
    command_add(sockets_show, "show network", "clients and server info");

    return (true);
}

void net_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (!net_init_done) {
        return;
    }

    SDLNet_Quit();

    int s;
    for (s = 0; s < MAX_SOCKETS; s++) {
        if (net.sockets[s].local_logname) {
            myfree(net.sockets[s].local_logname);
        }

        if (net.sockets[s].remote_logname) {
            myfree(net.sockets[s].remote_logname);
        }
    }

    memset(&net, 0, sizeof(net));

    net_init_done = false;
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
        DBG("Resolve host %s port %d", 
            SERVER_DEFAULT_HOST, 
            SERVER_DEFAULT_PORT);

        if ((SDLNet_ResolveHost(&listen_address, 
                                SERVER_DEFAULT_HOST,
                                SERVER_DEFAULT_PORT)) == -1) {
            ERR("Cannot resolve host %s port %d", 
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

        return (s);
    }

    WARN("Failed to listen");
    return (0);
}

socket *socket_connect (IPaddress address)
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
        DBG("Resolve client host %s port %d", 
            SERVER_DEFAULT_HOST, 
            SERVER_DEFAULT_PORT);

        if ((SDLNet_ResolveHost(&connect_address, 
                                SERVER_DEFAULT_HOST,
                                SERVER_DEFAULT_PORT)) == -1) {
            WARN("Cannot resolve host %s port %d", 
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

    s->remote_ip = connect_address;
    s->local_ip = *SDLNet_UDP_GetPeerAddress(s->udp_socket, -1);

    return (s);
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

static boolean sockets_show (tokens_t *tokens, void *context)
{
    int si;

    for (si = 0; si < MAX_SOCKETS; si++) {
        const socketp s = &net.sockets[si];

        if (!s->open) {
            continue;
        }

        if (s->server) {
            CON("[%d] Server", si);
        } else {
            CON("[%d] Client", si);
        }

        CON("  Local IP : %s", socket_get_local_logname(s));

        if (s->client) {
            CON("  Remote IP: %s", socket_get_remote_logname(s));
        }

        CON("  Tx: %u Rx: %u", s->tx, s->rx);
        CON("  Tx error: %u Rx error: %u Bad message: %u", 
            s->tx_error, s->rx_error, s->rx_bad_msg);
    }

    return (true);
}

IPaddress socket_get_local_ip (const socketp s)
{
    return (s->local_ip);
}

IPaddress socket_get_remote_ip (const socketp s)
{
    return (s->remote_ip);
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

void socket_set_server (socketp s, boolean c)
{
    s->server = c;
}

boolean socket_get_server (const socketp s)
{
    return (s->server);
}

void socket_set_client (socketp s, boolean c)
{
    s->client = c;
}

boolean socket_get_client (const socketp s)
{
    return (s->client);
}

void socket_set_channel (socketp s, int c)
{
    s->channel = c;
}

boolean socket_get_channel (const socketp s)
{
    return (s->channel);
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

void send_ping (socketp s, uint16_t seq, uint32_t ts)
{
    UDPpacket *packet;      

    packet = SDLNet_AllocPacket(MAX_PACKET_SIZE);
    if (!packet) {
        ERR("Out of packet space, pak %d", MAX_PACKET_SIZE);
        return;
    }

    uint8_t *data = packet->data;
    uint8_t *odata = data;

    packet->address = socket_get_remote_ip(s);

    SDLNet_Write16(MSG_TYPE_PING, data);               
    data += sizeof(uint16_t);

    SDLNet_Write16(seq, data);               
    data += sizeof(uint16_t);

    SDLNet_Write32(ts, data);               
    data += sizeof(uint32_t);

    packet->len = data - odata;

    LOG("Ping [%s] seq %d, ts %d", socket_get_remote_logname(s), seq, ts);

    if (SDLNet_UDP_Send(socket_get_udp_socket(s),
                        socket_get_channel(s), packet) < 1) {
        ERR("no UDP packet sent");

        socket_count_inc_pak_tx_error(s);
    } else {
        socket_count_inc_pak_tx(s);
    }
        
    SDLNet_FreePacket(packet);
}

void send_pong (socketp s, uint16_t seq, uint32_t ts)
{
    UDPpacket *packet;      

    packet = SDLNet_AllocPacket(MAX_PACKET_SIZE);
    if (!packet) {
        ERR("Out of packet space, pak %d", MAX_PACKET_SIZE);
        return;
    }

    uint8_t *data = packet->data;
    uint8_t *odata = data;

    packet->address = socket_get_remote_ip(s);

    SDLNet_Write16(MSG_TYPE_PONG, data);               
    data += sizeof(uint16_t);

    SDLNet_Write16(seq, data);               
    data += sizeof(uint16_t);

    SDLNet_Write32(ts, data);               
    data += sizeof(uint32_t);

    packet->len = data - odata;

    if (SDLNet_UDP_Send(socket_get_udp_socket(s),
                        socket_get_channel(s), packet) < 1) {
        ERR("no UDP packet sent");

        socket_count_inc_pak_tx_error(s);
    } else {
        socket_count_inc_pak_tx(s);
    }
        
    SDLNet_FreePacket(packet);
}

void receive_ping (socketp s, UDPpacket *packet, uint8_t *data)
{
    uint16_t seq = SDLNet_Read16(data);
    data += sizeof(uint16_t);

    uint32_t ts = SDLNet_Read32(data);
    data += sizeof(uint32_t);

    char *tmp = iptodynstr(packet->address);
    LOG("Pong [%s] seq %d", tmp, seq);
    myfree(tmp);

    send_pong(s, seq, ts);
}

void receive_pong (socketp s, UDPpacket *packet, uint8_t *data)
{
    uint16_t seq = SDLNet_Read16(data);
    data += sizeof(uint16_t);

    uint32_t ts = SDLNet_Read32(data);
    data += sizeof(uint32_t);

    char *tmp = iptodynstr(packet->address);
    LOG("Pong [%s] seq %d, elapsed %u",
        tmp, seq, time_get_time_cached() - ts);

    myfree(tmp);
}
