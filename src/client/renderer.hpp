#pragma once
#include <threepp/threepp.hpp>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <map>

class Shot;
class Client;

class Renderer {
private:
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

    void OnClientAdd(Client *client);
    void OnClientRemove(Client *client);
    void OnShotAdd(Shot *client);
    void OnShotRemove(Shot *client);

public:
    GLFWwindow* window;
    static Renderer &GetInstance();

    void Init();
    bool ShouldClose();
    void Close();
    float GetDeltaTime();
    void Update();
};