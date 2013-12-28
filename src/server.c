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
static SDLNet_SocketSet socklist;
static UDPsocket udpserver;

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

    for (p = 0; p <= MAX_CLIENTS; p++, port++) {

        SDLNet_Write16(port, &server_address.port);
        port = SDLNet_Read16(&server_address.port);

        char *tmp = iptodynstr(server_address);
        LOG("Trying server on: %s", tmp);
        myfree(tmp);

        udpserver = SDLNet_UDP_Open(port);
        if (!udpserver) {
            ERR("Failed to open local socket %s", SDLNet_GetError());
            continue;
        }

        socklist = SDLNet_AllocSocketSet(MAX_CLIENTS + 1);
        if (!socklist) {
            ERR("Failed to alloc socket list %s", SDLNet_GetError());
            continue;
        }

        if (SDLNet_UDP_AddSocket(socklist, udpserver) == -1) {
            ERR("Failed to add client to socket list %s", SDLNet_GetError());
            continue;
        }

        break;
    }

    if (p == MAX_CLIENTS) {
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
    UDPpacket *packet;      
    packet = SDLNet_AllocPacket(1024);
    numready = SDLNet_CheckSockets(socklist, waittime);
    if(numready == -1)
    {
            printf("SDLNet_CheckSockets: %s\n", SDLNet_GetError());
            perror("SDLNet_CheckSockets");
    } else if (numready) 
    {
            printf("There are %d sockets with activity!\n", numready);
            // check all sockets with SDLNet_SocketReady and handle the active ones.
            if(SDLNet_SocketReady(udpserver))
            {
                    numpks = SDLNet_UDP_Recv(udpserver, packet);
                    if(numpks)
                    {
                            //process UDP xxpacket
                            y = SDLNet_Read16(packet->data); //recieve UDP packet, and set y cord to UDP packet data
                            x = SDLNet_Read16(packet->data+2); //recieve UDP packet, and set y cord to UDP packet data
                            printf("Recieve X,Y = %d,%d\n",x,y);   //not working... 
                    }       
            }       
            SDLNet_FreePacket(packet);
            packet = NULL;
    }
}
