/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
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
#include "server.h"
#include "client.h"
#include "wid_game_map_server.h"
#include "thing.h"
#include "mzip_lib.h"

tree_rootp sockets;

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

    if (!sockets) {
        sockets = tree_alloc(TREE_KEY_TWO_INTEGER, "TREE ROOT: sockets");
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

static socket *socket_connect (IPaddress address, boolean server_side_client)
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

    s->server_side_client = server_side_client;

    if (!server_side_client) {
        s->client = true;
    }

    s->remote_ip = connect_address;
    s->local_ip = *SDLNet_UDP_GetPeerAddress(s->udp_socket, -1);

    LOG("Socket connect to %s", socket_get_remote_logname(s));

    if (debug_socket_connect_enabled) {
        LOG("       from      %s", socket_get_local_logname(s));
    }

    return (s);
}

socket *socket_connect_from_client (IPaddress address)
{
    return (socket_connect(address, false));
}

socket *socket_connect_from_server (IPaddress address)
{
    return (socket_connect(address, true));
}

static void socket_destroy (socketp s)
{
    verify(s);

    socket_set_connected(s, false);

    if (s->server) {
        LOG("Socket destroy [listen %s]", socket_get_local_logname(s));
    } else {
        LOG("Socket destroy [to %s]", socket_get_remote_logname(s));
        LOG("               [from %s]", socket_get_local_logname(s));
    }

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

    if (s == server_socket) {
        server_socket = 0;
    }
}

void socket_disconnect (socketp s)
{
    verify(s);

    tree_remove_found_node(sockets, &s->tree.node);

    socket_destroy(s);

    myfree(s);
}

void socket_tx_msg (socketp s, UDPpacket *packet)
{
    msg_type type;

    type = *(packet->data);

    if (type != MSG_COMPRESSED) {
        s->tx_msg[type]++;
    }

    if (type == MSG_COMPRESSED) {
        /*
         * Resend of an already compressed message but to another client? I 
         * hope so.
         */
    } else if (packet->len > 200) {
        /*
         * A good enough size for compression to work and give a smaller 
         * packet.
         */
        unsigned char *tmp = miniz_compress2(packet->data, &packet->len, 9);

        if (packet->len > MAX_PACKET_SIZE) {
            DIE("compress fail");
        }

        *packet->data = MSG_COMPRESSED;
        memcpy(packet->data + 1, tmp, packet->len);
        myfree(tmp);
        packet->len++;
    }

    if (SDLNet_UDP_Send(socket_get_udp_socket(s),
                        socket_get_channel(s), packet) < 1) {
        ERR("no UDP packet sent: %s", SDLNet_GetError());
        WARN("  packet: %p", packet);
        WARN("  udp: %p", socket_get_udp_socket(s));
        WARN("  remote: %s", socket_get_remote_logname(s));
        WARN("  local: %s", socket_get_local_logname(s));
        WARN("  channel: %d", socket_get_channel(s));

        socket_count_inc_pak_tx_error(s);
    } else {
        socket_count_inc_pak_tx(s);
    }
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

    if ((ip1 == 0) && (ip2 == 0) && (ip3 == 0) && (ip4 == 0)) {
        return (dynprintf("0.0.0.0:%u", port));
    }

    if (!(hostname = SDLNet_ResolveIP(&ip))) {
        return (dynprintf("IPv4 %u.%u.%u.%u:%u",
                          hostname, ip1, ip2, ip3, ip4, port));

    }

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

        CON("  Tell           : tx %u, rx %u",
            s->tx_msg[MSG_TELL], 
            s->rx_msg[MSG_TELL]);

        CON("  Join           : tx %u, rx %u",
            s->tx_msg[MSG_CLIENT_JOIN], 
            s->rx_msg[MSG_CLIENT_JOIN]);

        CON("  Client Leave   : tx %u, rx %u",
            s->tx_msg[MSG_CLIENT_LEAVE], 
            s->rx_msg[MSG_CLIENT_LEAVE]);

        CON("  Client Close   : tx %u, rx %u",
            s->tx_msg[MSG_CLIENT_CLOSE], 
            s->rx_msg[MSG_CLIENT_CLOSE]);

        CON("  Server Shout   : tx %u, rx %u",
            s->tx_msg[MSG_SERVER_SHOUT], 
            s->rx_msg[MSG_SERVER_SHOUT]);

        CON("  Server Status  : tx %u, rx %u",
            s->tx_msg[MSG_SERVER_STATUS], 
            s->rx_msg[MSG_SERVER_STATUS]);

        CON("  Server Close   : tx %u, rx %u",
            s->tx_msg[MSG_SERVER_CLOSE], 
            s->rx_msg[MSG_SERVER_CLOSE]);

        CON("  Server map upd : tx %u, rx %u",
            s->tx_msg[MSG_SERVER_MAP_UPDATE], 
            s->rx_msg[MSG_SERVER_MAP_UPDATE]);

        CON("  Player update  : tx %u, rx %u",
            s->tx_msg[MSG_SERVER_PLAYER_UPDATE], 
            s->rx_msg[MSG_SERVER_PLAYER_UPDATE]);

        CON("  Client move    : tx %u, rx %u",
            s->tx_msg[MSG_CLIENT_PLAYER_MOVE], 
            s->rx_msg[MSG_CLIENT_PLAYER_MOVE]);

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

    CON("Name                 Quality  Latency       Local IP            Remote IP");
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

        if (total_attempts) {
            avg_latency /= total_attempts;

            CON("%-20s %3.0f pct %5d ms %-20s %-20s", 
                name,
                ((float)((float)response / (float)total_attempts)) * 100.0,
                avg_latency,
                socket_get_local_logname(s),
                socket_get_remote_logname(s));
            CON("%-20s", sock);
        } else {
            CON("%-20s                  %-20s %-20s", 
                name,
                socket_get_local_logname(s),
                socket_get_remote_logname(s));
            CON("%-20s", sock);
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

    ts = time_get_time_cached();

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
    verify(s);

    return (s->local_ip);
}

IPaddress socket_get_remote_ip (const socketp s)
{
    verify(s);

    return (s->remote_ip);
}

const char *socket_get_name (const socketp s)
{
    verify(s);

    return (s->name);
}

const char *socket_get_pclass (const socketp s)
{
    verify(s);

    return (s->pclass);
}

void socket_set_name (socketp s, const char *name)
{
    verify(s);

    if (!name) {
        memset(s->name, 0, sizeof(s->name));
    } else {
        strncpy(s->name, name, sizeof(s->name) - 1);
    }
}

void socket_set_pclass (socketp s, const char *pclass)
{
    verify(s);

    if (!pclass) {
        memset(s->pclass, 0, sizeof(s->pclass));
    } else {
        strncpy(s->pclass, pclass, sizeof(s->pclass) - 1);
    }
}

const char * socket_get_local_logname (const socketp s)
{
    verify(s);

    if (!s->local_logname) {
        s->local_logname = iptodynstr(s->local_ip);
    }

    return (s->local_logname);
}

const char * socket_get_remote_logname (const socketp s)
{
    verify(s);

    if (!s->remote_logname) {
        s->remote_logname = iptodynstr(s->remote_ip);
    }

    return (s->remote_logname);
}

boolean socket_get_server (const socketp s)
{
    verify(s);

    return (s->server);
}

boolean socket_get_client (const socketp s)
{
    verify(s);

    return (s->client);
}

boolean socket_get_server_side_client (const socketp s)
{
    verify(s);

    return (s->server_side_client);
}

msg_server_status *socket_get_server_status (const socketp s)
{
    verify(s);

    return (&s->server_status);
}

void socket_set_channel (socketp s, int c)
{
    verify(s);

    s->channel = c;
}

boolean socket_get_channel (const socketp s)
{
    verify(s);

    return (s->channel);
}

void socket_set_connected (socketp s, boolean c)
{
    verify(s);

    if (c == s->connected) {
        return;
    }

    if (socket_get_server(s)) {
        if (c) {
            LOG("Server present %s", socket_get_remote_logname(s));
        } else {
            LOG("Server disconnect from %s", socket_get_remote_logname(s));
        }
    } else if (socket_get_client(s)) {
        if (c) {
            LOG("Server present %s", socket_get_remote_logname(s));
        } else {
            LOG("Server disconnect from %s", socket_get_remote_logname(s));
        }
    } else {
        if (c) {
            LOG("Client present %s", socket_get_remote_logname(s));
        } else {
            LOG("Client disconnect from %s", socket_get_remote_logname(s));
        }
    }

    s->connected = c;

    /*
     * For aliveness checks so they see this as a new connection.
     */
    s->tx = 0;
}

boolean socket_get_connected (const socketp s)
{
    verify(s);

    return (s->connected);
}

UDPsocket socket_get_udp_socket (const socketp s)
{
    verify(s);

    return (s->udp_socket);
}

SDLNet_SocketSet socket_get_socklist (const socketp s)
{
    verify(s);

    return (s->socklist);
}

aplayerp socket_get_player (const socketp s)
{
    verify(s);

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
    verify(s);

    if (type < MSG_MAX) {
        s->rx++;
        s->rx_msg[type]++;
    } else {
        socket_count_inc_pak_rx_bad_msg(s);
    }
}

void socket_count_inc_pak_tx (const socketp s)
{
    verify(s);

    s->tx++;
}

static void socket_count_inc_pak_rx_error (const socketp s, UDPpacket *packet)
{
    verify(s);

    s->rx_error++;

    char *tmp = iptodynstr(read_address(packet));
    LOG("Bad socket message from %s", tmp);
    myfree(tmp);
}

void socket_count_inc_pak_tx_error (const socketp s)
{
    verify(s);

    s->tx_error++;
}

void socket_count_inc_pak_rx_bad_msg (const socketp s)
{
    verify(s);

    s->rx_bad_msg++;
}

void socket_tx_ping (socketp s, uint8_t seq, uint32_t ts)
{
    verify(s);

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
    verify(s);

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
    verify(s);

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
    verify(s);

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
    verify(s);

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
    strncpy(msg.pclass, s->pclass, min(sizeof(msg.pclass) - 1, strlen(s->pclass))); 

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
    verify(s);

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
    socket_set_pclass(s, msg.pclass);

    /*
     * Update the player structure.
     */
    aplayer *p = s->player;
    if (!p) {
        p = myzalloc(sizeof(*p), "player");

        socket_set_player(s, p);
    }

    memcpy(p->name, msg.name, SMALL_STRING_LEN_MAX);
    p->local_ip = s->local_ip;
    p->remote_ip = s->remote_ip;
}

boolean socket_tx_client_join (socketp s, uint32_t *key)
{
    verify(s);

    if (!socket_get_udp_socket(s)) {
        WARN("no socket to join on");
        return (false);
    }

    /*
     * Refresh the server with our name.
     */
    if (!socket_get_client(s)) {
        WARN("not a client, cannot join");
        return (false);
    }

    if (!s->connected) {
        WARN("Server is not present, cannot join yet");
        return (false);
    }

    UDPpacket *packet = socket_alloc_msg();

    msg_client_join msg = {0};
    msg.type = MSG_CLIENT_JOIN;

    *key = time_get_time_cached();
    SDLNet_Write32(*key, &msg.key);

    strncpy(msg.name, s->name, min(sizeof(msg.name) - 1, strlen(s->name))); 
    strncpy(msg.pclass, s->pclass, min(sizeof(msg.pclass) - 1, strlen(s->pclass))); 

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

boolean socket_rx_client_join (socketp s, UDPpacket *packet, uint8_t *data)
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
        LOG("Rx Join (rejected) from %s \"%s\"", tmp, msg.name);
        LOG("  current players %u", global_config.server_current_players);
        LOG("  max     players %u", global_config.server_max_players);
        myfree(tmp);

        socket_tx_tell(s, "Join rejected:", msg.name, "Too many players");
        return (false);
    }

    thing_templatep thing_template = thing_template_find(msg.pclass);
    if (!thing_template) {
        thing_template = thing_template_find_short_name(msg.pclass);
        if (!thing_template) {
            char *tmp = iptodynstr(read_address(packet));
            LOG("Rx Join (rejected) from %s \"%s\" unknown class %s", 
                tmp, msg.name, msg.pclass);
            myfree(tmp);

            socket_tx_tell(s, "Join rejected:", msg.name, "Unknown player class");
            return (false);
        }
    }

    if (debug_socket_players_enabled) {
        char *tmp = iptodynstr(read_address(packet));
        LOG("Rx Join from %s \"%s\"", tmp, msg.name);
        myfree(tmp);
    }

    socket_set_name(s, msg.name);
    socket_set_pclass(s, msg.pclass);

    /*
     * Update the player structure.
     */
    aplayer *p = s->player;
    if (!p) {
        p = myzalloc(sizeof(*p), "player");

        socket_set_player(s, p);
    }

    memcpy(p->name, msg.name, SMALL_STRING_LEN_MAX);
    memcpy(p->pclass, msg.pclass, SMALL_STRING_LEN_MAX);

    p->local_ip = s->local_ip;
    p->remote_ip = s->remote_ip;
    p->key = SDLNet_Read32(&msg.key);

    wid_game_map_server_visible();

    widp w = 
        wid_game_map_server_replace_tile(wid_game_map_server_grid_container,
                                         0, 0,
                                         thing_template);
    p->thing = wid_get_thing(w);

    return (true);
}

void socket_tx_client_leave (socketp s)
{
    verify(s);

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
    socket_set_pclass(s, 0);

    socket_tx_msg(s, packet);
        
    socket_free_msg(packet);
}

boolean socket_rx_client_leave (socketp s, UDPpacket *packet, uint8_t *data)
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
        LOG("Rx bad leave from %s", tmp);
        myfree(tmp);

        socket_tx_tell(s, "Leave rejected:", "Unknown player", "Not in game");
        return (false);
    }

    if (debug_socket_players_enabled) {
        char *tmp = iptodynstr(read_address(packet));
        LOG("Rx leave from %s", tmp);
        myfree(tmp);
    }

    return (true);
}

void socket_tx_client_close (socketp s)
{
    verify(s);

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

    LOG("Tx Client Close [to %s]", socket_get_remote_logname(s));

    packet->len = sizeof(msg);
    write_address(packet, socket_get_remote_ip(s));

    socket_tx_msg(s, packet);
        
    socket_free_msg(packet);
}

void socket_rx_client_close (socketp s, UDPpacket *packet, uint8_t *data)
{
    verify(s);

    msg_client_close msg = {0};

    if (packet->len != sizeof(msg)) {
        socket_count_inc_pak_rx_error(s, packet);
        return;
    }

    memcpy(&msg, packet->data, sizeof(msg));

    char *tmp = iptodynstr(read_address(packet));
    LOG("Rx Client Close from %s", tmp);
    myfree(tmp);

    socket_disconnect(s);
}

/*
 * The server is relaying a shout to clients.
 */
static void socket_tx_client_shout_relay (socketp s, const char *txt,
                                          socketp from)
{
    verify(s);

    if (!socket_get_udp_socket(s)) {
        return;
    }

    UDPpacket *packet = socket_alloc_msg();

    msg_client_shout msg = {0};
    msg.type = MSG_CLIENT_SHOUT;
    strncpy(msg.txt, txt, min(sizeof(msg.txt) - 1, strlen(txt))); 

    if (from && from->name) {
        char *name = from->name;
        strncpy(msg.from, name, min(sizeof(msg.from) - 1, strlen(name))); 
    }

    memcpy(packet->data, &msg, sizeof(msg));

    if (debug_socket_players_enabled) {
        LOG("Tx Client Shout [to %s] \"%s\"", 
            socket_get_remote_logname(s), txt);
    }

    packet->len = sizeof(msg);
    write_address(packet, socket_get_remote_ip(s));

    socket_tx_msg(s, packet);
        
    socket_free_msg(packet);
}

void socket_tx_client_shout (socketp s, const char *txt)
{
    verify(s);

    if (!socket_get_udp_socket(s)) {
        return;
    }

    UDPpacket *packet = socket_alloc_msg();

    msg_client_shout msg = {0};
    msg.type = MSG_CLIENT_SHOUT;
    strncpy(msg.txt, txt, min(sizeof(msg.txt) - 1, strlen(txt))); 

    memcpy(packet->data, &msg, sizeof(msg));

    if (debug_socket_players_enabled) {
        LOG("Tx Client Shout [to %s] \"%s\"", 
            socket_get_remote_logname(s), txt);
    }

    packet->len = sizeof(msg);
    write_address(packet, socket_get_remote_ip(s));

    socket_tx_msg(s, packet);
        
    socket_free_msg(packet);
}

void socket_rx_client_shout (socketp s, UDPpacket *packet, uint8_t *data)
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

    if (msg.from[0]) {
        LOG("SHOUT: \"%s\" from %s", txt, msg.from);
    } else {
        LOG("SHOUT: \"%s\"", txt);
    }

    if (debug_socket_players_enabled) {
        char *tmp = iptodynstr(read_address(packet));
        LOG("Rx Client Shout from %s \"%s\"", tmp, txt);
        myfree(tmp);
    }

    char *tmp = dynprintf("%s: \"%s\"", from, txt);
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

    /*
     * This is for relaying the shout from the server to clietns.
     */
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

        /*
         * Only talk to players who joined this server.
         */
        if (!sp->player) {
            continue;
        }

        /*
         * Include the source of the spammer.
         */
        socket_tx_client_shout_relay(sp, txt, s);
    }
}

void socket_tx_server_shout (const char *txt)
{
    UDPpacket *packet = socket_alloc_msg();

    msg_server_shout msg = {0};
    msg.type = MSG_SERVER_SHOUT;
    strncpy(msg.txt, txt, min(sizeof(msg.txt) - 1, strlen(txt))); 

    memcpy(packet->data, &msg, sizeof(msg));

    if (debug_socket_players_enabled) {
        LOG("Tx Server Shout \"%s\"", txt);
    }

    packet->len = sizeof(msg);

    socketp sp;

    TREE_WALK(sockets, sp) {
        if (!sp->connected) {
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

        write_address(packet, socket_get_remote_ip(sp));

        socket_tx_msg(sp, packet);
    }
        
    socket_free_msg(packet);
}

void socket_tx_server_shout_except_to (const char *txt, socketp except)
{
    UDPpacket *packet = socket_alloc_msg();

    msg_server_shout msg = {0};
    msg.type = MSG_SERVER_SHOUT;
    strncpy(msg.txt, txt, min(sizeof(msg.txt) - 1, strlen(txt))); 

    memcpy(packet->data, &msg, sizeof(msg));

    if (debug_socket_players_enabled) {
        LOG("Tx Server Shout \"%s\"", txt);
    }

    packet->len = sizeof(msg);

    socketp sp;

    TREE_WALK(sockets, sp) {
        if (sp == except) {
            continue;
        }

        if (!sp->connected) {
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

        write_address(packet, socket_get_remote_ip(sp));

        socket_tx_msg(sp, packet);
    }
        
    socket_free_msg(packet);
}

void socket_rx_server_shout (socketp s, UDPpacket *packet, uint8_t *data)
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

    LOG("SERVER MESSAGE: \"%s\"", txt);

    if (debug_socket_players_enabled) {
        char *tmp = iptodynstr(read_address(packet));
        LOG("Rx Server Shout from %s \"%s\"", tmp, txt);
        myfree(tmp);
    }

    char *tmp = dynprintf(" server message \"%s\"", txt);
    widp w = wid_button_transient(tmp, 0);
    color c = BLACK;
    c.a = 150;
    wid_set_color(w, WID_COLOR_BG, c);
    wid_set_color(w, WID_COLOR_TL, c);
    wid_set_color(w, WID_COLOR_BR, c);
    wid_move_to_pct_centered(w, 0.5, 0.1);
    wid_set_text_outline(w, true);
    myfree(tmp);
}

void socket_tx_tell (socketp s, 
                     const char *from,
                     const char *to,
                     const char *txt)
{
    verify(s);

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

    LOG("TELL: from \"%s\" to \"%s\" msg \"%s\"", from, to, txt);

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

    if (debug_socket_players_enabled) {
        LOG("Rx Client Tell from %s \"%s\"", from, txt);
    }

    if (!socket_get_server(s)) {
        char *tmp = dynprintf("%s: says \"%s\"", from, txt);
        widp w = wid_button_transient(tmp, 0);
        color c = BLACK;
        c.a = 150;
        wid_set_color(w, WID_COLOR_BG, c);
        wid_set_color(w, WID_COLOR_TL, c);
        wid_set_color(w, WID_COLOR_BR, c);
        wid_move_to_pct_centered(w, 0.5, 0.1);
        wid_set_text_outline(w, true);
        myfree(tmp);

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

    msg_server_status msg = {0};
    msg.type = MSG_SERVER_STATUS;

    if (global_config.user_server_name[0]) {
        strncpy(msg.server_name, global_config.user_server_name,
                min(sizeof(msg.server_name), 
                    strlen(global_config.user_server_name))); 
    } else {
        strncpy(msg.server_name, global_config.server_name,
                min(sizeof(msg.server_name), 
                    strlen(global_config.server_name))); 
    }

    msg.server_max_players = global_config.server_max_players;
    msg.server_current_players = global_config.server_current_players;

    socketp s;
    uint32_t si = 0;

    /*
     * Add all current players.
     */
    TREE_WALK(sockets, s) {
        if (!s->server_side_client) {
            continue;
        }

        aplayer *p = s->player;
        if (!s->player) {
            continue;
        }

        thingp t = p->thing;
        if (!t) {
            continue;
        }

        if (si >= MAX_PLAYERS) {
            ERR("too many players to send all in message");
            continue;
        }

        msg_player_state *msg_tx = &msg.players[si];

        strncpy(msg_tx->name, p->name, min(sizeof(msg_tx->name), 
                                           strlen(p->name))); 

        strncpy(msg_tx->pclass, p->pclass, min(sizeof(msg_tx->pclass), 
                                           strlen(p->pclass))); 

        SDLNet_Write32(p->local_ip.host, &msg_tx->local_ip.host);
        SDLNet_Write16(p->local_ip.port, &msg_tx->local_ip.port);

        SDLNet_Write32(p->remote_ip.host, &msg_tx->remote_ip.host);
        SDLNet_Write16(p->remote_ip.port, &msg_tx->remote_ip.port);

        msg_tx->quality = s->quality;
        SDLNet_Write16(s->avg_latency, &msg_tx->avg_latency);
        SDLNet_Write16(s->min_latency, &msg_tx->min_latency);
        SDLNet_Write16(s->max_latency, &msg_tx->max_latency);

        SDLNet_Write32(t->score, &msg_tx->score);
        SDLNet_Write32(t->health, &msg_tx->health);
        SDLNet_Write16(t->tree.key, &msg_tx->thing_id);

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
                LOG("Tx Status [to %s]",
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
                              msg_server_status *status)
{
    verify(s);

    msg_server_status *msg;

    if (packet->len != sizeof(*msg)) {
        socket_count_inc_pak_rx_error(s, packet);
        return;
    }

    uint32_t pi;

    msg = (typeof(msg)) packet->data;

    for (pi = 0; pi < MAX_PLAYERS; pi++) {
        msg_player_state *p = &status->players[pi];
        msg_player_state *msg_rx = &msg->players[pi];

        memcpy(p->name, msg_rx->name, SMALL_STRING_LEN_MAX);
        memcpy(p->pclass, msg_rx->pclass, SMALL_STRING_LEN_MAX);

        p->local_ip.host = SDLNet_Read32(&msg_rx->local_ip.host);
        p->local_ip.port = SDLNet_Read16(&msg_rx->local_ip.port);

        p->remote_ip.host = SDLNet_Read32(&msg_rx->remote_ip.host);
        p->remote_ip.port = SDLNet_Read16(&msg_rx->remote_ip.port);

        p->quality = msg_rx->quality;

        p->avg_latency = SDLNet_Read16(&msg_rx->avg_latency);
        p->min_latency = SDLNet_Read16(&msg_rx->min_latency);
        p->max_latency = SDLNet_Read16(&msg_rx->max_latency);

        p->score = SDLNet_Read32(&msg_rx->score);
        p->health = SDLNet_Read32(&msg_rx->health);
        p->thing_id = SDLNet_Read16(&msg_rx->thing_id);

        p->key = SDLNet_Read32(&msg_rx->key);

        if (!p->name[0]) {
            continue;
        }

        if (debug_socket_players_enabled) {
            char *tmp = iptodynstr(read_address(packet));
            LOG("Rx Status from %s %u:\"%s\"", tmp, pi, p->name);
            myfree(tmp);
        }
    }

    status->server_max_players = msg->server_max_players;
    status->server_current_players = msg->server_current_players;
    memcpy(status->server_name, msg->server_name, sizeof(status->server_name));

    memcpy(&s->server_status, status, sizeof(s->server_status));
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
    verify(s);

    msg_server_close *msg;

    if (packet->len != sizeof(*msg)) {
        socket_count_inc_pak_rx_error(s, packet);
        return;
    }

    char *tmp = iptodynstr(read_address(packet));
    LOG("Rx Server down from %s", tmp);
    myfree(tmp);

    socket_set_connected(s, false);
}

uint32_t socket_get_quality (socketp s)
{
    verify(s);

    return (s->quality);
}

uint32_t socket_get_avg_latency (socketp s)
{
    verify(s);

    return (s->avg_latency);
}

uint32_t socket_get_min_latency (socketp s)
{
    verify(s);

    return (s->min_latency);
}

uint32_t socket_get_max_latency (socketp s)
{
    verify(s);

    return (s->max_latency);
}

uint32_t socket_get_rx (socketp s)
{
    verify(s);

    return (s->min_latency);
}

uint32_t socket_get_tx (socketp s)
{
    verify(s);

    return (s->tx);
}

uint32_t socket_get_rx_error (socketp s)
{
    verify(s);

    return (s->min_latency);
}

uint32_t socket_get_tx_error (socketp s)
{
    verify(s);

    return (s->min_latency);
}

uint32_t socket_get_rx_bad_msg (socketp s)
{
    verify(s);

    return (s->min_latency);
}

void socket_tx_client_move (socketp s, 
                            thingp t,
                            const boolean up,
                            const boolean down,
                            const boolean left, 
                            const boolean right)
{
    if (!socket_get_udp_socket(s)) {
        return;
    }

    static uint32_t ts;

    if (!time_have_x_hundredths_passed_since(
                            DELAY_HUNDREDTHS_PLAYER_TX_POSITION, ts)) {
        return;
    }

    ts = time_get_time_cached();

    msg_client_move msg = {0};
    msg.type = MSG_CLIENT_PLAYER_MOVE;
    msg.dir = (up << 3) | (down << 2) | (left << 1) | right;

    SDLNet_Write16(t->x * THING_COORD_SCALE, &msg.x);               
    SDLNet_Write16(t->y * THING_COORD_SCALE, &msg.y);               

    UDPpacket *packet = socket_alloc_msg();

    memcpy(packet->data, &msg, sizeof(msg));

    packet->len = sizeof(msg);
    write_address(packet, socket_get_remote_ip(s));

    socket_tx_msg(s, packet);
        
    socket_free_msg(packet);
}

void socket_server_rx_client_move (socketp s, UDPpacket *packet, uint8_t *data)
{
    verify(s);

    msg_client_move msg = {0};

    if (packet->len != sizeof(msg)) {
        socket_count_inc_pak_rx_error(s, packet);
        return;
    }

    memcpy(&msg, packet->data, sizeof(msg));

    aplayer *p = s->player;
    if (!p) {
        return;
    }

    const boolean up    = (msg.dir & (1 << 3)) ? 1 : 0;
    const boolean down  = (msg.dir & (1 << 2)) ? 1 : 0;
    const boolean left  = (msg.dir & (1 << 1)) ? 1 : 0;
    const boolean right = (msg.dir & (1 << 0)) ? 1 : 0;

    thingp t = p->thing;
    if (!t) {
        return;
    }

    msg.x = SDLNet_Read16(&msg.x);
    msg.y = SDLNet_Read16(&msg.y);

    double x = ((double)msg.x) / THING_COORD_SCALE;
    double y = ((double)msg.y) / THING_COORD_SCALE;

    thing_server_move(t, x, y, up, down, left, right);
}
