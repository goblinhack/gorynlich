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
#include "tree.h"
#include "wid_server.h"

/*
 * Which socket we have actually joined on.
 */
socketp client_joined_server;
static uint32_t client_joined_server_key;

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
static boolean client_socket_shout(char *shout);
static boolean client_socket_tell(char *from, char *to, char *msg);
static boolean client_socket_set_name(char *name);
static void client_check_still_in_game(void);

static aplayer client_players[MAX_PLAYERS];

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

    LOG("Client trying to connect to %s", socket_get_remote_logname(s));

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

    if (!global_config.name[0]) {
        strncpy(global_config.name, "nameless", 
                sizeof(global_config.name) - 1);
    }

    client_init_done = true;

    return (true);
}

void client_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (client_init_done) {
        client_init_done = false;

        if (client_joined_server) {
            verify(client_joined_server);

            socket_tx_client_close(client_joined_server);
        }
    }
}

static void client_alive_check (void)
{
    socketp s;

    sockets_quality_check();

    TREE_WALK(sockets, s) {
        if (!socket_get_client(s)) {
            continue;
        }

        /*
         * Don't kill off new born connections.
         */
        if (socket_get_tx(s) < 10) {
            continue;
        }

        if (socket_get_quality(s) <= SOCKET_PING_FAIL_THRESHOLD) {
            /*
             * Clients try forever. Server clients disconnect.
             */
            socket_set_connected(s, false);

            if (client_joined_server == s) {
                verify(client_joined_server);

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

    socketp s;

    TREE_WALK(sockets, s) {
        if (!socket_get_client(s)) {
            continue;
        }

        ts = time_get_time_cached();
        socket_tx_ping(s, seq, ts);
    }

    /*
     * Every few seconds check for dead peers.
     */
    if (ts && !(seq % 3)) {
        client_alive_check();
    }

    wid_server_redo(true /* soft refresh */);

    seq++;
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
        WARN("need to set a name");
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

    return (true);
}

static boolean client_socket_close (char *host, char *port)
{
    uint32_t portno;
    socketp s = 0;

    if (!host && !port) {
        TREE_WALK(sockets, s) {
            break;
        }

        if (!s) {
            WARN("Do not know which socket to close");
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
        verify(client_joined_server);

        client_socket_leave();
    }

    LOG("Client disconnecting %s", socket_get_remote_logname(s));

    socket_tx_client_close(s);

    socket_disconnect(s);

    return (true);
}

boolean client_socket_join (char *host, char *port, uint16_t portno)
{
    if (client_joined_server) {
        WARN("Leave the current server first");
        return (false);
    }

    socketp s = 0;

    if (!host && !port) {
        TREE_WALK(sockets, s) {
            break;
        }

        if (!s) {
            WARN("Do not know which socket to join");
            return (false);
        }
    } else {
        if (!host || !*host) {
            host = SERVER_DEFAULT_HOST;
        }

        if (!portno) {
            if (port && *port) {
                portno = atoi(port);
            } else {
                portno = SERVER_DEFAULT_PORT;
            }
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

    socket_set_name(s, global_config.name);

    if (!socket_tx_client_join(s, &client_joined_server_key)) {
        return (false);
    }

    LOG("Client joining %s", socket_get_remote_logname(s));

    client_joined_server = s;

    verify(client_joined_server);

    return (true);
}

static boolean client_socket_leave_implicit (void)
{
    if (!client_joined_server) {
        return (false);
    }

    LOG("YOU WERE DROPPED FROM THE SERVER");

    client_joined_server = 0;

    memset(client_players, 0, sizeof(client_players));

    return (true);
}

boolean client_socket_leave (void)
{
    if (!client_joined_server) {
        WARN("Join a server first before trying to leave");
        return (false);
    }

    verify(client_joined_server);

    LOG("Client leaving %s", 
        socket_get_remote_logname(client_joined_server));

    socket_tx_client_leave(client_joined_server);

    client_joined_server = 0;

    memset(client_players, 0, sizeof(client_players));

    return (true);
}

static boolean client_socket_shout (char *shout)
{
    if (!client_joined_server) {
        WARN("Join a server first before trying to shout");
        return (false);
    }

    verify(client_joined_server);

    if (!shout || !*shout) {
        WARN("No message to shout");
        return (false);
    }

    socket_tx_client_shout(client_joined_server, shout);

    return (true);
}

static boolean client_socket_tell (char *from, char *to, char *msg)
{
    if (!client_joined_server) {
        WARN("Join a server first before trying to speak");
        return (false);
    }

    verify(client_joined_server);

    if (!from || !*from) {
        WARN("no sender for tell");
        return (false);
    }

    if (!to || !*to) {
        WARN("no recipient to tell");
        return (false);
    }

    if (!msg || !*msg) {
        WARN("no message to tell");
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
    if (!name || !*name) {
        WARN("need to set a name");
        return (false);
    }

    strncpy(global_config.name, name, sizeof(global_config.name) - 1);

    CON("Client name set to \"%s\"", name);

    if (client_joined_server) {
        socket_set_name(client_joined_server, name);

        socket_tx_name(client_joined_server);
    }

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

    boolean r = client_socket_join(host, port, 0);

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
        WARN("no message");
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
    char to[PLAYER_NAME_LEN_MAX] = {0};
    uint32_t i = 1;
    char *tmp;

    tmp = 0;

    char *s = tokens->args[i++];
    if (!s || !*s) {
        WARN("no one to tell");
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
        WARN("no message");
        return (false);
    }

    boolean r = client_socket_tell(global_config.name, to, tmp);

    myfree(tmp);

    return (r);
}

static void client_poll (void)
{
    socketp s;

    if (client_joined_server) {
        verify(client_joined_server);
    }

    TREE_WALK(sockets, s) {
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
            case MSG_PONG:
                socket_rx_pong(s, packet, data);
                break;

            case MSG_PING:
                socket_rx_ping(s, packet, data);
                break;

            case MSG_CLIENT_SHOUT:
                socket_rx_client_shout(s, packet, data);
                break;

            case MSG_SERVER_SHOUT:
                socket_rx_server_shout(s, packet, data);
                break;

            case MSG_TELL:
                socket_rx_tell(s, packet, data);
                break;

            case MSG_SERVER_STATUS:
                if (s == client_joined_server) {
                    socket_rx_server_status(s, packet, data, 
                                    &client_players[0]);

                    client_check_still_in_game();
                }

                break;

            case MSG_SERVER_CLOSE:
                socket_rx_server_close(s, packet, data);

                client_socket_leave_implicit();
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
    CON("Name           Quality  Latency      Remote IP      Local IP     Score ");
    CON("----           -------  ------- --------------- --------------- -------");

    uint32_t pi;

    for (pi = 0; pi < MAX_PLAYERS; pi++) {
        aplayer *p = &client_players[pi];

        char *tmp = iptodynstr(p->local_ip);
        char *tmp2 = iptodynstr(p->remote_ip);

        CON("[%d] %-10s %3d pct %5d ms %-15s %-15s %07d", 
            pi, 
            p->name,
            p->quality,
            p->avg_latency,
            tmp,
            tmp2,
            p->score);

        myfree(tmp);
        myfree(tmp2);
    }

    return (true);
}

static void client_check_still_in_game (void)
{
    uint32_t pi;

    if (!client_joined_server) {
        return;
    }

    verify(client_joined_server);

    if (!socket_get_connected(client_joined_server)) {
        return;
    }

    for (pi = 0; pi < MAX_PLAYERS; pi++) {
        aplayer *p = &client_players[pi];

        if (!p->name[0]) {
            continue;
        }

        if (p->key != client_joined_server_key) {
            continue;
        }

        if (strcmp(p->name, global_config.name)) {
            continue;
        }

        if (!p->connection_confrimed) {
            p->connection_confrimed = true;
            LOG("Server has added you, \"%s\" to the game", 
                global_config.name);
        }

        return;
    }

    LOG("Server does not report you in the game!");

    LOG("  You are player: \"%s\", ID %u", 
        global_config.name, client_joined_server_key);

    for (pi = 0; pi < MAX_PLAYERS; pi++) {
        aplayer *p = &client_players[pi];

        LOG("  Player %u is \"%s\", ID %u ", pi+1, p->name, p->key);
    }
}

