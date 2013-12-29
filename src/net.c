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
#include "net.h"
#include "string.h"
#include "slre.h"
#include "command.h"

boolean is_server;
boolean is_client;
IPaddress server_address = {0};
IPaddress no_address = {0};
network net;

static boolean net_show(tokens_t *tokens, void *context);
static boolean net_init_done;

boolean net_init (void)
{
    if (net_init_done) {
        return (true);
    }

    if (SDLNet_Init() < 0) {
        ERR_TB("cannot init SDL_net");
        return (false);
    }

    net_init_done = true;
    command_add(net_show, "show network", "clients and server info");

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

socket *net_listen (IPaddress address)
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

    if (!s) {
        ERR_TB("No more sockets are available");
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
            ERR_TB("Cannot resolve host %s port %d", 
                SERVER_DEFAULT_HOST, 
                SERVER_DEFAULT_PORT);
            return (false);
        }

        if (!memcmp(&no_address, &listen_address, sizeof(no_address))) {
            ERR_TB("Cannot get a local port to listen on");
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

        char *tmp = iptodynstr(listen_address);
        DBG("Trying to listen on: %s", tmp);

        s->udp_socket = SDLNet_UDP_Open(port);
        if (!s->udp_socket) {
            ERR_TB("SDLNet_UDP_Open %s failed", tmp);
            ERR("  %s", SDLNet_GetError());
            myfree(tmp);
            continue;
        }

        s->channel = SDLNet_UDP_Bind(s->udp_socket, -1, &listen_address);
        if (s->channel < 0) {
            ERR_TB("SDLNet_UDP_Bind %s failed", tmp);
            ERR("  %s", SDLNet_GetError());
            myfree(tmp);
            continue;
        }

        s->socklist = SDLNet_AllocSocketSet(MAX_SOCKETS);
        if (!s->socklist) {
            ERR_TB("SDLNet_AllocSocketSet %s failed", tmp);
            ERR("  %s", SDLNet_GetError());
            myfree(tmp);
            continue;
        }

        if (SDLNet_UDP_AddSocket(s->socklist, s->udp_socket) == -1) {
            ERR_TB("SDLNet_UDP_AddSocket %s failed", tmp);
            ERR("  %s", SDLNet_GetError());
            myfree(tmp);
            continue;
        }

        s->local_logname = tmp;
        s->open = true;
        s->local_ip = listen_address;

        return (s);
    }

    ERR("Failed to listen");
    return (0);
}

socket *net_connect (IPaddress address)
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

    if (!s) {
        ERR("No more sockets are available");
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
            ERR("Cannot resolve host %s port %d", 
                SERVER_DEFAULT_HOST, 
                SERVER_DEFAULT_PORT);
            return (false);
        }

        if (!memcmp(&no_address, &connect_address, sizeof(no_address))) {
            ERR("Cannot get a local port to connect on");
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

    char *tmp = iptodynstr(connect_address);
    DBG("Trying to connect to %s", tmp);

    s->udp_socket = SDLNet_UDP_Open(0);
    if (!s->udp_socket) {
        ERR_TB("SDLNet_UDP_Open %s failed", tmp);
        ERR("  %s", SDLNet_GetError());
        myfree(tmp);
        return (false);
    }

    s->channel = SDLNet_UDP_Bind(s->udp_socket, -1, &connect_address);
    if (s->channel < 0) {
        ERR_TB("SDLNet_UDP_Bind %s failed", tmp);
        ERR("  %s", SDLNet_GetError());
        myfree(tmp);
        return (false);
    }
    myfree(tmp);

    s->local_logname = tmp;
    s->open = true;

    s->remote_ip = connect_address;
    s->remote_logname = iptodynstr(s->remote_ip);

    s->local_ip = *SDLNet_UDP_GetPeerAddress(s->udp_socket, -1);
    s->local_logname = iptodynstr(s->local_ip);

    return (s);
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

static boolean net_show (tokens_t *tokens, void *context)
{
    const char *prefix = "%-30s %-30s %-6s";

    CON(prefix, "Local", "Remote", "Type");
    CON(prefix, "-----", "------", "----");

    int si;
    for (si = 0; si < MAX_SOCKETS; si++) {
        const socket *s = &net.sockets[si];

        if (!s->open) {
            continue;
        }

        CON(prefix, 
            s->local_logname ? s->local_logname : "n/a",
            s->remote_logname ? s->remote_logname : "n/a",
            s->server ? "Server" : "Client");
    }

    return (true);
}

