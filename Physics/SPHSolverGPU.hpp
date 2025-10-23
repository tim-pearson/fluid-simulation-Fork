#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <string>
#include "Photon/Renderer.h"
#include "Photon/shader.h"

// SPH Parameters structure (must match shader UBO layout)
struct SPHParams {
    float h;                     // Smoothing length
    float h2, h3, h6;            // Precomputed powers
    float h9;
    float restDensity;           // Rest density
    float mass;                  // Particle mass
    float pressureMultiplier;    // Pressure coefficient
    float viscosity;             // Viscosity coefficient
    float gravity_m;             // Gravity magnitude
    float bounce;                // Bounce coefficient
    float radius;                // Particle radius
    float maxSpeed;              // Maximum velocity
    float epsilon;               // Small value for numerical stability
    float numParticlesF;         // Number of particles (as float)
    float dt;                    // Time step
    glm::vec4 boxPos;            // Bounding box position
    glm::vec4 boxSize;           // Bounding box size
    glm::vec4 gridMin;           // Grid minimum corner
    glm::ivec4 gridDim;          // Grid dimensions (xyz) + numParticles (w)
    uint32_t numCells;           // Total number of grid cells
    uint32_t padding2[3];        // Padding for alignment
    glm::vec4 wallVelMin;        // Velocity of min boundary wall
    glm::vec4 wallVelMax;        // Velocity of max boundary wall
    glm::vec4 prevBoxPos;        // Previous box position (for computing wall velocity)
    glm::vec4 prevBoxSize;       // Previous box size
};

class SPHSolverGPU {
private:
    size_t m_particleCount = 0;
    uint32_t m_paddedParticleCount = 0;
    
    glm::ivec3 m_gridDim = glm::ivec3(0);
    glm::vec3 m_gridMin = glm::vec3(0);
    size_t m_numCells = 0;
    float m_h = 0;
    
    // GPU buffers
    GLuint m_ubo = 0;                    // Uniform buffer (parameters)
    GLuint m_posBuffer = 0;              // positions (binding 0)
    GLuint m_velBuffer = 0;              // Velocities (binding 1)
    GLuint m_densPresBuffer = 0;         // Density/Pressure (binding 2)
    GLuint m_forceBuffer = 0;            // Forces (binding 3)
    GLuint m_sortKeysBuffer = 0;         // Sort keys (binding 4)
    GLuint m_cellStartBuffer = 0;        // Cell start indices (binding 5)
    GLuint m_predictedPosBuffer = 0;     // Predicted positions (binding 6)
    
    // Compute shaders
    Shader m_predictHashShader{std::string(SHADER_DIR) + "sph_predict_hash.comp"};
    Shader m_densityShader{std::string(SHADER_DIR) + "sph_density.comp"};
    Shader m_forceIntegrateShader{std::string(SHADER_DIR) + "sph_force_integrate.comp"};
    Shader m_bitonicSortShader{std::string(SHADER_DIR) + "gpu_bitonic_sort.comp"};
    Shader m_cellStartShader{std::string(SHADER_DIR) + "gpu_cell_start.comp"};
    Shader m_clearCellStartShader{std::string(SHADER_DIR) + "gpu_clear_cell_start.comp"};
    
    // Parameters
    SPHParams m_paramsUBOData;
public:
    SPHSolverGPU();
    ~SPHSolverGPU();

    void init(size_t particleCount);
    
    void step();
    
    GLuint getPositionBufferID() const { return m_posBuffer; }
    size_t getVelocityBufferID() const { return m_velBuffer; }
    SPHParams* getParams() { return &m_paramsUBOData; }
    size_t getParticleCount() const { return m_particleCount; }
    
    void cleanup();

private:
    void initParams();
    void initShaders();
    void initBuffers();

    void updateUBO();
    void updateGridBuffersGPU();

    void clearCellStart();
    void bitonicSortGPU();
    void buildCellStartGPU();
};