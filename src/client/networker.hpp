#pragma once
#include "enet.h"
#include "netmsg.hpp"

class Shot;

class Networker {
private:
    ENetHost* client;
    ENetPeer* server;

    void MsgRecv(ENetEvent event);

public:
    static Networker &GetInstance();

    int Init();
    void Update(); 
    void Close();

    void MsgSend_Init();
    void MsgSend_Location();
    void MsgSend_Shot(Shot *shot);
    
    void MsgRecv_Connection(ServerMsg_Connection *msg);
    void MsgRecv_Disconnection(ServerMsg_Disconnection *msg);
    void MsgRecv_Location(ENetEvent event, ServerMsg_Location *msg);
    void MsgRecv_Shot(ServerMsg_Shot *msg);
};