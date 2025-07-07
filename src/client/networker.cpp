#include "networker.hpp"
#include <vector>
#include <string.h>
#include <spdlog/spdlog.h>
#include <glm/glm.hpp>
#include "client.hpp"
#include "shot.hpp"
#include "netmsg.hpp"
#include "player.hpp"

Networker &Networker::GetInstance() {
    static Networker instance;
    return instance;
}

void Networker::MsgRecv(ENetEvent event) {
    ServerMsg *msg = reinterpret_cast<ServerMsg*>(event.packet->data);

    switch (msg->type) {
    case ServerMsg_Type_CONNECTION: {
        ServerMsg_Connection *conn = reinterpret_cast<ServerMsg_Connection*>(msg);
        MsgRecv_Connection(conn);
        break;
    }
    case ServerMsg_Type_LOCATION: {
        ServerMsg_Location *loc = reinterpret_cast<ServerMsg_Location*>(msg);
        MsgRecv_Location(event, loc);
        break;
    }
    case ServerMsg_Type_DISCONNECTION: {
        ServerMsg_Disconnection *dis = reinterpret_cast<ServerMsg_Disconnection*>(msg);
        MsgRecv_Disconnection(dis);
        break;
    }
    case ServerMsg_Type_SHOT: {
        ServerMsg_Shot *msgShot = reinterpret_cast<ServerMsg_Shot*>(msg);
        MsgRecv_Shot(msgShot);
        break;
    }
    default:
        break;
    }
}

int Networker::Init() {
    if (enet_initialize() != 0) {
        spdlog::error("Failed to initialize ENet.");
        return EXIT_FAILURE;
    }
    atexit(enet_deinitialize);

    // Create the ENet client host
    client = enet_host_create(nullptr, 1, 2, 0, 0);
    if (!client) {
        spdlog::error("Failed to create ENet client host.");
        return EXIT_FAILURE;
    }

    // Set up the server address
    ENetAddress address;
    enet_address_set_host(&address, "127.0.0.1");
    address.port = 1234;

    // Connect to the server
    server = enet_host_connect(client, &address, 2, 0);
    if (!server) {
        spdlog::error("No available peers for initiating connection.");
        enet_host_destroy(client);
        return EXIT_FAILURE;
    }

    // Wait for the connection to succeed (with timeout)
    ENetEvent event;
    if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        spdlog::debug("Connected to server.");
    } else {
        spdlog::info("Connection to server failed.");
        enet_peer_reset(server);
        enet_host_destroy(client);
        return EXIT_FAILURE;
    }

    enet_host_flush(client);
    return 0;
}

void Networker::Close() {
    enet_peer_disconnect(server, 0);
    enet_host_flush(client);
    enet_host_destroy(client);
}

void Networker::MsgSend_Init() {
    ClientMsg_Init init;
    init.type = ClientMsg_Type_INIT;
    strcpy(init.name, Player::GetInstance().GetName().c_str());
    ENetPacket* packet = enet_packet_create(&init, sizeof(init), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(server, 0, packet);
    enet_host_flush(client);
}

void Networker::MsgSend_Location() {
    ClientMsg_Location loc;
    loc.type = ClientMsg_Type_LOCATION;
    loc.location = Player::GetInstance().GetLocation();
    //spdlog::info("loc is x={}, y={}, z={}", loc.location.position.x, loc.location.position.y, loc.location.position.z);    
    ENetPacket* packet = enet_packet_create(&loc, sizeof(loc), ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);
    enet_peer_send(server, 0, packet);
}

void Networker::MsgSend_Shot(Shot *shot) {
    ClientMsg_Shot msgShot;
    msgShot.type = ClientMsg_Type_SHOT;
    msgShot.position = shot->GetPosition();
    msgShot.velocity = shot->GetVelocity();
    ENetPacket* packet = enet_packet_create(&msgShot, sizeof(msgShot), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(server, 0, packet);
}

void Networker::Update() {
    ENetEvent event;
    
    while (enet_host_service(client, &event, 0) > 0) {
        switch (event.type) {
        case ENET_EVENT_TYPE_RECEIVE:
            MsgRecv(event);
            enet_packet_destroy(event.packet);
            break;

        default:
            break;
        }
    }
}

void Networker::MsgRecv_Connection(ServerMsg_Connection *msg) {
    Client *client = new Client(msg->clientId, msg->name);
    spdlog::debug("{} joined", client->ToString());
}

void Networker::MsgRecv_Disconnection(ServerMsg_Disconnection *msg) {
    Client *client = Client::GetClient(msg->clientId);

    if (client == nullptr) {
        spdlog::error("Client disconnected, but client was not found");
    } else {
        spdlog::debug("{} disconnected.", client->GetName());
        delete client;
    }
}

void Networker::MsgRecv_Location(ENetEvent event, ServerMsg_Location *msg) {
    Client *client = Client::GetClient(msg->clientId);
    spdlog::info("Updating {} location x={},y={},z={}", client->ToString(), msg->location.position.x, msg->location.position.y, msg->location.position.z);

    if (!client)
        spdlog::error("Client with clientId {} does not exist", msg->clientId);

    client->SetLocation(msg->location);
}

void Networker::MsgRecv_Shot(ServerMsg_Shot *msg) {
    Shot *shot = new Shot(msg->clientId, msg->position, msg->velocity);
}