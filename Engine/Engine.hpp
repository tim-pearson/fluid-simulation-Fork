#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Photon/Renderer.h"

#include "Shader.hpp"
#include "Camera.hpp"
#include "imguiUI.hpp"
#include "Scene.hpp"

#include <vector>

// hardcoded version
const int OPENGL_VERSION_MAJOR = 4;
const int OPENGL_VERSION_MINOR = 5;
const int GLSL_VERSION = OPENGL_VERSION_MAJOR * 100 + OPENGL_VERSION_MINOR * 10;

class Engine {
private:
    struct Window {
        GLFWwindow* handle = nullptr;
        int width = 800;
        int height = 600;
    } m_window;

    FboSize m_mainFboSize{800, 600, false};

    Renderer m_renderer{};
    ImguiUI m_ui{};
    SPHSolverGPU m_sphSolver{};
    Scene m_scene{m_sphSolver};


    UI_Struct m_uiStruct{};

    std::vector<Shader> m_shaderPrograms;

    Camera m_camera;
    CameraController m_cameraController{m_camera};

    double m_inputTime = 0.0;

public:
    Engine();
    ~Engine();

    void run();

    void resetWindowSize(uint32_t width, uint32_t height) {
        m_window.width = width;
        m_window.height = height;
    }

private:
    void initWindow();
    void initOpenGL();
    void initRenderTarget();
    void initUI();
    void initShaders();

    void glConfig();

    void fillUIStruct();

    void processInput(GLFWwindow* window, float deltaTime);

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
};