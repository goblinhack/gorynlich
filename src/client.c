/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 *
 * A useful hack to help debug stale/free/bad pointers.
 */


#include "SDL_timer.h"

#include "main.h"
#include "socket.h"
#include "client.h"
#include "time.h"
#include "slre.h"
#include "command.h"
#include "player.h"
#include "string.h"

/*
 * Which socket we have actually joined on.
 */
static socketp client_joined_server;

static void client_socket_tx_ping(void);
static boolean client_init_done;
static void client_poll(void);
static boolean client_set_name(tokens_t *tokens, void *context);
static boolean client_shout(tokens_t *tokens, void *context);
static boolean client_tell(tokens_t *tokens, void *context);
static boolean client_players_show(tokens_t *tokens, void *context);
static boolean client_join(tokens_t *tokens, void *context);
static boolean client_leave(tokens_t *tokens, void *context);
static boolean client_open(tokens_t *tokens, void *context);
static boolean client_close(tokens_t *tokens, void *context);
static boolean client_socket_leave(void);
static boolean client_socket_shout(char *shout);
static boolean client_socket_tell(char *from, char *to, char *msg);
static boolean client_socket_join(char *host, char *port);
static boolean client_socket_set_name(char *name);

aplayer client_players[MAX_SOCKETS];
static char client_name[PLAYER_NAME_MAX];
static boolean joined;

boolean client_init (void)
{
    if (!is_client) {
        return (true);
    }

    if (client_init_done) {
        return (true);
    }

    socketp s = 0;

    /*
     * Connector.
     */
    s = socket_connect(server_address, false /* client side */);
    if (!s) {
        WARN("Client failed to connect");
        return (false);
    }

    LOG("Client connecting to   %s", socket_get_remote_logname(s));
    LOG("                  from %s", socket_get_local_logname(s));

    command_add(client_set_name, "set name [A-Za-z0-9_-]*",
                "set player name");

    command_add(client_open, "open [A-Za-z0-9_-.]* [0-9_-]*",
                "loosely connect to server <ip> <port>");

    command_add(client_close, "close [A-Za-z0-9_-.]* [0-9_-]*",
                "close a connect to server <ip> <port>");

    command_add(client_join, "join [A-Za-z0-9_-.]* [0-9_-]*",
                "join server <ip> <port>");

    command_add(client_leave, "leave current server game",
                "leave game");

    command_add(client_shout, "shout [A-Za-z0-9_-]*",
                "shout to players");

    command_add(client_tell, "tell [A-Za-z0-9_-]* [A-Za-z0-9_-]*",
                "send a message to a player");

    command_add(client_players_show, "show players", 
                "show all players state");

    strncpy(client_name, "nameless", sizeof(client_name) - 1);

    client_init_done = true;

    return (true);
}

void client_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (client_init_done) {
        client_init_done = false;
    }
}

static void client_alive_check (void)
{
    uint32_t si;

    sockets_quality_check();

    for (si = 0; si < MAX_SOCKETS; si++) {
        const socketp s = socket_get(si);

        if (!socket_get_open(s)) {
            continue;
        }

        if (!socket_get_client(s)) {
            continue;
        }

        /*
         * Don't kill off new born connections.
         */
        if (socket_get_tx(s) < 10) {
            continue;
        }

        if (socket_get_quality(s) < SOCKET_PING_FAIL_THRESHOLD) {
            /*
             * Clients try forever. Server clients disconnect.
             */
            LOG("Server down [%s] qual %u percent",
                socket_get_remote_logname(s), socket_get_quality(s));

            if (client_joined_server == s) {
                client_socket_leave();
            }
        }
    }
}

static void client_socket_tx_ping (void)
{
    static uint32_t ts;
    static uint8_t seq;

    if (!time_have_x_tenths_passed_since(10, ts)) {
        return;
    }

    uint32_t si;

    for (si = 0; si < MAX_SOCKETS; si++) {
        socketp s = socket_get(si);
        if (!s) {
            continue;
        }

        if (!socket_get_client(s)) {
            continue;
        }

        ts = time_get_time_cached();
        socket_tx_ping(s, seq, ts);
    }

    seq++;

    /*
     * Every 10 seconds check for dead peers.
     */
    if (seq % 10) {
        client_alive_check();
    }
}

void client_tick (void)
{
    if (!is_client) {
        return;
    }

    client_poll();
    client_socket_tx_ping();
}

/*
 * User has entered a command, run it
 */
static boolean client_set_name (tokens_t *tokens, void *context)
{
    char *s = tokens->args[2];

    if (!s || !*s) {
        ERR("need to set a name");
        return (false);
    }

    boolean r = client_socket_set_name(s);

    return (r);
}

static boolean client_socket_open (char *host, char *port)
{
    uint32_t portno;
    socketp s = 0;

    if (!host || !*host) {
        host = SERVER_DEFAULT_HOST;
    }

    if (port && *port) {
        portno = atoi(port);
    } else {
        portno = SERVER_DEFAULT_PORT;
    }

    if (SDLNet_ResolveHost(&server_address, host, portno)) {
        ERR("cannot resolve host %s port %u", host, portno);
        return (false);
    }

    /*
     * Connector.
     */
    s = socket_connect(server_address, false /* client side */);
    if (!s) {
        WARN("Client failed to connect");
        return (false);
    }

    LOG("Client connecting to %s", socket_get_remote_logname(s));
    LOG("                from %s", socket_get_local_logname(s));

    return (true);
}

static boolean client_socket_close (char *host, char *port)
{
    uint32_t portno;
    socketp s = 0;

    if (!host && !port) {
        uint32_t si;

        for (si = 0; si < MAX_SOCKETS; si++) {
            s = socket_get(si);
            if (!s) {
                continue;
            }

            if (socket_get_open(s)) {
                break;
            }
        }

        if (si == MAX_SOCKETS) {
            ERR("do not know which socket to close");
            return (false);
        }
    } else {
        if (!host || !*host) {
            host = SERVER_DEFAULT_HOST;
        }

        if (port && *port) {
            portno = atoi(port);
        } else {
            portno = SERVER_DEFAULT_PORT;
        }

        if (SDLNet_ResolveHost(&server_address, host, portno)) {
            ERR("cannot resolve host %s port %u", host, portno);
            return (false);
        }

        /*
         * Connector.
         */
        s = socket_find_remote_ip(server_address);
        if (!s) {
            WARN("Client failed to connect");
            return (false);
        }
    }

    if (client_joined_server == s) {
        client_socket_leave();
    }

    LOG("Client disconnecting %s", socket_get_remote_logname(s));
    LOG("                from %s", socket_get_local_logname(s));

    socket_tx_close(s);

    socket_disconnect(s);

    return (true);
}

static boolean client_socket_join (char *host, char *port)
{
    if (client_joined_server) {
        ERR("Leave the current server first");
        return (false);
    }

    uint32_t portno;
    socketp s = 0;

    if (!host && !port) {
        uint32_t si;

        for (si = 0; si < MAX_SOCKETS; si++) {
            s = socket_get(si);
            if (!s) {
                continue;
            }

            if (socket_get_open(s)) {
                break;
            }
        }

        if (si == MAX_SOCKETS) {
            ERR("do not know which socket to join on");
            return (false);
        }
    } else {
        if (!host || !*host) {
            host = SERVER_DEFAULT_HOST;
        }

        if (port && *port) {
            portno = atoi(port);
        } else {
            portno = SERVER_DEFAULT_PORT;
        }

        if (SDLNet_ResolveHost(&server_address, host, portno)) {
            ERR("cannot resolve host %s port %u", host, portno);
            return (false);
        }

        /*
         * Connector.
         */
        s = socket_connect(server_address, false /* client side */);
        if (!s) {
            WARN("Client failed to connect");
            return (false);
        }
    }

    socket_set_name(s, client_name);

    socket_tx_join(s);

    LOG("Client joining %s", socket_get_remote_logname(s));
    LOG("          from %s", socket_get_local_logname(s));

    client_joined_server = s;

    joined = true;

    return (true);
}

static boolean client_socket_leave (void)
{
    if (!client_joined_server) {
        ERR("Join a server first");
        return (false);
    }

    LOG("Client leaving %s", 
        socket_get_remote_logname(client_joined_server));

    socket_tx_leave(client_joined_server);

    client_joined_server = 0;

    joined = true;

    return (true);
}

static boolean client_socket_shout (char *shout)
{
    if (!client_joined_server) {
        ERR("Join a server first");
        return (false);
    }

    if (!shout || !*shout) {
        ERR("no message");
        return (false);
    }

    socket_tx_shout(client_joined_server, shout);

    return (true);
}

static boolean client_socket_tell (char *from, char *to, char *msg)
{
    if (!client_joined_server) {
        ERR("Join a server first");
        return (false);
    }

    if (!from || !*from) {
        ERR("no sender");
        return (false);
    }

    if (!to || !*to) {
        ERR("no recipient");
        return (false);
    }
    if (!msg || !*msg) {
        ERR("no message");
        return (false);
    }

    socket_tx_tell(client_joined_server, from, to, msg);

    return (true);
}

/*
 * User has entered a command, run it
 */
static boolean client_socket_set_name (char *name)
{
    if (!client_joined_server) {
        ERR("Join a server first");
        return (false);
    }

    if (!name || !*name) {
        ERR("need to set a name");
        return (false);
    }

    CON("Client name set to \"%s\"", 
        socket_get_name(client_joined_server));

    socket_set_name(client_joined_server, name);

    return (true);
}

/*
 * User has entered a command, run it
 */
boolean client_open (tokens_t *tokens, void *context)
{
    char *host = tokens->args[1];
    char *port = tokens->args[2];

    boolean r = client_socket_open(host, port);

    return (r);
}

/*
 * User has entered a command, run it
 */
boolean client_close (tokens_t *tokens, void *context)
{
    char *host = tokens->args[1];
    char *port = tokens->args[2];

    boolean r = client_socket_close(host, port);

    return (r);
}

/*
 * User has entered a command, run it
 */
boolean client_join (tokens_t *tokens, void *context)
{
    char *host = tokens->args[1];
    char *port = tokens->args[2];

    boolean r = client_socket_join(host, port);

    return (r);
}

/*
 * User has entered a command, run it
 */
boolean client_leave (tokens_t *tokens, void *context)
{
    boolean r = client_socket_leave();

    return (r);
}

/*
 * User has entered a command, run it
 */
boolean client_shout (tokens_t *tokens, void *context)
{
    char shout[PLAYER_MSG_MAX] = {0};
    uint32_t i = 1;
    char *tmp;

    tmp = 0;

    for (;;) {

        char *s = tokens->args[i++];
        if (!s || !*s) {
            break;
        }

        if (tmp) {
            char *n = dynprintf("%s %s", tmp, s);
            myfree(tmp);
            tmp = n;
        } else {
            tmp = dynprintf("%s", s);
        }
    }

    if (!tmp || !*tmp) {
        ERR("no message");
        return (false);
    }

    strncpy(shout, tmp, sizeof(shout) - 1);

    boolean r = client_socket_shout(shout);

    myfree(tmp);

    return (r);
}

/*
 * User has entered a command, run it
 */
boolean client_tell (tokens_t *tokens, void *context)
{
    char from[PLAYER_NAME_MAX] = {0};
    char to[PLAYER_NAME_MAX] = {0};
    char msg[PLAYER_MSG_MAX] = {0};
    uint32_t i = 1;
    char *tmp;

    tmp = 0;

    char *s = tokens->args[i++];
    if (!s || !*s) {
        ERR("no one to tell");
        return (false);
    }

    strncpy(to, s, sizeof(to) - 1);

    for (;;) {

        char *s = tokens->args[i++];
        if (!s || !*s) {
            break;
        }

        if (tmp) {
            char *n = dynprintf("%s %s", tmp, s);
            myfree(tmp);
            tmp = n;
        } else {
            tmp = dynprintf("%s", s);
        }
    }

    if (!tmp || !*tmp) {
        ERR("no message");
        return (false);
    }

    boolean r = client_socket_tell(from, to, msg);

    myfree(tmp);

    return (r);
}

static void client_poll (void)
{
    uint32_t si;

    for (si = 0; si < MAX_SOCKETS; si++) {
        socketp s = socket_get(si);
        if (!s) {
            continue;
        }

        if (!socket_get_open(s)) {
            continue;
        }

        if (!socket_get_client(s)) {
            continue;
        }

        if (!socket_get_socklist(s)) {
            continue;
        }

        int waittime = 0;
        int numready = SDLNet_CheckSockets(socket_get_socklist(s), waittime);
        if (numready <= 0) {
            continue;
        }

        UDPpacket *packet;      

        packet = SDLNet_AllocPacket(MAX_PACKET_SIZE);
        if (!packet) {
            ERR("Out of packet space, pak %d", MAX_PACKET_SIZE);
            continue;
        }

        int i;
        for (i = 0; i < numready; i++) {
            if (!SDLNet_SocketReady(socket_get_udp_socket(s))) {
                continue;
            }

            int paks = SDLNet_UDP_Recv(socket_get_udp_socket(s), packet);
            if (paks != 1) {
                ERR("Pak rx failed: %s", SDLNet_GetError());
                continue;
            }

            uint8_t *data = packet->data;
            msg_type type = *data++;

            socket_count_inc_pak_rx(s, type);

            switch (type) {
            case MSG_TYPE_PONG:
                socket_rx_pong(s, packet, data);
                break;

            case MSG_TYPE_PING:
                socket_rx_ping(s, packet, data);
                break;

            case MSG_TYPE_SHOUT:
                socket_rx_shout(s, packet, data);
                break;

            case MSG_TYPE_TELL:
                socket_rx_tell(s, packet, data);
                break;

            case MSG_TYPE_PLAYERS_ALL:
                socket_rx_players_all(s, packet, data, &client_players[0]);
                break;

            default:
                ERR("Unknown message type received [%u]", type);
            }
        }

        SDLNet_FreePacket(packet);
    }
}

/*
 * User has entered a command, run it
 */
static boolean client_players_show (tokens_t *tokens, void *context)
{
    CON("Name                 Quality  Latency       Remote IP       Score ");
    CON("----                 -------  ------- -------------------- -------");

    uint32_t si;

    for (si = 0; si < MAX_SOCKETS; si++) {
        aplayer *p = &client_players[si];

        if (!p->name[0]) {
            continue;
        }

        char *tmp2 = iptodynstr(p->remote_ip);

        CON("%-20s %3d pct %5d ms %-20s %07d", 
            p->name,
            p->quality,
            p->avg_latency,
            tmp2,
            p->score);

        myfree(tmp2);

    }

    return (true);
}
