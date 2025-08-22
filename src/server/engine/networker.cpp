#include "networker.hpp"
#include <spdlog/spdlog.h>
#include <enet.h>
#include "netmsg.hpp"

int Networker::Init() {
    if (enet_initialize() != 0) {
        spdlog::error("Failed to initialize ENet.");
        return EXIT_FAILURE;
    }
    atexit(enet_deinitialize);

    ENetAddress address;

    address.host = ENET_HOST_ANY; // Listen on all interfaces
    address.port = LISTEN_PORT;

    server = enet_host_create(&address, MAX_CLIENTS, NUM_CHANNELS, 0, 0);

    if (!server) {
        spdlog::error("Failed to create ENet server host");
        return EXIT_FAILURE;
    }

    spdlog::info("Server started, waiting for connections...");
    return 0;
}

void Networker::Update(std::function<void(Client *, ClientMsg *)> callback,
                        std::function<void(Client *)> disconnectionCallback,
                        std::function<void(Client **)> connectionCallback) {
    ENetEvent event;

    while (enet_host_service(server, &event, 0) > 0) {
        switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT: {
            Client *new_client;
            connectionCallback(&new_client);
            peers[new_client] = event.peer;
            spdlog::debug("New connection");
            break;
        }
        case ENET_EVENT_TYPE_RECEIVE: {
            ClientMsg *msg = reinterpret_cast<ClientMsg*>(event.packet->data);
            spdlog::debug("Received message of type: {}", Debug_ClientMsgToString(*msg));
            Client *client = GetClient(event.peer);
            callback(client, msg);
            enet_packet_destroy(event.packet);
            break;
        }
        case ENET_EVENT_TYPE_DISCONNECT: {
            Client *client = GetClient(event.peer);
            peers.erase(client);
            disconnectionCallback(client);
            spdlog::debug("Disconnection");
            break;
        }
        default:
            break;
        }
    }
}

void Networker::Close() {
    enet_host_destroy(server);
}

Client *Networker::GetClient(ENetPeer *peer) {
    for (const auto &pair : peers) {
        if (pair.second == peer) {
            return pair.first;
        }
    }

    spdlog::error("Networker (GetClient): Client not found with peer!");
    return nullptr;
}

// void Networker::Event_MsgRecv(ENetEvent event) {
//     ClientMsg *msg = reinterpret_cast<ClientMsg*>(event.packet->data);

//     switch (msg->type) {
//     case ClientMsg_Type_INIT: {
//         ClientMsg_Init *init = reinterpret_cast<ClientMsg_Init*>(msg);
//         MsgRecv_InitClient(event, init);
//         break;
//     }
//     case ClientMsg_Type_LOCATION: {
//         ClientMsg_Location *location = reinterpret_cast<ClientMsg_Location*>(msg);
//         MsgRecv_Location(event, location);
//         break;
//     }
//     case ClientMsg_Type_SHOT: {
//         ClientMsg_Shot *shotMsg = reinterpret_cast<ClientMsg_Shot*>(msg);
//         MsgRecv_Shot(event, shotMsg);
//         break;
//     }
//     case ClientMsg_Type_REQUEST_SPAWN: {
//         ClientMsg_RequestSpawn *reqMsg = reinterpret_cast<ClientMsg_RequestSpawn*>(msg);
//         MsgRecv_RequestSpawn(event, reqMsg);
//         break;
//     }
//     case ClientMsg_Type_SPAWN: {
//         ClientMsg_Spawn *reqMsg = reinterpret_cast<ClientMsg_Spawn*>(msg);
//         MsgRecv_Spawn(event, reqMsg);
//         break;
//     }
//     default:
//         spdlog::debug("Msg type {} not found", (int) msg->type);
//         break;
//     }
// }

// void Networker::Event_Disconnection(ENetEvent event) {
//     Client *client = Client::GetClient(GetClient(event.peer));
//     spdlog::debug("Client leaving: {}", client->ToString());

//     spdlog::info("Clients:");
//     for (const auto &pair : peers) {
//         spdlog::info("- {}: {}", pair.first, pair.second->connectID);
//     }

//     if (client == nullptr) {
//         spdlog::error("Peer disconnected, but player was not found");
//     } else {
//         client = Client::GetClient(GetClient(event.peer));
//         spdlog::info("{} disconnected.", client->ToString());

//         ServerMsg_Disconnection msg;
//         msg.type = ServerMsg_Type_DISCONNECTION;
//         msg.clientId = client->GetId();
//         SendToClientsExcept(&msg, sizeof(msg), client);
//         peers.erase(client->GetId());
//         delete client;
//     }

//     event.peer->data = nullptr;
// }

// void Networker::MsgRecv_InitClient(ENetEvent event, ClientMsg_Init *init) {
//     char ip[INET6_ADDRSTRLEN];
//     enet_address_get_host_ip(&event.peer->address, ip, sizeof(ip));

//     // Create new player
//     Client *new_client = new Client(init->name);
//     peers[new_client->GetId()] = event.peer;
//     spdlog::info("{} joined", new_client->ToString());

//     ServerMsg_Connection connection;
//     connection.type = ServerMsg_Type_CONNECTION;
//     connection.clientId = new_client->GetId();
//     strcpy(connection.name, new_client->GetName().c_str());
//     SendToClientsExcept(&connection, sizeof(connection), new_client);

//     for (Client *client : Client::clients) {
//         if (client == new_client) continue;
//         // Send everyone else to this player
//         ServerMsg_Connection conn;
//         conn.type = ServerMsg_Type_CONNECTION;
//         conn.clientId = client->GetId();
//         strcpy(conn.name, client->GetName().c_str());
//         SendToClient(new_client, &conn, sizeof(conn));
//     }
    
//     spdlog::info("Clients:");
//     for (const auto &pair : peers) {
//         spdlog::info("- {}: {}", pair.first, pair.second->connectID);
//     }
// }

// void Networker::MsgRecv_Location(ENetEvent event, ClientMsg_Location *msg) {
//     Client *client = Client::GetClient(GetClient(event.peer));
//     client->SetLocation(msg->location);

//     ServerMsg_Location loc;
//     loc.type = ServerMsg_Type_LOCATION;
//     loc.clientId = client->GetId();
//     loc.location = client->GetLocation();
//     SendToClientsExcept(&loc, sizeof(loc), client);
// }

// void Networker::MsgRecv_Shot(ENetEvent event, ClientMsg_Shot *msg) {
//     Client *client = Client::GetClient(GetClient(event.peer));
//     Shot *shot = new Shot(msg->localShotId, client, msg->position, msg->velocity);

//     ServerMsg_Shot shotMsg;
//     shotMsg.type = ServerMsg_Type_SHOT;
//     shotMsg.globalShotId = shot->GetGlobalId();
//     shotMsg.position = msg->position;
//     shotMsg.velocity = msg->velocity;
//     SendToClientsExcept(&shotMsg, sizeof(shotMsg), client);
// }

// void Networker::MsgRecv_RequestSpawn(ENetEvent event, ClientMsg_RequestSpawn *msg) {
//     Client *client = Client::GetClient(GetClient(event.peer));
//     ServerMsg_AllowSpawn allowMsg;
//     allowMsg.type = ServerMsg_Type_ALLOW_SPAWN;
//     allowMsg.location.position = glm::vec3(0, 5, 0);
//     allowMsg.location.rotation = glm::quat(glm::vec3(0, 0, 0));
//     SendToClient(client, &allowMsg, sizeof(allowMsg));
// }

// void Networker::MsgRecv_Spawn(ENetEvent event, ClientMsg_Spawn *msg) {
//     spdlog::debug("Received spawn message for client {}", event.peer->connectID);
//     Client *client = Client::GetClient(GetClient(event.peer));
//     client->SetAlive(true);
//     ServerMsg_Spawn spawnMsg;
//     spawnMsg.type = ServerMsg_Type_SPAWN;
//     spawnMsg.clientId = client->GetId();
//     spawnMsg.location = client->GetLocation();
//     SendToClientsExcept(&spawnMsg, sizeof(spawnMsg), client);
// }

// void Networker::MsgSend_RemoveShot(Shot *shot) {
//     // Remove from owner (local ID)
//     ServerMsg_RemoveShot msgOwner;
//     msgOwner.type = ServerMsg_Type_REMOVE_SHOT;
//     msgOwner.shotId = shot->GetLocalId();
//     msgOwner.clientId = 0;
//     SendToClient(shot->GetOwner(), &msgOwner, sizeof(msgOwner));

//     // Remove from everyone else (global ID)
//     ServerMsg_RemoveShot msgRest;
//     msgRest.type = ServerMsg_Type_REMOVE_SHOT;
//     msgRest.shotId = shot->GetGlobalId();
//     msgRest.clientId = shot->GetOwner()->GetId();
//     SendToClientsExcept(&msgRest, sizeof(msgRest), shot->GetOwner());
// }

// void Networker::MsgSend_Die(Client *client, Shot *shot) {
//     client->SetAlive(false);

//     ServerMsg_Death msg;
//     msg.type = ServerMsg_Type_DEATH;
//     msg.clientId = 0;
//     SendToClient(client, &msg, sizeof(msg));

//     // Also send to all other clients
//     msg.clientId = client->GetId();
//     SendToClientsExcept(&msg, sizeof(msg), client);
// }

// void Networker::SendToClientsExcept(void *packet, int len, Client *client) {
//     for (const auto &pair : peers) {
//         if (client->IsPeer(pair.first)) continue;
//         ENetPacket* enetPacket = enet_packet_create(packet, len, ENET_PACKET_FLAG_RELIABLE);
//         enet_peer_send(pair.second, 0, enetPacket);
//     }
// }

// void Networker::SendToClient(Client *client, void *data, int len) {
//     auto it = peers.find(client->GetId());
//     if (it == peers.end()) {
//         // Client not found
//         return;
//     }

//     ENetPeer* peer = it->second;
//     ENetPacket* packet = enet_packet_create(data, len, ENET_PACKET_FLAG_RELIABLE);
//     enet_peer_send(peer, 0, packet);
// }