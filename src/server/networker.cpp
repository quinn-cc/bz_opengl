#include "networker.hpp"
#include <spdlog/spdlog.h>
#include <enet.h>
#include "netmsg.hpp"
#include "client.hpp"
#include "shot.hpp"

Networker &Networker::GetInstance() {
    static Networker instance;
    return instance;
}

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

void Networker::Update() {
    ENetEvent event;
    int result = enet_host_service(server, &event, 0);

    if (result > 0) {
        switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT:
            //NetEvent_Connect(event);
            break;

        case ENET_EVENT_TYPE_RECEIVE:
            Event_MsgRecv(event);
            enet_packet_destroy(event.packet);
            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            Event_Disconnection(event);
            break;
        }
    } else if (result < 0) {
        spdlog::error("enet_host_service error!");
        exit(1);
    }
}

void Networker::Close() {
    enet_host_destroy(server);
}

int Networker::GetClient(ENetPeer *peer) {
    for (const auto &pair : peers) {
        if (pair.second == peer) {
            return pair.first;
        }
    }

    spdlog::error("Client not found with peer!");
    return -1;
}

void Networker::Event_MsgRecv(ENetEvent event) {
    ClientMsg *msg = reinterpret_cast<ClientMsg*>(event.packet->data);

    switch (msg->type) {
    case ClientMsg_Type_INIT: {
        ClientMsg_Init *init = reinterpret_cast<ClientMsg_Init*>(msg);
        MsgRecv_InitClient(event, init);
        break;
    }
    case ClientMsg_Type_LOCATION: {
        ClientMsg_Location *location = reinterpret_cast<ClientMsg_Location*>(msg);
        MsgRecv_Location(event, location);
        break;
    }
    case ClientMsg_Type_SHOT: {
        ClientMsg_Shot *shotMsg = reinterpret_cast<ClientMsg_Shot*>(msg);
        MsgRecv_Shot(event, shotMsg);
    }
    default:
        break;
    }
}

void Networker::Event_Disconnection(ENetEvent event) {
    Client *client = Client::GetClient(GetClient(event.peer));

    if (client == nullptr) {
        spdlog::error("Peer disconnected, but player was not found");
    } else {
        client = Client::RemoveClient(GetClient(event.peer));
        spdlog::info("{} disconnected.", client->ToString());

        ServerMsg_Disconnection msg;
        msg.type = ServerMsg_Type_DISCONNECTION;
        msg.clientId = client->GetId();
        SendToClientsExcept(&msg, sizeof(msg), client);
        peers.erase(client->GetId());
        delete client;
    }

    event.peer->data = nullptr;
}

void Networker::MsgRecv_InitClient(ENetEvent event, ClientMsg_Init *init) {
    char ip[INET6_ADDRSTRLEN];
    enet_address_get_host_ip(&event.peer->address, ip, sizeof(ip));

    // Create new player
    Client *new_client = new Client(init->name);
    peers[new_client->GetId()] = event.peer;
    spdlog::info("{} joined", new_client->ToString());

    ServerMsg_Connection connection;
    connection.type = ServerMsg_Type_CONNECTION;
    connection.clientId = new_client->GetId();
    strcpy(connection.name, new_client->GetName().c_str());
    SendToClientsExcept(&connection, sizeof(connection), new_client);

    for (Client *client : Client::clients) {
        if (client == new_client) continue;
        // Send everyone else to this player
        ServerMsg_Connection conn;
        conn.type = ServerMsg_Type_CONNECTION;
        conn.clientId = client->GetId();
        strcpy(conn.name, client->GetName().c_str());
        SendToClient(new_client, &conn, sizeof(conn));
    }
}

void Networker::MsgRecv_Location(ENetEvent event, ClientMsg_Location *msg) {
    Client *client = Client::GetClient(GetClient(event.peer));
    client->SetLocation(msg->location);

    ServerMsg_Location loc;
    loc.type = ServerMsg_Type_LOCATION;
    loc.clientId = client->GetId();
    loc.location = client->GetLocation();
    SendToClientsExcept(&loc, sizeof(loc), client);
}

void Networker::MsgRecv_Shot(ENetEvent event, ClientMsg_Shot *msg) {
    Client *client = Client::GetClient(GetClient(event.peer));
    Shot *shot = new Shot(client, msg->position, msg->velocity);

    ServerMsg_Shot shotMsg;
    shotMsg.type = ServerMsg_Type_SHOT;
    shotMsg.clientId = client->GetId();
    shotMsg.position = msg->position;
    shotMsg.velocity = msg->velocity;
    SendToClientsExcept(&shotMsg, sizeof(shotMsg), client);
}

void Networker::SendToClientsExcept(void *packet, int len, Client *client) {
    for (const auto &pair : peers) {
        if (client->IsPeer(pair.first)) continue;
        ENetPacket* enetPacket = enet_packet_create(packet, len, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(pair.second, 0, enetPacket);
    }
}

void Networker::SendToClient(Client *client, void *data, int len) {
    auto it = peers.find(client->GetId());
    if (it == peers.end()) {
        // Client not found
        return;
    }

    ENetPeer* peer = it->second;
    ENetPacket* packet = enet_packet_create(data, len, ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, 0, packet);
}