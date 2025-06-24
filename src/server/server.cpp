#include <enet.h>
#include <iostream>
#include <arpa/inet.h>
#include <vector>
#include "player.hpp"
#include "netmsg.hpp"
#include <spdlog/spdlog.h>

#define PORT 1234
#define MAX_CLIENTS 32
#define NUM_CHANNELS 2

std::vector<Player *> g_players;
ENetHost *server;

int generateClientId() {
    return rand();
}

void NetEvent_Disconnect(ENetEvent event) {
    Player *player = nullptr;

    for (Player *p : g_players) {
        if (p->isPeer(event.peer)) {
            player = p;
        }
    }

    if (player == nullptr) {
        spdlog::error("Peer disconnected, but player was not found");
    } else {
        g_players.erase(std::remove(g_players.begin(), g_players.end(), player), g_players.end());
        spdlog::info("{} disconnected.", player->toString());
        delete player;
    }

    event.peer->data = nullptr;
}

/*
void NetEvent_RecievedPacket(ENetEvent event) {
    Netdata *netdata = reinterpret_cast<Netdata*>(event.packet->data);

    if (netdata->type == Netdata_Type_LOCATION) {
        Netdata_Location *ndata_location = reinterpret_cast<Netdata_Location*>(netdata);
        int clientId = ndata_location->clientId;

        for (Player p : g_players) {
            if (p.clientId != clientId) {
                Netdata_Location location;
                p.sendPacket(&location, sizeof(Netdata_Location));
            }
        }
    }
}
*/

void NetEvent_InitPlayer(ENetEvent event, ClientMsg_Init *init) {
    char ip[INET6_ADDRSTRLEN];
    enet_address_get_host_ip(&event.peer->address, ip, sizeof(ip));

    int clientId = generateClientId();

    // Create new player
    Player *player = new Player();
    player->server = server;
    player->clientId = clientId;
    player->peer = event.peer;
    player->name = std::string(init->name);

    spdlog::info("{} joined", player->toString());

    for (Player *p : g_players) {
        ServerMsg_Connection connection;
        connection.type = ServerMsg_Type_CONNECTION;
        connection.clientId = clientId;
        strcpy(connection.name, player->name.c_str());

        // Send the new connection to everyone else
        p->sendPacket(&connection, sizeof(ServerMsg_Connection));
        spdlog::debug("Sent initialization update to {}", p->toString());
    }

    // Add the player
    g_players.push_back(player);
}

void NetEvent_RecievedPacket(ENetEvent event) {
    ClientMsg *msg = reinterpret_cast<ClientMsg*>(event.packet->data);

    switch (msg->type) {
    case ClientMsg_Type_INIT: {
        ClientMsg_Init *init = reinterpret_cast<ClientMsg_Init*>(msg);
        NetEvent_InitPlayer(event, init);
        break;
    }
    default:
        break;
    }
}

int main(int argc, char** argv) {
    spdlog::set_level(spdlog::level::debug);

    if (enet_initialize() != 0) {
        std::cerr << "Failed to initialize ENet.\n";
        return EXIT_FAILURE;
    }
    atexit(enet_deinitialize);

    ENetAddress address;

    address.host = ENET_HOST_ANY; // Listen on all interfaces
    address.port = PORT;

    server = enet_host_create(&address, MAX_CLIENTS, NUM_CHANNELS, 0, 0);

    if (!server) {
        std::cerr << "Failed to create ENet server host.\n";
        return EXIT_FAILURE;
    }

    std::cout << "Server started, waiting for connections...\n";

    ENetEvent event;
    while (true) {
        int result = enet_host_service(server, &event, -1);  // wait indefinitely
        if (result > 0) {
            switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                //NetEvent_Connect(event);
                break;

            case ENET_EVENT_TYPE_RECEIVE:
                NetEvent_RecievedPacket(event);
                enet_packet_destroy(event.packet);
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                NetEvent_Disconnect(event);
                break;
            }
        } else if (result < 0) {
            spdlog::error("enet_host_service error!");
            break;  // or handle error appropriately
        }
    }

    enet_host_destroy(server);

    return 0;
}

