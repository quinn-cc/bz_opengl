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
            bz::ServerMsg msg;

            if (!msg.ParseFromArray(event.packet->data, event.packet->dataLength)) {
                spdlog::error("Failed to parse ServerMsg");
                enet_packet_destroy(event.packet);
                return;
            }

            switch (msg.payload_case()) {
            
            case bz::ServerMsg::kPlayerJoin: {
                ServerMsg_PlayerJoin* joinMsg = new ServerMsg_PlayerJoin();
                joinMsg->clientId = msg.player_join().client_id();
                joinMsg->state.name = msg.player_join().state().name();
                joinMsg->state.position.x = msg.player_join().state().position().x();
                joinMsg->state.position.y = msg.player_join().state().position().y();
                joinMsg->state.position.z = msg.player_join().state().position().z();
                joinMsg->state.rotation.w = msg.player_join().state().rotation().w();
                joinMsg->state.rotation.x = msg.player_join().state().rotation().x();
                joinMsg->state.rotation.y = msg.player_join().state().rotation().y();
                joinMsg->state.rotation.z = msg.player_join().state().rotation().z();
                joinMsg->state.velocity.x = msg.player_join().state().velocity().x();
                joinMsg->state.velocity.y = msg.player_join().state().velocity().y();
                joinMsg->state.velocity.z = msg.player_join().state().velocity().z();
                joinMsg->state.alive = msg.player_join().state().alive();
                receivedMessages.push_back({ event.packet, joinMsg });
                break;
            }

            case bz::ServerMsg::kPlayerLeave: {
                ServerMsg_PlayerLeave* leaveMsg = new ServerMsg_PlayerLeave();
                leaveMsg->clientId = msg.player_leave().client_id();
                receivedMessages.push_back({ event.packet, leaveMsg });
                break;
            }

            case bz::ServerMsg::kPlayerState: {
                ServerMsg_PlayerState* stateMsg = new ServerMsg_PlayerState();
                stateMsg->clientId = msg.player_state().client_id();
                stateMsg->state.name = msg.player_state().state().name();
                stateMsg->state.position.x = msg.player_state().state().position().x();
                stateMsg->state.position.y = msg.player_state().state().position().y();
                stateMsg->state.position.z = msg.player_state().state().position().z();
                stateMsg->state.rotation.w = msg.player_state().state().rotation().w();
                stateMsg->state.rotation.x = msg.player_state().state().rotation().x();
                stateMsg->state.rotation.y = msg.player_state().state().rotation().y();
                stateMsg->state.rotation.z = msg.player_state().state().rotation().z();
                stateMsg->state.velocity.x = msg.player_state().state().velocity().x();
                stateMsg->state.velocity.y = msg.player_state().state().velocity().y();
                stateMsg->state.velocity.z = msg.player_state().state().velocity().z();
                stateMsg->state.alive = msg.player_state().state().alive();
                receivedMessages.push_back({ event.packet, stateMsg });
                break;
            }

            case bz::ServerMsg::kPlayerParameters: {
                ServerMsg_PlayerParameters* paramsMsg = new ServerMsg_PlayerParameters();
                paramsMsg->clientId = msg.player_parameters().client_id();
                for (const auto& [key, val] : msg.player_parameters().params().params()) {
                    paramsMsg->params[key] = val;
                }
                receivedMessages.push_back({ event.packet, paramsMsg });
                break;
            }

            case bz::ServerMsg::kPlayerLocation: {
                ServerMsg_PlayerLocation* locMsg = new ServerMsg_PlayerLocation();
                locMsg->clientId = msg.player_location().client_id();
                locMsg->position.x = msg.player_location().position().x();
                locMsg->position.y = msg.player_location().position().y();
                locMsg->position.z = msg.player_location().position().z();
                locMsg->rotation.w = msg.player_location().rotation().w();
                locMsg->rotation.x = msg.player_location().rotation().x();
                locMsg->rotation.y = msg.player_location().rotation().y();
                locMsg->rotation.z = msg.player_location().rotation().z();
                locMsg->velocity.x = msg.player_location().velocity().x();
                locMsg->velocity.y = msg.player_location().velocity().y();
                locMsg->velocity.z = msg.player_location().velocity().z();
                receivedMessages.push_back({ event.packet, locMsg });
                break;
            }

            case bz::ServerMsg::kPlayerSpawn: {
                ServerMsg_PlayerSpawn* spawnMsg = new ServerMsg_PlayerSpawn();
                spawnMsg->clientId = msg.player_spawn().client_id();
                spawnMsg->position.x = msg.player_spawn().position().x();
                spawnMsg->position.y = msg.player_spawn().position().y();
                spawnMsg->position.z = msg.player_spawn().position().z();
                spawnMsg->rotation.w = msg.player_spawn().rotation().w();
                spawnMsg->rotation.x = msg.player_spawn().rotation().x();
                spawnMsg->rotation.y = msg.player_spawn().rotation().y();
                spawnMsg->rotation.z = msg.player_spawn().rotation().z();
                spawnMsg->velocity.x = msg.player_spawn().velocity().x();
                spawnMsg->velocity.y = msg.player_spawn().velocity().y();
                spawnMsg->velocity.z = msg.player_spawn().velocity().z();
                receivedMessages.push_back({ event.packet, spawnMsg });
                break;
            }

            case bz::ServerMsg::kPlayerDeath: {
                ServerMsg_PlayerDeath* deathMsg = new ServerMsg_PlayerDeath();
                deathMsg->clientId = msg.player_death().client_id();
                receivedMessages.push_back({ event.packet, deathMsg });
                break;
            }

            case bz::ServerMsg::kCreateShot: {
                ServerMsg_CreateShot* shotMsg = new ServerMsg_CreateShot();
                shotMsg->globalShotId = msg.create_shot().global_shot_id();
                shotMsg->position.x = msg.create_shot().position().x();
                shotMsg->position.y = msg.create_shot().position().y();
                shotMsg->position.z = msg.create_shot().position().z();
                shotMsg->velocity.x = msg.create_shot().velocity().x();
                shotMsg->velocity.y = msg.create_shot().velocity().y();
                shotMsg->velocity.z = msg.create_shot().velocity().z();
                receivedMessages.push_back({ event.packet, shotMsg });
                break;
            }

            case bz::ServerMsg::kRemoveShot: {
                ServerMsg_RemoveShot* removeMsg = new ServerMsg_RemoveShot();
                removeMsg->shotId = msg.remove_shot().shot_id();
                removeMsg->isGlobalId = msg.remove_shot().is_global_id();
                receivedMessages.push_back({ event.packet, removeMsg });
                break;
            }

            case bz::ServerMsg::kInit: {
                ServerMsg_Init* initMsg = new ServerMsg_Init();
                initMsg->clientId = msg.init().client_id();
                initMsg->serverName = msg.init().server_name();
                for (const auto& [key, val] : msg.init().default_player_params().params()) {
                    initMsg->defaultPlayerParams[key] = val;
                }
                
                // Copy the world data
                const std::string& worldDataStr = msg.init().world_data();
                const std::byte* dataPtr = reinterpret_cast<const std::byte*>(worldDataStr.data());
                initMsg->worldData = std::vector<std::byte>(dataPtr, dataPtr + worldDataStr.size());
                
                receivedMessages.push_back({ event.packet, initMsg });
                break;
            }

            case bz::ServerMsg::kChat: {
                ServerMsg_Chat* chatMsg = new ServerMsg_Chat();
                chatMsg->fromId = msg.chat().from_id();
                chatMsg->toId = msg.chat().to_id();
                chatMsg->text = msg.chat().text();
                receivedMessages.push_back({ event.packet, chatMsg });
                break;
            }

            default:
                spdlog::warn("Received unknown ServerMsg type");
                enet_packet_destroy(event.packet);
                break;
            }
            
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

