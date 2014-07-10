/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 *
 * A useful hack to help debug stale/free/bad pointers.
 */

#include "main.h"
#include "socket.h"
#include "client.h"
#include "time.h"
#include "slre.h"
#include "command.h"
#include "string.h"
#include "wid_server_join.h"
#include "wid_game_map_client.h"
#include "wid.h"
#include "thing.h"
#include "mzip_lib.h"
#include "wid_dead.h"
#include "level.h"
#include "name.h"

/*
 * Which socket we have actually joined on.
 */
socketp client_joined_server;
uint32_t client_joined_server_when;
static uint32_t client_joined_server_key;
uint32_t client_player_died;

static void client_socket_tx_ping(void);
static uint8_t client_init_done;
static void client_poll(void);
static uint8_t client_set_name(tokens_t *tokens, void *context);
static uint8_t client_set_pclass(tokens_t *tokens, void *context);
static uint8_t client_shout(tokens_t *tokens, void *context);
static uint8_t client_tell(tokens_t *tokens, void *context);
static uint8_t client_players_show(tokens_t *tokens, void *context);
static uint8_t client_join(tokens_t *tokens, void *context);
static uint8_t client_leave(tokens_t *tokens, void *context);
static uint8_t client_open(tokens_t *tokens, void *context);
static uint8_t client_close(tokens_t *tokens, void *context);
static uint8_t client_socket_tell(char *from, char *to, char *msg);
static void client_check_still_in_game(void);

static msg_server_status server_status;
static uint8_t server_connection_confirmed;

/*
 * Saved state for rejoins.
 */
static uint16_t last_portno;
static char *last_host;

uint8_t client_init (void)
{
    if (client_init_done) {
        return (true);
    }

    socketp s = 0;

    /*
     * Connector.
     */
    if (is_client) {
        s = socket_connect_from_client(server_address);
        if (!s) {
            WARN("Client failed to connect");
            return (false);
        }
    }

    command_add(client_set_name, "set name [A-Za-z0-9_-]*",
                "set player name");

    command_add(client_set_pclass, "set class [A-Za-z0-9_-]*",
                "set player class e.g warrior");

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

    if (!global_config.pclass[0]) {
        strncpy(global_config.pclass, "warrior", 
                sizeof(global_config.pclass) - 1);
    }

    if (!global_config.name[0]) {
        strncpy(global_config.name, name_random(global_config.pclass),
                sizeof(global_config.name) - 1);
    }

    client_init_done = true;

    if (!on_server) {
        if (!wid_server_join_init()) {
            DIE("wid server join init");
        }
    }

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

    if (last_host) {
        myfree(last_host);
        last_host = 0;
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

    if (!time_have_x_tenths_passed_since(DELAY_TENTHS_PING, ts)) {
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

    wid_server_join_redo(true /* soft refresh */);

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
static uint8_t client_set_name (tokens_t *tokens, void *context)
{
    char *s = tokens->args[2];

    if (!s || !*s) {
        WARN("need to set a name");
        return (false);
    }

    uint8_t r = client_socket_set_name(s);

    return (r);
}

/*
 * User has entered a command, run it
 */
static uint8_t client_set_pclass (tokens_t *tokens, void *context)
{
    char *s = tokens->args[2];

    if (!s || !*s) {
        WARN("need to set a string");
        return (false);
    }

    uint8_t r = client_socket_set_pclass(s);

    return (r);
}

static uint8_t client_socket_open (char *host, char *port)
{
    uint32_t portno;
    socketp s = 0;

    if (!host || !*host) {
        host = SERVER_DEFAULT_HOST;
    }

    if (port && *port) {
        portno = atoi(port);
    } else {
        portno = global_config.server_port;
    }

    LOG("Client: Trying to resolve server address %s:%u", host, portno);

    if (SDLNet_ResolveHost(&server_address, host, portno)) {
        MSG_BOX("Open socket, cannot resolve %s:%u", host, portno);
        return (false);
    }

    LOG("Client: Connecting to server address %s:%u", host, portno);

    /*
     * Connector.
     */
    s = socket_connect_from_client(server_address);
    if (!s) {
        WARN("Client failed to connect");
        return (false);
    }

    return (true);
}

uint8_t client_socket_close (char *host, char *port)
{
    uint32_t portno;
    socketp s = 0;

    if (!host && !port) {
        TREE_WALK(sockets, s) {
            if (socket_get_client(s)) {
                break;
            }
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
            portno = global_config.server_port;
        }

        if (SDLNet_ResolveHost(&server_address, host, portno)) {
            ERR("Close socket, cannot resolve %s:%u", host, portno);
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

    LOG("Client: Disconnecting %s", socket_get_remote_logname(s));

    socket_tx_client_close(s);

    socket_disconnect(s);

    return (true);
}

uint8_t client_socket_join (char *host, char *port, uint16_t portno,
                            uint8_t quiet)
{
    if (client_joined_server) {
        WARN("Leave the current server first before trying to join again");
        return (false);
    }

    if (portno) {
        last_portno = portno;
    }

    if (!portno) {
        portno = last_portno;
    }

    if (host) {
        if (last_host) {
            myfree(last_host);
        }
        last_host = dupstr(host, "last host");;
    }

    if (!host) {
        host = last_host;
    }

    socketp s = 0;

    if (!host && !port) {
        TREE_WALK(sockets, s) {
            if (socket_get_client(s)) {
                break;
            }
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
                portno = global_config.server_port;
            }
        }

        if (SDLNet_ResolveHost(&server_address, host, portno)) {
            if (quiet) {
                WARN("Cannot join %s:%u", host, portno);
            } else {
                MSG_BOX("Cannot join %s:%u", host, portno);
            }
            return (false);
        }

        /*
         * Connector.
         */
        s = socket_connect_from_client(server_address);
        if (!s) {
            if (quiet) {
                WARN("Join, failed to connect");
            } else {
                MSG_BOX("Join, failed to connect");
            }
            return (false);
        }
    }

    socket_set_name(s, global_config.name);
    socket_set_pclass(s, global_config.pclass);

    if (!socket_tx_client_join(s, &client_joined_server_key)) {
        if (!quiet) {
            MSG(CRITICAL, "Join failed");
        }
        return (false);
    }

    LOG("Client: Joining server %s", socket_get_remote_logname(s));

    client_joined_server = s;
    client_joined_server_when = time_get_time_cached();

    verify(client_joined_server);

    return (true);
}

static uint8_t client_socket_leave_implicit (void)
{
    if (!client_joined_server) {
        return (false);
    }

    MSG_BOX("YOU WERE DROPPED FROM THE SERVER");

    client_joined_server = 0;

    memset(&server_status, 0, sizeof(server_status));
    server_connection_confirmed = false;

    return (true);
}

uint8_t client_socket_leave (void)
{
    if (!client_joined_server) {
        WARN("Join a server first before trying to leave");
        return (false);
    }

    verify(client_joined_server);

    socket_tx_client_leave(client_joined_server);

    client_joined_server = 0;

    memset(&server_status, 0, sizeof(server_status));
    server_connection_confirmed = false;

    return (true);
}

uint8_t client_socket_shout (char *shout)
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

    socket_tx_client_shout(client_joined_server, CHAT, shout);

    return (true);
}

static uint8_t client_socket_tell (char *from, char *to, char *msg)
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
uint8_t client_socket_set_name (const char *name)
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
uint8_t client_socket_set_pclass (const char *pclass)
{
    if (!pclass || !*pclass) {
        WARN("need to set a pclass");
        return (false);
    }

    strncpy(global_config.pclass, pclass, sizeof(global_config.pclass) - 1);

    CON("Client pclass set to \"%s\"", pclass);

    if (client_joined_server) {
        socket_set_pclass(client_joined_server, pclass);

        socket_tx_name(client_joined_server);
    }

    return (true);
}

/*
 * User has entered a command, run it
 */
uint8_t client_open (tokens_t *tokens, void *context)
{
    char *host = tokens->args[1];
    char *port = tokens->args[2];

    uint8_t r = client_socket_open(host, port);

    return (r);
}

/*
 * User has entered a command, run it
 */
uint8_t client_close (tokens_t *tokens, void *context)
{
    char *host = tokens->args[1];
    char *port = tokens->args[2];

    uint8_t r = client_socket_close(host, port);

    return (r);
}

/*
 * User has entered a command, run it
 */
uint8_t client_join (tokens_t *tokens, void *context)
{
    char *host = tokens->args[1];
    char *port = tokens->args[2];

    uint8_t r = client_socket_join(host, port, 0, false /* quiet */);

    return (r);
}

/*
 * User has entered a command, run it
 */
uint8_t client_leave (tokens_t *tokens, void *context)
{
    uint8_t r = client_socket_leave();

    return (r);
}

/*
 * User has entered a command, run it
 */
uint8_t client_shout (tokens_t *tokens, void *context)
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

    uint8_t r = client_socket_shout(shout);

    myfree(tmp);

    return (r);
}

/*
 * User has entered a command, run it
 */
uint8_t client_tell (tokens_t *tokens, void *context)
{
    char to[SMALL_STRING_LEN_MAX] = {0};
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

    uint8_t r = client_socket_tell(global_config.name, to, tmp);

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

            uint8_t *data;
            uint8_t *odata;
            uint8_t *pdata;
            uint8_t uncompressed = false;

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

            case MSG_SERVER_STATUS: {
                /*
                 * This is an update of all players in the game.
                 */
                msg_server_status latest_status;

                memset(&latest_status, 0, sizeof(latest_status));

                socket_rx_server_status(s, packet, data, &latest_status);

                if (s == client_joined_server) {
                    client_check_still_in_game();
                }

                uint8_t redo = false;

                if (server_status.server_current_players !=
                    latest_status.server_current_players) {
                    LOG("Client: Number of players in game now %u", 
                        latest_status.server_current_players);

                    redo = true;
                }

                if (client_level) {
                    level_set_level_no(client_level, latest_status.level_no);
                }

                if (server_status.level_no != latest_status.level_no) {

                    LOG("Client: Level no %u", latest_status.level_no);
                    redo = true;
                }

                memcpy(&server_status, &latest_status, sizeof(server_status));

                wid_game_map_client_score_update(client_level, redo);
                break;
            }

            case MSG_SERVER_HISCORE: {
                /*
                 * This is an update of the hiscores as the player has died.
                 */
                msg_server_hiscores latest_hiscores;

                memset(&latest_hiscores, 0, sizeof(latest_hiscores));

                socket_rx_server_hiscore(s, packet, data, &latest_hiscores);

                wid_dead_visible(latest_hiscores.players[0].player_name,
                                 latest_hiscores.players[0].death_reason,
                                 latest_hiscores.rejoin_allowed);

                break;
            }

            case MSG_SERVER_MAP_UPDATE:
                socket_client_rx_map_update(s, packet, data);
                break;

            case MSG_SERVER_PLAYER_UPDATE:
                /*
                 * This is an update of a single players carried items.
                 */
                socket_client_rx_player_update(s, packet, data);

                wid_game_map_client_score_update(client_level, true /* redo */);
                break;

            case MSG_SERVER_CLOSE:
                socket_rx_server_close(s, packet, data);

                client_socket_leave_implicit();
                break;

            default:
                ERR("Unknown client message type received [%u]", type);
            }

            if (uncompressed) {
                packet->data = pdata;
                myfree(odata);
            }
        }

        SDLNet_FreePacket(packet);
    }
}

/*
 * User has entered a command, run it
 */
static uint8_t client_players_show (tokens_t *tokens, void *context)
{
    CON("Name                    Quality  Latency      Local IP       Remote IP    Score ");
    CON("----                    -------  ------- --------------- --------------- -------");

    uint32_t pi;

    for (pi = 0; pi < MAX_PLAYERS; pi++) {
        msg_player_state *p = &server_status.players[pi];

        char *tmp = iptodynstr(p->local_ip);
        char *tmp2 = iptodynstr(p->remote_ip);

        CON("[%d] %-10s/%8s %3d pct %5d ms %-15s %-15s %07d", 
            pi, 
            p->name,
            p->pclass,
            p->quality,
            p->avg_latency,
            tmp2,
            tmp,
            p->score);

        myfree(tmp);
        myfree(tmp2);
    }

    return (true);
}

msg_player_statep client_get_player (int n)
{
    msg_player_state *p = &server_status.players[n];
    return (p);
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

    if (!time_have_x_tenths_passed_since(DELAY_TENTHS_PING,
                                         client_joined_server_when)) {
        return;
    }

    for (pi = 0; pi < MAX_PLAYERS; pi++) {
        msg_player_state *p = &server_status.players[pi];

        if (!p->name[0]) {
            continue;
        }

        if (p->key != client_joined_server_key) {
            continue;
        }

        if (strcmp(p->name, global_config.name)) {
            continue;
        }

        if (!server_connection_confirmed) {
            server_connection_confirmed = true;
            MSG(INFO, "%s, joined the game", p->name);

            player = thing_client_find(p->thing_id);

            /*
             * Needed twice for some reason to adjust the scrollbar as the
             * level is filled with tiles.
             */
            wid_game_map_client_scroll_adjust(true /* adjust */);
            wid_game_map_client_scroll_adjust(true /* adjust */);

            if (!player) {
                ERR("failed to find player in map update");
                continue;
            }

            if (!wid_game_map_client_window) {
                ERR("Client: No game map window");
                continue;
            }

            wid_visible(wid_game_map_client_window, 0);
        }

        return;
    }

    /*
     * Only if no report for a long time, whinge.
     */
    if (!time_have_x_tenths_passed_since(DELAY_TENTHS_PING * 5,
                                         client_joined_server_when)) {
        return;
    }

    if (client_player_died) {
        return;
    }

    MSG(CRITICAL, "Server does not report you in the game!");

    LOG("Client:  You are player: \"%s\", ID %u", 
        global_config.name, client_joined_server_key);

    server_connection_confirmed = false;

    for (pi = 0; pi < MAX_PLAYERS; pi++) {
        msg_player_state *p = &server_status.players[pi];

        LOG("Client:  Player %u is \"%s\", ID %u ", pi+1, p->name, p->key);
    }
}

