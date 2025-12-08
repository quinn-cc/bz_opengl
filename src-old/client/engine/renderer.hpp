#pragma once
#include <threepp/threepp.hpp>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <threepp/materials/ShaderMaterial.hpp>
#include <map>
#include "game.hpp"
#include <fstream>
#include <sstream>
#include <string>


#define JUMP_HEIGHT 2.0f
#define CAMERA_FOV 60.0f
#define RADAR_SHOT_SIZE 0.25f
#define RADAR_CLIENT_SIZE 0.7f

class Shot;
class Client;
class Engine;

class Renderer {
private:
    Game *game;
    threepp::GLRenderer *renderer;

    /*
     * Main scene
     */
    std::shared_ptr<threepp::Scene> scene;
    std::shared_ptr<threepp::PerspectiveCamera> camera;
    std::map<Client *, std::shared_ptr<threepp::Object3D>> clientMeshes;
    std::map<Shot *, std::shared_ptr<threepp::Mesh>> shotMeshes;
    
    /*
     * Radar
     */
    
    std::shared_ptr<threepp::OrthographicCamera> radarCamera;
    std::shared_ptr<threepp::GLRenderTarget> radarRenderTarget;
    std::shared_ptr<threepp::ShaderMaterial> radarMat;
    std::shared_ptr<threepp::Mesh> radarPlayer;
    std::shared_ptr<threepp::FloatBufferAttribute> fovPositionAttr;
    std::shared_ptr<threepp::MeshBasicMaterial> radarShotMaterial;
    std::map<Shot *, std::shared_ptr<threepp::Mesh>> radarShotMeshes;
    std::shared_ptr<threepp::MeshBasicMaterial> radarClientMaterial;
    std::map<Client *, std::shared_ptr<threepp::Mesh>> radarClientMeshes;

    std::chrono::time_point<std::chrono::system_clock> lastFrameTime;
    TimeUtils::duration deltaTime;
    
    threepp::Vector3 toInternal(const glm::vec3 &v);
    threepp::Quaternion toInternal(const glm::quat &q);
    glm::vec3 toGLM(const threepp::Vector3 &v);
    glm::quat toGLM(const threepp::Quaternion &q);
    bool closed;

    void applyMaterialAndShadows(const std::shared_ptr<threepp::Object3D>& obj);
    void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    std::string loadShader(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open shader file: " + path);
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
    float getVFOV() {
        float vFOV = glm::radians(CAMERA_FOV); // convert degrees to radians
        float aspect = camera->aspect;          // your camera aspect ratio
        float hFOV = 2.f * atan(tan(vFOV / 2.f) * aspect);
        hFOV = glm::degrees(hFOV);            // convert back to degrees if needed
        return hFOV;
    }
    void updateFovLines();

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