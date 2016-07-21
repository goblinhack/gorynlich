/*
* Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#include "main.h"

#include "socket_util.h"
#include "client.h"
#include "time_util.h"
#include "slre.h"
#include "command.h"
#include "string_util.h"
#include "wid_server_join.h"
#include "wid_game_map_client.h"
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
static uint8_t client_shout(tokens_t *tokens, void *context);
static uint8_t client_tell(tokens_t *tokens, void *context);
static uint8_t client_player_show(tokens_t *tokens, void *context);
static uint8_t client_socket_tell(char *from, char *to, char *msg);
static void client_check_still_in_game(level_pos_t);

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

    command_add(client_shout, "shout [A-Za-z0-9_-]*",
                "shout to players");

    command_add(client_tell, "tell [A-Za-z0-9_-]* [A-Za-z0-9_-]*",
                "send a message to a player");

    command_add(client_player_show, "show client player", 
                "show client player state");

    client_init_done = true;

    if (!on_server) {
        if (!wid_server_join_init()) {
            ERR("wid server join init");
        }
    }

    return (true);
}

void client_fini (void)
{
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

    if (single_player_mode) {
        return;
    }

    sockets_quality_check();

    TREE_WALK(sockets, s) {
        if (!socket_get_client(s)) {
            continue;
        }

        /*
         * Don't kill off new born connections.
         */
        if (socket_get_tx(s) < 100) {
            continue;
        }

        if (socket_get_quality(s) <= SOCKET_PING_FAIL_THRESHOLD) {
            /*
             * Clients try forever. Server clients disconnect.
             */
            LOG("Client: Quality lower than ping threshold");

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
    gsocketp s;

    TREE_WALK(sockets, s) {
        if (!socket_get_client(s)) {
            continue;
        }

        socket_tx_ping(s, &s->tx_ping_seq, time_get_time_ms());
    }

    /*
     * Every few seconds check for dead peers.
     */
    static uint32_t ts;

    if (time_have_x_tenths_passed_since(10, ts)) {
        ts = time_get_time_ms();
        client_alive_check();
        wid_server_join_redo(true /* soft refresh */);
    }
}

void client_tick (void)
{
    if (!is_client) {
        return;
    }

    client_poll();

    client_socket_tx_ping();

    static uint32_t ts;

    if (!time_have_x_tenths_passed_since(
            DELAY_TENTHS_CLIENT_TO_SERVER_STATUS, ts)) {
        return;
    }

    ts = time_get_time_ms();

    if (player) {
        thing_stats_client_modified(&player->stats);
    }
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
            MESG(CRITICAL, "Join failed");
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
        ERR("need to set a pname");
        return (false);
    }

    /*
     * Cater for overlapping pointers.
     */
    if (strcmp(global_config.stats.pname, name)) {
        strlcpy(global_config.stats.pname, name, 
                sizeof(global_config.stats.pname) - 1);

        CON("Client name set to \"%s\"", name);
    }

    if (client_joined_server) {
        socket_set_name(client_joined_server, name);

        socket_tx_client_status(client_joined_server);
    }

    return (true);
}

/*
 * User has entered a command, run it
 */
uint8_t client_socket_set_pclass (const char *pclass)
{
    if (!pclass || !*pclass) {
        ERR("need to set a pclass");
        return (false);
    }

    /*
     * Cater for overlapping pointers.
     */
    if (strcmp(global_config.stats.pclass, pclass)) {
        strlcpy(global_config.stats.pclass, pclass, 
                sizeof(global_config.stats.pclass) - 1);

        CON("Client pclass set to \"%s\"", pclass);
    }

    if (client_joined_server) {
        socket_set_pclass(client_joined_server, pclass);

        socket_tx_client_status(client_joined_server);
    }

    return (true);
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

    strlcpy(shout, tmp, sizeof(shout) - 1);

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

    strlcpy(to, s, sizeof(to) - 1);

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

/*
 * We've received an update from the server for our player.
 * We need to merge in the status and update the stats windows.
 */
static void client_rx_server_status (gsocketp s, 
                                     UDPpacket *packet, 
                                     uint8_t *data)
{
    msg_server_status latest_status = {0};

    socket_rx_server_status(s, packet, data, &latest_status);

    /*
     * If this is a status from a server we are not connected to, just ignore 
     * it.
     */
    if (!latest_status.you_are_playing_on_this_server) {
        return;
    }

    /*
     * Received on a socket we are no longer joined on?
     */
    if (s != client_joined_server) {
        return;
    }

    /*
     * Look for our name in the update to know the server has acked our join.
     */
    client_check_still_in_game(latest_status.level_pos);

    uint8_t redo = false;

    if (client_level) {
        level_set_level_pos(client_level, latest_status.level_pos);
        level_set_seed(client_level, latest_status.seed);
    }

    if (memcmp(&server_status.level_pos, &latest_status.level_pos,
               sizeof(level_pos_t))) {
        LOG("Client: Level no changed to %d.%d", 
            latest_status.level_pos.y,
            latest_status.level_pos.x);
        redo = true;
    }

    if (server_status.seed != latest_status.seed) {
        LOG("Client: Level seed changed to %u", 
            latest_status.seed);
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

    global_config.server_current_players = latest_status.server_current_players;
    global_config.client_level_pos = latest_status.level_pos;

    /*
     * Left a level?
     */
    if (latest_status.level_hide) {
        if (client_level) {
            things_level_destroyed(client_level, true /* keep players */);
        }

        /*
         * To allow the player to be centered in the new level if it is a 
         * different size.
         */
        if (wid_game_map_client_grid_container &&
            wid_game_map_client_grid_container->grid) {
            wid_game_map_client_grid_container->grid->bounds_locked = 0;
        }
    }

    /*
     * Left or joined a level?
     */
    if (server_status.level_hide != latest_status.level_hide) {
        if (latest_status.level_hide) {
            client_level_ending();
        } else {
            client_level_starting();
        }
    }

    /*
     * Save the received status globally. We will then compare with the player 
     * status for changes.
     */
    memcpy(&server_status, &latest_status, sizeof(server_status));

    /*
     * Check the player is not dead and gone.
     */
    if (!player) {
        return;
    }

    msg_player_state *server_stats = &server_status.player;
    thing_statsp new_stats = &server_stats->stats;
    thing_statsp old_stats = &player->stats;

    /*
     * If the server is a bit behind our changes, wait for it to merge them 
     * in.
     */
    int version_delta = 
                    (int)new_stats->client_version - 
                    (int)old_stats->client_version;

    if ((version_delta >= 0) || (version_delta < -255)) {
        /*
         * Server has latest version; (accounted for wraparond(.
         */
    } else {
        /*
         * Server is behind.
         */
        LOG("Server is behind latest version of stats, client: "
            "%d server: %d, ignore", 
            old_stats->client_version, new_stats->client_version);

        return;
    }

    /*
     * Some fields we don't care too much if they change.
     */
    thing_stats changed_stats;
    memcpy(&changed_stats, new_stats, sizeof(changed_stats));
    changed_stats.thing_id = old_stats->thing_id;
    changed_stats.on_server = old_stats->on_server;
    new_stats = &changed_stats;

    /*
     * Now see what really changed and if we need to update scores.
     */
    int changed = false;

    if (memcmp(old_stats, new_stats, sizeof(thing_stats))) {
        LOG("Client: %s player stats changed on server:", 
            thing_logname(player));

        thing_stats_diff(old_stats, new_stats);

        /*
         * If the stats change, update the inventory
         */
        memcpy(old_stats, new_stats, sizeof(thing_stats));
        thing_stats_set_on_server(player, player->on_server);

        wid_game_map_client_score_update(client_level, redo);

        /*
         * Update the weapon placement as the thing might be dying.
         */
        thing_set_weapon_placement(player);

        changed = true;
    }

    new_stats = &server_stats->stats;
    memcpy(old_stats, new_stats, sizeof(thing_stats));
    thing_stats_set_on_server(player, player->on_server);

    memcpy(&s->stats, new_stats, sizeof(thing_stats));
    stats_set_on_server(&s->stats, player->on_server);

    if (changed) {
        /*
         * Ack the stats change sen on the server, so the server knows we are 
         * up to data and can detect changes in our stats.
         */
        THING_LOG(player, "ack stats change on server");

        stats_bump_version(&player->stats);

        thing_stats_client_modified(&player->stats);
    }
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
                client_rx_server_status(s, packet, data);
                break;
            }

            case MSG_SERVER_HISCORE: {
                /*
                 * This is an update of the hiscores as the player has died.
                 */
                msg_server_hiscores latest_hiscores;

                memset(&latest_hiscores, 0, sizeof(latest_hiscores));

                LEVEL_LOG(client_level, "Received hiscores from server");

                socket_rx_server_hiscore(s, packet, data, &latest_hiscores);

                client_player_fully_dead(&latest_hiscores);
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

static void client_check_still_in_game (level_pos_t level_pos)
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

            server_connection_confirmed = true;

            LOG("Client: %s joined, ID %d", p->stats.pname, p->stats.thing_id);

            music_play_game(level_pos);

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

    MESG(CRITICAL, "Server does not report you in the game!");

    CON("Client: You are player: \"%s\", ID %u", 
        global_config.stats.pname, 
        client_joined_server_key);

    server_connection_confirmed = false;
}
