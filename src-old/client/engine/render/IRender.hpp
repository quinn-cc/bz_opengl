#pragma once
#include <GLFW/glfw3.h>
#include "threepp/threepp.hpp"
#include <glm/glm.hpp>
#include <fstream>
#include <sstream>
#include <string>

namespace Render {

    class IRender {
    protected:
        GLFWwindow &window;
        std::shared_ptr<threepp::Scene> scene;
        std::shared_ptr<threepp::Scene> radarScene;

        threepp::Vector3 toInternal(const glm::vec3& v) {
            return threepp::Vector3(v.x, v.y, v.z);
        }

        threepp::Quaternion toInternal(const glm::quat& q) {
            return threepp::Quaternion(q.x, q.y, q.z, q.w);
        }

        glm::vec3 toGLM(const threepp::Vector3& v) {
            return glm::vec3(v.x, v.y, v.z);
        }

        glm::quat toGLM(const threepp::Quaternion& q) {
            return glm::quat(q.w, q.x, q.y, q.z);
        }

        std::string loadShader(const std::string& path) {
            std::ifstream file(path);
            if (!file.is_open()) {
                throw std::runtime_error("Failed to open shader file: " + path);
            }
            std::stringstream buffer;
            buffer << file.rdbuf();
            return buffer.str();
        }

        

    public:
        IRender(
            GLFWwindow &w,
            std::shared_ptr<threepp::Scene> scene,
            std::shared_ptr<threepp::Scene> radarScene
        ) : window(w), scene(scene), radarScene(radarScene) {};
        virtual ~IRender() = default;
        virtual void update() = 0;
    };

}