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
IPaddress client_address = {0};
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
    const char *prefix = "  %-40s %-6s";
    LOG(prefix, "Host", "Type");
    LOG(prefix, "----", "----");

    int s;
    for (s = 0; s < MAX_SOCKETS; s++) {
        if (!net.sockets[s].open) {
            LOG(prefix, "-", "-");
            continue;
        }

        LOG(prefix, net.sockets[s].logname, 
            net.sockets[s].server ? "server" : "client");
    }

    return (true);
}
