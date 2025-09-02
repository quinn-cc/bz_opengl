#pragma once
#include <GLFW/glfw3.h>
#include "threepp/threepp.hpp"
#include <glm/glm.hpp>

namespace Render {

    class IRender {
    protected:
        GLFWwindow &window;

        static threepp::Vector3 toInternal(const glm::vec3& v) {
            return threepp::Vector3(v.x, v.y, v.z);
        }

        static threepp::Quaternion toInternal(const glm::quat& q) {
            return threepp::Quaternion(q.x, q.y, q.z, q.w);
        }

        static glm::vec3 toGLM(const threepp::Vector3& v) {
            return glm::vec3(v.x, v.y, v.z);
        }

        static glm::quat toGLM(const threepp::Quaternion& q) {
            return glm::quat(q.w, q.x, q.y, q.z);
        }

    public:
        IRender(GLFWwindow &w) : window(w) {};
        virtual ~IRender() = default;
        virtual void update() = 0;
    };

}