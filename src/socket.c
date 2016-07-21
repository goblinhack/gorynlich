/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#include <SDL.h>
#include <SDL_net.h>

#include "main.h"
#include "socket_util.h"
#include "string_util.h"
#include "slre.h"
#include "command.h"
#include "time_util.h"
#include "player.h"
#include "wid.h"
#include "wid_button.h"
#include "color.h"
#include "tree.h"
#include "server.h"
#include "client.h"
#include "wid_game_map_server.h"
#include "wid_player_action.h"
#include "thing.h"
#include "mzip_lib.h"
#include "wid_hiscore.h"
#include "level.h"

tree_rootp sockets;

uint8_t debug_socket_ping_enabled = 0;

uint8_t on_server;
uint8_t is_client;
uint8_t is_headless;
uint8_t single_player_mode;

IPaddress server_address = {0};
IPaddress no_address = {0};

static void socket_destroy(gsocketp s);
static uint8_t sockets_show_all(tokens_t *tokens, void *context);
static uint8_t sockets_show_summary(tokens_t *tokens, void *context);
static void socket_tx_queue_flush_all(void);
static void socket_rx_queue_flush_all(void);
static void socket_tx_queue_flush(gsocketp s);
static void socket_rx_queue_flush(gsocketp s);

static uint8_t socket_init_done;

/*
 * Used when the server detects a client.
 */
int send_quick_ping;

uint8_t socket_init (void)
{
    if (socket_init_done) {
        return (true);
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

    socket_init_done = true;

    return (true);
}

void socket_fini (void)
{
    if (!socket_init_done) {
        return;
    }

    socket_tx_queue_flush_all();
    socket_rx_queue_flush_all();

    SDLNet_Quit();

    tree_destroy(&sockets, (tree_destroy_func)socket_destroy);

    socket_init_done = false;
}

static gsocketp socket_create (IPaddress address, int type)
{
    gsocketp s;

    if (!sockets) {
        sockets = tree_alloc(TREE_KEY_THREE_INTEGER, "TREE ROOT: sockets");
    }

    uint16_t port = address.port;
    if (!port) {
        ERR("Specify a local port to listen on");
        return (0);
    }

    /*
     * Create a new socket.
     */
    s = (typeof(s)) myzalloc(sizeof(*s), "TREE NODE: socket");

    s->tree.key2 = type;
    s->tree.key3 = address.host;
    s->tree.key4 = address.port;

    if (!tree_insert(sockets, &s->tree.node)) {
        ERR("failed to add socket");
        return (0);
    }

    if (!single_player_mode) {
        s->socklist = SDLNet_AllocSocketSet(1);
        if (!s->socklist) {
            char *tmp = iptodynstr(address);
            ERR("SDLNet_AllocSocketSet %s failed", tmp);
            WARN("  %s", SDLNet_GetError());
            myfree(tmp);

            socket_disconnect(s);
            return (0);
        }
    }

    return (s);
}

gsocketp socket_listen (IPaddress address)
{
    IPaddress listen_address;

    /*
     * Relisten?
     */
    gsocketp s = socket_find(address, SOCKET_LISTEN);
    if (s) {
        if (s->udp_socket) {
            return (s);
        }
    } else {
        s = socket_create(address, SOCKET_LISTEN);
        if (!s) {
            char *tmp = iptodynstr(address);
            WARN("socket_create %s failed", tmp);
            WARN("  %s", SDLNet_GetError());
            myfree(tmp);
            return (0);
        }
    }

    if (!single_player_mode) {
        /*
         * Use 0.0.0.0 for listening always as it is different from the client
         * connecting on 127.0.0.1
         */
        address.host = 0;

        uint16_t port = SDLNet_Read16(&address.port);
        s->udp_socket = SDLNet_UDP_Open(port);
        if (!s->udp_socket) {
            char *tmp = iptodynstr(address);
            WARN("SDLNet_UDP_Open %s failed", tmp);
            WARN("  %s", SDLNet_GetError());
            myfree(tmp);

            socket_disconnect(s);
            return (0);
        }

        s->channel = SDLNet_UDP_Bind(s->udp_socket, -1, &listen_address);
        if (s->channel < 0) {
            char *tmp = iptodynstr(address);
            WARN("SDLNet_UDP_Bind %s failed", tmp);
            WARN("  %s", SDLNet_GetError());
            myfree(tmp);

            socket_disconnect(s);
            return (0);
        }

        if (SDLNet_UDP_AddSocket(s->socklist, s->udp_socket) == -1) {
            char *tmp = iptodynstr(address);
            WARN("SDLNet_UDP_AddSocket %s failed", tmp);
            WARN("  %s", SDLNet_GetError());
            myfree(tmp);

            socket_disconnect(s);
            return (0);
        }
    }

    s->local_ip = address;
    s->server = true;

    return (s);
}

gsocketp socket_find (IPaddress address, int type)
{
    gsocket findme;
    gsocketp s;

    memset(&findme, 0, sizeof(findme));
    findme.tree.key2 = type;
    findme.tree.key3 = address.host;
    findme.tree.key4 = address.port;

    s = (typeof(s)) tree_find(sockets, &findme.tree.node);

    return (s);
}

static gsocketp socket_connect (IPaddress address, uint8_t server_side_client)
{
    IPaddress connect_address = address;

    /*
     * Reopen?
     */
    gsocketp s = socket_find(address, SOCKET_CONNECT + server_side_client);
    if (s) {
        return (s);
    }

    s = socket_create(address, SOCKET_CONNECT + server_side_client);
    if (!s) {
        return (0);
    }

    if (!single_player_mode) {
        s->udp_socket = SDLNet_UDP_Open(0);
        if (!s->udp_socket) {
            char *tmp = iptodynstr(connect_address);
            WARN("SDLNet_UDP_Open %s failed", tmp);
            WARN("  %s", SDLNet_GetError());
            myfree(tmp);

            socket_disconnect(s);
            return (0);
        }

        s->channel = SDLNet_UDP_Bind(s->udp_socket, -1, &connect_address);
        if (s->channel < 0) {
            char *tmp = iptodynstr(connect_address);
            WARN("SDLNet_UDP_Bind %s failed", tmp);
            WARN("  %s", SDLNet_GetError());
            myfree(tmp);

            socket_disconnect(s);
            return (0);
        }

        if (SDLNet_UDP_AddSocket(s->socklist, s->udp_socket) == -1) {
            char *tmp = iptodynstr(connect_address);
            WARN("SDLNet_UDP_AddSocket %s failed", tmp);
            WARN("  %s", SDLNet_GetError());
            myfree(tmp);

            socket_disconnect(s);
            return (0);
        }
    }

    s->server_side_client = server_side_client;

    if (!server_side_client) {
        s->client = true;
    }

    s->remote_ip = connect_address;

    if (!single_player_mode) {
        s->local_ip = *SDLNet_UDP_GetPeerAddress(s->udp_socket, -1);
    }

    LOG("Socket connect to %s", socket_get_remote_logname(s));
    LOG("       from       %s", socket_get_local_logname(s));

    return (s);
}

gsocketp socket_connect_from_client (IPaddress address)
{
    gsocketp s;

    s = socket_connect(address, false);

    return (s);
}

gsocketp socket_connect_from_server (IPaddress address)
{
    gsocketp s;

    s = socket_connect(address, true);

    return (s);
}

static void socket_destroy (gsocketp s)
{
    verify(s);

    socket_set_connected(s, false);

    if (s->server) {
        LOG("Socket destroy listen %s", socket_get_local_logname(s));
    } else {
        LOG("Socket destroy to %s", socket_get_remote_logname(s));
        LOG("               from %s", socket_get_local_logname(s));
    }

    socket_tx_queue_flush(s);
    socket_rx_queue_flush(s);

    if (s->socklist) {
        SDLNet_FreeSocketSet(s->socklist);
        s->socklist = 0;
    }

    if (s->udp_socket) {
        SDLNet_UDP_Unbind(s->udp_socket, s->channel);

        SDLNet_UDP_Close(s->udp_socket);

        s->udp_socket = 0;
    }

    if (s->local_logname) {
        myfree((char *)s->local_logname);
    }

    if (s->remote_logname) {
        myfree((char *)s->remote_logname);
    }

    socket_set_player(s, 0);

    if (single_player_mode) {
        socket_set_connected(s, false);
    }

    if (s == server_socket) {
        server_socket = 0;
    }
}

void socket_disconnect (gsocketp s)
{
    verify(s);

    tree_remove_found_node(sockets, &s->tree.node);

    socket_destroy(s);

    myfree(s);
}

/*
 * User has entered a command, run it
 */
uint8_t debug_socket_ping_enable (tokens_t *tokens, void *context)
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

gsocketp socket_find_local_ip (IPaddress address)
{
    gsocketp s;
    TREE_WALK(sockets, s) {
        if (cmp_address(&address, &s->local_ip)) {
            return (s);
        }
    }

    return (0);
}

gsocketp socket_find_remote_ip (IPaddress address)
{
    gsocketp s;
    TREE_WALK(sockets, s) {
        if (cmp_address(&address, &s->remote_ip)) {
            return (s);
        }
    }

    return (0);
}

gsocketp socket_find_server_side_remote_ip (IPaddress address)
{
    gsocketp s;
    TREE_WALK(sockets, s) {
        if (!s->server_side_client) {
            continue;
        }

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

    if ((ip1 == 0) && (ip2 == 0) && (ip3 == 0) && (ip4 == 0)) {
        return (dynprintf("0.0.0.0:%u", port));
    }

    LOG("Resolve %d.%d.%d.%d",ip1,ip2,ip3,ip4);

    if (!(hostname = SDLNet_ResolveIP(&ip))) {
        LOG("Resolved %d.%d.%d.%d, no host",ip1,ip2,ip3,ip4);

        return (dynprintf("IPv4 %u.%u.%u.%u:%u",
                          ip1, ip2, ip3, ip4, port));

    }

    LOG("Resolved %d.%d.%d.%d as %s",ip1,ip2,ip3,ip4, hostname);

    if (strstr(hostname, "localhost")) {
        hostname = "lhost";
    }

    if ((ip1 == 0) && (ip2 == 0) && (ip3 == 0) && (ip4 == 0)) {
        return (dynprintf("%s:%u",
                           hostname && *hostname ? hostname : "",
                           port));
    }

    if ((ip1 == 127) && (ip2 == 0) && (ip3 == 0) && (ip4 == 1)) {
        return (dynprintf("%s:%u",
                           hostname && *hostname ? hostname : "",
                           port));
    }

    if (is_ip_address(hostname)) {
        return (dynprintf("%s:%u", hostname, port));
    } else {
        return (dynprintf("%s%s%u.%u.%u.%u:%u",
                        hostname && *hostname ? hostname : "",
                        hostname && *hostname ? "," : "",
                        ip1, ip2, ip3, ip4, port));
    }
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

static uint8_t sockets_show_all (tokens_t *tokens, void *context)
{
    uint32_t si = 0;

    gsocketp s;
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

        if (s->tx_msg[MSG_PING] || s->rx_msg[MSG_PING]) {
            CON("  Ping           : tx %u, rx %u",
                s->tx_msg[MSG_PING], 
                s->rx_msg[MSG_PING]);
        }

        if (s->tx_msg[MSG_PONG] || s->rx_msg[MSG_PONG]) {
            CON("  Pong           : tx %u, rx %u",
                s->tx_msg[MSG_PONG], 
                s->rx_msg[MSG_PONG]);
        }

        if (s->tx_msg[MSG_TELL] || s->rx_msg[MSG_TELL]) {
            CON("  Tell           : tx %u, rx %u",
                s->tx_msg[MSG_TELL], 
                s->rx_msg[MSG_TELL]);
        }

        if (s->tx_msg[MSG_CLIENT_JOIN] || s->rx_msg[MSG_CLIENT_JOIN]) {
            CON("  Join           : tx %u, rx %u",
                s->tx_msg[MSG_CLIENT_JOIN], 
                s->rx_msg[MSG_CLIENT_JOIN]);
        }

        if (s->tx_msg[MSG_CLIENT_LEAVE] || s->rx_msg[MSG_CLIENT_LEAVE]) {
            CON("  Client Leave   : tx %u, rx %u",
                s->tx_msg[MSG_CLIENT_LEAVE], 
                s->rx_msg[MSG_CLIENT_LEAVE]);
        }

        if (s->tx_msg[MSG_CLIENT_CLOSE] || s->rx_msg[MSG_CLIENT_CLOSE]) {
            CON("  Client Close   : tx %u, rx %u",
                s->tx_msg[MSG_CLIENT_CLOSE], 
                s->rx_msg[MSG_CLIENT_CLOSE]);
        }

        if (s->tx_msg[MSG_SERVER_SHOUT] || s->rx_msg[MSG_SERVER_SHOUT]) {
            CON("  Server Shout   : tx %u, rx %u",
                s->tx_msg[MSG_SERVER_SHOUT], 
                s->rx_msg[MSG_SERVER_SHOUT]);
        }

        if (s->tx_msg[MSG_CLIENT_STATUS] || s->rx_msg[MSG_CLIENT_STATUS]) {
            CON("  Client status  : tx %u, rx %u",
                s->tx_msg[MSG_CLIENT_STATUS], 
                s->rx_msg[MSG_CLIENT_STATUS]);
        }

        if (s->tx_msg[MSG_SERVER_STATUS] || s->rx_msg[MSG_SERVER_STATUS]) {
            CON("  Server Status  : tx %u, rx %u",
                s->tx_msg[MSG_SERVER_STATUS], 
                s->rx_msg[MSG_SERVER_STATUS]);
        }

        if (s->tx_msg[MSG_SERVER_CLOSE] || s->rx_msg[MSG_SERVER_CLOSE]) {
            CON("  Server Close   : tx %u, rx %u",
                s->tx_msg[MSG_SERVER_CLOSE], 
                s->rx_msg[MSG_SERVER_CLOSE]);
        }

        if (s->tx_msg[MSG_SERVER_MAP_UPDATE] || 
            s->rx_msg[MSG_SERVER_MAP_UPDATE]) {
            CON("  Server map upd : tx %u, rx %u",
                s->tx_msg[MSG_SERVER_MAP_UPDATE], 
                s->rx_msg[MSG_SERVER_MAP_UPDATE]);
        }

        if (s->tx_msg[MSG_CLIENT_PLAYER_MOVE] || 
            s->rx_msg[MSG_CLIENT_PLAYER_MOVE]) {
            CON("  Client move    : tx %u, rx %u",
                s->tx_msg[MSG_CLIENT_PLAYER_MOVE], 
                s->rx_msg[MSG_CLIENT_PLAYER_MOVE]);
        }

        if (s->tx_msg[MSG_CLIENT_PLAYER_ACTION] || 
            s->rx_msg[MSG_CLIENT_PLAYER_ACTION]) {
            CON("  Client action  : tx %u, rx %u",
                s->tx_msg[MSG_CLIENT_PLAYER_ACTION], 
                s->rx_msg[MSG_CLIENT_PLAYER_ACTION]);
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
        uint32_t avg_latency_rtt = 0;
        uint32_t max_latency_rtt = 0;
        uint32_t min_latency_rtt = (uint32_t) -1;

        FOR_ALL_IN_ARRAY(latency, s->latency_rtt) {
            if (*latency == (typeof(*latency)) -1) {
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
            avg_latency_rtt += *latency;

            if (*latency > max_latency_rtt) {
                max_latency_rtt = *latency;
            }

            if (*latency < min_latency_rtt) {
                min_latency_rtt = *latency;
            }
        }

        total_attempts = no_response + response;

        if (total_attempts) {
            avg_latency_rtt /= total_attempts;

            CON("  Quality  : success %2.2f percent, fails %2.2f percent",
                ((float)((float)response / (float)total_attempts)) * 100.0,
                ((float)((float)no_response / (float)total_attempts)) * 100.0);

            CON("  Latency  : max %u ms, min %u ms, average %u ms",
                max_latency_rtt, min_latency_rtt, avg_latency_rtt);
        }
    }

    return (true);
}

static uint8_t sockets_show_summary (tokens_t *tokens, void *context)
{
    int si = 0;

    CON("Name                 Quality  Latency       Local IP            Remote IP");
    CON("----                 -------  ------- -------------------- ------------------");
        
    gsocketp s;
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
        uint32_t avg_latency_rtt = 0;
        uint32_t max_latency_rtt = 0;
        uint32_t min_latency_rtt = (uint32_t) -1;

        FOR_ALL_IN_ARRAY(latency, s->latency_rtt) {
            if (*latency == (typeof(*latency)) -1) {
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
            avg_latency_rtt += *latency;

            if (*latency > max_latency_rtt) {
                max_latency_rtt = *latency;
            }

            if (*latency < min_latency_rtt) {
                min_latency_rtt = *latency;
            }
        }

        total_attempts = no_response + response;

        const char *name = socket_get_name(s);
        const char *sock;

        if (client_joined_server && (s == client_joined_server)) {
            sock = "joined client";
        } else if (server_socket && (s == server_socket)) {
            sock = "local server";
        } else if (socket_get_server(s)) {
            sock = "other server";
        } else if (socket_get_server_side_client(s)) {
            sock = "server side client";
        } else if (socket_get_client(s)) {
            sock = "client";
        } else {
            sock = "none";
        }

        if (!name) {
            name = "noname";
        }

        if (total_attempts) {
            avg_latency_rtt /= total_attempts;

            if (*name) {
                CON("%-20s %3.0f pct %5d ms %-20s %-20s", 
                    name,
                    ((float)((float)response / (float)total_attempts)) * 100.0,
                    avg_latency_rtt,
                    socket_get_local_logname(s),
                    socket_get_remote_logname(s));
                CON("%-20s", sock);
            } else {
                CON("%-20s %3.0f pct %5d ms %-20s %-20s", 
                    sock,
                    ((float)((float)response / (float)total_attempts)) * 100.0,
                    avg_latency_rtt,
                    socket_get_local_logname(s),
                    socket_get_remote_logname(s));
            }
        } else {
            if (*name) {
                CON("%-20s                  %-20s %-20s", 
                    name,
                    socket_get_local_logname(s),
                    socket_get_remote_logname(s));
            } else {
                CON("%-20s                  %-20s %-20s", 
                    sock,
                    socket_get_local_logname(s),
                    socket_get_remote_logname(s));
            }
        }
    }

    return (true);
}

void sockets_quality_check (void)
{
    /*
     * If called by both server and client, make sure we only run once per 
     * sec.
     */
    static uint32_t ts;

    if (!time_have_x_tenths_passed_since(DELAY_TENTHS_SOCKET_QUALITY_CHECK, ts)) {
        return;
    }

    ts = time_get_time_ms();

    gsocketp s;

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
        uint32_t avg_latency_rtt = 0;
        uint32_t max_latency_rtt = 0;
        uint32_t min_latency_rtt = (uint32_t) -1;

        FOR_ALL_IN_ARRAY(latency, s->latency_rtt) {
            if (*latency == (typeof(*latency)) -1) {
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
            avg_latency_rtt += *latency;

            if (*latency > max_latency_rtt) {
                max_latency_rtt = *latency;
            }

            if (*latency < min_latency_rtt) {
                min_latency_rtt = *latency;
            }
        }

        total_attempts = no_response + response;

        if (total_attempts) {
            s->quality = 
                ((float)((float)response / (float)total_attempts)) * 100.0,

            avg_latency_rtt /= total_attempts;
            s->avg_latency_rtt = avg_latency_rtt;
            s->min_latency_rtt = min_latency_rtt;
            s->max_latency_rtt = max_latency_rtt;
        }
    }
}

void sockets_alive_check (void)
{
    sockets_quality_check();

    gsocketp s;

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
            CON("%p: Peer down [%s] qual %u percent",
                s, socket_get_remote_logname(s), s->quality);

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

IPaddress socket_get_local_ip (const gsocketp s)
{
    verify(s);

    return (s->local_ip);
}

IPaddress socket_get_remote_ip (const gsocketp s)
{
    verify(s);

    return (s->remote_ip);
}

const char *socket_get_name (const gsocketp s)
{
    verify(s);

    return (s->stats.pname);
}

const char *socket_get_pclass (const gsocketp s)
{
    verify(s);

    return (s->stats.pclass);
}

const thing_statsp socket_get_player_stats (const gsocketp s)
{
    verify(s);

    return (&s->stats);
}

void socket_set_name (gsocketp s, const char *name)
{
    verify(s);

    if (!name) {
        memset(s->stats.pname, 0, sizeof(s->stats.pname));
    } else {
        strlcpy(s->stats.pname, name, sizeof(s->stats.pname) - 1);
    }
}

void socket_set_pclass (gsocketp s, const char *pclass)
{
    verify(s);

    if (!pclass) {
        memset(s->stats.pclass, 0, sizeof(s->stats.pclass));
    } else {
        strlcpy(s->stats.pclass, pclass, sizeof(s->stats.pclass) - 1);
    }
}

/*
 * true on changed
 */
int socket_set_player_stats (gsocketp s, const thing_statsp stats)
{
    verify(s);

    if (!stats) {
        memset(&s->stats, 0, sizeof(thing_stats));
        return (true);
    }

    if (!thing_stats_diff(&s->stats, stats)) {
        /*
         * No change
         */
        return (false);
    }

    memcpy(&s->stats, stats, sizeof(thing_stats));
    return (true);
}

const char * socket_get_local_logname (const gsocketp s)
{
    verify(s);

    if (!s->local_logname) {
        s->local_logname = iptodynstr(s->local_ip);
    }

    return (s->local_logname);
}

const char * socket_get_remote_logname (const gsocketp s)
{
    verify(s);

    if (!s->remote_logname) {
        s->remote_logname = iptodynstr(s->remote_ip);
    }

    return (s->remote_logname);
}

uint8_t socket_get_server (const gsocketp s)
{
    verify(s);

    return (s->server);
}

uint8_t socket_get_client (const gsocketp s)
{
    verify(s);

    return (s->client);
}

uint8_t socket_get_server_side_client (const gsocketp s)
{
    verify(s);

    return (s->server_side_client);
}

msg_server_status *socket_get_server_status (const gsocketp s)
{
    verify(s);

    return (&s->server_status);
}

void socket_set_channel (gsocketp s, int c)
{
    verify(s);

    s->channel = c;
}

uint8_t socket_get_channel (const gsocketp s)
{
    verify(s);

    return (s->channel);
}

void socket_set_connected (gsocketp s, uint8_t c)
{
    verify(s);

    if (c == s->connected) {
        return;
    }

    if (socket_get_server(s)) {
        if (c) {
            LOG("%p: SERVER PRESENT %s", s, socket_get_remote_logname(s));
        } else {
            LOG("%p: SERVER DISCONNECT from %s", s, socket_get_remote_logname(s));
        }
    } else if (socket_get_client(s)) {
        if (c) {
            LOG("%p: SERVER PRESENT %s", s, socket_get_remote_logname(s));
        } else {
            LOG("%p: SERVER DISCONNECT from %s", s, socket_get_remote_logname(s));
        }
    } else {
        if (c) {
            LOG("%p: CLIENT PRESENT %s", s, socket_get_remote_logname(s));

            send_quick_ping = 1;
        } else {
            LOG("%p: CLIENT DISCONNECT from %s", s, socket_get_remote_logname(s));
        }
    }

    s->connected = c;

    /*
     * For aliveness checks so they see this as a new connection.
     */
    s->tx = 0;
}

uint8_t socket_get_connected (const gsocketp s)
{
    verify(s);

    return (s->connected);
}

UDPsocket socket_get_udp_socket (const gsocketp s)
{
    verify(s);

    return (s->udp_socket);
}

SDLNet_SocketSet socket_get_socklist (const gsocketp s)
{
    verify(s);

    return (s->socklist);
}

aplayerp socket_get_player (const gsocketp s)
{
    verify(s);

    return (s->player);
}

void socket_set_player (const gsocketp s, aplayer *p)
{
    thingp t = 0;

    verify(s);

    if (s->player) {
        verify(s->player);

        t = s->player->thing;
        if (t) {
            verify(t);
        }
    }

    if (!p) {
        if (t) {
            t->player = 0;
        }

        if (s->player) {
            s->player->thing = 0;
            s->player->socket = 0;
            myfree(s->player);
            s->player = 0;
        }

        return;
    }

    s->player = p;
    verify(p);

    p->socket = s;
}

void socket_count_inc_pak_rx (const gsocketp s, msg_type type)
{
    verify(s);

    if (type < MSG_MAX) {
        s->rx++;
        s->rx_msg[type]++;
    } else {
        socket_count_inc_pak_rx_bad_msg(s);
    }
}

void socket_count_inc_pak_tx (const gsocketp s)
{
    verify(s);

    s->tx++;
}

static void socket_count_inc_pak_rx_error (const gsocketp s, UDPpacket *packet)
{
    verify(s);

    s->rx_error++;

    char *tmp = iptodynstr(read_address(packet));
    LOG("Bad socket message from %s", tmp);
    myfree(tmp);
}

void socket_count_inc_pak_tx_error (const gsocketp s)
{
    verify(s);

    s->tx_error++;
}

void socket_count_inc_pak_rx_bad_msg (const gsocketp s)
{
    verify(s);

    s->rx_bad_msg++;
}

UDPpacket *packet_alloc (void)
{
    UDPpacket *packet;
    
    packet = SDLNet_AllocPacket(MAX_PACKET_SIZE);
    if (!packet) {
        ERR("Out of packet space, pak %u", MAX_PACKET_SIZE);
    }

    newptr(packet, "pak");

    return (packet);
}

UDPpacket *packet_dup (const UDPpacket *packet)
{
    UDPpacket *dup;

    verify(packet);

    dup = SDLNet_AllocPacket(MAX_PACKET_SIZE);
    if (!dup) {
        ERR("Out of packet space, len %d", packet->len);
    }

    newptr(dup, "pak dup");

    dup->len = packet->len;
    memcpy(dup->data, packet->data, packet->len);
    memcpy(&dup->address, &packet->address, sizeof(packet->address));
    dup->channel = packet->channel;

    return (dup);
}

static UDPpacket *packet_dup_no_copy (const UDPpacket *packet,
                                      const int adjustment)
{
    UDPpacket *dup;

    verify(packet);

    dup = SDLNet_AllocPacket(MAX_PACKET_SIZE);
    if (!dup) {
        ERR("Out of packet space, len %d", packet->len);
    }

    newptr(dup, "pak dup");

    memcpy(&dup->address, &packet->address, sizeof(packet->address));
    dup->channel = packet->channel;

    return (dup);
}

void packet_free (UDPpacket *packet)
{
    oldptr(packet);

    SDLNet_FreePacket(packet);
}

void socket_tx_ping (gsocketp s, uint8_t *seq, uint32_t ts)
{
    if (single_player_mode) {
        socket_set_connected(s, true);
        return;
    }

    verify(s);

    if (!socket_get_udp_socket(s)) {
        return;
    }

    if (!s->rx) {
        if (!time_have_x_tenths_passed_since(
                    DELAY_TENTHS_PING_WHEN_NO_RESPONSE, s->tx_last_ping)) {
            return;
        }
    } else {
        if (s->rx < 20) {
            /*
             * Faster pings to get going with joining the level.
             */
            if (!time_have_x_tenths_passed_since(
                        DELAY_TENTHS_PING / 4, s->tx_last_ping)) {
                return;
            }
        } else {
            if (!time_have_x_tenths_passed_since(
                        DELAY_TENTHS_PING, s->tx_last_ping)) {
                return;
            }
        }
    }

    UDPpacket *packet = packet_alloc();

    uint8_t *data = packet->data;
    uint8_t *odata = data;

    *data++ = MSG_PING;
    *data++ = *seq;

    SDLNet_Write32(ts, data);               
    data += sizeof(uint32_t);

    s->latency_rtt[*seq % ARRAY_SIZE(s->latency_rtt)] = (uint32_t) -1;

    if (debug_socket_ping_enabled) {
        CON("%p: Tx Ping to %s seq %u, ts %u", 
            s, socket_get_remote_logname(s), *seq, ts);
    }

    s->tx_last_ping = time_get_time_ms();
    (*seq)++;

    packet->len = (int32_t)(data - odata);

    write_address(packet, socket_get_remote_ip(s));

    socket_tx_enqueue(s, &packet);
}

void socket_tx_pong (gsocketp s, uint8_t seq, uint32_t ts)
{
    verify(s);

    if (!single_player_mode) {
        return;
    }

    if (!socket_get_udp_socket(s)) {
        return;
    }

    msg_pong msg = {0};
    msg.type = MSG_PONG;
    msg.seq = seq;
    msg.ts = ts;

    if (global_config.user_server_name[0]) {
        strlcpy(msg.server_name, global_config.user_server_name,
                min(sizeof(msg.server_name), 
                    strlen(global_config.user_server_name))); 
    } else {
        strlcpy(msg.server_name, global_config.server_name,
                min(sizeof(msg.server_name), 
                    strlen(global_config.server_name))); 
    }

    /*
     * Add current player names.
     */
    uint32_t p = 0;
    thingp t = 0;

    TREE_OFFSET_WALK_UNSAFE(server_player_things, t) {
        if (p >= MAX_PLAYERS) {
            break;
        }

        strlcpy(msg.player_name[p], t->stats.pname,
                min(sizeof(msg.player_name[p]), strlen(t->stats.pname) + 1)); 
        p++;
    }

    if (server_level) {
        const char *title = level_get_title(server_level);
        if (title) {
            strlcpy(msg.level_name, title,
                    min(sizeof(msg.level_name), strlen(title) + 1)); 
        }
    }

    msg.server_max_players = global_config.server_max_players;
    msg.server_current_players = global_config.server_current_players;

    if (debug_socket_ping_enabled) {
        CON("%p: Tx Pong to %s seq %u, ts %u", 
            s, socket_get_remote_logname(s), seq, ts);
    }

    UDPpacket *packet = packet_alloc();

    memcpy(packet->data, &msg, sizeof(msg));

    packet->len = sizeof(msg);

    write_address(packet, socket_get_remote_ip(s));

    socket_tx_enqueue(s, &packet);
}

void socket_rx_ping (gsocketp s, UDPpacket *packet, uint8_t *data)
{
    verify(s);

    uint8_t seq = *data++;
    uint32_t ts = SDLNet_Read32(data);

    if (debug_socket_ping_enabled) {
        char *tmp = iptodynstr(read_address(packet));
        CON("%p: Rx Ping from %s, seq %u", s, tmp, seq);
        myfree(tmp);
    }

    socket_tx_pong(s, seq, ts);

    socket_set_connected(s, true);
}

void socket_rx_pong (gsocketp s, UDPpacket *packet, uint8_t *data)
{
    verify(s);

    msg_pong *msg;

    if (packet->len != sizeof(*msg)) {
        socket_count_inc_pak_rx_error(s, packet);
        return;
    }

    msg = (typeof(msg)) packet->data;

    uint8_t seq = msg->seq;
    uint32_t ts = msg->ts;

    if (debug_socket_ping_enabled) {
        char *tmp = iptodynstr(read_address(packet));
        CON("%p: Rx Pong from %s, seq %u, elapsed %d ms", s, tmp, seq,
            time_get_time_ms() - ts);
        myfree(tmp);
    }

    s->latency_rtt[seq % ARRAY_SIZE(s->latency_rtt)] = 
                    time_get_time_ms() - ts;

    strlcpy(s->server_name, msg->server_name,
            min(sizeof(s->server_name), 
                strlen(msg->server_name) + 1)); 

    uint32_t p;

    for (p = 0; p < MAX_PLAYERS; p++) {
        strlcpy(s->player_name[p], msg->player_name[p],
                min(sizeof(s->player_name[p]), 
                    strlen(msg->player_name[p]) + 1)); 
    }

    if (client_level) {
        level_set_title(client_level, msg->level_name);
    }

    s->server_max_players = msg->server_max_players;
    s->server_current_players = msg->server_current_players;
}

void socket_tx_client_status (gsocketp s)
{
    verify(s);

    /*
     * Refresh the server with our name.
     */
    if (!socket_get_client(s)) {
        return;
    }

    if (!single_player_mode) {
        if (!s->connected) {
            return;
        }
    }

    UDPpacket *packet = packet_alloc();

    msg_client_status msg = {0};
    msg.type = MSG_CLIENT_STATUS;

    memcpy(&msg.stats, &s->stats, sizeof(thing_stats));
    memcpy(packet->data, &msg, sizeof(msg));

    if (!single_player_mode) {
        LOG("Client: Tx Client Status ver %d to %s \"%s\"", 
            s->stats.client_version,
            socket_get_remote_logname(s), s->stats.pname);
        thing_stats_dump(&s->stats);
    }

    packet->len = sizeof(msg);

    write_address(packet, socket_get_remote_ip(s));

    socket_tx_enqueue(s, &packet);
}

void socket_rx_client_status (gsocketp s, UDPpacket *packet, uint8_t *data)
{
    verify(s);

    msg_client_status msg = {0};

    if (packet->len != sizeof(msg)) {
        socket_count_inc_pak_rx_error(s, packet);
        return;
    }

    memcpy(&msg, packet->data, sizeof(msg));

    char *tmp = iptodynstr(read_address(packet));
    LOG("Server: Rx Client Status from %s \"%s\"", tmp, msg.stats.pname);
    myfree(tmp);

    socket_set_player_stats(s, &msg.stats);

    /* 
     * Update the player structure.
     */
    aplayer *p = s->player;
    if (!p) {
        p = (typeof(p)) myzalloc(sizeof(*p), "player");
    }

    /*
     * Get the new stats from the client and the old stats and then compare 
     * them so we can see what changed on the client.
     */
    thing_stats new_stats_from_client;
    memcpy(&new_stats_from_client, &msg.stats, 
           sizeof(new_stats_from_client));

    thing_stats old_stats_from_client;
    memcpy(&old_stats_from_client, &p->stats_from_client, 
           sizeof(old_stats_from_client));

    /*
     * Save the client stats so we can compare again later.
     */
    memcpy(&p->stats_from_client, &new_stats_from_client, 
           sizeof(new_stats_from_client));

// LOG("server, rx version %d",new_stats.client_version);

    /*
     * Merge them together.
     */
    thing_stats server_stats;

    if (p->thing) {
        memcpy(&server_stats, &p->thing->stats, sizeof(server_stats));
    } else {
        memcpy(&server_stats, &old_stats_from_client, sizeof(old_stats_from_client));
    }

    int changed = thing_stats_merge(&server_stats, &old_stats_from_client, &new_stats_from_client);

    if (p->thing) {
        memcpy(&p->thing->stats, &server_stats, sizeof(server_stats));
    }

    /*
     * We keep stats on the socket in case the player gets disconnected.
     */
    socket_set_player(s, p);

    if (changed) {
        /*
         * Update the client that we have merged their stat change in.
         */
        socket_tx_server_status(s);
    }

    /*
     * See if abilities have been unlocked.
     */
    if (p->thing) {
        thing_stats_check_for_changes(p->thing);
    }

    p->local_ip = s->local_ip;
    p->remote_ip = s->remote_ip;
}

uint8_t socket_tx_client_join (gsocketp s, uint32_t *key)
{
    verify(s);

    /*
     * Refresh the server with our name.
     */
    if (!socket_get_client(s)) {
        WARN("not a client, cannot join");
        return (false);
    }

    /*
     * Complain if it takes too long to connect and looks like an issue.
     */
    static int failed = 0;

    if (!s->connected) {
        if (failed++ > 10) {
            WARN("Client: Server %s is not present, cannot join yet",
                 socket_get_remote_logname(s));
        }
        return (false);
    }

    failed = 0;

    UDPpacket *packet = packet_alloc();

    msg_client_join msg = {0};
    msg.type = MSG_CLIENT_JOIN;

    *key = time_get_time_ms();
    SDLNet_Write32(*key, &msg.key);

    memcpy(&msg.stats, &s->stats, sizeof(thing_stats));

    memcpy(packet->data, &msg, sizeof(msg));

    LOG("Client: Tx Join to %s \"%s\"", 
        socket_get_remote_logname(s), s->stats.pname);

    packet->len = sizeof(msg);

    write_address(packet, socket_get_remote_ip(s));

    socket_tx_enqueue(s, &packet);

    return (true);
}

uint8_t socket_rx_client_join (gsocketp s, UDPpacket *packet, uint8_t *data)
{
    verify(s);

    msg_client_join msg = {0};

    if (packet->len != sizeof(msg)) {
        socket_count_inc_pak_rx_error(s, packet);
        return (false);
    }

    memcpy(&msg, packet->data, sizeof(msg));

    /*
     * Check for player limits.
     */
    if (global_config.server_current_players + 1 > 
            global_config.server_max_players) {

        char *tmp = iptodynstr(read_address(packet));
        LOG("Server: Rx Join (rejected) from %s \"%s\"", tmp, msg.stats.pname);
        LOG("  current players %u", global_config.server_current_players);
        LOG("  max     players %u", global_config.server_max_players);
        myfree(tmp);

        socket_tx_tell(s, "Server god", msg.stats.pname, 
                       "Join rejected, too many players");
        return (false);
    }

    tpp tp = tp_find(msg.stats.pclass);
    if (!tp) {
        tp = tp_find_short_name(msg.stats.pclass);
        if (!tp) {
            char *tmp = iptodynstr(read_address(packet));
            LOG("Server: Rx Join (rejected) from %s \"%s\" unknown class %s", 
                tmp, msg.stats.pname, msg.stats.pclass);
            myfree(tmp);

            socket_tx_tell(s, "Server god", msg.stats.pname,
                           "Join rejected, unknown player class");
            return (false);
        }
    }

    char *tmp = iptodynstr(read_address(packet));
    LOG("Server: Rx Join from %s \"%s\"", tmp, msg.stats.pname);
    myfree(tmp);

    socket_set_player_stats(s, &msg.stats);

    /*
     * Update the player structure.
     */
    aplayer *p = s->player;
    if (!p) {
        p = (typeof(p)) myzalloc(sizeof(*p), "player");

        socket_set_player(s, p);
    }

    memcpy(&p->stats_from_client, &msg.stats, sizeof(thing_stats));

    p->local_ip = s->local_ip;
    p->remote_ip = s->remote_ip;
    p->key = SDLNet_Read32(&msg.key);

    if (!global_config.server_current_players) {
        global_config.server_level_pos = p->stats_from_client.level_pos;

        LOG("Server: First player, start at level %d.%d",
            global_config.server_level_pos.y,
            global_config.server_level_pos.x);
    }

    if (!global_config.server_level_pos.x && 
        !global_config.server_level_pos.y) {

        global_config.server_level_pos.x = (myrand() % 10) + 1;
        global_config.server_level_pos.y = 1;

        LOG("Server: No level set, start at level %d.%d",
            global_config.server_level_pos.y,
            global_config.server_level_pos.x);
    }

    wid_game_map_server_visible();

    /*
     * Pop the player off of the map.
     */
    widp w = 
        wid_game_map_server_replace_tile(wid_game_map_server_grid_container,
                                         0, 0, 
                                         0, /* thing */
                                         tp,
                                         0 /* tpp_data */,
                                         0 /* item */,
                                         &msg.stats);
    if (!w) {
        ERR("no player");
    }

    thingp t = wid_get_thing(w);
    verify(t);

    p->thing = t;
    t->player = p;

    return (true);
}

void socket_tx_client_leave (gsocketp s)
{
    verify(s);

    /*
     * Refresh the server with our name.
     */
    if (!socket_get_client(s)) {
        return;
    }

    if (!s->connected) {
        return;
    }

    UDPpacket *packet = packet_alloc();

    msg_client_leave msg = {0};
    msg.type = MSG_CLIENT_LEAVE;

    memcpy(packet->data, &msg, sizeof(msg));

    LOG("Client: Tx leave to %s \"%s\"", 
        socket_get_remote_logname(s), s->stats.pname);

    packet->len = sizeof(msg);
    write_address(packet, socket_get_remote_ip(s));

    socket_set_name(s, 0);
    socket_set_pclass(s, 0);
    socket_set_player_stats(s, 0);

    socket_tx_enqueue(s, &packet);
}

uint8_t socket_rx_client_leave (gsocketp s, UDPpacket *packet, uint8_t *data)
{
    verify(s);

    msg_client_leave msg = {0};

    if (packet->len != sizeof(msg)) {
        socket_count_inc_pak_rx_error(s, packet);
        return (false);
    }

    memcpy(&msg, packet->data, sizeof(msg));

    /*
     * Check for player limits.
     */
    if (!s->player) {
        char *tmp = iptodynstr(read_address(packet));
        LOG("Server: Rx bad leave from %s", tmp);
        myfree(tmp);

        socket_tx_tell(s, "Server", "Unknown player", 
                       "Join rejected, not in game");
        return (false);
    }

    char *tmp = iptodynstr(read_address(packet));
    LOG("Server: Rx leave from %s", tmp);
    myfree(tmp);

    return (true);
}

void socket_tx_client_close (gsocketp s)
{
    verify(s);

    /*
     * Refresh the server with our name.
     */
    if (!socket_get_client(s)) {
        return;
    }

    if (!s->connected) {
        return;
    }

    UDPpacket *packet = packet_alloc();

    msg_client_close msg = {0};
    msg.type = MSG_CLIENT_CLOSE;

    memcpy(packet->data, &msg, sizeof(msg));

    LOG("Client: Tx Close to %s", socket_get_remote_logname(s));

    packet->len = sizeof(msg);

    write_address(packet, socket_get_remote_ip(s));

    socket_tx_enqueue(s, &packet);
}

void socket_rx_client_close (gsocketp s, UDPpacket *packet, uint8_t *data)
{
    verify(s);

    msg_client_close msg = {0};

    if (packet->len != sizeof(msg)) {
        socket_count_inc_pak_rx_error(s, packet);
        return;
    }

    memcpy(&msg, packet->data, sizeof(msg));

    char *tmp = iptodynstr(read_address(packet));
    LOG("Server: Rx Close from %s", tmp);
    myfree(tmp);

    socket_disconnect(s);
}

/*
 * The server is relaying a shout to clients.
 */
static void socket_tx_client_shout_relay (gsocketp s, 
                                          uint32_t level,
                                          const char *txt,
                                          gsocketp from)
{
    verify(s);

    if (!socket_get_udp_socket(s)) {
        return;
    }

    UDPpacket *packet = packet_alloc();

    msg_client_shout msg = {0};
    msg.type = MSG_CLIENT_SHOUT;
    msg.level = level;
    strlcpy(msg.txt, txt, sizeof(msg.txt) - 1);

    if (from && from->stats.pname[0]) {
        char *name = from->stats.pname;
        strlcpy(msg.from, name, min(sizeof(msg.from) - 1, strlen(name))); 
    } else {
        const char *name = "server";
        strlcpy(msg.from, name, min(sizeof(msg.from) - 1, strlen(name))); 
    }

    memcpy(packet->data, &msg, sizeof(msg));

    LOG("Client: Tx Shout to %s \"%s\"", 
        socket_get_remote_logname(s), txt);

    packet->len = sizeof(msg);

    write_address(packet, socket_get_remote_ip(s));

    socket_tx_enqueue(s, &packet);
}

void socket_tx_client_shout (gsocketp s, 
                             uint32_t level,
                             const char *txt)
{
    verify(s);

    if (!socket_get_udp_socket(s)) {
        return;
    }

    UDPpacket *packet = packet_alloc();

    msg_client_shout msg = {0};
    msg.type = MSG_CLIENT_SHOUT;
    msg.level = level;
    strlcpy(msg.txt, txt, sizeof(msg.txt) - 1);

    memcpy(packet->data, &msg, sizeof(msg));

    LOG("Client: Tx Shout to %s \"%s\"", 
        socket_get_remote_logname(s), txt);

    packet->len = sizeof(msg);

    write_address(packet, socket_get_remote_ip(s));

    socket_tx_enqueue(s, &packet);
}

void socket_rx_client_shout (gsocketp s, UDPpacket *packet, uint8_t *data)
{
    verify(s);

    msg_client_shout msg = {0};

    if (packet->len != sizeof(msg)) {
        socket_count_inc_pak_rx_error(s, packet);
        return;
    }

    memcpy(&msg, packet->data, sizeof(msg));

    char txt[PLAYER_MSG_MAX + 1] = {0};
    char from[SMALL_STRING_LEN_MAX + 1] = {0};

    memcpy(txt, msg.txt, PLAYER_MSG_MAX);
    memcpy(from, msg.from, SMALL_STRING_LEN_MAX);

    if (!single_player_mode) {
        char *tmp = iptodynstr(read_address(packet));
        LOG("Server: Rx Shout from %s \"%s\"", tmp, txt);
        myfree(tmp);
    }

    if (from[0]) {
        MESG(msg.level, "%s says, %s", from, txt);
    } else {
        /*
         * This is on the server receiving from the client.
         */
    }

    if (socket_get_client(s)) {
        return;
    }

    /*
     * This is for relaying the shout from the server to clients.
     */
    gsocketp sp;

    int sent = 0;

    TREE_WALK(sockets, sp) {
        if (sp == s) {
            continue;
        }

        if (!single_player_mode) {
            if (!sp->connected) {
                continue;
            }
        }

        if (!sp->server_side_client) {
            continue;
        }

        /*
         * Only talk to players who joined this server.
         */
        if (!sp->player) {
            continue;
        }

        /*
         * Include the source of the spammer.
         */
        socket_tx_client_shout_relay(sp, msg.level, txt, s);

        sent++;
    }

    if (sent == 0) {
        static const char *messages[] = {
            "But no one else is here",
            "The void echoes back",
            "No one is listening",
            "I'll talk to you if no one else will",
            "I have no friends either",
            "It's quite here",
            "I'm lonely",
            "Crickets",
            "The sound of silence",
            "Do you always talk to yourself?",
            "Never mind, I talk to myself too",
            "I watch you when you sleep",
            "I'm behind you. Watching",
            "You can talk to me",
            "Tell me about your mother",
            "I see. Interesting",
            "I never knew that about you",
            "You need a diary",
            "Eh? You talkin to me?",
            "I told you to stop talking to me",
            "Give it up dude, there's no one here",
            "Have you heard of the internet? It has people",
            "Never mind, we're together at least",
            "You're never alone with a computer",
            "Finally, you talk to just me",
            "Finally, you open your heart to me",
            "Say again?",
            "I don't follow?",
            "I see",
            "Well, that's understandable",
            "I disagree. Have you considered not doing that?",
            "Pull yourself together",
            "Hmm. Interesting.",
            "Interesting viewpoint. Wrong of course, but interesting",
            "I hear voices",
            "Are you real?",
            "Strange words echo through the dungeon",
            "I hear voices in my head",
            "Have you considered talking to someone",
            "I love you",
        };

        socket_tx_client_shout_relay(s, msg.level, 
                                     messages[myrand() % ARRAY_SIZE(messages)],
                                     0 /* from */);
    }
}

void socket_tx_server_shout_at_all_players (uint32_t level, 
                                            double x,
                                            double y,
                                            const char *txt)
{
    gsocketp sp;

    TREE_WALK(sockets, sp) {
        if (!single_player_mode) {
            if (!sp->connected) {
                continue;
            }
        }

        if (!sp->server_side_client) {
            continue;
        }

        /*
         * Only talk to players who joined this server.
         */
        if (!sp->player) {
            continue;
        }

        if (!single_player_mode) {
            LOG("Server: Tx Shout \"%s\" to (all players) %s", txt,
                socket_get_remote_logname(sp));
        }

        UDPpacket *packet = packet_alloc();

        msg_server_shout msg = {0};
        msg.type = MSG_SERVER_SHOUT;
        msg.level = level;

        if (sp->player->thing) {
            msg.thing_id = sp->player->thing->thing_id;
        }

        msg.x = (int)x;
        msg.y = (int)y;

        strlcpy(msg.txt, txt, sizeof(msg.txt) - 1);

        memcpy(packet->data, &msg, sizeof(msg));

        packet->len = sizeof(msg);

        write_address(packet, socket_get_remote_ip(sp));

        socket_tx_enqueue(sp, &packet);
    }
}

void 
socket_tx_server_shout_over (uint32_t level,
                             uint32_t thing_id,
                             double x,
                             double y,
                             const char *txt)
{
    gsocketp sp;

    TREE_WALK(sockets, sp) {
        if (!single_player_mode) {
            if (!sp->connected) {
                continue;
            }
        }

        if (!sp->server_side_client) {
            continue;
        }

        /*
         * Only talk to players who joined this server.
         */
        if (!sp->player) {
            continue;
        }

        UDPpacket *packet = packet_alloc();

        msg_server_shout msg = {0};
        msg.type = MSG_SERVER_SHOUT;
        msg.thing_id = thing_id;
        msg.level = level;
        msg.x = x;
        msg.y = y;

        strlcpy(msg.txt, txt, sizeof(msg.txt) - 1);

        memcpy(packet->data, &msg, sizeof(msg));

        packet->len = sizeof(msg);

        LOG("Client: Tx Shout %s over \"%s\"", 
            socket_get_remote_logname(sp), txt);

        write_address(packet, socket_get_remote_ip(sp));

        socket_tx_enqueue(sp, &packet);
    }
}

void 
socket_tx_server_shout_at_all_players_except (gsocketp except,
                                              uint32_t level, 
                                              double x,
                                              double y,
                                              const char *txt)
{
    gsocketp sp;

    TREE_WALK(sockets, sp) {
        if (sp == except) {
            continue;
        }

        if (!single_player_mode) {
            if (!sp->connected) {
                continue;
            }
        }

        if (!sp->server_side_client) {
            continue;
        }

        /*
         * Only talk to players who joined this server.
         */
        if (!sp->player) {
            continue;
        }

        UDPpacket *packet = packet_alloc();

        msg_server_shout msg = {0};
        msg.type = MSG_SERVER_SHOUT;
        msg.level = level;
        msg.x = (int)x;
        msg.y = (int)y;

        if (sp->player->thing) {
            msg.thing_id = sp->player->thing->thing_id;
        }

        strlcpy(msg.txt, txt, sizeof(msg.txt) - 1);

        memcpy(packet->data, &msg, sizeof(msg));

        packet->len = sizeof(msg);

        if (!single_player_mode) {
            LOG("Server: Tx Shout \"%s\" to (all except one) %s", txt,
                socket_get_remote_logname(sp));
        }

        write_address(packet, socket_get_remote_ip(sp));

        socket_tx_enqueue(sp, &packet);
    }
}

void socket_tx_server_shout_only_to (gsocketp target,
                                     uint32_t level,
                                     double x,
                                     double y,
                                     const char *txt)
{
    gsocketp sp;

    TREE_WALK(sockets, sp) {
        if (sp != target) {
            continue;
        }

        if (!sp->server_side_client) {
            continue;
        }

        /*
         * Only talk to players who joined this server.
         */
        if (!sp->player) {
            continue;
        }

        UDPpacket *packet = packet_alloc();

        msg_server_shout msg = {0};
        msg.type = MSG_SERVER_SHOUT;
        msg.level = level;
        msg.x = x;
        msg.y = y;

        if (sp->player->thing) {
            msg.thing_id = sp->player->thing->thing_id;
        }

        strlcpy(msg.txt, txt, sizeof(msg.txt) - 1);

        memcpy(packet->data, &msg, sizeof(msg));

        packet->len = sizeof(msg);

        if (!single_player_mode) {
            LOG("Server: Tx Shout \"%s\" to (one player) %s", txt,
                socket_get_remote_logname(sp));
        }

        write_address(packet, socket_get_remote_ip(sp));

        socket_tx_enqueue(sp, &packet);
    }
}

void socket_rx_server_shout (gsocketp s, UDPpacket *packet, uint8_t *data)
{
    verify(s);

    msg_server_shout msg = {0};

    if (packet->len != sizeof(msg)) {
        socket_count_inc_pak_rx_error(s, packet);
        return;
    }

    memcpy(&msg, packet->data, sizeof(msg));

    char txt[PLAYER_MSG_MAX + 1] = {0};
    memcpy(txt, msg.txt, PLAYER_MSG_MAX);

    if (!single_player_mode) {
        char *tmp = iptodynstr(read_address(packet));
        LOG("Client: Rx Shout from %s \"%s\"", tmp, txt);
        myfree(tmp);
    }

    MSG_CLIENT_SHOUT_AT(msg.level, 
                        msg.thing_id, 
                        msg.x, msg.y, "%s", txt);
}

void socket_tx_tell (gsocketp s, 
                     const char *from,
                     const char *to,
                     const char *txt)
{
    verify(s);

    if (!single_player_mode) {
        if (!s->connected) {
            return;
        }
    }

    UDPpacket *packet = packet_alloc();

    msg_tell msg = {0};
    msg.type = MSG_TELL;

    strlcpy(msg.from, from, min(sizeof(msg.from) - 1, strlen(from))); 
    strlcpy(msg.to, to, min(sizeof(msg.to) - 1, strlen(to))); 
    strlcpy(msg.txt, txt, sizeof(msg.txt) - 1);

    memcpy(packet->data, &msg, sizeof(msg));

    LOG("Tx Tell %s from \"%s\" to \"%s\" msg \"%s\"", 
        socket_get_remote_logname(s), from, to, txt);

    packet->len = sizeof(msg);

    write_address(packet, socket_get_remote_ip(s));

    socket_tx_enqueue(s, &packet);
}

void socket_rx_tell (gsocketp s, UDPpacket *packet, uint8_t *data)
{
    verify(s);

    msg_tell msg = {0};

    if (packet->len != sizeof(msg)) {
        socket_count_inc_pak_rx_error(s, packet);
        return;
    }

    memcpy(&msg, packet->data, sizeof(msg));

    char txt[PLAYER_MSG_MAX + 1] = {0};
    char from[SMALL_STRING_LEN_MAX + 1] = {0};
    char to[SMALL_STRING_LEN_MAX + 1] = {0};

    memcpy(txt, msg.txt, PLAYER_MSG_MAX);
    memcpy(from, msg.from, SMALL_STRING_LEN_MAX);
    memcpy(to, msg.to, SMALL_STRING_LEN_MAX);

    LOG("TELL: from \"%s\" to \"%s\" msg \"%s\"", from, to, txt);

    if (!socket_get_server(s)) {
        MESG(CHAT, "%s, says %s", from, txt);
        return;
    }

    gsocketp sp;

    TREE_WALK(sockets, sp) {

        if (sp == s) {
            continue;
        }

        if (!single_player_mode) {
            if (!sp->connected) {
                continue;
            }
        }

        if (!sp->server_side_client) {
            continue;
        }

        if (!strcasecmp(socket_get_name(sp), to)) {
            socket_tx_tell(sp, from, to, txt);
        }
    }
}

/*
 * Send each player their status update
 */
void socket_tx_server_status (gsocketp s_in)
{
    static uint32_t ts;

    if (!s_in) {
        if (!time_have_x_hundredths_passed_since(
                DELAY_HUNDREDTHS_SERVER_TO_CLIENT_PLAYER_UPDATE, ts)) {
            return;
        }
    }

    ts = time_get_time_ms();

    msg_server_status msg = {0};
    msg.type = MSG_SERVER_STATUS;

    if (server_level) {
        msg.level_pos = server_level->level_pos;
        msg.seed = server_level->seed;
        msg.server_current_players = global_config.server_current_players;
        msg.level_hide = level_is_ready_to_fade_out(server_level);
    } else {
        /*
         * Game over?
         */
        msg.level_hide = true;
    }

    gsocketp s;

    /*
     * Walk all players and send them their updates.
     *
     * Also send to sockets with no players as someone may be pinging the 
     * server to check if they can join.
     */
    TREE_WALK(sockets, s) {
        if (s_in && (s != s_in)) {
            continue;
        }

        if (!s->server_side_client) {
            continue;
        }

        msg_player_state *msg_tx = &msg.player;

        msg.you_are_playing_on_this_server = 0;

        aplayer *p = s->player;
        if (p) {
            msg.you_are_playing_on_this_server = 1;

            thingp t = p->thing;
            if (t) {
                memcpy(&msg_tx->stats, &t->stats, sizeof(thing_stats));

                thingp shield_carry_anim = thing_shield_carry_anim(t);
                if (shield_carry_anim) {
                    msg_tx->stats.shield = thing_stats_get_hp(shield_carry_anim);
                }

                if (!single_player_mode) {
                    LOG("Server: Tx Server Status to %s", socket_get_remote_logname(s));
                    thing_dump(t);
                }

                SDLNet_Write32(p->local_ip.host, &msg_tx->local_ip.host);
                SDLNet_Write16(p->local_ip.port, &msg_tx->local_ip.port);

                SDLNet_Write32(p->remote_ip.host, &msg_tx->remote_ip.host);
                SDLNet_Write16(p->remote_ip.port, &msg_tx->remote_ip.port);
            }
        }

        UDPpacket *packet = packet_alloc();

        memcpy(packet->data, &msg, sizeof(msg));

        if (debug_socket_ping_enabled) {
            LOG("Server: Tx Server Status to %s", socket_get_remote_logname(s));
        }

        packet->len = sizeof(msg);

        write_address(packet, socket_get_remote_ip(s));

        socket_tx_enqueue(s, &packet);
    }
}

/*
 * Receive an array of all current players from the server.
 */
void socket_rx_server_status (gsocketp s, UDPpacket *packet, uint8_t *data,
                              msg_server_status *status)
{
    verify(s);

    msg_server_status *msg;

    if (packet->len != sizeof(*msg)) {
        socket_count_inc_pak_rx_error(s, packet);
        return;
    }

    msg = (typeof(msg)) packet->data;

    msg_player_state *p = &status->player;
    msg_player_state *msg_rx = &msg->player;

    memcpy(&p->stats, &msg_rx->stats, sizeof(thing_stats));

    p->local_ip.host = SDLNet_Read32(&msg_rx->local_ip.host);
    p->local_ip.port = SDLNet_Read16(&msg_rx->local_ip.port);

    p->remote_ip.host = SDLNet_Read32(&msg_rx->remote_ip.host);
    p->remote_ip.port = SDLNet_Read16(&msg_rx->remote_ip.port);

    if (debug_socket_ping_enabled) {
        char *tmp = iptodynstr(read_address(packet));
        if (msg->you_are_playing_on_this_server) {
            LOG("Client: Rx Server Status from %s, current player \"%s\"", 
                tmp, p->stats.pname);
        } else {
            LOG("Client: Rx Server Status from %s", tmp);
        }
        myfree(tmp);
    }

    status->level_pos = msg->level_pos;
    status->seed = msg->seed;
    status->server_current_players = global_config.server_current_players;
    status->level_hide = msg->level_hide;
    status->you_are_playing_on_this_server = msg->you_are_playing_on_this_server;

    memcpy(&s->server_status, status, sizeof(s->server_status));
// LOG("client, rx version: %d",s->stats.client_version);
}

/*
 * Send an array of all current players to all clients.
 */
void socket_tx_server_hiscore (gsocketp only,
                               const char *player_name,
                               const char *death_reason,
                               uint32_t score)
{
    msg_server_hiscores msg = {0};
    msg.type = MSG_SERVER_HISCORE;
    msg.rejoin_allowed = global_config.server_current_players > 1;

    hiscore *hi[MAX_HISCORES+1] = {0};
    uint32_t hi_index = 0;
    hiscore *h;

    if (!death_reason) {
        ERR("Player %s death with no reason", player_name);
    }

    { TREE_WALK_REVERSE(hiscores, h) {
        hi[hi_index++] = h;
        if (hi_index >= MAX_HISCORES) {
            break;
        }
    } }

    /*
     * Add all current players.
     */
    gsocketp s;
    TREE_WALK(sockets, s) {
        if (only) {
            if (only != s) {
                continue;
            }
        }

        uint32_t i;

        for (i = 0; i < hi_index; i++) {
            h = hi[i];
            if (!h) {
                break;
            }

            msg_player_hiscore *msg_tx = &msg.players[i + 1];

            strlcpy(msg_tx->player_name, h->player_name, 
                    min(sizeof(msg_tx->player_name),
                        strlen(h->player_name) + 1)); 

            strlcpy(msg_tx->death_reason, h->death_reason, 
                    min(sizeof(msg_tx->death_reason),
                        strlen(h->death_reason) + 1)); 

            SDLNet_Write32(h->tree.key2, &msg_tx->score);
        }
    }

    /*
     * Now put the currently croaked it player in first place.
     */
    {
        msg_player_hiscore *msg_tx = &msg.players[0];

        strlcpy(msg_tx->player_name, player_name, 
                min(sizeof(msg_tx->player_name), strlen(player_name) + 1)); 

        strlcpy(msg_tx->death_reason, death_reason, 
                min(sizeof(msg_tx->death_reason), strlen(death_reason) + 1)); 

        SDLNet_Write32(score, &msg_tx->score);
    }

    {
        TREE_WALK(sockets, s) {
            if (only) {
                if (only != s) {
                    continue;
                }
            }

            if (!single_player_mode) {
                if (!s->connected) {
                    continue;
                }
            }

            if (!s->server_side_client) {
                continue;
            }

            LOG("Server: Tx hiscore to %s", socket_get_remote_logname(s));

            UDPpacket *packet = packet_alloc();

            memcpy(packet->data, &msg, sizeof(msg));

            packet->len = sizeof(msg);

            write_address(packet, socket_get_remote_ip(s));

            socket_tx_enqueue(s, &packet);
        }
    }
}

/*
 * Receive an array of all current players from the server.
 */
void socket_rx_server_hiscore (gsocketp s, UDPpacket *packet, 
                               uint8_t *data,
                               msg_server_hiscores *hiscore)
{
    verify(s);

    msg_server_hiscores *msg;

    if (packet->len != sizeof(*msg)) {
        socket_count_inc_pak_rx_error(s, packet);
        return;
    }

    uint32_t pi;

    msg = (typeof(msg)) packet->data;

    hiscore->rejoin_allowed = msg->rejoin_allowed;

    for (pi = 0; pi < MAX_HISCORES + 1; pi++) {
        msg_player_hiscore *p = &hiscore->players[pi];
        msg_player_hiscore *msg_rx = &msg->players[pi];

        memcpy(p->player_name, msg_rx->player_name, SMALL_STRING_LEN_MAX);

        memcpy(p->death_reason, msg_rx->death_reason, SMALL_STRING_LEN_MAX);

        p->score = SDLNet_Read32(&msg_rx->score);

        if (!p->player_name[0]) {
            continue;
        }

        char *tmp = iptodynstr(read_address(packet));
        LOG("Client: Rx hiscore from %s %u:\"%s\"", tmp, pi, p->player_name);
        myfree(tmp);
    }
}

/*
 * Tell all players the server is down.
 */
void socket_tx_server_close (void)
{
    gsocketp s;

    TREE_WALK(sockets, s) {
        if (!single_player_mode) {
            if (!s->connected) {
                continue;
            }
        }

        if (!s->server_side_client) {
            continue;
        }

        LOG("Server: Tx down to %s",
            socket_get_remote_logname(s));

        msg_server_close msg = {0};
        msg.type = MSG_SERVER_CLOSE;

        UDPpacket *packet = packet_alloc();

        memcpy(packet->data, &msg, sizeof(msg));

        packet->len = sizeof(msg);

        write_address(packet, socket_get_remote_ip(s));

        socket_tx_enqueue(s, &packet);
    }
}

void socket_rx_server_close (gsocketp s, UDPpacket *packet, uint8_t *data)
{
    verify(s);

    msg_server_close *msg;

    if (packet->len != sizeof(*msg)) {
        socket_count_inc_pak_rx_error(s, packet);
        return;
    }

    char *tmp = iptodynstr(read_address(packet));
    LOG("Server: Rx down from %s", tmp);
    myfree(tmp);

    socket_set_connected(s, false);
}

uint32_t socket_get_quality (gsocketp s)
{
    verify(s);

    return (s->quality);
}

uint32_t socket_get_avg_latency_rtt (gsocketp s)
{
    verify(s);

    return (s->avg_latency_rtt);
}

uint32_t socket_get_min_latency_rtt (gsocketp s)
{
    verify(s);

    return (s->min_latency_rtt);
}

uint32_t socket_get_max_latency_rtt (gsocketp s)
{
    verify(s);

    return (s->max_latency_rtt);
}

uint32_t socket_get_rx (gsocketp s)
{
    verify(s);

    return (s->rx);
}

uint32_t socket_get_tx (gsocketp s)
{
    verify(s);

    return (s->tx);
}

uint32_t socket_get_rx_error (gsocketp s)
{
    verify(s);

    return (s->rx_error);
}

uint32_t socket_get_tx_error (gsocketp s)
{
    verify(s);

    return (s->tx_error);
}

uint32_t socket_get_rx_bad_msg (gsocketp s)
{
    verify(s);

    return (s->rx_bad_msg);
}

const char *socket_get_server_name (gsocketp s)
{
    verify(s);

    return (s->server_name);
}

const char *socket_get_other_player_name (gsocketp s, const uint32_t p)
{
    verify(s);

    return (s->player_name[p]);
}

uint32_t socket_get_max_players (gsocketp s)
{
    verify(s);

    return (s->server_max_players);
}

uint32_t socket_get_current_players (gsocketp s)
{
    verify(s);

    return (s->server_current_players);
}

void socket_tx_player_move (gsocketp s, 
                            thingp t,
                            const uint8_t up,
                            const uint8_t down,
                            const uint8_t left, 
                            const uint8_t right,
                            const uint8_t fire,
                            const uint8_t magic)
{
    /*
     * Allow firing to always be sent.
     */
    if (!fire && !magic) {
        static uint32_t ts;

        if (!time_have_x_hundredths_passed_since(
                DELAY_HUNDREDTHS_CLIENT_TO_SERVER_PLAYER_POSITION, ts)) {
            return;
        }

        ts = time_get_time_ms();
    }

    msg_player_move msg = {0};
    msg.type = MSG_CLIENT_PLAYER_MOVE;
    msg.dir = (magic << 5) | 
              (fire << 4) |
              (up << 3) |
              (down << 2) |
              (left << 1) |
              right;

    SDLNet_Write16(t->x * THING_COORD_SCALE, &msg.x);               
    SDLNet_Write16(t->y * THING_COORD_SCALE, &msg.y);               

    UDPpacket *packet = packet_alloc();

    memcpy(packet->data, &msg, sizeof(msg));

    packet->len = sizeof(msg);

    write_address(packet, socket_get_remote_ip(s));

    socket_tx_enqueue(s, &packet);

    if (magic) {
        t->stats.magic--;

        wid_player_action_hide(true /* fast */, false /* player quit */);
        wid_player_action_visible(&t->stats, true /* fast */);
    }
}

void socket_server_rx_player_move (gsocketp s, UDPpacket *packet, uint8_t *data)
{
    verify(s);

    msg_player_move msg = {0};

    if (packet->len != sizeof(msg)) {
        socket_count_inc_pak_rx_error(s, packet);
        return;
    }

    memcpy(&msg, packet->data, sizeof(msg));

    aplayer *p = s->player;
    if (!p) {
        return;
    }

    const uint8_t magic = (msg.dir & (1 << 5)) ? 1 : 0;
    const uint8_t fire  = (msg.dir & (1 << 4)) ? 1 : 0;
    const uint8_t up    = (msg.dir & (1 << 3)) ? 1 : 0;
    const uint8_t down  = (msg.dir & (1 << 2)) ? 1 : 0;
    const uint8_t left  = (msg.dir & (1 << 1)) ? 1 : 0;
    const uint8_t right = (msg.dir & (1 << 0)) ? 1 : 0;

    thingp t = p->thing;
    if (!t) {
        return;
    }

    msg.x = SDLNet_Read16(&msg.x);
    msg.y = SDLNet_Read16(&msg.y);

    double x = ((double)msg.x) / THING_COORD_SCALE;
    double y = ((double)msg.y) / THING_COORD_SCALE;

    thing_server_move(t, x, y, up, down, left, right, fire, magic);
}

void socket_tx_player_action (gsocketp s, 
                              thingp t,
                              const uint8_t action,
                              const uint32_t action_bar_index,
                              const int change_selection_only)
{
    if (!s) {
        ERR("no socket");
        return;
    }

    msg_player_action msg = {0};
    msg.type = MSG_CLIENT_PLAYER_ACTION;
    msg.action = action;
    msg.action_bar_index = (uint8_t) action_bar_index;
    msg.change_selection_only = (uint8_t) change_selection_only;

    UDPpacket *packet = packet_alloc();

    memcpy(packet->data, &msg, sizeof(msg));

    packet->len = sizeof(msg);

    write_address(packet, socket_get_remote_ip(s));

    socket_tx_enqueue(s, &packet);
}

void socket_server_rx_player_action (gsocketp s, UDPpacket *packet, 
                                     uint8_t *data)
{
    verify(s);

    msg_player_action msg = {0};

    if (packet->len != sizeof(msg)) {
        socket_count_inc_pak_rx_error(s, packet);
        return;
    }

    memcpy(&msg, packet->data, sizeof(msg));

    aplayer *p = s->player;
    if (!p) {
        return;
    }

    thingp t = p->thing;
    if (!t) {
        /*
         * Player is dead?
         */
        return;
    }

    uint8_t action = msg.action;
    uint32_t action_bar_index = msg.action_bar_index;
    uint32_t change_selection_only = msg.change_selection_only;

    thing_server_action(t, action, action_bar_index, change_selection_only);
}

static UDPpacket *packet_finalize (gsocketp s, UDPpacket *packet)
{
#ifdef ENABLE_PAK_EXTRA_HEADER
    int add = 1;
#else
    int add = 0;
#endif

    verify(s);
    verify(packet);

    if (!add) {
        return (packet);
    }

    /*
     * Calculate the checksum.
     */
    uint8_t csum = 0;
    uint16_t i;
    uint8_t *tmp = packet->data;
    const uint16_t len = packet->len;

    for (i = 0; i < len; i++) {
        csum += tmp[i];
    }

    /*
     * Make a copy and add the header.
     */
    UDPpacket *copy = packet_dup_no_copy(packet, 2);
    copy->len = packet->len + 2;
    memcpy(copy->data + 2, packet->data, packet->len);
    packet_free(packet);
    packet = copy;

    LOG("Tx seq %d, csum %d, len %d, to [%s]", s->tx_seq, csum, packet->len,
        socket_get_remote_logname(s));

    /*
     * Add the new header.
     */
    packet->data[0] = csum;
    packet->data[1] = s->tx_seq;

    s->tx_seq++;

#ifdef ENABLE_PACKET_DUMP
    hex_dump_CON(packet->data, 0, packet->len);
#endif

    return (packet);
}

UDPpacket *packet_definalize (gsocketp s, UDPpacket *packet)
{
#ifdef ENABLE_PAK_EXTRA_HEADER
    int remove = 1;
#else
    int remove = 0;
#endif

    verify(s);
    verify(packet);

    if (!remove) {
        return (packet);
    }

    const uint8_t in_csum = packet->data[0];
    const uint8_t rx_seq = packet->data[1];

    /*
     * Check the checksum.
     */
    uint8_t csum = 0;
    uint16_t i;
    uint8_t *tmp = packet->data;
    const uint16_t len = packet->len;

    /*
     * Skip 2 bytes of header
     */
    for (i = 2; i < len; i++) {
        csum += tmp[i];
    }

    int failed = 0;

    if (csum != in_csum) {
        ERR("checksum mismatch, expected %d, received %d", csum, in_csum);
        failed = 1;
    }

    /*
     * Check the sequence if we have received a previous sequence number.
     */
    if (s->rx_seq_valid) {
        if (rx_seq != s->rx_seq) {
            ERR("sequence mismatch, expected %d not %d", s->rx_seq, rx_seq);
            failed = 1;
        }
    }

    s->rx_seq_valid = 1;
    s->rx_seq = rx_seq + 1;

    LOG("Rx seq %d, csum %d, len %d, from [%s]", rx_seq, in_csum, packet->len,
        socket_get_remote_logname(s));

    /*
     * Give back the original packet minus the header.
     */
    UDPpacket *copy = packet_dup_no_copy(packet, -2);
    copy->len = packet->len - 2;
    memcpy(copy->data, packet->data + 2, copy->len);
    packet_free(packet);
    packet = copy;

#ifdef ENABLE_PACKET_DUMP
    hex_dump_CON(packet->data, 0, packet->len);
#endif

    if (failed) {
        ERR("packet layer error");
    }

    return (packet);
}

uint8_t *packet_decompress (UDPpacket *packet, uint8_t *uncompressed)
{
    uint8_t *data;

    /*
     * Uncompress the packet if it has an invalid type.
     */
    if (*packet->data == MSG_COMPRESSED) {
        data = miniz_uncompress(packet->data + 1, &packet->len);
        packet->data = data;
        *uncompressed = true;
    } else {
        data = packet->data;
        *uncompressed = false;
    }

    return (data);
}

void packet_compress (UDPpacket *packet)
{
    if (single_player_mode) {
        return;
    }

#ifdef ENABLE_PACKET_DUMP
    LOG("Tx pre compressed:");
    hex_dump_CON(packet->data, 0, packet->len);
#endif

    unsigned char *tmp = miniz_compress2(packet->data, &packet->len, 9);

    if (packet->len > MAX_PACKET_SIZE) {
        ERR("compress fail");
    }

    *packet->data = MSG_COMPRESSED;

    uint8_t *data = packet->data + 1;
    uint16_t len = packet->len; 

    memcpy(data, tmp, len);
    packet->len++;
    myfree(tmp);

#ifdef ENABLE_PACKET_DUMP
    LOG("Tx post compressed:");
    hex_dump_CON(packet->data, 0, packet->len);
#endif
}

static int socket_udp_send (gsocketp s, UDPpacket *packet)
{
    int ret = 0;

    /*
     * If single player mode and client and server are on the same machine.
     */
    if (single_player_mode && 
        is_client && 
        on_server && 
        server_socket) {

        gsocketp other_socket = 0;
        gsocketp matched_socket = 0;

#ifdef DEBUG_SINGLE_PLAYER_SOCKET
        /*
         * In single player mode, walk the othr sockets looking for whom to 
         * loop the packet back to without going to udp.
         */
        { TREE_WALK(sockets, other_socket) {
            const char *sock2;

            if (other_socket->server) {
                sock2 = "server";
            } else if (other_socket->client) {
                sock2 = "client";
            } else if (other_socket->server_side_client) {
                sock2 = "server side client";
            } else {
                sock2 = 0;
            }

            LOG("    %p cand %s/%s(%s)", 
                other_socket, 
                socket_get_local_logname(other_socket), 
                socket_get_remote_logname(other_socket), 
                sock2);
        } }
#endif

        TREE_WALK(sockets, other_socket) {
            if (s->server_side_client) {
                if (other_socket->client) {
                    matched_socket = other_socket;
                    break;
                }
                continue;
            }

            if (s->remote_ip.port == other_socket->local_ip.port) {
                matched_socket = other_socket;
                break;
            }
        }

        if (matched_socket) {
            other_socket = matched_socket;
        } else {
            other_socket = 0;
        }

        if (other_socket) {
            if (((int)other_socket->rx_queue_size) >= MAX_SOCKET_QUEUE_SIZE - 1) {
                ERR("packet queue full");
                packet_free(packet);
                return (-1);
            }

            other_socket->rx_queue_size++;
            other_socket->rx_queue[other_socket->rx_queue_tail++] = packet;

#ifdef DEBUG_SINGLE_PLAYER_SOCKET
            const char *sock;

            if (socket_get_client(s)) {
                sock = "client";
            } else if (client_joined_server && (s == client_joined_server)) {
                sock = "server size joined client";
            } else if (server_socket && (s == server_socket)) {
                sock = "local server";
            } else if (socket_get_server(s)) {
                sock = "other server";
            } else if (socket_get_server_side_client(s)) {
                sock = "server side client";
            } else if (socket_get_client(s)) {
                sock = "client";
            } else {
                sock = 0;
            }

            LOG("       -> %s/%s (to %p)", 
                socket_get_local_logname(s), 
                socket_get_remote_logname
                (s), other_socket);
#endif
            return (1);
        }

        packet_free(packet);

        return (1);
    }

    if (socket_get_udp_socket(s)) {
        ret = SDLNet_UDP_Send(socket_get_udp_socket(s), socket_get_channel(s), packet);
    }

    packet_free(packet);

    return (ret);
}


/*
 * Pull a packet off of the queue and send it on the UDP port for the socket.
 */
static int socket_tx_queue_send_packet (gsocketp s)
{
    if (!s->tx_queue_size) {
        return (0);
    }

    UDPpacket *packet;

    packet = s->tx_queue[s->tx_queue_head++];
    verify(packet);
    s->tx_queue_size--;

    /*
     * Do last changes to the packet like adding sequence numbers
     */
    packet = packet_finalize(s, packet);

#ifdef ENABLE_PACKET_DUMP
    LOG("%p: Send slot %d", s, s->tx_queue_head - 1);
    hex_dump_CON(packet->data, 0, packet->len);
#endif

    if (socket_udp_send(s, packet) < 1) {
        /*
         * Only warn about sockets we really care about.
         */
        if (single_player_mode) {
            return (1);
        }

        if (s->connected) {
            ERR("no UDP packet sent to %s: %s",
                socket_get_remote_logname(s),
                SDLNet_GetError());
        }

        socket_count_inc_pak_tx_error(s);
    } else {
        socket_count_inc_pak_tx(s);
    }

    return (1);
}

void socket_tx_enqueue (gsocketp s, UDPpacket **packet_in)
{
    UDPpacket *packet;
    msg_type type;

    packet = *packet_in;
    *packet_in = 0;

    type = (typeof(type)) *(packet->data);

    if (type != MSG_COMPRESSED) {
        s->tx_msg[type]++;
    }

    if (type == MSG_COMPRESSED) {
        /*
         * Resend of an already compressed message but to another client? I 
         * hope so.
         */
    } else if (packet->len > PACKET_LEN_COMPRESS_THRESHOLD) {
        /*
         * A good enough size for compression to work and give a smaller 
         * packet.
         */
        packet_compress(packet);
    }

    if (s->tx_queue_size) {
        uint8_t tx_queue_tail = s->tx_queue_tail - 1;

        UDPpacket *super_packet = s->tx_queue[tx_queue_tail];

        verify(super_packet);

        const uint8_t head_type = super_packet->data[0];

        uint16_t fragment_len = packet->len;

        /*
         * Len is of the data only, not including the super packet header.
         */
        uint16_t oldlen = super_packet->len;

        /*
         * Work out how big the packet would be if we added a fragment onto 
         * it.
         */
        uint16_t newlen = oldlen + fragment_len + sizeof(msg_super_packet);

        /*
         * Bear in mind the head packet may not yet be a fragment so we have 
         * to add the super header packet on too.
         */
        if (head_type != MSG_SUPER_PACKET) {
            newlen += sizeof(msg_super_packet);
        }

        /*
         * If we can add fragments into the packet without going over, do so
         */
        if (newlen < MAX_PACKET_SIZE) {
            /*
             * If the old packet needs a header, add it and turn it into a 
             * fragment.
             */
            if (head_type != MSG_SUPER_PACKET) {
                memmove(super_packet->data + sizeof(msg_super_packet), 
                        super_packet->data, 
                        oldlen);

                super_packet->data[0] = MSG_SUPER_PACKET;
                SDLNet_Write16(oldlen, super_packet->data + 1);
                oldlen += sizeof(msg_super_packet);
            }

            /*
             * Add on the new packet as a fragment now.
             */
            uint8_t *next_fragment = super_packet->data + oldlen;
            *next_fragment = MSG_SUPER_PACKET;
            SDLNet_Write16(fragment_len, next_fragment + 1);
            memcpy(next_fragment + sizeof(msg_super_packet),
                   packet->data, fragment_len);

            super_packet->len = newlen;

            /*
             * Whee! We do not need to add to the queue if we added this as a 
             * fragment.
             */
            packet_free(packet);
            return;
        }
    }

    if (((int)s->tx_queue_size) == MAX_SOCKET_QUEUE_SIZE - 1) {
        socket_tx_queue_flush(s);

        if (((int)s->tx_queue_size) == MAX_SOCKET_QUEUE_SIZE - 1) {
            ERR("socket tx queue stuck");
        }
    }

    if (s->tx_queue_size >= (MAX_SOCKET_QUEUE_SIZE / 4) * 3) {
        ERR("Socket 75%% congested %s", socket_get_remote_logname(s));
    }

    s->tx_queue_size++;
    s->tx_queue[s->tx_queue_tail++] = packet;

#ifdef ENABLE_PACKET_DUMP
    LOG("s %p Enqueued slot %d size %d",s,  s->tx_queue_tail - 1, s->tx_queue_size);
    hex_dump_CON(packet->data, 0, packet->len);
#endif
}

/*
 * Pull a packet from UDP and enqueue it for processing locally
 */
UDPpacket *socket_rx_dequeue (gsocketp s)
{
    UDPpacket *packet;

    if (!s->rx_queue_size) {
        return (0);
    }

    packet = s->rx_queue[s->rx_queue_head];
    verify(packet);

    if (s->server) {
        gsocketp s = socket_find_server_side_remote_ip(read_address(packet));
        if (!s) {
            char *tmp = iptodynstr(read_address(packet));
            LOG("Server: New client from %s", tmp);
            myfree(tmp);

            socket_connect_from_server(read_address(packet));
        }
    }

    /*
     * If this is a super packet, pull off a fragment.
     */
    if (packet->data[0] == MSG_SUPER_PACKET) {
        UDPpacket *super_packet = packet;

        /*
         * Read the head fragment size.
         */
        uint16_t fragment_len = SDLNet_Read16(super_packet->data + 1);

        /*
         * We need to adjust the super packet after removing the head fragment
         * by this len
         */
        uint16_t move_len = fragment_len + sizeof(msg_super_packet);

        /*
         * Allocate a fragment and set it up with similar source data.
         */
        UDPpacket *fragment = packet_alloc();
        fragment->channel = super_packet->channel;
        memcpy(&fragment->address, &super_packet->address, sizeof(IPaddress));
        memcpy(fragment->data, super_packet->data + sizeof(msg_super_packet), 
               fragment_len);
        fragment->len = fragment_len;

        /*
         * If this is the last fragment, free the super packet.
         */
        uint16_t new_super_packet_len = super_packet->len - move_len;

        if (!new_super_packet_len) {
            /*
             * Last fragment. Discard the super packet.
             */
            packet_free(super_packet);
            packet = fragment;
        } else {
            /*
             * More fragments. Now move the data for the next fragment to the 
             * head.
             */
            memmove(super_packet->data, 
                    super_packet->data + move_len, new_super_packet_len);

            super_packet->len = new_super_packet_len;
            return (fragment);
        }
    }

    s->rx_queue_head++;
    s->rx_queue_size--;

    return (packet);
}

static int socket_rx_queue_receive_packets (gsocketp s)
{
    int count = 0;
    int waittime = 0;

    if (!s) {
        return (0);
    }
    if (!socket_get_socklist(s)) {
        return (0);
    }

    int numready = SDLNet_CheckSockets(socket_get_socklist(s), waittime);
    if (numready <= 0) {
        return (count);
    }

    int i;
    for (i = 0; i < numready; i++) {
        int ready = SDLNet_SocketReady(socket_get_udp_socket(s));
        if (ready == 0) {
            return (count);
        }

        UDPpacket *packet = packet_alloc();

        int paks = SDLNet_UDP_Recv(socket_get_udp_socket(s), packet);
        if (paks != 1) {
            LOG("Client: UDP rx failed: error='%s' paks=%d", 
                SDLNet_GetError(), paks);
            return (count);
        }

#ifdef ENABLE_PACKET_DUMP
        LOG("Receive");
        hex_dump_CON(packet->data, 0, packet->len);
#endif
        if (((int)s->rx_queue_size) >= MAX_SOCKET_QUEUE_SIZE - 1) {
            return (count);
        }

        s->rx_queue_size++;
        s->rx_queue[s->rx_queue_tail++] = packet;

        count++;
    }

    return (count);
}

static void socket_tx_queue_flush_all (void)
{
    gsocketp s;

    LOG("Flush all tx sockets");

    TREE_WALK(sockets, s) {
        socket_tx_queue_flush(s);
    }
}

static void socket_tx_queue_flush (gsocketp s)
{
    LOG("Flushing tx socket %s", socket_get_local_logname(s));

    while (socket_tx_queue_send_packet(s)) { }
}

static void socket_rx_queue_flush (gsocketp s)
{
    LOG("Flushing rx socket %s", socket_get_local_logname(s));

    for (;;) {
        UDPpacket *packet = socket_rx_dequeue(s);
        if (!packet) {
            break;
        }

        packet_free(packet);
    }
}

static void socket_rx_queue_flush_all (void)
{
    gsocketp s;

    LOG("Flush all rx sockets");

    TREE_WALK(sockets, s) {
        socket_rx_queue_flush(s);
    }
}

void socket_tx_tick (void)
{
    gsocketp s;

    if (single_player_mode) {
        TREE_WALK(sockets, s) {
            while (socket_tx_queue_send_packet(s)) {
            }
        }

    } else {
        TREE_WALK(sockets, s) {
            int loop = 0;

            while (loop++ < MAX_SOCKET_TX_PACKETS_PER_LOOP) {
                if (!socket_tx_queue_send_packet(s)) {
                    break;
                }
            }

            if (s->tx_queue_size > MAX_SOCKET_QUEUE_SIZE / 2) {
                socket_tx_queue_flush(s);
            }
        }
    } 
}

void socket_rx_tick (void)
{
    gsocketp s;

    if (single_player_mode) {
        return;
    }

    TREE_WALK(sockets, s) {
        for (;;) {
            if (!socket_rx_queue_receive_packets(s)) {
                break;
            }
        }
    }
}
