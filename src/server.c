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

static boolean server_init_done;
static socketp server_socket;

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
    socket_set_server(s, true);

    server_init_done = true;

    return (true);
}

void server_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (server_init_done) {
        server_init_done = false;
    }
}

static void server_poll (void)
{
    socketp s = server_socket;

    if (!s) {
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

        socketp s = socket_find_remote_ip(packet->address);
        if (!s) {
            s = socket_connect(packet->address);
            if (!s) {
                ERR("Pak rx failed to create client");
                continue;
            }
        }

        uint8_t *data = packet->data;
        msg_type type = SDLNet_Read16(data);
        data += sizeof(uint16_t);

        socket_count_inc_pak_rx(s);

        switch (type) {
        case MSG_TYPE_PING:
            receive_ping(s, packet, data);
            break;

        case MSG_TYPE_PONG:
            receive_pong(s, packet, data);
            break;

        default:
            socket_count_inc_pak_rx_bad_msg(s);
            ERR("Unknown message type received [%u", type);
        }
    }

    SDLNet_FreePacket(packet);
}

static void server_send_ping (void)
{
    static uint32_t ts;
    static uint32_t seq;

    if (!time_have_x_tenths_passed_since(10, ts)) {
        return;
    }

    ts = time_get_time_cached();

    int si;

    for (si = 0; si < MAX_SOCKETS; si++) {
        const socketp s = socket_get(si);
        if (!s) {
            continue;
        }

        if (!socket_get_open(s)) {
            continue;
        }

        if (socket_get_server(s)) {
            continue;
        }

        ts = time_get_time_cached();
        send_ping(s, seq, ts);
    }

    seq++;

    /*
     * Every 10 seconds check for dead peers.
     */
    if (seq % 10) {
        sockets_alive_check();
    }
}

void server_tick (void)
{
    if (!is_server) {
        return;
    }

    server_poll();
    server_send_ping();
}
