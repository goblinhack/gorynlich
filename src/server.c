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
#include "server.h"
#include "time.h"
#include "slre.h"
#include "command.h"
#include "player.h"
#include "wid_game_map.h"
#include "string.h"

static boolean server_init_done;
static socketp server_socket;

static boolean server_players_show(tokens_t *tokens, void *context);
static boolean server_shout(tokens_t *tokens, void *context);

boolean server_init (void)
{
    if (!is_server) {
        return (true);
    }

    if (server_init_done) {
        return (true);
    }

    socketp s;

    s = socket_listen(server_address);
    if (!s) {
        ERR("Server failed to listen");
        return (false);
    }

    LOG("Server listening on %s", socket_get_local_logname(s));

    server_socket = s;

    if (is_client) {
        command_add(server_players_show, "show server players", 
                    "show all players state");
    } else {
        command_add(server_players_show, "show players", 
                    "show all players state");
    }

    command_add(server_shout, "notify [A-Za-z0-9_-]*",
                "shout to all players");

    server_init_done = true;

    return (true);
}

void server_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (server_init_done) {
        server_init_done = false;

        socket_tx_server_shout("SERVER GOING DOWN");

        socket_tx_server_close();
    }
}

static void server_rx_client_join (socketp s)
{
    aplayerp p = socket_get_player(s);
    if (!p) {
        WARN("Received join %s, but no player on socket",
             socket_get_remote_logname(s));
        return;
    }

    LOG("\"%s\" joined the game from %s", p->name,
        socket_get_remote_logname(s));

    char *tmp = dynprintf("%s joined the game", p->name);
    socket_tx_server_shout(tmp);
    myfree(tmp);

    wid_game_visible();
}

static void server_rx_client_leave_implicit (socketp s)
{
    aplayerp p = socket_get_player(s);
    if (!p) {
        return;
    }

    wid_game_map_wid_destroy();

    socket_set_player(s, 0);
}

static void server_rx_client_leave (socketp s)
{
    aplayerp p = socket_get_player(s);
    if (!p) {
        WARN("Received leave %s, but no player on socket",
             socket_get_remote_logname(s));
        return;
    }

    LOG("\"%s\" left the game from %s", p->name,
        socket_get_remote_logname(s));

    char *tmp = dynprintf("%s left the game", p->name);
    socket_tx_server_shout(tmp);
    myfree(tmp);

    server_rx_client_leave_implicit(s);
}

static void server_rx_client_close (socketp s)
{
    aplayerp p = socket_get_player(s);
    if (!p) {
        /*
         * Not an error. Just not in our game.
         */
        return;
    }

    LOG("\"%s\" suddenly left of the game from %s", p->name,
        socket_get_remote_logname(s));

    char *tmp = dynprintf("%s suddenly left the game", p->name);
    socket_tx_server_shout(tmp);
    myfree(tmp);

    server_rx_client_leave_implicit(s);
}

static void server_poll (void)
{
    socketp s = server_socket;

    if (!s) {
        ERR("no server socket to listen on");
        return;
    }

    if (!socket_get_socklist(s)) {
        ERR("no socklist to listen on");
        return;
    }

    int waittime = 0;
    int numready = SDLNet_CheckSockets(socket_get_socklist(s), waittime);
    if (numready <= 0) {
        return;
    }

    UDPpacket *packet = SDLNet_AllocPacket(MAX_PACKET_SIZE);
    if (!packet) {
        ERR("out of packet space, pak %d", MAX_PACKET_SIZE);
        return;
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

        socketp s = socket_find_remote_ip(read_address(packet));
        if (!s) {

            char *tmp = iptodynstr(read_address(packet));
            LOG("Server new client from %s", tmp);
            myfree(tmp);

            s = socket_connect_from_server(read_address(packet));
            if (!s) {
                continue;
            }
        }

        uint8_t *data = packet->data;
        msg_type type = *data++;

        socket_count_inc_pak_rx(s, type);

        switch (type) {
        case MSG_PING:
            socket_rx_ping(s, packet, data);

            socket_tx_server_status();
            break;

        case MSG_PONG:
            socket_rx_pong(s, packet, data);
            break;

        case MSG_NAME:
            socket_rx_name(s, packet, data);
            break;

        case MSG_CLIENT_JOIN:
            if (socket_rx_client_join(s, packet, data)) {
                server_rx_client_join(s);
            }
            break;

        case MSG_CLIENT_LEAVE:
            if (socket_rx_client_leave(s, packet, data)) {
                server_rx_client_leave(s);
            }
            break;

        case MSG_CLIENT_CLOSE:
            server_rx_client_close(s);
            socket_rx_client_close(s, packet, data);
            break;

        case MSG_CLIENT_SHOUT:
            socket_rx_client_shout(s, packet, data);
            break;

        case MSG_TELL:
            socket_rx_tell(s, packet, data);
            break;

        default:
            ERR("Unknown message type received [%u", type);
        }
    }

    SDLNet_FreePacket(packet);
}

static void server_alive_check (void)
{
    socketp s;

    sockets_quality_check();

    TREE_WALK(sockets, s) {
        if (!socket_get_server_side_client(s)) {
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
            aplayerp p = socket_get_player(s);

            if (p) {
                char *tmp = dynprintf("%s connection dropped", p->name);
                socket_tx_client_shout(s, tmp);
                myfree(tmp);

                LOG("\"%s\" dropped out of the game from %s", p->name,
                    socket_get_remote_logname(s));
            }

            server_rx_client_leave_implicit(s);

            socket_disconnect(s);
        }
    }
}

static void server_socket_tx_ping (void)
{
    static uint32_t ts;
    static uint8_t seq;

    if (!time_have_x_tenths_passed_since(10, ts)) {
        return;
    }

    /*
     * Every few seconds check for dead peers.
     */
    if (ts && (!(seq % 3))) {
        server_alive_check();
    }

    ts = time_get_time_cached();

    socketp s;

    TREE_WALK(sockets, s) {

        if (socket_get_server(s)) {
            continue;
        }

        ts = time_get_time_cached();
        socket_tx_ping(s, seq, ts);
    }

    seq++;
}

void server_tick (void)
{
    if (!is_server) {
        return;
    }

    server_poll();
    server_socket_tx_ping();
}

/*
 * User has entered a command, run it
 */
static boolean server_players_show (tokens_t *tokens, void *context)
{
    CON("Name           Quality  Latency      Remote IP      Local IP     Score ");
    CON("----           -------  ------- --------------- --------------- -------");

    uint32_t pi;

    pi = 0;

    socketp s;
    TREE_WALK(sockets, s) {

        aplayer *p = socket_get_player(s);
        if (!p) {
            continue;
        }

        if (!p->name[0]) {
            continue;
        }

        char *tmp = iptodynstr(p->local_ip);
        char *tmp2 = iptodynstr(p->remote_ip);

        pi++;

        CON("[%d] %-10s %3d pct %5d ms %-15s %-15s %07d", 
            pi,
            p->name,
            p->quality,
            p->avg_latency,
            tmp,
            tmp2,
            p->score);

        myfree(tmp2);
        myfree(tmp);
    }

    return (true);
}

/*
 * User has entered a command, run it
 */
boolean server_shout (tokens_t *tokens, void *context)
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

    socket_tx_server_shout(shout);

    myfree(tmp);

    return (true);
}
