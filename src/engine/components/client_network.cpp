#include "engine/components/client_network.hpp"
#include "spdlog/spdlog.h"

ClientNetwork::ClientNetwork() {
    if (enet_initialize() != 0) {
        spdlog::error("Failed to initialize ENet");
        exit(1);
    }

    atexit(enet_deinitialize);

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

void ClientNetwork::flushPeekedMessages() {
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

void ClientNetwork::update() {
    ENetEvent event;
    
    while (enet_host_service(client, &event, 0) > 0) {
        switch (event.type) {
        case ENET_EVENT_TYPE_RECEIVE: {
            ServerMsg *msg = reinterpret_cast<ServerMsg*>(event.packet->data);
            receivedMessages.push_back({ event.packet, msg });
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

bool ClientNetwork::connect(const std::string &addr, uint16_t port, int timeoutMs) {
    bool ret = false;
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
    if (enet_host_service(client, &event, timeoutMs) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        spdlog::info("Connected to server.");
        ret = true;
    } else {
        spdlog::info("Connection to server failed.");
        enet_peer_reset(server);
        enet_host_destroy(client);
    }

    enet_host_flush(client);
    return ret;
}

