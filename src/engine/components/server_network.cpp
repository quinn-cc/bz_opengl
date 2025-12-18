#include "engine/components/server_network.hpp"
#include "spdlog/spdlog.h"

ServerNetwork::ServerNetwork(uint16_t port,
    std::function<void(client_id)> connectionCallback,
    std::function<void(client_id)> disconnectionCallback,
    int maxClients, int numChannels) {

    this->connectionCallback = connectionCallback;
    this->disconnectionCallback = disconnectionCallback;

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
    client_id id = 0;
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

            void *raw = malloc(event.packet->dataLength);
            memcpy(raw, event.packet->data, event.packet->dataLength);
            ClientMsg *msgCopy = reinterpret_cast<ClientMsg*>(raw);
            receivedMessages.push_back(msgCopy);

            break;
        }
        case ENET_EVENT_TYPE_CONNECT: {
            client_id newClientId = getNextClientId();
            void *peer = malloc(sizeof(ENetPeer));
            memcpy(peer, event.peer, sizeof(ENetPeer));
            clients[newClientId] = reinterpret_cast<ENetPeer*>(peer);
            connectionCallback(newClientId);
            break;
        }
        case ENET_EVENT_TYPE_DISCONNECT: {
            client_id discClientId = getClient(event.peer);
            disconnectionCallback(discClientId);
            free(clients[discClientId]);
            clients.erase(discClientId);
            break;
        }
        default:
            break;
        }
    
        enet_packet_destroy(event.packet);
    }
}

void ServerNetwork::pop(ClientMsg* msg) {
    auto it = std::find(receivedMessages.begin(), receivedMessages.end(), msg);
    if (it != receivedMessages.end()) {
        receivedMessages.erase(it);
        delete msg;
    }
}

std::vector<client_id> ServerNetwork::getClients() const {
    std::vector<client_id> clientIds;
    for (const auto& [id, peer] : clients) {
        clientIds.push_back(id);
    }
    return clientIds;
}