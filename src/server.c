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

typedef struct {
    boolean open;
    UDPsocket udp_socket;
    IPaddress client;
} socket;

typedef struct {
    socket sockets[MAX_SOCKETS];
    SDLNet_SocketSet socklist;
} network;

network net;
static boolean server_init_done;

boolean server_init (void)
{
    if (server_init_done) {
        return (true);
    }

    IPaddress no_address = {0};
    uint16_t p;

    /*
     * If no server address is given, try and start one.
     */
    if (!memcmp(&no_address, &server_address, sizeof(no_address))) {
        if ((SDLNet_ResolveHost(&server_address, DEFAULT_HOST,
                                DEFAULT_PORT)) == -1) {
            ERR("cannot resolve host %s port %d", DEFAULT_HOST, 
                DEFAULT_PORT);
            return (false);
        }

        if (!memcmp(&no_address, &server_address, sizeof(no_address))) {
            ERR("cannot get a local port to start the server");
            return (false);
        }
    }

    uint16_t port = SDLNet_Read16(&server_address.port);

    for (p = 0; p <= MAX_SOCKETS; p++, port++) {

        SDLNet_Write16(port, &server_address.port);
        port = SDLNet_Read16(&server_address.port);

        char *tmp = iptodynstr(server_address);
        LOG("Trying server on: %s", tmp);
        myfree(tmp);

        net.sockets[0].udp_socket = SDLNet_UDP_Open(port);
        if (!net.sockets[0].udp_socket) {
            ERR("Failed to open local socket %s", SDLNet_GetError());
            continue;
        }

        net.sockets[0].client = server_address;
        net.sockets[0].open = true;

        net.socklist = SDLNet_AllocSocketSet(MAX_SOCKETS);
        if (!net.socklist) {
            ERR("Failed to alloc socket list %s", SDLNet_GetError());
            continue;
        }

        if (SDLNet_UDP_AddSocket(net.socklist, net.sockets[0].udp_socket) == -1) {
            ERR("Failed to add client to socket list %s", SDLNet_GetError());
            continue;
        }

        break;
    }

    if (p == MAX_SOCKETS) {
        ERR("Failed to start server");
        return (false);
    }

    char *tmp = iptodynstr(server_address);

    LOG("Started server successfully on: %s", tmp);

    myfree(tmp);

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
            char *tmp = iptodynstr(server_address);
            ERR("Pak rx failed on: %s: %s", tmp, SDLNet_GetError());
            myfree(tmp);
            continue;
        }

        char *tmp = iptodynstr(server_address);
        LOG("Pak rx on: %s", tmp);
        myfree(tmp);

        int y = SDLNet_Read16(packet->data);
        int x = SDLNet_Read16(packet->data+2);
        LOG("Recieve X,Y = %d,%d",x,y);   //not working... 
    }

    SDLNet_FreePacket(packet);
}
