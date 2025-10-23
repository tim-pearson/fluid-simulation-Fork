#include "Renderer.hpp"

#include <utility>
#include <stdexcept>

Renderer::Renderer() {}

Renderer::~Renderer() {}

void Renderer::cleanup() {
    m_simpleRenderSystem.cleanup();
    m_cubeMapRenderSystem.cleanup();

    glDeleteFramebuffers(1, &m_mainFrame.fbo);
    glDeleteTextures(1, &m_mainFrame.colorBuffer);
    glDeleteRenderbuffers(1, &m_mainFrame.depthBuffer);
}

void Renderer::initFrameBuffer(uint32_t width, uint32_t height) {
    if (m_mainFrame.fbo != 0) {
        glDeleteFramebuffers(1, &m_mainFrame.fbo);
        glDeleteTextures(1, &m_mainFrame.colorBuffer);
        glDeleteRenderbuffers(1, &m_mainFrame.depthBuffer);
    }
    m_mainFrame = createFrameBuffer(width, height);
}

void Renderer::renderToFbo(uint32_t width, uint32_t height) {
    glBindFramebuffer(GL_FRAMEBUFFER, m_mainFrame.fbo);
    glViewport(0, 0, width, height);
    render();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::renderToScreen(uint32_t width, uint32_t height) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);
    render();
}

void Renderer::render() {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_simpleRenderSystem.render(m_renderInfo);
    m_cubeMapRenderSystem.render(m_renderInfo);
    renderLight();
}

FrameBuffer Renderer::createFrameBuffer(uint32_t width, uint32_t height) {
    FrameBuffer fb;
    glGenFramebuffers(1, &fb.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo);

    glGenTextures(1, &fb.colorBuffer);
    glBindTexture(GL_TEXTURE_2D, fb.colorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb.colorBuffer, 0);

    glGenRenderbuffers(1, &fb.depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, fb.depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fb.depthBuffer);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("Failed to create framebuffer");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return fb;
}

void Renderer::renderLight() {
    if (m_lightShaderProgram == 0 || m_renderInfo.lights.empty()) return;

    for (const auto& light : m_renderInfo.lights) {
        glUseProgram(m_lightShaderProgram);
        glBindVertexArray(light.dummyVAO);
        glUniform3fv(glGetUniformLocation(m_lightShaderProgram, "inPos"), 1, glm::value_ptr(light.pos));
        glUniform3fv(glGetUniformLocation(m_lightShaderProgram, "inColor"), 1, glm::value_ptr(light.color));
        glUniformMatrix4fv(glGetUniformLocation(m_lightShaderProgram, "viewProj"), 1, GL_FALSE, glm::value_ptr(m_renderInfo.projectionMatrix * m_renderInfo.viewMatrix));
        glDrawArrays(GL_POINTS, 0, 1);
    }
}