#pragma once
#include "raylib.h"
#include "netmsg.hpp"
#include <enet.h>

void Render_Init();
void Render_Draw();
void parseArgs(int argc, char *argv[]);
int initNet();
void NetUpdate_Init();
void NetUpdate_Location(Vector3 position);
void NetEvent_PlayerConnection(ENetEvent event, ServerMsg_Connection *conn);
void NetEvent_UpdatePlayerLocation(ENetEvent event, ServerMsg_Location *loc);
void NetEvent_RecievedPacket(ENetEvent event);
void NetEvent_Listener();
void Listen_Input();
