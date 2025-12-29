#include "engine/components/server_network.hpp"
#include "spdlog/spdlog.h"

ServerNetwork::ServerNetwork(uint16_t port, int maxClients, int numChannels) {
    if (enet_initialize() != 0) {
        spdlog::error("Failed to initialize ENet");
        exit(1);
    }

    atexit(enet_deinitialize);

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port;

    server = enet_host_create(&address, maxClients, numChannels, 0, 0);
    if (server == nullptr) {
        spdlog::error("ServerNetwork::ServerNetwork: Failed to create ENet server host.");
        return;
    }

    spdlog::info("Server started on port {}", port);
}

ServerNetwork::~ServerNetwork() {
    enet_host_destroy(server);
}

void ServerNetwork::flushPeekedMessages() {
    receivedMessages.erase(
        std::remove_if(
            receivedMessages.begin(),
            receivedMessages.end(),
            [](const MsgData& msgData) {
                if (msgData.peeked) {
                    if (msgData.packet == nullptr) {
                        // For connection/disconnection messages
                        // we allocated the message on the heap
                        delete msgData.msg;
                    } else {
                        enet_packet_destroy(msgData.packet);
                    }
                }
                return msgData.peeked;
            }
        ),
        receivedMessages.end()
    );
}

client_id ServerNetwork::getClient(ENetPeer *peer) {
    for (const auto& [id, p] : clients) {
        if (p == peer) {
            return id;
        }
    }

    spdlog::warn("ServerNetwork::getClient: Peer not found in clients map.");
    return static_cast<client_id>(0); // Invalid client_id
}

client_id ServerNetwork::getNextClientId() {
    client_id id = FIRST_CLIENT_ID;
    while (clients.find(id) != clients.end()) {
        ++id;
    }
    return id;
}

void ServerNetwork::update() {
    ENetEvent event;

    while (enet_host_service(server, &event, 0) > 0) {
        switch (event.type) {
        case ENET_EVENT_TYPE_RECEIVE: {
            ClientMsg *msg = reinterpret_cast<ClientMsg*>(event.packet->data);
            msg->clientId = getClient(event.peer);
            receivedMessages.push_back({ event.packet, msg });
            break;
        }
        case ENET_EVENT_TYPE_CONNECT: {
            client_id newClientId = getNextClientId();
            clients[newClientId] = event.peer;
            ClientMsg_Connection* connMsg = new ClientMsg_Connection();
            connMsg->clientId = newClientId;
            enet_address_get_host_ip(&event.peer->address, connMsg->ip, sizeof(connMsg->ip));
            receivedMessages.push_back({ nullptr, connMsg });
            break;
        }
        case ENET_EVENT_TYPE_DISCONNECT: {
            client_id discClientId = getClient(event.peer);
            clients.erase(discClientId);
            ClientMsg_Disconnection* discMsg = new ClientMsg_Disconnection();
            discMsg->clientId = discClientId;
            receivedMessages.push_back({ nullptr, discMsg });
            break;
        }
        default:
            break;
        }
    }
}

std::vector<client_id> ServerNetwork::getClients() const {
    std::vector<client_id> clientIds;
    for (const auto& [id, peer] : clients) {
        clientIds.push_back(id);
    }
    return clientIds;
}