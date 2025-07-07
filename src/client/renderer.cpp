#include "renderer.hpp"
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include "client.hpp"
#include "shot.hpp"
#include "player.hpp"

Renderer &Renderer::GetInstance() {
    static Renderer instance;
    return instance;
}

Vector3 Renderer::toInternal(glm::vec3 &v) {
    return Vector3{ v.x, v.y, v.z };
}

Quaternion Renderer::toInternal(glm::quat &q) {
    return Quaternion{ q.x, q.y, q.z, q.w };
}

glm::vec3 Renderer::toGLM(Vector3 &v) {
    return glm::vec3{ v.x, v.y, v.z };
}

glm::quat Renderer::toGLM(Quaternion &q) {
    return glm::quat{ q.x, q.y, q.z, q.w };
}

void Renderer::Init() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetConfigFlags(FLAG_WINDOW_ALWAYS_RUN);
    InitWindow(800, 600, "BZFlag v3");
    SetTargetFPS(60);

    camera = { 0 };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
}

void Renderer::Close() {
    CloseWindow();
}

void Renderer::BeginFrame() {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    BeginMode3D(camera);

    DrawGrid(100, 1.0f);

    //Vector3 p = {0, 0, 0};
    //DrawModel(model, p, 1.0f, WHITE);
}

void Renderer::EndFrame() {
    EndMode3D();
    EndDrawing();
}

float Renderer::GetDeltaTime() {
    return GetFrameTime();
}

void Renderer::Update() {
    glm::vec3 lookDir = Player::GetInstance().GetForwardVector();
    glm::vec3 offset = { 0.0f, 0.5f, 0.0f };
    glm::vec3 pos = Player::GetInstance().GetLocation().position + offset;
    camera.position = toInternal(pos);
    camera.target = Vector3Add(camera.position, toInternal(lookDir));
}

void Renderer::Draw(Client *client) {
    rlPushMatrix();

    Location loc = client->GetInterpolatedLocation();
    rlTranslatef(loc.position.x, loc.position.y, loc.position.z);

    Vector3 axis;
    float angle;
    QuaternionToAxisAngle(toInternal(loc.rotation), &axis, &angle);
    rlRotatef(RAD2DEG * angle, axis.x, axis.y, axis.z);

    DrawCubeV({0, 0, 0}, {1, 1, 1}, RED);
    DrawCubeWiresV({0, 0, 0}, {1, 1, 1}, BLACK);
    rlPopMatrix();
}

void Renderer::Draw(Shot *shot) {
    float dt = GetFrameTime();
    
    glm::vec3 pos = shot->GetPosition();
    DrawSphere(toInternal(pos), BULLET_SIZE, GREEN);
}


//Model model;

//void Render_Init() {
    //model = LoadModel("/home/quinn/bz_opengl/data/models/tank/tank.gltf");     // Supports OBJ, GLTF, IQM
    //Texture2D texture = LoadTexture("/home/quinn/bz_opengl/data/models/tank/quinn/bz_opengl/01_-_Default_baseColor.png");
    //model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
    //texture = LoadTexture("/home/quinn/bz_opengl/data/models/tank/quinn/bz_opengl/02_-_Default_baseColor.png");
    //model.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
    //texture = LoadTexture("/home/quinn/bz_opengl/data/models/tank/quinn/bz_opengl/03_-_Default_baseColor.png");
    //model.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
//}