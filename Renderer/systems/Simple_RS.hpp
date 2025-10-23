#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

#include "Texture.hpp"
#include "Buffer.hpp"
#include "RenderStructs.hpp"

struct Simple_Material {
    glm::vec3 color{0.2f, 0.5f, 0.8f};
    float padding;
};

struct Simple_Renderable {
    MeshBuffer meshBuffer;
    MeshBuffer instanceBuffer;
    MeshBuffer colorBuffer;
    Simple_Material material;
    size_t instanceCount = 0;
};

class Simple_RS {
private:
    GLuint m_shaderProgram;
    Simple_Renderable* m_renderable;
public:
    Simple_RS();
    ~Simple_RS();

    void cleanup();
    void init(GLuint shaderProgram);
    void setRenderable(Simple_Renderable* renderable) { m_renderable = renderable; }
    void render(RenderInfo& renderInfo);
};