#pragma once
#include <threepp/threepp.hpp>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Shot;
class Client;

class Renderer {
private:
    std::shared_ptr<threepp::Scene> scene;
    std::shared_ptr<threepp::PerspectiveCamera> camera;
    threepp::GLRenderer *renderer;

    std::chrono::time_point<std::chrono::system_clock> lastFrameTime;
    double deltaTime;
    
    threepp::Vector3 toInternal(glm::vec3 &v);
    threepp::Quaternion toInternal(glm::quat &q);
    glm::vec3 toGLM(threepp::Vector3 &v);
    glm::quat toGLM(threepp::Quaternion &q);

public:
    GLFWwindow* window;
    static Renderer &GetInstance();

    void Init();
    bool ShouldClose();
    void Close();
    void BeginFrame();
    void EndFrame();
    float GetDeltaTime();
    void Update();

    void Draw(Client *client);
    void Draw(Shot *shot);
};