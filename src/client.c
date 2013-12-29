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
#include "client.h"

static void client_transmit(void);
static boolean client_init_done;

boolean client_init (void)
{
    if (client_init_done) {
        return (true);
    }

    socket *s;

    s = net_listen(no_address);
    if (!s) {
        ERR("Client failed to listen");
        return (false);
    }

    s->server = true;

    LOG("Client listening on %s", s->logname);

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

void client_poll (void)
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
            char *tmp = iptodynstr(connect_address);
            ERR("Pak rx failed on: %s: %s", tmp, SDLNet_GetError());
            myfree(tmp);
            continue;
        }

        char *tmp = iptodynstr(connect_address);
        LOG("Pak rx on: %s", tmp);
        myfree(tmp);

        int y = SDLNet_Read16(packet->data);
        int x = SDLNet_Read16(packet->data+2);
        LOG("Recieve X,Y = %d,%d",x,y);   //not working... 
    }

    SDLNet_FreePacket(packet);
client_transmit();
}

static void client_transmit (void)
{
    UDPpacket *packet;      

    packet = SDLNet_AllocPacket(MAX_PACKET_SIZE);
    if (!packet) {
        ERR("out of packet space, pak %d", MAX_PACKET_SIZE);
        return;
    }

    uint8_t data[4];
    memset(data, 0, sizeof(data));

static int x;
static int y;
x++;
y++;
y++;
    LOG("Sending X,Y = %d,%d\n", x,y);

    packet->address = listen_address;
    SDLNet_Write16(y,data);                 
    SDLNet_Write16(x,data+2);               
    packet->len = sizeof(data);
    packet->data = (Uint8 *)data;

#if 0
    if (SDLNet_UDP_Send(udpclient, channel_server, packet) < 1) {
        ERR("no UDP packet sent");
    } 
        
#endif
    SDLNet_FreePacket(packet);
}
