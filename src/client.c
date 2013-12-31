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

static void client_send_ping(void);
static boolean client_init_done;
static socketp client_connect_socket;
static void client_poll(void);
static boolean client_set_name(tokens_t *tokens, void *context);

boolean client_init (void)
{
    if (!is_client) {
        return (true);
    }

    if (client_init_done) {
        return (true);
    }

    socketp s;

    /*
     * Connector.
     */
    s = socket_connect(server_address);
    if (!s) {
        WARN("Client failed to connect");
        return (false);
    }

    client_connect_socket = s;
    socket_set_client(s, true);

    LOG("Client connecting to   %s", socket_get_remote_logname(s));
    LOG("Client connecting from %s", socket_get_local_logname(s));

    command_add(client_set_name, "set name [A-Za-z0-9_-]*",
                "set client name");

    socket_set_name(client_connect_socket, dupstr("no name", "client name"));

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

static void client_send_ping (void)
{
    static uint32_t ts;
    static uint32_t seq;

    if (!time_have_x_tenths_passed_since(10, ts)) {
        return;
    }

    ts = time_get_time_cached();

    send_ping(client_connect_socket, seq++, ts);
}

void client_tick (void)
{
    if (!is_client) {
        return;
    }

    client_poll();
    client_send_ping();
}

/*
 * User has entered a command, run it
 */
boolean client_set_name (tokens_t *tokens, void *context)
{
    char *s = tokens->args[2];

    if (!client_connect_socket) {
        ERR("No open socket to name");
        return (false);
    }

    socket_set_name(client_connect_socket, dupstr(s, "socket name"));

    CON("Client name set to \"%s\"", socket_get_name(client_connect_socket));

    return (true);
}

static void client_poll (void)
{
    socketp s = client_connect_socket;
    if (!s) {
        return;
    }

    int waittime = 0;
    int numready = SDLNet_CheckSockets(socket_get_socklist(s), waittime);
    if (numready <= 0) {
        return;
    }

    UDPpacket *packet;      

    packet = SDLNet_AllocPacket(MAX_PACKET_SIZE);
    if (!packet) {
        ERR("Out of packet space, pak %d", MAX_PACKET_SIZE);
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

        uint8_t *data = packet->data;
        msg_type type = *data++;

        socket_count_inc_pak_rx(s);

        switch (type) {
        case MSG_TYPE_PONG:
            receive_pong(s, packet, data);
            break;

        case MSG_TYPE_PING:
            receive_ping(s, packet, data);
            break;

        default:
            socket_count_inc_pak_rx_bad_msg(s);
            ERR("Unknown message type received [%u]", type);
        }
    }

    SDLNet_FreePacket(packet);
}
