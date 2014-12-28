/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include "SDL_timer.h"

#include "main.h"
#include "socket_util.h"
#include "server.h"
#include "time_util.h"
#include "slre.h"
#include "command.h"
#include "player.h"
#include "wid_game_map_server.h"
#include "wid_server_create.h"
#include "string_util.h"
#include "thing.h"
#include "mzip_lib.h"

static uint8_t server_init_done;
gsocketp server_socket;

static uint8_t server_players_show(tokens_t *tokens, void *context);
static uint8_t server_shout(tokens_t *tokens, void *context);

uint8_t server_start (IPaddress address)
{
    if (server_socket) {
        LOG("Server is already running; cannot restart");
        return (true);
    }

    gsocketp s;

    address.host = 0;

    s = socket_listen(address);
    if (!s) {
        char *tmp = iptodynstr(address);
        MSG_BOX("Failed to start server on %s", tmp);
        myfree(tmp);
        return (false);
    }

    LOG("Server: Listening on %s", socket_get_local_logname(s));

    server_socket = s;

    on_server = true;

    return (true);
}

void server_stop (void)
{
    if (!server_socket) {
        return;
    }

    socket_tx_server_shout_at_all_players(CRITICAL, "SERVER GOING DOWN");

    socket_tx_server_close();

    socket_disconnect(server_socket);
    server_socket = 0;

    on_server = false;
}

uint8_t server_init (void)
{
    if (server_init_done) {
        return (true);
    }

    command_add(server_players_show, "show server players", 
                "show all players state");

    command_add(server_shout, "notify [A-Za-z0-9_-]*",
                "shout to all players");

    uint16_t portno;

    portno = global_config.user_server_port;
    if (!portno) {
        portno = global_config.server_port;
        if (!portno) {
            portno = SERVER_DEFAULT_PORT;
        }
    }

    LOG("Server: Trying to resolve server address %s:%u", 
        SERVER_DEFAULT_HOST, portno);

    if (SDLNet_ResolveHost(&server_address, SERVER_DEFAULT_HOST, portno)) {
        MSG_BOX("Open socket, cannot resolve %s:%u",
               SERVER_DEFAULT_HOST, portno);
        return (false);
    }

    if (on_server) {
        if (!server_start(server_address)) {
            ERR("Server failed to listen");
        }
    }

    if (!wid_server_create_init()) {
        DIE("wid server create init");
    }

    server_init_done = true;

    return (true);
}

void server_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (server_init_done) {
        server_init_done = false;

        server_stop();
    }
}

static void server_rx_client_join (gsocketp s)
{
    aplayerp p = socket_get_player(s);
    if (!p) {
        WARN("Received join %s, but no player on socket",
             socket_get_remote_logname(s));
        return;
    }

    global_config.server_current_players++;

    LOG("Server: Player \"%s\" (ID %u) joined from %s", 
        p->stats_from_client.pname, 
        p->key, socket_get_remote_logname(s));

    char *tmp = dynprintf("%s joined", 
                          p->stats_from_client.pname);
    socket_tx_server_shout_at_all_players_except(s, WARNING, tmp);
    myfree(tmp);

    LOG("Server: Total players now %u", global_config.server_current_players);

    socket_server_tx_map_update(s, server_boring_things,
                                "rx client join boring things");
    socket_server_tx_map_update(s, server_active_things,
                                "rx client join active things");

    wid_game_map_server_visible();
}

static void server_rx_client_leave_implicit (gsocketp s)
{
    aplayerp p = socket_get_player(s);
    if (!p) {
        return;
    }

    global_config.server_current_players--;

    if (!global_config.server_current_players) {
        LOG("Server: Last player left the game, destroy the map");

        wid_game_map_server_wid_destroy(false /* keep players*/);
    } else {
        LOG("Server: %u player(s) left, do not destroy the map",
            global_config.server_current_players);
    }

    if (p->thing) {
        thing_dead(p->thing, 0, "the network");
    }

    socket_set_player(s, 0);
}

static void server_rx_client_leave (gsocketp s)
{
    aplayerp p = socket_get_player(s);
    if (!p) {
        WARN("Received leave %s, but no player on socket",
             socket_get_remote_logname(s));
        return;
    }

    LOG("Server: \"%s\" (ID %u) left from %s", 
        p->stats_from_client.pname,
        p->key, socket_get_remote_logname(s));

    char *tmp = dynprintf("%s left the game", 
                          p->stats_from_client.pname);
    socket_tx_server_shout_at_all_players(INFO, tmp);
    myfree(tmp);

    server_rx_client_leave_implicit(s);
}

static void server_rx_client_close (gsocketp s)
{
    aplayerp p = socket_get_player(s);
    if (!p) {
        /*
         * Not an error. Just not in our game.
         */
        return;
    }

    LOG("Server: \"%s\" (ID %u) suddenly left from %s", 
        p->stats_from_client.pname,
        p->key, socket_get_remote_logname(s));

    char *tmp = dynprintf("%s suddenly left the game", 
                          p->stats_from_client.pname);
    socket_tx_server_shout_at_all_players(WARNING, tmp);
    myfree(tmp);

    server_rx_client_leave_implicit(s);
}

static void server_poll (void)
{
    gsocketp s = server_socket;

    if (!s) {
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

    int i;
    for (i = 0; i < numready; i++) {
        int ready = SDLNet_SocketReady(socket_get_udp_socket(s));
        if (ready == 0) {
            break;
        }

        UDPpacket *packet = packet_alloc();

        int paks = SDLNet_UDP_Recv(socket_get_udp_socket(s), packet);
        if (paks != 1) {
            LOG("Server: UDP rx failed: error='%s' paks=%d", 
                SDLNet_GetError(), paks);
            continue;
        }

        gsocketp s = socket_find_remote_ip(read_address(packet));
        if (!s) {
            char *tmp = iptodynstr(read_address(packet));
            LOG("Server: New client from %s", tmp);
            myfree(tmp);

            s = socket_connect_from_server(read_address(packet));
            if (!s) {
                continue;
            }
        }

        uint8_t *data;
        uint8_t *odata;
        uint8_t *pdata;
        uint8_t uncompressed = false;

        /*
         * Remove any optional header
         */
        packet = packet_definalize(s, packet);

        /*
         * Uncompress the packet if it has an invalid type.
         */
        if (*packet->data == MSG_COMPRESSED) {
            data = miniz_uncompress(packet->data + 1, &packet->len);
            odata = data;
            pdata = packet->data;
            packet->data = data;
            uncompressed = true;
        } else {
            data = packet->data;
            odata = data;
            pdata = data;
        }

        msg_type type = (typeof(type)) *data++;

        socket_count_inc_pak_rx(s, type);

        switch (type) {
        case MSG_PING:
            socket_rx_ping(s, packet, data);
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

        case MSG_CLIENT_PLAYER_MOVE:
            socket_server_rx_player_move(s, packet, data);
            break;

        case MSG_CLIENT_PLAYER_ACTION:
            socket_server_rx_player_action(s, packet, data);
            break;

        default:
            ERR("Unknown server message type received [%u", type);
        }

        if (uncompressed) {
            packet->data = pdata;
            myfree(odata);
        }

        packet_free(packet);
    }
}

static void server_alive_check (void)
{
    gsocketp s;

    sockets_quality_check();

    TREE_WALK(sockets, s) {
        if (!socket_get_server_side_client(s)) {
            continue;
        }

        /*
         * Don't kill off new born connections.
         */
        if (socket_get_tx(s) < 200) {
            continue;
        }

        if (socket_get_quality(s) < SOCKET_PING_FAIL_THRESHOLD) {
            /*
             * Clients try forever. Server clients disconnect.
             */
            aplayerp p = socket_get_player(s);

            if (s->server_side_client) {
                MSG_SERVER_SHOUT_AT_ALL_PLAYERS(
                    POPUP, "Connection lost to %s",
                    socket_get_remote_logname(s));

                if (p) {
                    LOG("Server: \"%s\" (ID %u) dropped out from %s", 
                        p->stats_from_client.pname, 
                        p->key, 
                        socket_get_remote_logname(s));
                } else {
                    LOG("Server: Dropped connection to %s", 
                        socket_get_remote_logname(s));
                }

                LOG("Server: Sent %d packets, quality %u",
                    socket_get_tx(s),
                    (socket_get_quality(s) < SOCKET_PING_FAIL_THRESHOLD));
            } else {
                MSG(POPUP, "Connection at %d%% quality", 
                    socket_get_quality(s));

                continue;
            }

            server_rx_client_leave_implicit(s);

            socket_disconnect(s);
        } else {
            LOG("Server: Sent %d packets to %s, quality %u",
                socket_get_tx(s),
                socket_get_remote_logname(s),
                socket_get_quality(s));
        }
    }
}

static void server_socket_tx_ping (void)
{
    static uint32_t ts;
    static uint8_t seq;

    if (!time_have_x_tenths_passed_since(DELAY_TENTHS_PING, ts)) {
        return;
    }

    ts = time_get_time_cached();

    /*
     * Every few seconds check for dead peers.
     */
    if (ts && (!(seq % 3))) {
        server_alive_check();
    }

    gsocketp s;

    TREE_WALK(sockets, s) {

        if (socket_get_client(s)) {
            continue;
        }

        /*
         * Don't ping thyself if both server and client.
         */
        if (socket_get_server(s)) {
            continue;
        }

        socket_tx_ping(s, seq, time_get_time_cached());
    }

    seq++;
}

static void server_socket_tx_map_updates (void)
{
    static uint32_t ts;

    if (!time_have_x_hundredths_passed_since(
            DELAY_HUNDREDTHS_SERVER_TO_CLIENT_MAP_UPDATE, ts)) {
        return;
    }

    ts = time_get_time_cached();

    socket_server_tx_map_update(0, server_active_things,
                                "rx client join active things");

    socket_server_tx_map_update(0, server_boring_things,
                                "rx client join boring things");
}

static void server_socket_tx_player_updates (void)
{
    static uint32_t ts;

    if (!time_have_x_hundredths_passed_since(
            DELAY_HUNDREDTHS_SERVER_TO_CLIENT_PLAYER_UPDATE, ts)) {
        return;
    }

    ts = time_get_time_cached();

    /*
     * Send players their items lists
     */
    socket_tx_server_status();
}

void server_tick (void)
{
    if (!on_server) {
        return;
    }

    server_poll();
    server_socket_tx_ping();
    server_socket_tx_player_updates();
    server_socket_tx_map_updates();
}

/*
 * User has entered a command, run it
 */
static uint8_t server_players_show (tokens_t *tokens, void *context)
{
    CON("Name               Remote IP      Local IP     Quality");
    CON("----           --------------- --------------- -------");

    uint32_t pi;

    pi = 0;

    gsocketp s;
    TREE_WALK(sockets, s) {

        aplayer *p = socket_get_player(s);
        if (!p) {
            continue;
        }

        if (!p->stats_from_client.pname[0]) {
            continue;
        }

        char *tmp = iptodynstr(p->local_ip);
        char *tmp2 = iptodynstr(p->remote_ip);

        pi++;

        CON("[%d] %-10s %-15s %-15s %d", 
            pi,
            p->stats_from_client.pname,
            tmp,
            tmp2,
            socket_get_quality(s));

        myfree(tmp2);
        myfree(tmp);
    }

    return (true);
}

/*
 * User has entered a command, run it
 */
uint8_t server_shout (tokens_t *tokens, void *context)
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

    socket_tx_server_shout_at_all_players(CHAT, shout);

    myfree(tmp);

    return (true);
}
