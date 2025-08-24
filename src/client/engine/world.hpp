#include <vector>
#include <glm/glm.hpp>
#include <string>

struct MeshData {
    std::vector<glm::vec3> vertices;
    std::vector<unsigned int> indices;
};

std::vector<MeshData> loadGLB(const std::string &filename);
