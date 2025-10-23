#include "CubeMap_RS.hpp"

#include <glm/gtc/type_ptr.hpp>

CubeMap_RS::CubeMap_RS() : m_skyBox(nullptr) {}
CubeMap_RS::~CubeMap_RS() {}

void CubeMap_RS::cleanup() {}

void CubeMap_RS::init(GLuint shaderProgram) {
    m_shaderProgram = shaderProgram;
}

void CubeMap_RS::render(RenderInfo& renderInfo) {
    if (!m_skyBox) return;
    glDepthFunc(GL_LEQUAL);

    glUseProgram(m_shaderProgram);
    // remove translation from the view matrix to render the skybox correctly
    glm::mat4 view = renderInfo.viewMatrix;
    view = glm::mat4(glm::mat3(view));
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(renderInfo.projectionMatrix));

    glActiveTexture(GL_TEXTURE0 + m_skyBox->cubeMapTexture.getTextureUnit());
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyBox->cubeMapTexture.getTextureId());
    glUniform1i(glGetUniformLocation(m_shaderProgram, "skybox"), m_skyBox->cubeMapTexture.getTextureUnit());
    
    m_skyBox->meshBuffer.bind();
    m_skyBox->meshBuffer.draw();

    glDepthFunc(GL_LESS);
}