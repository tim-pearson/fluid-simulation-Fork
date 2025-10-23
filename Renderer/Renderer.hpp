#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

#include "systems/Simple_RS.hpp"
#include "systems/CubeMap_RS.hpp"

#include <vector>

struct FrameBuffer {
    GLuint fbo = 0;
    GLuint colorBuffer = 0;
    GLuint depthBuffer = 0;
};

class Renderer {
private:
    FrameBuffer m_mainFrame;
    RenderInfo m_renderInfo;
    GLuint m_lightShaderProgram = 0;

    Simple_RS m_simpleRenderSystem;
    CubeMap_RS m_cubeMapRenderSystem;

public:
    Renderer();
    ~Renderer();

    void cleanup();

    void setRenderable(Simple_Renderable* renderable) {
        m_simpleRenderSystem.setRenderable(renderable);
    }

    void setSkyBox(SkyBox* skyBox) {
        m_cubeMapRenderSystem.setSkyBox(skyBox);
    }

    void initFrameBuffer(uint32_t width, uint32_t height);
    void initSimpleShaders(GLuint shaderProg) { m_simpleRenderSystem.init(shaderProg); }
    void initCubeMapShaders(GLuint shaderProg) { m_cubeMapRenderSystem.init(shaderProg); }
    void setLightShaderProgram(GLuint shaderProg) { m_lightShaderProgram = shaderProg; }

    GLuint getMainFrameColor() const { return m_mainFrame.colorBuffer; }
    
    void renderToFbo(uint32_t width, uint32_t height);
    void renderToScreen(uint32_t width, uint32_t height);
    void setRenderInfo(const RenderInfo& renderInfo) { m_renderInfo = renderInfo; }
    
private:
    void render();
    void renderLight();
    FrameBuffer createFrameBuffer(uint32_t width, uint32_t height);
};