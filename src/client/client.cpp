#include <enet.h>
#include <iostream>
#include <cstring>
#include <spdlog/spdlog.h>
#include "raylib.h"
#include "raymath.h"
#include "netmsg.hpp"
#include <math.h>
#include "cxxopts.hpp"
#include "player.hpp"

Vector3 position = { 0.0f, 0.5f, 0.0f };
float cubeYaw = 0.0f;
const float moveSpeed = 10.0f;
const float turnSpeed = -2.0f;

// Camera setup
Camera3D camera = { 0 };

std::string g_name;
ENetHost* client;
ENetPeer* server;

std::vector<Player *> g_players;

Player *getPlayerById(int clientId) {
    for (Player *player : g_players) {
        if (player->clientId == clientId) {
            return player;
        }
    }

    return nullptr;
}

void Render_Init() {
    InitWindow(800, 600, "raylib [C++] - First-Person Turning Cube on Grid");
    SetTargetFPS(60);
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
}



void Render_Draw() {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    BeginMode3D(camera);

    DrawGrid(100, 1.0f); // Big grid for navigation reference

    for (Player *p : g_players) {
        p->Draw();
    }

    EndMode3D();

    DrawText("Arrow keys: UP/DOWN to move, LEFT/RIGHT to rotate", 10, 10, 20, DARKGRAY);
    EndDrawing();
}

void parseArgs(int argc, char *argv[]) {
    /*
     * Command-line options
     */
    cxxopts::Options options("BZ", "This is the client.");
    options.add_options()
        ("n,name", "Name to display", cxxopts::value<std::string>()->default_value("default"));

    auto result = options.parse(argc, argv);
    g_name = result["name"].as<std::string>();
}

int initNet() {
    if (enet_initialize() != 0) {
        std::cerr << "Failed to initialize ENet.\n";
        return EXIT_FAILURE;
    }
    atexit(enet_deinitialize);

    // Create the ENet client host
    client = enet_host_create(nullptr, 1, 2, 0, 0);
    if (!client) {
        std::cerr << "Failed to create ENet client host.\n";
        return EXIT_FAILURE;
    }

    // Set up the server address
    ENetAddress address;
    enet_address_set_host(&address, "127.0.0.1");
    address.port = 1234;

    // Connect to the server
    server = enet_host_connect(client, &address, 2, 0);
    if (!server) {
        std::cerr << "No available peers for initiating connection.\n";
        enet_host_destroy(client);
        return EXIT_FAILURE;
    }

    // Wait for the connection to succeed (with timeout)
    ENetEvent event;
    if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        std::cout << "Connected to server.\n";
    } else {
        std::cerr << "Connection to server failed.\n";
        enet_peer_reset(server);
        enet_host_destroy(client);
        return EXIT_FAILURE;
    }

    enet_host_flush(client);
    return 0;
}

void NetUpdate_Init() {
    ClientMsg_Init init;
    init.type = ClientMsg_Type_INIT;
    strcpy(init.name, g_name.c_str());
    ENetPacket* packet = enet_packet_create(&init, sizeof(init), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(server, 0, packet);
    enet_host_flush(client);
}

void NetUpdate_Location(Vector3 position) {
    ClientMsg_Location loc;
    loc.type = ClientMsg_Type_LOCATION;
    loc.position = position;
    ENetPacket* packet = enet_packet_create(&loc, sizeof(loc), ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);
    enet_peer_send(server, 0, packet);
}

void NetEvent_PlayerConnection(ENetEvent event, ServerMsg_Connection *conn) {
    Player *player = new Player();
    player->name = conn->name;
    player->clientId = conn->clientId;
    player->position.x = 0;
    player->position.y = 0;
    player->position.z = 0;
    g_players.push_back(player);
    spdlog::debug("{} joined", std::string(conn->name));
}

void NetEvent_UpdatePlayerLocation(ENetEvent event, ServerMsg_Location *loc) {
    Player *player = getPlayerById(loc->clientId);

    if (!player)
        spdlog::error("Player with clientId {} does not exist", loc->clientId);

    player->position = loc->position;
}

void NetEvent_RecievedPacket(ENetEvent event) {
    ServerMsg *msg = reinterpret_cast<ServerMsg*>(event.packet->data);

    switch (msg->type) {
    case ServerMsg_Type_CONNECTION: {
        ServerMsg_Connection *conn = reinterpret_cast<ServerMsg_Connection*>(msg);
        NetEvent_PlayerConnection(event, conn);
        break;
    }
    case ServerMsg_Type_LOCATION: {
        ServerMsg_Location *loc = reinterpret_cast<ServerMsg_Location*>(msg);
        NetEvent_UpdatePlayerLocation(event, loc);
        break;
    }
    default:
        break;
    }
}

void NetEvent_Listener() {
    ENetEvent event;
    while (enet_host_service(client, &event, 0) > 0) {
        switch (event.type) {
        case ENET_EVENT_TYPE_RECEIVE:
            NetEvent_RecievedPacket(event);
            enet_packet_destroy(event.packet);
            break;

        default:
            break;
        }
    }
}

void Listen_Input() {
    float dt = GetFrameTime();

    // Rotation (Y-axis yaw)
    if (IsKeyDown(KEY_LEFT))  cubeYaw -= turnSpeed * dt;
    if (IsKeyDown(KEY_RIGHT)) cubeYaw += turnSpeed * dt;

    // Forward direction from yaw
    Vector3 lookDir = {
        sinf(cubeYaw),
        0.0f,
        cosf(cubeYaw)
    };

    // Movement
    if (IsKeyDown(KEY_UP))
        position = Vector3Add(position, Vector3Scale(lookDir, moveSpeed * dt));
    if (IsKeyDown(KEY_DOWN))
        position = Vector3Add(position, Vector3Scale(lookDir, -moveSpeed * dt));

    // Update camera
    camera.position = Vector3Add(position, (Vector3){ 0.0f, 0.5f, 0.0f });
    camera.target   = Vector3Add(camera.position, lookDir);

    NetUpdate_Location(position);
}

int main(int argc, char *argv[]) {
    spdlog::set_level(spdlog::level::debug);
    parseArgs(argc, argv);
    Render_Init();
    initNet();
    NetUpdate_Init();

    // Main loop: send and receive
    while (!WindowShouldClose()) {
        NetEvent_Listener();
        Listen_Input();
        Render_Draw();
    }

cleanup:
    enet_peer_disconnect(server, 0);
    enet_host_flush(client);
    enet_host_destroy(client);
    
    CloseWindow();
    return 0;
}