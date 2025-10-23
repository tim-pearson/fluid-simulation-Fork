#include "imguiUI.hpp"

#include <algorithm>
#include <iostream>

ImguiUI::ImguiUI() {}

ImguiUI::~ImguiUI() {}

void ImguiUI::cleanup() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    m_viewport = nullptr;
    m_io = nullptr;
    m_window = nullptr;
}

void ImguiUI::init(GLFWwindow* window, const std::string& glsl_version) {
    m_window = window;
    // glsl version is expected to be in the format "450"
    m_versionMajor = std::stoi(glsl_version.substr(0, 1));
    m_versionMinor = std::stoi(glsl_version.substr(1, 1));

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    m_io = &ImGui::GetIO();
    m_viewport = ImGui::GetMainViewport();

    m_io->ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
    m_io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport
    m_io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Navigation

    m_dockspaceFlags = ImGuiDockNodeFlags_None;

    m_windowFlags = ImGuiWindowFlags_NoTitleBar
                  | ImGuiWindowFlags_NoCollapse
                  | ImGuiWindowFlags_NoResize
                  | ImGuiWindowFlags_NoMove
                  | ImGuiWindowFlags_NoBringToFrontOnFocus
                  | ImGuiWindowFlags_NoNavFocus;

    ImGui::StyleColorsDark();

    std::string version = "#version " + glsl_version;

    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init(version.c_str());
}

void ImguiUI::beginRender() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Docking setup
    beginDockSpace();
}

void ImguiUI::render(UI_Struct& ui_struct) {
    renderScenePanel((ImTextureID)(intptr_t)ui_struct.main_fbo_tex, ui_struct.mainFboSize);
    renderInfoPanel(ui_struct);
    ImGui::Render();
}

void ImguiUI::endRender() {
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    if (m_io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup);
    }
}

void ImguiUI::renderScenePanel(ImTextureID sceneTexture, FboSize* mainFboSize) {
    ImGui::Begin("Scene");
    ImVec2 avail = ImGui::GetContentRegionAvail();
    if (m_prevAvail.x != avail.x || m_prevAvail.y != avail.y) {
        m_prevAvail = avail;
        mainFboSize->resized = true;
        mainFboSize->width = static_cast<uint32_t>(std::max(avail.x, 1.0f));
        mainFboSize->height = static_cast<uint32_t>(std::max(avail.y, 1.0f));
    }
    ImGui::Image(sceneTexture, avail, ImVec2(0, 1), ImVec2(1, 0));
    ImGui::End();
}

void ImguiUI::renderInfoPanel(UI_Struct& ui_struct) {
    ImGui::Begin("Info");
    info();
    sceneSettings(ui_struct.scene, ui_struct.lights);
    shaders(ui_struct.shaders);
    ImGui::End();
}

void ImguiUI::info() {
    current_time = glfwGetTime();
    if (current_time - last_updated_time >= 1.0) {
        ms_per_frame = 1000.0f / ImGui::GetIO().Framerate;  // or 1000.0f * DeltaTime
        last_updated_time = current_time;
    }

    if (ImGui::CollapsingHeader("Info Panel")) {
        ImGui::TextWrapped("Graphics API: OpenGL %d.%d \n\n", m_versionMajor, m_versionMinor);
        ImGui::TextWrapped("Renderer: %s\n\n", glGetString(GL_RENDERER));
        ImGui::TextWrapped("Vendor: %s\n\n", glGetString(GL_VENDOR));
        ImGui::TextWrapped("Version: %s\n\n", glGetString(GL_VERSION));
        ImGui::TextWrapped("Frame time : %.1f ms\n\n", ms_per_frame);
    }
}

void ImguiUI::sceneSettings(Scene* scene, std::vector<Light>* lights) {
    if (ImGui::CollapsingHeader("Scene Settings")) {
        if (ImGui::Button(scene->isPaused() ? "Play" : "Pause")) {
            scene->pause();
        }
        
        ImGui::Text("Particle Count: %zu", scene->getParticleCount());
        
        if (ImGui::TreeNode("Lights")) {
            for (size_t i = 0; i < lights->size(); ++i) {
                std::string lightName = "Light " + std::to_string(i);
                if (ImGui::TreeNode(lightName.c_str())) {
                    ImGui::ColorEdit3("Color", &lights->at(i).color[0]);
                    ImGui::SliderFloat3("Position", &lights->at(i).pos[0], -10.0f, 10.0f);
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        } else if (ImGui::TreeNode("SPH Params")) {
            SPHParams* params = scene->getSPHSolver()->getParams();
            ImGui::SliderFloat("Rest Density", &params->restDensity, 500.0f, 2000.0f);
            ImGui::SliderFloat("Viscosity", &params->viscosity, 0.0f, 1.0f);
            ImGui::SliderFloat("Pressure Multiplier", &params->pressureMultiplier, 0.1f, 10.0f);
            ImGui::SliderFloat("Gravity", &params->gravity_m, -1.0f, 0.0f);
            ImGui::SliderFloat("Time Step (dt)", &params->dt, 0.00001f, 0.001f);
            ImGui::SliderFloat("Bounce", &params->bounce, 0.0f, 1.0f);
            ImGui::SliderFloat("Max Speed", &params->maxSpeed, 1.0f, 100.0f);
            ImGui::DragFloat3("Box Position", &params->boxPos[0], 0.01f);
            ImGui::DragFloat3("Box Size", &params->boxSize[0], 0.01f, 0.1f, 50.0f);
            ImGui::TreePop();
        }
    }
}

void ImguiUI::shaders(std::vector<Shader>* shaders) {
    if (ImGui::CollapsingHeader("Shaders")) {
        for (size_t i = 0; i < shaders->size(); ++i) {
            std::string str = "Reload " + std::to_string(shaders->at(i).getProgramId());
            if (ImGui::Button(str.c_str())) {
                m_onShaderReload(i);
            }
        }
    }
}

void ImguiUI::beginDockSpace() {
    ImGui::SetNextWindowPos(m_viewport->Pos);
    ImGui::SetNextWindowSize(m_viewport->Size);
    ImGui::SetNextWindowViewport(m_viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("DockSpace", nullptr, m_windowFlags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspace_id = ImGui::GetID("DockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), m_dockspaceFlags);

    ImGui::End();
}