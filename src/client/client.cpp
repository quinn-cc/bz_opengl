#include <enet.h>
#include <iostream>
#include <cstring>
#include <spdlog/spdlog.h>
#include "raylib.h"
#include "raymath.h"
#include "netmsg.hpp"
#include <math.h>
#include "cxxopts.hpp"

Vector3 cubePos = { 0.0f, 0.5f, 0.0f };
float cubeYaw = 0.0f;
const float moveSpeed = 10.0f;
const float turnSpeed = -2.0f;

// Camera setup
Camera3D camera = { 0 };

std::string g_name;
ENetHost* client;
ENetPeer* server;

void Render_Init() {
    InitWindow(800, 600, "raylib [C++] - First-Person Turning Cube on Grid");
    SetTargetFPS(60);
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
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
    if (IsKeyDown(KEY_UP))    cubePos = Vector3Add(cubePos, Vector3Scale(lookDir, moveSpeed * dt));
    if (IsKeyDown(KEY_DOWN))  cubePos = Vector3Add(cubePos, Vector3Scale(lookDir, -moveSpeed * dt));

    // Update camera
    camera.position = Vector3Add(cubePos, (Vector3){ 0.0f, 0.5f, 0.0f });
    camera.target   = Vector3Add(camera.position, lookDir);
}

void Render_Draw() {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    BeginMode3D(camera);

    DrawGrid(100, 1.0f); // Big grid for navigation reference

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

void initToServer() {
    ClientMsg_Init init;
    init.type = ClientMsg_Type_INIT;
    strcpy(init.name, g_name.c_str());
    ENetPacket* packet = enet_packet_create(&init, sizeof(init), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(server, 0, packet);
    enet_host_flush(client);
}

void NetEvent_PlayerConnection(ENetEvent event, ServerMsg_Connection *conn) {
    spdlog::debug("{} joined", std::string(conn->name));
}

void NetEvent_RecievedPacket(ENetEvent event) {
    ServerMsg *msg = reinterpret_cast<ServerMsg*>(event.packet->data);

    switch (msg->type) {
    case ServerMsg_Type_CONNECTION: {
        ServerMsg_Connection *conn = reinterpret_cast<ServerMsg_Connection*>(msg);
        NetEvent_PlayerConnection(event, conn);
        break;
    }
    default:
        break;
    }
}

void Event_Listener() {
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

int main(int argc, char *argv[]) {
    spdlog::set_level(spdlog::level::debug);
    parseArgs(argc, argv);
    Render_Init();
    initNet();
    initToServer();

    // Main loop: send and receive
    while (!WindowShouldClose()) {
        Event_Listener();
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