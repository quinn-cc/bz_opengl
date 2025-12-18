#include "engine/components/client_network.hpp"
#include "spdlog/spdlog.h"

ClientNetwork::ClientNetwork() {
    client = enet_host_create(nullptr, 1, 2, 0, 0);
    if (client == nullptr) {
        spdlog::error("ClientNetwork::connect: Failed to create ENet client host.");
        return;
    }
}

ClientNetwork::~ClientNetwork() {
    enet_peer_disconnect(server, 0);
    enet_host_flush(client);
    enet_host_destroy(client);
}

void ClientNetwork::update() {
    ENetEvent event;
    
    while (enet_host_service(client, &event, 0) > 0) {
        switch (event.type) {
        case ENET_EVENT_TYPE_RECEIVE: {
            ServerMsg *msg = reinterpret_cast<ServerMsg*>(event.packet->data);

            void *raw = malloc(event.packet->dataLength);
            memcpy(raw, event.packet->data, event.packet->dataLength);
            ServerMsg *msgCopy = reinterpret_cast<ServerMsg*>(raw);
            receivedMessages.push_back(msgCopy);

            enet_packet_destroy(event.packet);
            break;
        }
        case ENET_EVENT_TYPE_DISCONNECT: {
            spdlog::info("Disconnected from server.");
            break;
        }
        case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT: {
            spdlog::info("Disconnected from server due to timeout.");
            break;
        }
        default:
            break;
        }
    }
}

void ClientNetwork::connect(const std::string &addr, uint16_t port) {
    // Set up the server address
    ENetAddress address;
    enet_address_set_host(&address, addr.c_str());
    address.port = port;

    // Connect to the server
    server = enet_host_connect(client, &address, 2, 0);
    if (!server) {
        spdlog::error("No available peers for initiating connection.");
        enet_host_destroy(client);
    }

    // Wait for the connection to succeed (with timeout)
    ENetEvent event;
    if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        spdlog::debug("Connected to server.");
    } else {
        spdlog::info("Connection to server failed.");
        enet_peer_reset(server);
        enet_host_destroy(client);
    }

    enet_host_flush(client);
}

void ClientNetwork::pop(ServerMsg* msg) {
    auto it = std::find(receivedMessages.begin(), receivedMessages.end(), msg);
    if (it != receivedMessages.end()) {
        receivedMessages.erase(it);
        delete msg;
    }
}