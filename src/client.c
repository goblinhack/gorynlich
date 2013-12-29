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
static socket *client_listen_socket;
static socket *client_connect_socket;

boolean client_init (void)
{
    if (client_init_done) {
        return (true);
    }

    socket *s;

    /*
     * Connector.
     */
    s = net_connect(connect_address);
    if (!s) {
        ERR("Client failed to connect");
        return (false);
    }

    client_connect_socket = s;
    s->client = true;
    LOG("Client connecting to   %s", s->remote_logname);
    LOG("Client connecting from %s", s->local_logname);

#if 0
    /*
     * Listener.
     */
    s = net_listen(ip2);
    if (!s) {
        ERR("Client failed to listen");
        return (false);
    }

    client_listen_socket = s;
    s->client = true;
    LOG("Client listening on %s", s->logname);

#endif

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

static void client_poll (void)
{
    socket *s = client_listen_socket;
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

        char *tmp = iptodynstr(connect_address);
        LOG("Client Pak rx on: %s", tmp);
        myfree(tmp);

        int y = SDLNet_Read16(packet->data);
        int x = SDLNet_Read16(packet->data+2);
        LOG("Client Recieve X,Y = %d,%d",x,y);   //not working... 
    }

    SDLNet_FreePacket(packet);
}

static void client_transmit (void)
{
static int done = 0;
if (done > 1) {
return;
}
done++;
    socket *s = client_connect_socket;
    if (!s) {
        return;
    }

    UDPpacket *packet;      

    packet = SDLNet_AllocPacket(MAX_PACKET_SIZE);
    if (!packet) {
        ERR("out of packet space, pak %d", MAX_PACKET_SIZE);
        return;
    }

    uint8_t *data = packet->data;

static int x;
static int y;
x++;
y++;
y++;
    LOG("Sending X,Y = %d,%d", x,y);

    packet->address = listen_address;
    SDLNet_Write16(y,data);                 
    SDLNet_Write16(x,data+2);               
    packet->len = sizeof(data);

    if (SDLNet_UDP_Send(s->udp_socket, s->channel, packet) < 1) {
        ERR("no UDP packet sent");
    } 
        
    SDLNet_FreePacket(packet);
}

void client_tick (void)
{
    client_poll();
    client_transmit();
}
