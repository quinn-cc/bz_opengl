#pragma once
#include "raylib.h"
#include <glm/glm.hpp>

class Shot;
class Client;

class Renderer {
private:
    Camera3D camera;
    
    Vector3 toInternal(glm::vec3 &v);
    Quaternion toInternal(glm::quat &q);
    glm::vec3 toGLM(Vector3 &v);
    glm::quat toGLM(Quaternion &q);

public:
    static Renderer &GetInstance();

    void Init();
    void Close();
    void BeginFrame();
    void EndFrame();
    float GetDeltaTime();
    void Update();

    void Draw(Client *client);
    void Draw(Shot *shot);
};