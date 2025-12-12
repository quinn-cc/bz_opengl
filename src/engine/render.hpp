#pragma once
#include <threepp/threepp.hpp>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <map>
#include <string>
#include "engine/types.hpp"

class Render {
private:
    void beginFrame();
    void endFrame();

    friend class Engine;

public:
    Render(GLFWwindow *window);
    ~Render();

    render_id create(std::string modelPath);
    void destroy(render_id id);
    void setPosition(render_id id, const glm::vec3 &position);
    void setRotation(render_id id, const glm::quat &rotation);
    void setVisible(render_id id, bool visible);

    unsigned int getRadarTextureId();
};  