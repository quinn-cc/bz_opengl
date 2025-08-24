#include "main.hpp"
#include <enet.h>
#include <iostream>
#include <arpa/inet.h>
#include <vector>
#include <random>
#include "netmsg.hpp"
#include <spdlog/spdlog.h>
#include <chrono>
#include <glm/glm.hpp>
#include "game.hpp"
#include "engine.hpp"

Game game;
Engine engine;

void Event_Init(Client *client, ClientMsg_Init *msg) {
    client->name = std::string(msg->name);

    ServerMsg_Connection sendMsg;
    sendMsg.clientId = client->id;
    sendMsg.type = ServerMsg_Type_CONNECTION;
    strncpy(sendMsg.name, client->name.c_str(), client->name.size());
    sendMsg.name[client->name.size()] = '\0';
    engine.networker.SendExcept<ServerMsg_Connection>(
        client,
        sendMsg
    );

    for (Client *otherClient : game.clients) {
        if (otherClient == client) continue;
        ServerMsg_PlayerState conn;
        conn.type = ServerMsg_Type_PLAYER_STATE;
        conn.clientId = otherClient->id;
        conn.location = otherClient->location;
        conn.alive = otherClient->alive;
        strncpy(conn.name, otherClient->name.c_str(), otherClient->name.size());
        conn.name[otherClient->name.size()] = '\0';
        engine.networker.Send(client, conn);
    }
}

void Event_RequestSpawn(Client *client, ClientMsg_RequestSpawn *msg) {
    Location spawnLocation;
    spawnLocation.position = glm::vec3(rand() % 50 - 25, 10.0f, rand() % 50 - 25);
    static std::uniform_real_distribution<float> dist(0.0f, glm::two_pi<float>());
    static std::mt19937 rng{std::random_device{}()};
    float angle = dist(rng); 
    spawnLocation.rotation = glm::angleAxis(angle, glm::vec3(0, 1, 0));

    client->alive = true;
    client->location = spawnLocation;

    ServerMsg_AllowSpawn sendMsg;
    sendMsg.type = ServerMsg_Type_ALLOW_SPAWN;
    sendMsg.location = spawnLocation;
    sendMsg.allow = true;
    engine.networker.Send<ServerMsg_AllowSpawn>(
        client,
        sendMsg
    );

    ServerMsg_Spawn spawnMsg;
    spawnMsg.type = ServerMsg_Type_SPAWN;
    spawnMsg.clientId = client->id;
    spawnMsg.location = spawnLocation;
    engine.networker.SendExcept<ServerMsg_Spawn>(
        client,
        spawnMsg
    );
}

void Event_Location(Client *client, ClientMsg_Location *msg) {
    client->location = msg->location;

    ServerMsg_Location sendMsg;
    sendMsg.type = ServerMsg_Type_LOCATION;
    sendMsg.clientId = client->id;
    sendMsg.location = msg->location;
    engine.networker.SendExcept<ServerMsg_Location>(
        client,
        sendMsg
    );
}

void Event_Shot(Client *client, ClientMsg_Shot *msg) {
    Shot *shot = new Shot(msg->localShotId, client, msg->position, msg->velocity);
    game.AddShot(shot);

    ServerMsg_Shot sendMsg;
    sendMsg.type = ServerMsg_Type_SHOT;
    sendMsg.globalShotId = shot->globalId;
    sendMsg.position = msg->position;
    sendMsg.velocity = msg->velocity;
    engine.networker.SendExcept<ServerMsg_Shot>(
        client,
        sendMsg
    );
}

void Send_Death(Client *client) {
    ServerMsg_Death sendMsg;
    sendMsg.type = ServerMsg_Type_DEATH;
    sendMsg.clientId = 0;
    engine.networker.Send<ServerMsg_Death>(
        client,
        sendMsg
    );

    sendMsg.clientId = client->id;
    engine.networker.SendExcept<ServerMsg_Death>(
        client,
        sendMsg
    );
}

void Event_Disconnection(Client *client) {
    game.RemoveClient(client);
    
    ServerMsg_Disconnection sendMsg;
    sendMsg.type = ServerMsg_Type_DISCONNECTION;
    sendMsg.clientId = client->id;
    engine.networker.SendExcept<ServerMsg_Disconnection>(
        client,
        sendMsg
    );

    delete client;
}

void Event_Connection(Client **new_client) {
    Client *client = new Client();
    game.AddClient(client);
    *new_client = client;
}

void Update() {
    TimeUtils::duration dt = game.runtime.GetDeltaTime();

    engine.networker.Update(
        [](Client *client, ClientMsg *msg) {
            switch (msg->type) {
            case ClientMsg_Type_INIT:
                Event_Init(client, reinterpret_cast<ClientMsg_Init *>(msg));
                break;
            case ClientMsg_Type_REQUEST_SPAWN:
                Event_RequestSpawn(client, reinterpret_cast<ClientMsg_RequestSpawn *>(msg));
                break;
            case ClientMsg_Type_LOCATION:
                Event_Location(client, reinterpret_cast<ClientMsg_Location *>(msg));
                break;
            case ClientMsg_Type_SHOT:
                Event_Shot(client, reinterpret_cast<ClientMsg_Shot *>(msg));
                break;
            default:
                spdlog::warn("Unknown message type: {}", (int)msg->type);
                break;
            }
        },
        [](Client *client) {
            // Handle disconnection
            Event_Disconnection(client);
        },
        [](Client **new_client) {
            // Handle connection
            Event_Connection(new_client);
        }
    );

    for (Shot *shot : game.shots) {
        if (shot->IsExpired()) {
            game.RemoveShot(shot);

            ServerMsg_RemoveShot ownerMsg;
            ownerMsg.type = ServerMsg_Type_REMOVE_SHOT;
            ownerMsg.shotId = shot->localId;
            ownerMsg.clientId = 0;
            engine.networker.Send<ServerMsg_RemoveShot>(
                shot->owner,
                ownerMsg
            );

            ServerMsg_RemoveShot restMsg;
            restMsg.type = ServerMsg_Type_REMOVE_SHOT;
            restMsg.shotId = shot->globalId;
            restMsg.clientId = shot->owner->id;
            engine.networker.SendExcept<ServerMsg_RemoveShot>(
                shot->owner,
                restMsg
            );

            continue;
        }

        shot->Update(dt);

        for (Client *client : game.clients) {
            if (glm::distance(shot->position, client->location.position) < 1.0f) {
                if (client->alive && shot->owner != client) {
                    Send_Death(client);
                }
            }
        }
    }
}

int main(int argc, char** argv) {
    spdlog::set_level(spdlog::level::debug);
    game.runtime.Init();
    engine.Init();

    while (true) {
        Update();
    }

    engine.Close();
    return 0;
}

