#pragma once

#include "glm/glm.hpp"

#include <string>
#include <vector>

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

class Model {
private:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    
public:
    Model() = default;
    ~Model() = default;

    void loadFromFile(const std::string& path);
    void CubeModel();
    void SphereModel(size_t resolution = 10, float radius = 1.0f);
    void CubeModelUnique();

    std::vector<Vertex>& getVertices() { return vertices; }
    std::vector<unsigned int>& getIndices() { return indices; }

    static std::vector<float> CubeVertices();
};