#pragma once
#include <map>
#include <enet.h>
#include "netmsg.hpp"

#define LISTEN_PORT 1234
#define MAX_CLIENTS 50
#define NUM_CHANNELS 2

class Client;

class Networker {
private:
    std::map<int, ENetPeer *> peers;
    ENetHost *server;

public:
    static Networker &GetInstance();
    
    int Init();
    void Update();
    void Close();

    int GetClient(ENetPeer *peer);

    void Event_MsgRecv(ENetEvent event);
    void Event_Disconnection(ENetEvent event);

    void MsgRecv_InitClient(ENetEvent event, ClientMsg_Init *msg);
    void MsgRecv_Location(ENetEvent event, ClientMsg_Location *msg);
    void MsgRecv_Shot(ENetEvent event, ClientMsg_Shot *msg);

    void InitClient(Client *client);
    void SendToClientsExcept(void *packet, int len, Client *client);
    void SendToClient(Client *client, void *data, int len);
};