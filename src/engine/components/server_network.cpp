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
            bz::ClientMsg msg;

            if (!msg.ParseFromArray(event.packet->data, event.packet->dataLength)) {
                spdlog::error("Failed to parse ServerMsg");
                return;
            }

            switch (msg.payload_case()) {
            
            case bz::ClientMsg::kInitMsg: {
                ClientMsg_Init* initMsg = new ClientMsg_Init();
                initMsg->clientId = getClient(event.peer);
                initMsg->name = msg.init_msg().name();
                receivedMessages.push_back({ event.packet, initMsg });
                break;
            }

            case bz::ClientMsg::kChatMsg: {
                ClientMsg_Chat* chatMsg = new ClientMsg_Chat();
                chatMsg->clientId = getClient(event.peer);
                chatMsg->toId = msg.chat_msg().toid();
                chatMsg->text = msg.chat_msg().text();
                receivedMessages.push_back({ event.packet, chatMsg });
                break;
            }

            case bz::ClientMsg::kLocationMsg: {
                ClientMsg_Location* locMsg = new ClientMsg_Location();
                locMsg->clientId = getClient(event.peer);
                locMsg->position.x = msg.location_msg().position().x();
                locMsg->position.y = msg.location_msg().position().y();
                locMsg->position.z = msg.location_msg().position().z();
                locMsg->rotation.w = msg.location_msg().rotation().w();
                locMsg->rotation.x = msg.location_msg().rotation().x();
                locMsg->rotation.y = msg.location_msg().rotation().y();
                locMsg->rotation.z = msg.location_msg().rotation().z();
                receivedMessages.push_back({ event.packet, locMsg });
                break;
            }

            case bz::ClientMsg::kRequestSpawnMsg: {
                ClientMsg_RequestSpawn* spawnMsg = new ClientMsg_RequestSpawn();
                spawnMsg->clientId = getClient(event.peer);
                receivedMessages.push_back({ event.packet, spawnMsg });
                break;
            }

            case bz::ClientMsg::kCreateShotMsg: {
                ClientMsg_CreateShot* shotMsg = new ClientMsg_CreateShot();
                shotMsg->clientId = getClient(event.peer);
                shotMsg->localShotId = msg.create_shot_msg().localshotid();
                shotMsg->position.x = msg.create_shot_msg().position().x();
                shotMsg->position.y = msg.create_shot_msg().position().y();
                shotMsg->position.z = msg.create_shot_msg().position().z();
                shotMsg->velocity.x = msg.create_shot_msg().velocity().x();
                shotMsg->velocity.y = msg.create_shot_msg().velocity().y();
                shotMsg->velocity.z = msg.create_shot_msg().velocity().z();
                receivedMessages.push_back({ event.packet, shotMsg });
                break;
            }

            default:
                spdlog::warn("Received unknown ClientMsg type");
                enet_packet_destroy(event.packet);
                break;

            }
            
            break;
        }
        case ENET_EVENT_TYPE_CONNECT: {
            client_id newClientId = getNextClientId();
            clients[newClientId] = event.peer;
            ClientMsg_Join* connMsg = new ClientMsg_Join();
            connMsg->clientId = newClientId;
            char ip[64];
            enet_address_get_host_ip(&event.peer->address, ip, sizeof(ip));
            connMsg->ip = std::string(ip);
            receivedMessages.push_back({ nullptr, connMsg });
            break;
        }
        case ENET_EVENT_TYPE_DISCONNECT: {
            client_id discClientId = getClient(event.peer);
            clients.erase(discClientId);
            ClientMsg_Leave* discMsg = new ClientMsg_Leave();
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