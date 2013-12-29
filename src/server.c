/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 *
 * A useful hack to help debug stale/free/bad pointers.
 */


#include "SDL_timer.h"

#include "main.h"
#include "net.h"
#include "server.h"

static boolean server_init_done;
static socket *server_socket;

boolean server_init (void)
{
    if (!is_server) {
        return (true);
    }

    if (server_init_done) {
        return (true);
    }

    socket *s;

    s = net_listen(listen_address);
    if (!s) {
        ERR("Server failed to listen");
        return (false);
    }

    LOG("Server listening on %s", s->local_logname);

    server_socket = s;
    s->server = true;

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
    socket *s = server_socket;

    if (!s) {
        return;
    }

    int waittime = 0;
    int numready = SDLNet_CheckSockets(s->socklist, waittime);
    if (numready <= 0) {
        return;
    }

    LOG("There are %d sockets with activity!", numready);

    UDPpacket *packet;      

    packet = SDLNet_AllocPacket(MAX_PACKET_SIZE);
    if (!packet) {
        ERR("out of packet space, pak %d", MAX_PACKET_SIZE);
        return;
    }

    int i;
    for (i = 0; i < numready; i++) {
        if (!SDLNet_SocketReady(s->udp_socket)) {
            continue;
        }

        int paks = SDLNet_UDP_Recv(s->udp_socket, packet);
        if (paks != 1) {
            char *tmp = iptodynstr(connect_address);
            ERR("Pak rx failed on: %s: %s", tmp, SDLNet_GetError());
            myfree(tmp);
            continue;
        }

        char *tmp = iptodynstr(packet->address);
        LOG("Server Pak rx on: %s", tmp);
        myfree(tmp);

        int y = SDLNet_Read16(packet->data);
        int x = SDLNet_Read16(packet->data+2);
        LOG("Server Recieve X,Y = %d,%d",x,y);   //not working... 
    }

    SDLNet_FreePacket(packet);
}

void server_tick (void)
{
    if (!is_server) {
        return;
    }

    server_poll();
}

