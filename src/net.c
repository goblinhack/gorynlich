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

boolean is_server;
boolean is_client;
IPaddress server_address = {0};
IPaddress client_address = {0};

static boolean net_init_done;
static host hosts[NET_MAX_HOSTS];
static void host_fini(void);

boolean net_init (void)
{
    if (SDLNet_Init() < 0) {
        ERR("cannot init SDL_net");
        return (false);
    }

    net_init_done = true;

    return (true);
}

void net_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (net_init_done) {
        host_fini();
        SDLNet_Quit();
        net_init_done = false;
    }
}

int net_test (int32_t argc, char *argv[])
{
    host_add(server_address, "server", true);
    host_add(server_address, "server", true);
    host_add(client_address, "neil", false);
    host_dump();

    if (is_server) {
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

    if (!(hostname = SDLNet_ResolveIP(&ip))) {
        return (dynprintf("IPv4 %u.%u.%u.%u:%u",
                          hostname, ip1, ip2, ip3, ip4, port));
    } else {
        return (dynprintf("[%s] %u.%u.%u.%u:%u",
                          hostname, ip1, ip2, ip3, ip4, port));
    }
}

const char *host_logname (hostp h)
{
    return (h->logname);
}

hostp host_add (IPaddress ip, const char *name, boolean server)
{
    hostp found = 0;

    FOR_ALL_IN_ARRAY(h, hosts) {
        if (!h->inuse) {
            continue;
        }

        if (!memcmp(&ip, &h->ip, sizeof(ip))) {
            found = h;
            break;
        }
    }

    if (!found) {
        FOR_ALL_IN_ARRAY(h, hosts) {
            if (!h->inuse) {
                found = h;
                break;
            }
        }

        if (!found) {
            char *tmp = iptodynstr(ip);
            ERR("Too many hosts, cannot add %s", tmp);
            myfree(tmp);
            return (false);
        }
    }

    found->inuse = true;
    found->ip = ip;
    found->server = server;

    if (found->name) {
        myfree(found->name);
    }
    found->name = dupstr(name, "host name");

    if (found->logname) {
        myfree(found->logname);
    }
    found->logname = iptodynstr(ip);

    return (found);
}

void host_dump (void)
{
    uint32_t i;

    i = 0;

    LOG("  %-40s %-6s %-10s %s", 
        "Host", 
        "Type",
        "Name", 
        "Delay(ms)");

    LOG("  %-40s %-6s %-10s %s", 
        "----", 
        "----", 
        "----",
        "---------");

    FOR_ALL_IN_ARRAY(h, hosts) {
        if (!h->inuse) {
            continue;
        }

        i++;

        LOG("%u %-40s %-6s %-10s %u", i, 
            h->logname, 
            h->server ? "server" : "client", 
            h->name, 
            h->delay_ms);
    }
}

static void host_fini (void)
{
    FOR_ALL_IN_ARRAY(h, hosts) {
        if (!h->inuse) {
            continue;
        }

        if (h->logname) {
            myfree(h->logname);
        }

        if (h->name) {
            myfree(h->name);
        }

        memset(h, 0, sizeof(*h));
    }
}
