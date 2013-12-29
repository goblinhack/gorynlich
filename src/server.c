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

boolean server_init (void)
{
    if (server_init_done) {
        return (true);
    }

    socket *s;

    s = net_listen(listen_address);
    if (!s) {
        ERR("Server failed to listen");
        return (false);
    }

    LOG("Server listening on %s", s->logname);

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

void server_tick (void)
{
    int waittime = 0;
    int numready = SDLNet_CheckSockets(net.socklist, waittime);
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
        if (!net.sockets[i].open) {
            continue;
        }

        if (!SDLNet_SocketReady(net.sockets[i].udp_socket)) {
            continue;
        }

        int paks = SDLNet_UDP_Recv(net.sockets[0].udp_socket, packet);
        if (paks != 1) {
            char *tmp = iptodynstr(listen_address);
            ERR("Pak rx failed on: %s: %s", tmp, SDLNet_GetError());
            myfree(tmp);
            continue;
        }

        char *tmp = iptodynstr(listen_address);
        LOG("Pak rx on: %s", tmp);
        myfree(tmp);

        int y = SDLNet_Read16(packet->data);
        int x = SDLNet_Read16(packet->data+2);
        LOG("Recieve X,Y = %d,%d",x,y);   //not working... 
    }

    SDLNet_FreePacket(packet);
}
