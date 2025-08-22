#pragma once
#include <threepp/threepp.hpp>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <map>
#include "game.hpp"

class Shot;
class Client;
class Engine;

class Renderer {
private:
    Game *game;

    std::shared_ptr<threepp::Scene> scene;
    std::shared_ptr<threepp::PerspectiveCamera> camera;
    threepp::GLRenderer *renderer;
    std::map<Client *, std::shared_ptr<threepp::Object3D>> clientMeshes;
    std::map<Shot *, std::shared_ptr<threepp::Mesh>> shotMeshes;

    std::chrono::time_point<std::chrono::system_clock> lastFrameTime;
    double deltaTime;
    
    threepp::Vector3 toInternal(glm::vec3 &v);
    threepp::Quaternion toInternal(glm::quat &q);
    glm::vec3 toGLM(threepp::Vector3 &v);
    glm::quat toGLM(threepp::Quaternion &q);
    bool closed;

    void applyMaterialAndShadows(const std::shared_ptr<threepp::Object3D>& obj);
    void framebuffer_size_callback(GLFWwindow* window, int width, int height);

public:
    GLFWwindow* window;

    void Init(Game *game);
    bool ShouldClose();
    void Close();
    float GetDeltaTime();
    void Update();
    void EndFrame();

    void AddClient(Client *client);
    void RemoveClient(Client *client);
    void AddShot(Shot *shot);
    void RemoveShot(Shot *shot);

    void OnResize(int width, int height);
};