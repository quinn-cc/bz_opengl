#include <enet.h>
#include <iostream>
#include <cstring>
#include <spdlog/spdlog.h>
#include <math.h>
#include <csignal>
#include <cstdlib>
#include "cxxopts.hpp"
#include "game.hpp"
#include "engine.hpp"

bool exitSignalRecieved = false;

Engine engine;
Game game;
std::string playerName = "default";
std::string address = "localhost";


void ParseArgs(int argc, char *argv[]) {
    cxxopts::Options options("BZ", "This is the client.");
    options.add_options()
        ("a,addr", "Address to connect to", cxxopts::value<std::string>()->default_value("localhost"));
    options.add_options()
        ("n,name", "Name to display", cxxopts::value<std::string>()->default_value("default"));
    auto result = options.parse(argc, argv);
    playerName = result["name"].as<std::string>();
    address = result["addr"].as<std::string>();
}

void Start() {
    engine.Init(&game);
    game.player.Init(playerName);
    engine.networker.Connect(address);
    engine.networker.Send<ClientMsg_Init>(game.player.Get<ClientMsg_Init>(), true);
}

void Event_Connection(ServerMsg_Connection *msg) {
    Client *client = new Client(msg->clientId, msg->name);
    game.AddClient(client);
    engine.renderer.AddClient(client);
}

void Event_Disconnection(ServerMsg_Disconnection *msg) {
    Client *client = game.GetClient(msg->clientId);
    if (client) {
        engine.renderer.RemoveClient(client);
        game.RemoveClient(client);
    }
    delete client;
}

void Event_Location(ServerMsg_Location *msg) {
    game.GetClient(msg->clientId)->SetLocation(msg->location);
}

void Event_AddShot(ServerMsg_Shot *msg) {
    Shot *shot = new Shot(msg->globalShotId, msg->position, msg->velocity);
    game.AddShot(shot);
    engine.renderer.AddShot(shot);
}

void Event_RemoveShot(ServerMsg_RemoveShot *msg) {
    Shot *shot;
    if (msg->clientId == 0) {
        shot = game.GetShotByLocalId(msg->shotId);
    } else {
        shot = game.GetShotByGlobalId(msg->shotId);
    }
    
    engine.renderer.RemoveShot(shot);
    game.RemoveShot(shot);
    delete shot;
}

void Event_AllowSpawn(ServerMsg_AllowSpawn *msg) {
    if (game.player.IsAlive()) {
        spdlog::debug("Player is already alive, ignoring spawn approval");
    } else {
        game.player.Spawn(msg->location);
        engine.networker.Send<ClientMsg_Spawn>(game.player.Get<ClientMsg_Spawn>(), true);
        engine.physics.Player_SetLocation(msg->location);
    }
}

void Event_Spawn(ServerMsg_Spawn *msg) {
    spdlog::debug("Got spawn message for client {}", msg->clientId);
    Client *client = game.GetClient(msg->clientId);
    client->Spawn(msg->location);
}

void Event_Death(ServerMsg_Death *msg) {
    spdlog::debug("Client {} died", msg->clientId);

    if (msg->clientId == 0) {
        game.player.Die();
    } else {
        Client *client = game.GetClient(msg->clientId);
        client->Die();
    }
}

void Update() {
    engine.input.Update();

    spdlog::debug("Input movement: x={}, y={}", engine.input.GetInputMap().movement.x, engine.input.GetInputMap().movement.y);

    if (game.player.IsAlive()) {
        if (engine.input.GetInputMap().fire) {
            Shot *shot = new Shot(game.player.GetLocation().position, game.player.GetForwardVector() * 40.0f);
            game.AddShot(shot);
            engine.renderer.AddShot(shot);
            engine.networker.Send<ClientMsg_Shot>(shot->Get<ClientMsg_Shot>());
        }

        if (engine.physics.Player_IsGrounded()) {
            engine.physics.Player_Move(engine.input.GetInputMap().movement);

            if (engine.input.GetInputMap().jump) {
                engine.physics.Player_Jump();
            }
        }
    } else {
        if (engine.input.GetInputMap().spawn) {
            spdlog::debug("Pushing spawn key");
            if (game.player.CanSpawn()) {
                spdlog::debug("Can spawn, spawning player");
                engine.networker.Send<ClientMsg_Spawn>(game.player.Get<ClientMsg_Spawn>());
            } else {
                spdlog::debug("Cannot spawn, requesting spawn");
                engine.networker.Send<ClientMsg_RequestSpawn>(game.player.Get<ClientMsg_RequestSpawn>());
            }
        }
    }

    for (Shot *shot : game.shots) {
        shot->Update(engine.renderer.GetDeltaTime());
    }

    engine.networker.Update([](ServerMsg *msg) {
        switch (msg->type) {
        case ServerMsg_Type_CONNECTION:
            Event_Connection(reinterpret_cast<ServerMsg_Connection *>(msg));
            break;
        case ServerMsg_Type_DISCONNECTION:
            Event_Disconnection(reinterpret_cast<ServerMsg_Disconnection *>(msg));
            break;
        case ServerMsg_Type_LOCATION:
            Event_Location(reinterpret_cast<ServerMsg_Location *>(msg));
            break;
        case ServerMsg_Type_SHOT:
            Event_AddShot(reinterpret_cast<ServerMsg_Shot *>(msg));
            break;
        case ServerMsg_Type_REMOVE_SHOT:
            Event_RemoveShot(reinterpret_cast<ServerMsg_RemoveShot *>(msg));
            break;
        case ServerMsg_Type_ALLOW_SPAWN:
            Event_AllowSpawn(reinterpret_cast<ServerMsg_AllowSpawn *>(msg));
            break;
        case ServerMsg_Type_SPAWN:
            Event_Spawn(reinterpret_cast<ServerMsg_Spawn *>(msg));
            break;
        case ServerMsg_Type_DEATH:
            Event_Death(reinterpret_cast<ServerMsg_Death *>(msg));
            break;
        default:
            spdlog::warn("Unknown message type: {}", (int)msg->type);
            break;
        }
    });

    engine.physics.Update();

    game.player.SetLocation(engine.physics.Player_GetLocation());

    if (game.player.LocationChanged()) {
        engine.networker.Send<ClientMsg_Location>(game.player.Get<ClientMsg_Location>());
    }

    game.player.Update();
    engine.renderer.Update();
    engine.gui.Update();
    engine.renderer.EndFrame();
    
    // Networker::GetInstance().Update();
    // Input::GetInstance().Update();
    // Physics::GetInstance().Update();

    // if (inputMap.fire) {
    //     float speed = 1;
    //     glm::vec3 velocity = GetForwardVector() * speed;
    //     Shot *shot = new Shot(location.position, velocity);
    //     Networker::GetInstance().MsgSend_Shot(shot);
    // }

    // if (inputMap.spawn) {
    //     Networker::GetInstance().MsgSend_RequestSpawn();
    // }

    // for (Shot *shot : Shot::shots) {
    //     shot->Update();
    // }

}

void Close() {
    spdlog::debug("Closing window");
    engine.Close();
}

void SignalHandlerClose(int signum) {
    exitSignalRecieved = true;
}

int main(int argc, char *argv[]) {
    std::signal(SIGINT, SignalHandlerClose);
    spdlog::set_level(spdlog::level::debug);
    ParseArgs(argc, argv);
    Start();

    while (!exitSignalRecieved && !engine.renderer.ShouldClose()) {
        Update();
    }

    Close();
    return 0;
}