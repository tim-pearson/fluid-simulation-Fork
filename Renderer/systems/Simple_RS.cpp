#include "Simple_RS.hpp"
#include <glm/gtc/type_ptr.hpp>

Simple_RS::Simple_RS() {}
Simple_RS::~Simple_RS() {}

void Simple_RS::cleanup() {
    
}

void Simple_RS::init(GLuint shaderProgram) {
    m_shaderProgram = shaderProgram;
}

void Simple_RS::render(RenderInfo& renderInfo) {
    if (!m_renderable || m_renderable->instanceCount == 0) return;

    glUseProgram(m_shaderProgram);
    
    glm::vec3 cameraPos = glm::vec3(glm::inverse(renderInfo.viewMatrix)[3]);
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(renderInfo.viewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(renderInfo.projectionMatrix));
    glUniform3fv(glGetUniformLocation(m_shaderProgram, "viewPos"), 1, glm::value_ptr(cameraPos));
    
    if (!renderInfo.lights.empty()) {
        glUniform3fv(glGetUniformLocation(m_shaderProgram, "lightPos"), 1, glm::value_ptr(renderInfo.lights[0].pos));
        glUniform3fv(glGetUniformLocation(m_shaderProgram, "lightColor"), 1, glm::value_ptr(renderInfo.lights[0].color));
    }
    
    glUniform3fv(glGetUniformLocation(m_shaderProgram, "baseColor"), 1, glm::value_ptr(m_renderable->material.color));
    
    m_renderable->meshBuffer.bind();
    m_renderable->meshBuffer.drawInstanced(m_renderable->instanceCount);
}