/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include "main.h"
#include <stdlib.h>

#include "socket_util.h"
#include "client.h"
#include "time_util.h"
#include "slre.h"
#include "command.h"
#include "string_util.h"
#include "wid_server_join.h"
#include "wid_game_map_client.h"
#include "wid_player_stats.h"
#include "wid_player_info.h"
#include "wid_player_inventory.h"
#include "wid_player_action.h"
#include "wid_chat.h"
#include "wid.h"
#include "thing.h"
#include "mzip_lib.h"
#include "wid_dead.h"
#include "level.h"
#include "name.h"
#include "music.h"

/*
 * Which socket we have actually joined on.
 */
gsocketp client_joined_server;
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
static uint8_t client_player_show(tokens_t *tokens, void *context);
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

    gsocketp s = 0;

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

    command_add(client_player_show, "show client player", 
                "show client player state");

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
    gsocketp s;

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

    if (!time_have_x_tenths_passed_since(DELAY_TENTHS_PING, ts)) {
        return;
    }

    gsocketp s;

    TREE_WALK(sockets, s) {
        if (!socket_get_client(s)) {
            continue;
        }

        ts = time_get_time_ms();

        /*
         * If nothing is on the remote end then dont ping it relentlessly.
         */
        if (!s->rx) {
            if (!time_have_x_tenths_passed_since(
                        DELAY_TENTHS_PING_WHEN_NO_RESPONSE, s->tx_last_ping)) {
                continue;
            }
        }

        s->tx_last_ping = ts;
        socket_tx_ping(s, s->tx_ping_seq++, ts);
    }

    /*
     * Every few seconds check for dead peers.
     */
    static int count;
    count++;
    if (ts && !(count % 5)) {
        client_alive_check();
    }

    {
        static uint32_t ts;

        if (time_have_x_tenths_passed_since(10, ts)) {
            wid_server_join_redo(true /* soft refresh */);

            ts = time_get_time_ms();
        }
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
static uint8_t client_set_name (tokens_t *tokens, void *context)
{
    char *s = tokens->args[2];

    if (!s || !*s) {
        DIE("need to set a name for client");
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
    gsocketp s = 0;
    const char *h;

    h = host;
    if (!h || !*h) {
        h = SERVER_DEFAULT_HOST;
    }

    if (port && *port) {
        portno = atoi(port);
    } else {
        portno = global_config.server_port;
    }

    LOG("Client: Trying to resolve server address %s:%u", h, portno);

    if (address_resolve(&server_address, h, portno)) {
        MSG_BOX("Open socket, cannot resolve %s:%u", h, portno);
        return (false);
    }

    LOG("Client: Connecting to server address %s:%u", h, portno);

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
    gsocketp s = 0;
    const char *h;

    h = host;

    if (!h && !port) {
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
        if (!h || !*h) {
            h = SERVER_DEFAULT_HOST;
        }

        if (port && *port) {
            portno = atoi(port);
        } else {
            portno = global_config.server_port;
        }

        LOG("Client: Trying to resolve server address %s:%u", h, portno);

        if (address_resolve(&server_address, h, portno)) {
            ERR("Close socket, cannot resolve %s:%u", h, portno);
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

uint8_t client_socket_join (const char *host, 
                            const char *port, uint16_t portno,
                            uint8_t quiet)
{
    const char *h;

    h = host;

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

    if (h) {
        if (last_host) {
            myfree(last_host);
        }
        last_host = dupstr(h, "last host");;
    }

    if (!h) {
        h = last_host;
    }

    LOG("Client: Join host %s, port %s, portno %u",
        h ? h : "-",port ? port: "-", portno);

    gsocketp s = 0;

    if (!h && !port) {
        TREE_WALK(sockets, s) {
            if (socket_get_client(s)) {
                break;
            }
        }

        if (!s) {
            h = SERVER_DEFAULT_HOST;
        }
    }

    if (!h && !port && !s) {
        TREE_WALK(sockets, s) {
            if (socket_get_client(s)) {
                break;
            }
        }

        if (!s) {
            h = SERVER_DEFAULT_HOST;
        }

    } else {
        if (!h || !*h) {
            h = SERVER_DEFAULT_HOST;
        }

        if (!portno) {
            if (port && *port) {
                portno = atoi(port);
            } else {
                portno = global_config.server_port;
            }
        }

        if (address_resolve(&server_address, h, portno)) {
            if (quiet) {
                WARN("Cannot join %s:%u", h, portno);
            } else {
                MSG_BOX("Cannot join %s:%u", h, portno);
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

    /*
     * If we hit join a server without creating a player, then we need
     * to generate some stats.
     */
    if (!global_config.stats.pname[0] ||
        !global_config.stats.pclass[0] ||
        !global_config.stats.hp) {

        LOG("Client: Need random stats set:");

        thing_statsp s;
        s = &global_config.stats;
        thing_stats_get_random(s, false /* new_random_name_and_class */);
    }

    /*
     * Copy stuff into the socket
     */
    socket_set_name(s, global_config.stats.pname);
    socket_set_pclass(s, global_config.stats.pclass);
    socket_set_player_stats(s, &global_config.stats);

    if (!socket_tx_client_join(s, &client_joined_server_key)) {
        if (!quiet) {
            MSG(CRITICAL, "Join failed");
        } else {
            LOG("Client: Joining failed to %s", socket_get_remote_logname(s));
        }
        return (false);
    }

    LOG("Client: Joining server %s", socket_get_remote_logname(s));
    thing_stats_dump(&global_config.stats);

    client_joined_server = s;
    client_joined_server_when = time_get_time_ms();

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

    LOG("Client: Leaving server %s", 
        socket_get_remote_logname(client_joined_server));

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
        DIE("need to set a pname");
        return (false);
    }

    /*
     * Cater for overlapping pointers.
     */
    if (strcmp(global_config.stats.pname, name)) {
        strncpy(global_config.stats.pname, name, 
                sizeof(global_config.stats.pname) - 1);

        CON("Client name set to \"%s\"", name);
    }

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
        DIE("need to set a pclass");
        return (false);
    }

    /*
     * Cater for overlapping pointers.
     */
    if (strcmp(global_config.stats.pclass, pclass)) {
        strncpy(global_config.stats.pclass, pclass, 
                sizeof(global_config.stats.pclass) - 1);

        CON("Client pclass set to \"%s\"", pclass);
    }

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

    uint8_t r = client_socket_tell(global_config.stats.pname, to, tmp);

    myfree(tmp);

    return (r);
}

static void client_poll (void)
{
    gsocketp s;

    if (client_joined_server) {
        verify(client_joined_server);
    }

    TREE_WALK(sockets, s) {
        if (!socket_get_client(s)) {
            continue;
        }

        for (;;) {
            UDPpacket *packet = socket_rx_dequeue(s);
            if (!packet) {
                break;
            }

            /*
             * Remove any optional header
             */
            packet = packet_definalize(s, packet);

            uint8_t *data;
            uint8_t *odata;
            uint8_t *pdata;
            uint8_t uncompressed;

            /*
             * Uncompress the packet if it has an invalid type.
             */
            pdata = packet->data;
            data = packet_decompress(packet, &uncompressed);
            odata = data;

            msg_type type = (msg_type) *data++;

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
                 * This is an update our player state on the server.
                 */
                msg_server_status latest_status;

                memset(&latest_status, 0, sizeof(latest_status));

                socket_rx_server_status(s, packet, data, &latest_status);

                /*
                 * If this is a status from a server we are not connected to, 
                 * just ignore it.
                 */
                if (!latest_status.you_are_playing_on_this_server) {
                    break;
                }

                if (s == client_joined_server) {
                    client_check_still_in_game();
                }

                uint8_t redo = false;

                if (client_level) {
                    level_set_level_no(client_level, latest_status.level_no);
                }

                if (server_status.level_no != latest_status.level_no) {
                    LOG("Client: Level no %u", latest_status.level_no);
                    redo = true;
                }

                /*
                 * Someone joined or left?
                 */
                if (server_status.server_current_players != 
                        latest_status.server_current_players) {
                    if (latest_status.server_current_players > 1) {
                        wid_visible(wid_chat_window, 0);
                    } else {
                        wid_hide(wid_chat_window, 0);
                    }
                }

                global_config.server_current_players =
                                latest_status.server_current_players;
                global_config.level_no =
                                latest_status.level_no;

                if (latest_status.level_hide) {
                    if (client_level) {
                        things_level_destroyed(client_level,
                                               true /* keep players */);
                    }
                }

                if (server_status.level_hide != latest_status.level_hide) {
                    if (latest_status.level_hide) {
                        LOG("Client: Hide level");

                        wid_hide(wid_game_map_client_grid_container, 
                                 wid_hide_delay);
                    } else {
                        /*
                         * Reveal the level and re-equip players.
                         */
                        LOG("Client: Reveal level");

                        wid_visible(wid_game_map_client_grid_container, 
                                    wid_visible_delay);

                        wid_game_map_client_scroll_adjust(1);
                    }
                }

                memcpy(&server_status, &latest_status, sizeof(server_status));

                msg_player_state *server_stats = &server_status.player;
                thing_statsp new_stats = &server_stats->stats;
                thing_statsp old_stats = &player->stats;

                /*
                 * If we've just changed weapons locally and receive an update
                 * with the old weapon then ignore that.
                 */
                if (time_get_time_ms() - player_action_bar_changed_at < ONESEC) {
                    new_stats->action_bar_index = old_stats->action_bar_index;
                }

                if (player) {
                    thing_stats changed_stats;

                    /*
                     * Some fields we don't care too much if they change.
                     */
                    memcpy(&changed_stats, new_stats, sizeof(changed_stats));
                    changed_stats.thing_id = old_stats->thing_id;
                    new_stats = &changed_stats;

                    /*
                     * Now see what really changed and if we need to update 
                     * scores.
                     */
                    if (memcmp(old_stats, new_stats, sizeof(thing_stats))) {
                        LOG("Client: %s player stats changed:",
                            thing_logname(player));

                        thing_stats_diff(old_stats, new_stats);

                        /*
                         * If the stats change, update the inventory
                         */
                        memcpy(old_stats, new_stats, sizeof(thing_stats));

                        wid_game_map_client_score_update(client_level, redo);
                    }

                    new_stats = &server_stats->stats;
                    memcpy(old_stats, new_stats, sizeof(thing_stats));
                }

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

            packet_free(packet);
        }
    }
}

/*
 * User has entered a command, run it
 */
static uint8_t client_player_show (tokens_t *tokens, void *context)
{
    CON("Name                       Local IP       Remote IP    ");
    CON("----                    --------------- ---------------");

    msg_player_state *p = &server_status.player;

    char *tmp = iptodynstr(p->local_ip);
    char *tmp2 = iptodynstr(p->remote_ip);

    CON("%-10s/%12s %-15s %-15s", 
        p->stats.pname,
        p->stats.pclass,
        tmp2,
        tmp);

    myfree(tmp);
    myfree(tmp2);

    return (true);
}

msg_player_statep client_get_player (void)
{
    msg_player_state *p = &server_status.player;
    return (p);
}

static void client_check_still_in_game (void)
{
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

    for (;;) {
        msg_player_state *p = &server_status.player;

        if (!p->stats.pname[0]) {
            break;
        }

        if (strcasecmp(p->stats.pname, global_config.stats.pname)) {
            break;
        }

        if (!server_connection_confirmed) {
            player = thing_client_find(p->stats.thing_id);
            if (!player) {
                ERR("failed to find player ID %d in map update",
                    p->stats.thing_id);
                break;
            }

            music_play_game();

            server_connection_confirmed = true;

            MSG(INFO, "%s joined", p->stats.pname);

            LOG("%s joined, ID %d", p->stats.pname, p->stats.thing_id);

            /*
             * Needed twice for some reason to adjust the scrollbar as the
             * level is filled with tiles.
             */
            wid_game_map_client_scroll_adjust(true /* adjust */);
            wid_game_map_client_scroll_adjust(true /* adjust */);

            if (!wid_game_map_client_window) {
                ERR("Client: No game map window");
                break;
            }

            wid_visible(wid_game_map_client_window, wid_visible_delay);
        }

        return;
    }

    /*
     * Only if no report for a long time, whinge.
     */
    if (!time_have_x_tenths_passed_since(DELAY_TENTHS_PING * 10,
                                         client_joined_server_when)) {
        return;
    }

    if (client_player_died) {
        return;
    }

    MSG(CRITICAL, "Server does not report you in the game!");

    LOG("Client: You are player: \"%s\", ID %u", 
        global_config.stats.pname, 
        client_joined_server_key);

    server_connection_confirmed = false;
}
