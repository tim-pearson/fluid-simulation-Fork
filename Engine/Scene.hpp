#pragma once

#include "Photon/Renderer.h"
#include "Shader.hpp"
#include "Model.hpp"
#include "SPHSolverGPU.hpp"

class Scene {
private:
    SPHSolverGPU& m_sphSolver;
    bool m_paused = false;

    Simple_Renderable m_sphRenderable;
    
    SkyBox m_skyBox;
    RenderInfo m_renderInfo;
    
public:
    Scene(SPHSolverGPU& sphSolver) : m_sphSolver(sphSolver) {}
    ~Scene();

    void cleanup();

    void pause() { m_paused = !m_paused; }
    bool isPaused() const { return m_paused; }

    void setViewMatrix(const glm::mat4& view) { m_renderInfo.viewMatrix = view; }
    void setProjectionMatrix(const glm::mat4& projection) { m_renderInfo.projectionMatrix = projection; }

    RenderInfo& getRenderInfo() { return m_renderInfo; }
    Simple_Renderable* getRenderable() { return &m_sphRenderable; }
    SkyBox* getSkyBox() { return &m_skyBox; }
    SPHSolverGPU* getSPHSolver() { return &m_sphSolver; }
    size_t getParticleCount() const { return m_sphSolver.getParticleCount(); }

    void initSPH(size_t particleCount = 1000);
    void AddLight();
    void AddSkyBox();
    void update();
    
private:
    void createSphereModel();
};