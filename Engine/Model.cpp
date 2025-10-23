#include "Model.hpp"

#include "glm/gtc/constants.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <stdexcept>

void Model::loadFromFile(const std::string& path) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) {
        throw std::runtime_error("TinyObj failed to load objects ==> " + warn + err);
    }

    vertices.clear();
    indices.clear();

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};

            if (index.vertex_index >= 0) {
                vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2],
                };
            }

            if (index.normal_index >= 0) {
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2],
                };
            }

            if (index.texcoord_index >= 0) {
                vertex.texCoords = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    attrib.texcoords[2 * index.texcoord_index + 1],
                };
            }

            vertices.push_back(vertex);
        }
    }
}

void Model::SphereModel(size_t resolution, float radius) {
    vertices.clear();
    size_t stacks = resolution;
    size_t slices = resolution * 2;

    // Generate vertices with smooth normals (pointing outward from center)
    for (size_t i = 0; i <= stacks; ++i) {
        float v = static_cast<float>(i) / static_cast<float>(stacks);
        float phi = v * glm::pi<float>();

        for (size_t j = 0; j <= slices; ++j) {
            float u = static_cast<float>(j) / static_cast<float>(slices);
            float theta = u * glm::two_pi<float>();

            glm::vec3 pos;
            pos.x = radius * sin(phi) * cos(theta);
            pos.y = radius * cos(phi);
            pos.z = radius * sin(phi) * sin(theta);
            
            // For a sphere, the normal is just the normalized position vector
            glm::vec3 normal = glm::normalize(pos);
            glm::vec2 texCoords(u, v);
            vertices.push_back({pos, normal, texCoords});
        }
    }

    // Now generate triangles using the vertices (you'll need indices for this)
    // Or if you want to avoid indices, duplicate vertices for each triangle:
    std::vector<Vertex> triangulatedVertices;
    
    for (size_t i = 0; i < stacks; ++i) {
        for (size_t j = 0; j < slices; ++j) {
            // Calculate vertex indices
            size_t current = i * (slices + 1) + j;
            size_t next = current + slices + 1;
            
            // First triangle
            if (i != 0) { // Skip top pole
                triangulatedVertices.push_back(vertices[current]);
                triangulatedVertices.push_back(vertices[current + 1]);
                triangulatedVertices.push_back(vertices[next + 1]);
            }
            
            // Second triangle  
            if (i != stacks - 1) { // Skip bottom pole
                triangulatedVertices.push_back(vertices[current]);
                triangulatedVertices.push_back(vertices[next + 1]);
                triangulatedVertices.push_back(vertices[next]);
            }
        }
    }
    
    vertices = triangulatedVertices;
}

void Model::CubeModel() {
    vertices = {
        // Back face
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
        {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
        {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
        {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},

        // Front face
        {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},

        // Left face
        {{-0.5f, 0.5f, 0.5f}, {-1.0, 0.0, 0.0}, {1.0f, 0.0f}},
        {{-0.5f, 0.5f, -0.5f}, {-1.0, 0.0, 0.0}, {1.0f, 1.0f}},
        {{-0.5f, -0.5f, -0.5f}, {-1.0, 0.0, 0.0}, {0.0f, 1.0f}},
        {{-0.5f, -0.5f, -0.5f}, {-1.0, 0.0, 0.0}, {0.0f, 1.0f}},
        {{-0.5f, -0.5f, 0.5f}, {-1.0, 0.0, 0.0}, {0.0f, 0.0f}},
        {{-0.5f, 0.5f, 0.5f}, {-1.0, 0.0, 0.0}, {1.0f, 0.0f}},

        // Right face
        {{0.5f, 0.5f, 0.5f}, {1.0, 0.0, 0.0}, {1.0f, 0.0f}},
        {{0.5f, 0.5f, -0.5f}, {1.0, 0.0, 0.0}, {1.0f, 1.0f}},
        {{0.5f, -0.5f, -0.5f}, {1.0, 0.0, 0.0}, {0.0f, 1.0f}},
        {{0.5f, -0.5f, -0.5f}, {1.0, 0.0, 0.0}, {0.0f, 1.0f}},
        {{0.5f, -0.5f, 0.5f}, {1.0, 0.0, 0.0}, {0.0f, 0.0f}},
        {{0.5f, 0.5f, 0.5f}, {1.0, 0.0, 0.0}, {1.0f, 0.0f}},

        // Bottom face
        {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
        {{0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
        {{0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
        {{-0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
        {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},

        // Top face
        {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
        {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
        {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
    };

    indices = {
        2, 1, 0, 5, 4, 3,       // Back face
        6, 7, 8, 9, 10, 11,     // Front face
        12, 13, 14, 15, 16, 17, // Left face
        20, 19, 18, 23, 22, 21, // Right face
        24, 25, 26, 27, 28, 29, // Bottom face
        32, 31, 30, 35, 34, 33  // Top face
    };
}

void Model::CubeModelUnique() {
    vertices = {
        // Back face
        {{0.5f, 0.5f, -0.5f}, {0, 0, -1}, {1, 1}},
        {{0.5f, -0.5f, -0.5f}, {0, 0, -1}, {1, 0}},
        {{-0.5f, -0.5f, -0.5f}, {0, 0, -1}, {0, 0}},

        {{-0.5f, -0.5f, -0.5f}, {0, 0, -1}, {0, 0}},
        {{-0.5f, 0.5f, -0.5f}, {0, 0, -1}, {0, 1}},
        {{0.5f, 0.5f, -0.5f}, {0, 0, -1}, {1, 1}},

        // Front face
        {{-0.5f, -0.5f, 0.5f}, {0, 0, 1}, {0, 0}},
        {{0.5f, -0.5f, 0.5f}, {0, 0, 1}, {1, 0}},
        {{0.5f, 0.5f, 0.5f}, {0, 0, 1}, {1, 1}},

        {{0.5f, 0.5f, 0.5f}, {0, 0, 1}, {1, 1}},
        {{-0.5f, 0.5f, 0.5f}, {0, 0, 1}, {0, 1}},
        {{-0.5f, -0.5f, 0.5f}, {0, 0, 1}, {0, 0}},

        // Left face
        {{-0.5f, 0.5f, 0.5f}, {-1, 0, 0}, {1, 0}},
        {{-0.5f, 0.5f, -0.5f}, {-1, 0, 0}, {1, 1}},
        {{-0.5f, -0.5f, -0.5f}, {-1, 0, 0}, {0, 1}},

        {{-0.5f, -0.5f, -0.5f}, {-1, 0, 0}, {0, 1}},
        {{-0.5f, -0.5f, 0.5f}, {-1, 0, 0}, {0, 0}},
        {{-0.5f, 0.5f, 0.5f}, {-1, 0, 0}, {1, 0}},

        // Right face
        {{0.5f, -0.5f, -0.5f}, {1, 0, 0}, {0, 1}},
        {{0.5f, 0.5f, -0.5f}, {1, 0, 0}, {1, 1}},
        {{0.5f, 0.5f, 0.5f}, {1, 0, 0}, {1, 0}},

        {{0.5f, 0.5f, 0.5f}, {1, 0, 0}, {1, 0}},
        {{0.5f, -0.5f, 0.5f}, {1, 0, 0}, {0, 0}},
        {{0.5f, -0.5f, -0.5f}, {1, 0, 0}, {0, 1}},

        // Bottom face
        {{-0.5f, -0.5f, -0.5f}, {0, -1, 0}, {0, 1}},
        {{0.5f, -0.5f, -0.5f}, {0, -1, 0}, {1, 1}},
        {{0.5f, -0.5f, 0.5f}, {0, -1, 0}, {1, 0}},

        {{0.5f, -0.5f, 0.5f}, {0, -1, 0}, {1, 0}},
        {{-0.5f, -0.5f, 0.5f}, {0, -1, 0}, {0, 0}},
        {{-0.5f, -0.5f, -0.5f}, {0, -1, 0}, {0, 1}},

        // Top face
        {{0.5f, 0.5f, 0.5f}, {0, 1, 0}, {1, 0}},
        {{0.5f, 0.5f, -0.5f}, {0, 1, 0}, {1, 1}},
        {{-0.5f, 0.5f, -0.5f}, {0, 1, 0}, {0, 1}},
        
        {{-0.5f, 0.5f, -0.5f}, {0, 1, 0}, {0, 1}},
        {{-0.5f, 0.5f, 0.5f}, {0, 1, 0}, {0, 0}},
        {{0.5f, 0.5f, 0.5f}, {0, 1, 0}, {1, 0}},
    };

    indices = {};
}

std::vector<float> Model::CubeVertices() {
    std::vector<float> skyboxVertices = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
    };
    return skyboxVertices;
}