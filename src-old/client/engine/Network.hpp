#pragma once
#include <enet.h>
#include <string>
#include <functional>
#include "netmsg.hpp"
#include <spdlog/spdlog.h>

namespace Network {

    class Network {
    private:
        ENetHost *client;
        ENetPeer *server;
    
    public:
        Network() : client(nullptr), server(nullptr) {
            if (enet_initialize() != 0) {
                spdlog::error("Failed to initialize ENet.");
                return;
            }
            atexit(enet_deinitialize);

            // Create the ENet client host
            client = enet_host_create(nullptr, 1, 2, 0, 0);
            if (!client) {
                spdlog::error("Failed to create ENet client host.");
                return;
            }
        };

        ~Network() {
            enet_peer_disconnect(server, 0);
            enet_host_flush(client);
            enet_host_destroy(client);
        }

        void connect(std::string addr) {
            ENetAddress address;
            enet_address_set_host(&address, addr.c_str());
            address.port = 1234;

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

        void poll(std::function<void(ServerMsg *)> callback) {
            ENetEvent event;
    
            while (enet_host_service(client, &event, 0) > 0) {
                switch (event.type) {
                case ENET_EVENT_TYPE_RECEIVE: {
                    ServerMsg *msg = reinterpret_cast<ServerMsg*>(event.packet->data);
                    callback(msg);
                    enet_packet_destroy(event.packet);
                    break;
                }
                default:
                    break;
                }
            }
        }
    };

}