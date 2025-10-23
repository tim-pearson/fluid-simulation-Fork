#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/euler_angles.hpp"

enum class RenderType : uint8_t {
    Simple = 0,
    PBR = 1
};

struct Transform {
    glm::mat4 m_matrix{1.0f};

    glm::vec3 pos{0.0f, 0.0f, 0.0f};
    glm::vec3 rot{0.0f, 0.0f, 0.0f};
    glm::vec3 scale{1.0f, 1.0f, 1.0f};

    void setPos(const glm::vec3& position) {
        pos = position;
        calcMatrix();
    }

    void setRot(const glm::vec3& rotation) {
        rot = rotation;
        calcMatrix();
    }

    void setScale(const glm::vec3& scaling) {
        scale = scaling;
        calcMatrix();
    }

    void calcMatrix() {
        glm::mat4 t = glm::translate(glm::mat4(1.0f), pos);
        glm::mat4 r = glm::yawPitchRoll(
            glm::radians(rot.y),
            glm::radians(rot.x),
            glm::radians(rot.z)
        );
        glm::mat4 s = glm::scale(glm::mat4(1.0f), scale);
        m_matrix = t * r * s;
    }

};

struct Light {
    glm::vec3 pos;
    glm::vec3 color;
    GLuint dummyVAO = 0;
};

struct RenderInfo {
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    std::vector<Light> lights;
};