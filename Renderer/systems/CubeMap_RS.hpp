#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

#include "Texture.hpp"
#include "Buffer.hpp"

#include "RenderStructs.hpp"

struct SkyBox {
    MeshBuffer meshBuffer;
    Texture cubeMapTexture;
};

class CubeMap_RS {
private:
    GLuint m_shaderProgram;
    SkyBox* m_skyBox;
public:
    CubeMap_RS();
    ~CubeMap_RS();

    void cleanup();

    void init(GLuint shaderProgram);

    void setSkyBox(SkyBox* skyBox) { m_skyBox = skyBox; }

    void render(RenderInfo& renderInfo);
};