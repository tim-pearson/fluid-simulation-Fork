#include "camera.hpp"

void Camera::init(float width, float height) {
    updateProjectionMatrix(width, height);
    updateViewMatrix();
}

void Camera::updateProjectionMatrix(float width, float height) {
    // hardcoded thrustum Depth
    projectionMatrix = glm::perspective(glm::radians(fov), width / height, 0.1f, 1000.0f);
}

void Camera::updateOrthographicMatrix(float near, float far) {
    projectionMatrix = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, near, far);
}

void Camera::updateViewMatrix() {
    // Update the front vector based on yaw and pitch
    front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    front.y = sin(glm::radians(pitch));
    front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));

    // update up vector based on roll
    up = {0.0f, 1.0f, 0.0f};
    glm::mat4 rollMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(roll), front);
    up = glm::normalize(glm::vec3(rollMatrix * glm::vec4(up, 0.0f)));

    // update right vector
    right = glm::normalize(glm::cross(front, up));

    viewMatrix = glm::lookAt(position, position + front, up);
}

void Camera::processRotation(float deltaYaw, float deltaPitch, float deltaRoll) {
    yaw += deltaYaw;
    pitch += deltaPitch;
    roll += deltaRoll;

    // Clamp pitch to avoid gimbal lock
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    if (yaw > 270.0f) yaw = -90.0f;
    if (yaw < -450.0f) yaw = -90.0f;


    updateViewMatrix();
}

void Camera::move(const glm::vec3& offset) {
    position += offset;
    updateViewMatrix();
}

void CameraController::processKeyboardInput(GLFWwindow* window, float deltaTime) {
    if (glfwGetKey(window, KeyMapping.FORWARD) == GLFW_PRESS) {
        camera.move(camera.getFront() * settings.speed* deltaTime);
    }
    if (glfwGetKey(window, KeyMapping.BACKWARD) == GLFW_PRESS) {
        camera.move(-camera.getFront() * settings.speed* deltaTime);
    }
    if (glfwGetKey(window, KeyMapping.LEFT) == GLFW_PRESS) {
        camera.move(-camera.getRight() * settings.speed* deltaTime);
    }
    if (glfwGetKey(window, KeyMapping.RIGHT) == GLFW_PRESS) {
        camera.move(camera.getRight() * settings.speed* deltaTime);
    }
    if (glfwGetKey(window, KeyMapping.UP) == GLFW_PRESS) {
        camera.move(camera.getUp() * settings.speed* deltaTime);
    }
    if (glfwGetKey(window, KeyMapping.DOWN) == GLFW_PRESS) {
        camera.move(-camera.getUp() * settings.speed* deltaTime);
    }
    if (glfwGetKey(window, KeyMapping.ROLL_LEFT) == GLFW_PRESS) {
        camera.processRotation(0.0f, 0.0f, -settings.rollSpeed * deltaTime);
    }
    if (glfwGetKey(window, KeyMapping.ROLL_RIGHT) == GLFW_PRESS) {
        camera.processRotation(0.0f, 0.0f, settings.rollSpeed * deltaTime);
    }

    if (glfwGetKey(window, KeyMapping.FPS) == GLFW_PRESS) {
        if (!fpsKeyPressed) {
            isFPS = !isFPS;
            if (isFPS) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
            firstMouse = true; // Reset mouse position on toggle
            fpsKeyPressed = true;
        }
    } else if (glfwGetKey(window, KeyMapping.FPS) == GLFW_RELEASE) {
        fpsKeyPressed = false; 
    }
}

void CameraController::processMouseInput(GLFWwindow* window, float deltaTime) {
    if (!isFPS) return;

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    if (firstMouse) {
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);
        firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos - lastX) * settings.sensitivity;
    float yoffset = static_cast<float>(lastY - ypos) * settings.sensitivity;

    camera.processRotation(xoffset, yoffset, 0.0f); // No roll adjustment here

    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);
}