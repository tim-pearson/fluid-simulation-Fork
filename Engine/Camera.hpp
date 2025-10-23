#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GLFW/glfw3.h>

struct KeyMapping {
    int FORWARD = GLFW_KEY_W;
    int BACKWARD = GLFW_KEY_S;
    int LEFT = GLFW_KEY_A;
    int RIGHT = GLFW_KEY_D;
    int UP = GLFW_KEY_SPACE;
    int DOWN = GLFW_KEY_LEFT_ALT;
    int FPS = GLFW_KEY_F;
    int ROLL_LEFT = GLFW_KEY_Q;
    int ROLL_RIGHT = GLFW_KEY_E;
};

struct CameraSettings {
    float speed = 2.5f;
    float sensitivity = 0.1f;
    float rollSpeed = 10.0f;
    float fov = 45.0f;
};

class Camera {
public:
    glm::mat4 viewMatrix{1.0f};
    glm::mat4 projectionMatrix{1.0f};
    glm::vec3 position{0.0f, 0.0f, 3.0f};
    glm::vec3 front{0.0f, 0.0f, -1.0f};
    glm::vec3 right{1.0f, 0.0f, 0.0f};
    glm::vec3 up{0.0f, 1.0f, 0.0f};
    float yaw{-90.0f};
    float pitch{0.0f};
    float roll{0.0f};
    float fov{45.0f};

    Camera() {}
    ~Camera() {}

    void init(float width, float height);
    void updateProjectionMatrix(float width, float height);
    void updateOrthographicMatrix(float near, float far);
    void updateViewMatrix();
    void processRotation(float deltaYaw, float deltaPitch, float deltaRoll);
    void move(const glm::vec3& offset);
    glm::mat4& getViewMatrix() {return viewMatrix;}
    glm::mat4& getProjectionMatrix() {return projectionMatrix;}
    glm::vec3& getPosition() {return position;}
    glm::vec3& getFront() {return front;}
    glm::vec3& getRight() {return right;}
    glm::vec3& getUp() {return up;}
    float& getYaw() {return yaw;}
    float& getPitch() {return pitch;}
    float& getRoll() {return roll;}
    float& getFov() {return fov;}

    void setPosition(const glm::vec3& pos) {position = pos;}
    void setFront(const glm::vec3& f) {front = f;}
    void setRight(const glm::vec3& r) {right = r;}
    void setUp(const glm::vec3& u) {up = u;}
    void setYaw(float y) {yaw = y;}
    void setPitch(float p) {pitch = p;}
    void setRoll(float r) {roll = r;}
    void setFov(float f) {fov = f;}
};

class CameraController {
private:
    Camera& camera;
    KeyMapping KeyMapping;
    float lastX{0.0f};
    float lastY{0.0f};
    bool firstMouse{true};
    bool isFPS{false};
    bool fpsKeyPressed{false};

public:
    CameraSettings settings;

    CameraController(Camera& cam) : camera(cam) {}

    void processKeyboardInput(GLFWwindow* window, float deltaTime);
    void processMouseInput(GLFWwindow* window, float deltaTime);

    float& getMovementSpeed() {return settings.speed;}
    float& getMouseSensitivity() {return settings.sensitivity;}
    float& getRollSpeed() {return settings.rollSpeed;}
};