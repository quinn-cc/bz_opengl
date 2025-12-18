#pragma once
#include <threepp/threepp.hpp>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <map>
#include <string>
#include <map>
#include "engine/types.hpp"

class Render {
    friend class Engine;

private:
    // Scene
    threepp::GLRenderer renderer;
    std::shared_ptr<threepp::Scene> scene;
    std::shared_ptr<threepp::PerspectiveCamera> camera;
    std::map<render_id, std::shared_ptr<threepp::Group>> objects;

    Render(GLFWwindow *window);
    ~Render();

public:
    render_id create(std::string modelPath);
    void destroy(render_id id);
    void setPosition(render_id id, const glm::vec3 &position);
    void setRotation(render_id id, const glm::quat &rotation);
    void setVisible(render_id id, bool visible);
    void setCameraPosition(const glm::vec3 &position);
    void setCameraRotation(const glm::quat &rotation);
};  