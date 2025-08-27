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
    
    std::shared_ptr<threepp::OrthographicCamera> radarCamera;
    std::shared_ptr<threepp::GLRenderTarget> radarRenderTarget;
    threepp::GLRenderer *renderer;
    std::map<Client *, std::shared_ptr<threepp::Object3D>> clientMeshes;
    std::map<Shot *, std::shared_ptr<threepp::Mesh>> shotMeshes;

    std::chrono::time_point<std::chrono::system_clock> lastFrameTime;
    TimeUtils::duration deltaTime;
    
    threepp::Vector3 toInternal(const glm::vec3 &v);
    threepp::Quaternion toInternal(const glm::quat &q);
    glm::vec3 toGLM(const threepp::Vector3 &v);
    glm::quat toGLM(const threepp::Quaternion &q);
    bool closed;

    void applyMaterialAndShadows(const std::shared_ptr<threepp::Object3D>& obj);
    void framebuffer_size_callback(GLFWwindow* window, int width, int height);

public:
    GLFWwindow* window;

    void Init(Game *game, GLFWwindow* window);
    bool ShouldClose();
    void Close();
    TimeUtils::duration GetDeltaTime();
    void Update();
    void EndFrame();

    void AddClient(Client *client);
    void RemoveClient(Client *client);
    void AddShot(Shot *shot);
    void RemoveShot(Shot *shot);
    unsigned int GetRadarTextureId();

    void OnResize(int width, int height);
};