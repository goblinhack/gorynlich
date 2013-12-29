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
IPaddress listen_address = {0};
IPaddress connect_address = {0};
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
        ERR("cannot init SDL_net");
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
        if (net.sockets[s].logname) {
            myfree(net.sockets[s].logname);
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

    for (si = 0; si < MAX_SOCKETS; si++) {
        s = &net.sockets[si];
        if (s->open) {
            continue;
        }
    }

    if (!s) {
        ERR("No more sockets are available");
        return (0);
    }

    /*
     * If no address is given, try and grab one from our well known base.
     */
    if (!memcmp(&no_address, &listen_address, sizeof(no_address))) {
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

    for (p = 0; p <= max_port; p++, port++) {

        SDLNet_Write16(port, &listen_address.port);
        port = SDLNet_Read16(&listen_address.port);

        /*
         * Check the port is not in use.
         */
        for (si = 0; si < MAX_SOCKETS; si++) {
            if (!memcmp(&listen_address, &s->ip, sizeof(IPaddress))) {
                break;
            }
        }

        if (si != MAX_SOCKETS) {
            continue;
        }

        char *tmp = iptodynstr(listen_address);
        DBG("Trying to listen on: %s", tmp);

        net.sockets[0].udp_socket = SDLNet_UDP_Open(port);
        if (!net.sockets[0].udp_socket) {
            ERR("SDLNet_UDP_Open %s failed: %s", tmp, SDLNet_GetError());
            myfree(tmp);
            continue;
        }

        net.sockets[0].channel = SDLNet_UDP_Bind(net.sockets[0].udp_socket,
                                                 -1,
                                                 &listen_address);
        if (net.sockets[0].channel < 0) {
            ERR("SDLNet_UDP_Bind %s failed: %s", tmp, SDLNet_GetError());
            myfree(tmp);
            continue;
        }

        net.socklist = SDLNet_AllocSocketSet(MAX_SOCKETS);
        if (!net.socklist) {
            ERR("SDLNet_AllocSocketSet %s failed: %s", tmp, SDLNet_GetError());
            myfree(tmp);
            continue;
        }

        if (SDLNet_UDP_AddSocket(net.socklist, 
                                 net.sockets[0].udp_socket) == -1) {
            ERR("SDLNet_UDP_AddSocket %s failed: %s", tmp, SDLNet_GetError());
            myfree(tmp);
            continue;
        }

        s->logname = tmp;
        s->open = true;
        s->ip = listen_address;
        s->listener = true;

        return (s);
    }

    ERR("Failed to listen");
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

static boolean net_show (tokens_t *tokens, void *context)
{
    const char *prefix = "  %-40s %-6s %-6s";

    CON(prefix, "Host", "Dir", "Type");
    CON(prefix, "----", "---", "---");

    int s;
    for (s = 0; s < MAX_SOCKETS; s++) {
        if (!net.sockets[s].open) {
            CON(prefix, "-", "-");
            continue;
        }

        CON(prefix, net.sockets[s].logname, 
            net.sockets[s].listener ? "listen" :
            net.sockets[s].connect ? "Connect" : "n/a",
            net.sockets[s].server ? "Server" : "Client");
    }

    return (true);
}

