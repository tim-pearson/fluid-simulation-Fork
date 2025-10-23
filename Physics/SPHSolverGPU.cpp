#include "SPHSolverGPU.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <numeric>
#include <cmath>

// hardcoded GPU workgroup size
#define WORKGROUP_SIZE 256

SPHSolverGPU::SPHSolverGPU() {}

SPHSolverGPU::~SPHSolverGPU() {
    cleanup();
}

void SPHSolverGPU::cleanup() {
    if (m_ubo) glDeleteBuffers(1, &m_ubo);
    if (m_posBuffer) glDeleteBuffers(1, &m_posBuffer);
    if (m_velBuffer) glDeleteBuffers(1, &m_velBuffer);
    if (m_densPresBuffer) glDeleteBuffers(1, &m_densPresBuffer);
    if (m_forceBuffer) glDeleteBuffers(1, &m_forceBuffer);
    if (m_sortKeysBuffer) glDeleteBuffers(1, &m_sortKeysBuffer);
    if (m_cellStartBuffer) glDeleteBuffers(1, &m_cellStartBuffer);
    if (m_predictedPosBuffer) glDeleteBuffers(1, &m_predictedPosBuffer);
    m_predictHashShader.cleanup();
    m_densityShader.cleanup();
    m_forceIntegrateShader.cleanup();
    m_bitonicSortShader.cleanup();
    m_cellStartShader.cleanup();
    m_clearCellStartShader.cleanup();
}

void SPHSolverGPU::step() {
    if (m_particleCount == 0) return;

    updateUBO();

    unsigned int numGroups = (m_particleCount + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE;
    
    // Compute Predicted Positions and Hashes
    m_predictHashShader.use();
    m_predictHashShader.dispatch(numGroups);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // GPU Grid Update (sort + build cell start)
    updateGridBuffersGPU();
    
    // Compute Density and Pressure
    m_densityShader.use();
    m_densityShader.dispatch(numGroups);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // Compute Forces and Integrate
    m_forceIntegrateShader.use();
    m_forceIntegrateShader.dispatch(numGroups);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void SPHSolverGPU::init(size_t particleCount) {
    glGenBuffers(1, &m_ubo);
    initShaders();
    m_particleCount = particleCount;
    initParams();

    // used for bitonic sort (next power of two)
    m_paddedParticleCount = 1;
    while (m_paddedParticleCount < particleCount) m_paddedParticleCount <<= 1;

    initBuffers();
}

void SPHSolverGPU::initShaders() {
    m_predictHashShader.init();
    m_densityShader.init();
    m_forceIntegrateShader.init();
    m_bitonicSortShader.init();
    m_cellStartShader.init();
    m_clearCellStartShader.init();
}

void SPHSolverGPU::initParams() {
    m_paramsUBOData.radius = 0.05f;
    m_paramsUBOData.h = 2.0f * m_paramsUBOData.radius;
    m_paramsUBOData.h2 = m_paramsUBOData.h * m_paramsUBOData.h;
    m_paramsUBOData.h3 = m_paramsUBOData.h2 * m_paramsUBOData.h;
    m_paramsUBOData.h6 = m_paramsUBOData.h3 * m_paramsUBOData.h3;
    m_paramsUBOData.h9 = m_paramsUBOData.h6 * m_paramsUBOData.h3;
    m_paramsUBOData.restDensity = 1000.0f;
    m_paramsUBOData.mass = m_paramsUBOData.restDensity * (4.0f / 3.0f) * glm::pi<float>() * std::pow(0.05f, 3);
    m_paramsUBOData.pressureMultiplier = 0.2f;
    m_paramsUBOData.viscosity = 0.100f;
    m_paramsUBOData.gravity_m = -0.4f;
    m_paramsUBOData.bounce = 0.5f;
    m_paramsUBOData.maxSpeed = 90.0f;
    m_paramsUBOData.epsilon = 1e-4f;
    m_paramsUBOData.numParticlesF = static_cast<float>(m_particleCount);
    m_paramsUBOData.dt = 0.001f;

    m_h = m_paramsUBOData.h;

    // Hardcoded Grid Parameters
    const float spaceSizeX = 30.0f;
    const float spaceSizeY = 10.0f;
    const float spaceSizeZ = 10.0f;

    m_gridMin = glm::vec3(-spaceSizeX / 2.0f, -spaceSizeY / 2.0f, -spaceSizeZ / 2.0f);
    m_gridDim.x = static_cast<int>(std::ceil(spaceSizeX / m_h));
    m_gridDim.y = static_cast<int>(std::ceil(spaceSizeY / m_h));
    m_gridDim.z = static_cast<int>(std::ceil(spaceSizeZ / m_h));
    m_numCells = m_gridDim.x * m_gridDim.y * m_gridDim.z;

    m_paramsUBOData.boxPos = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    m_paramsUBOData.boxSize = glm::vec4(3.0f, 3.0f, 5.0f, 0.0f);
    m_paramsUBOData.prevBoxPos = m_paramsUBOData.boxPos;
    m_paramsUBOData.prevBoxSize = m_paramsUBOData.boxSize;
    m_paramsUBOData.wallVelMin = glm::vec4(0.0f);
    m_paramsUBOData.wallVelMax = glm::vec4(0.0f);

    m_paramsUBOData.gridMin = glm::vec4(m_gridMin, 0.0f);
    m_paramsUBOData.gridDim = glm::ivec4(m_gridDim, static_cast<int>(m_particleCount));

    m_paramsUBOData.numParticlesF = static_cast<float>(m_particleCount);
    m_paramsUBOData.numCells = m_numCells;
}

void SPHSolverGPU::initBuffers() {
    glBindBuffer(GL_UNIFORM_BUFFER, m_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(SPHParams), &m_paramsUBOData, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Buffer 0: Position (actual positions)
    Buffer::createOrResizeSSBO(m_posBuffer, m_particleCount * sizeof(glm::vec4));
    
    std::vector<glm::vec4> initialPositions;
    initialPositions.reserve(m_particleCount);
    
    // spawn in particles
    float spacing = m_paramsUBOData.radius * 2.0f;
    int particlesPerDim = static_cast<int>(std::ceil(std::cbrt(m_particleCount)));
    float halfCubeSize = particlesPerDim * spacing / 2.0f;
    
    for (int i = 0; i < particlesPerDim; ++i) {
        for (int j = 0; j < particlesPerDim; ++j) {
            for (int k = 0; k < particlesPerDim; ++k) {
                if (initialPositions.size() < m_particleCount) {
                    glm::vec3 pos;
                    pos.x = i * spacing - halfCubeSize;
                    pos.y = j * spacing - halfCubeSize + m_paramsUBOData.boxPos.y + m_paramsUBOData.boxSize.y * 0.4f;
                    pos.z = k * spacing - halfCubeSize;
                    initialPositions.emplace_back(pos, 0.0f);
                }
            }
        }
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_posBuffer);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, initialPositions.size() * sizeof(glm::vec4), initialPositions.data());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_posBuffer);

    // Buffer 1: Velocity vec4 (xyz: velocity, w: mass)
    std::vector<glm::vec4> initialVelMass(m_particleCount, glm::vec4(0.0f, 0.0f, 0.0f, m_paramsUBOData.mass));
    Buffer::createOrResizeSSBO(m_velBuffer, m_particleCount * sizeof(glm::vec4));
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_velBuffer);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_particleCount * sizeof(glm::vec4), initialVelMass.data());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_velBuffer);

    // Buffer 2: Density/Pressure vec4 (x: density, y: pressure, z/w: padding)
    Buffer::createOrResizeSSBO(m_densPresBuffer, m_particleCount * sizeof(glm::vec4));
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_densPresBuffer);

    // Buffer 3: Force vec4 (xyz: force, w: padding)
    Buffer::createOrResizeSSBO(m_forceBuffer, m_particleCount * sizeof(glm::vec4));
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_forceBuffer);

    // Buffer 4: Sort Keys (uvec2: x=cell index, y=particle index)
    Buffer::createOrResizeSSBO(m_sortKeysBuffer, m_paddedParticleCount * sizeof(glm::uvec2));
    // Initialize padding with max values (will sort to end)
    if (m_paddedParticleCount > m_particleCount) {
        std::vector<glm::uvec2> padding(m_paddedParticleCount - m_particleCount, 
                                        glm::uvec2(0xFFFFFFFF, 0xFFFFFFFF));
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_sortKeysBuffer);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER,
                       m_particleCount * sizeof(glm::uvec2),
                       padding.size() * sizeof(glm::uvec2),
                       padding.data());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
    
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_sortKeysBuffer);

    // Buffer 5: Cell Start
    Buffer::createOrResizeSSBO(m_cellStartBuffer, m_numCells * sizeof(uint32_t));
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_cellStartBuffer);

    // Buffer 6: Predicted Positions
    Buffer::createOrResizeSSBO(m_predictedPosBuffer, m_particleCount * sizeof(glm::vec4));
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, m_predictedPosBuffer);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void SPHSolverGPU::updateUBO() {
    glm::vec3 wallVelMin = ((glm::vec3(m_paramsUBOData.boxPos) - glm::vec3(m_paramsUBOData.boxSize) * 0.5f) - 
                            (glm::vec3(m_paramsUBOData.prevBoxPos) - glm::vec3(m_paramsUBOData.prevBoxSize) * 0.5f)) / m_paramsUBOData.dt;
    glm::vec3 wallVelMax = ((glm::vec3(m_paramsUBOData.boxPos) + glm::vec3(m_paramsUBOData.boxSize) * 0.5f) - 
                            (glm::vec3(m_paramsUBOData.prevBoxPos) + glm::vec3(m_paramsUBOData.prevBoxSize) * 0.5f)) / m_paramsUBOData.dt;
    
    m_paramsUBOData.wallVelMin = glm::vec4(wallVelMin, 0.0f);
    m_paramsUBOData.wallVelMax = glm::vec4(wallVelMax, 0.0f);
    
    m_paramsUBOData.prevBoxPos = m_paramsUBOData.boxPos;
    m_paramsUBOData.prevBoxSize = m_paramsUBOData.boxSize;

    glBindBuffer(GL_UNIFORM_BUFFER, m_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(SPHParams), &m_paramsUBOData);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void SPHSolverGPU::updateGridBuffersGPU() {
    clearCellStart();
    bitonicSortGPU();
    buildCellStartGPU();
}

void SPHSolverGPU::clearCellStart() {
    m_clearCellStartShader.use();
    glUniform1ui(glGetUniformLocation(m_clearCellStartShader.getProgramId(), "numCells"), m_numCells);
    
    unsigned int numGroups = (m_numCells + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE;
    m_clearCellStartShader.dispatch(numGroups);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void SPHSolverGPU::bitonicSortGPU() {
    m_bitonicSortShader.use();
    GLuint shaderID = m_bitonicSortShader.getProgramId();
    
    glUniform1ui(glGetUniformLocation(shaderID, "numElements"), m_paddedParticleCount);

    for (uint32_t stage = 2; stage <= m_paddedParticleCount; stage <<= 1) {
        glUniform1ui(glGetUniformLocation(shaderID, "stage"), stage);
        for (uint32_t step = stage >> 1; step > 0; step >>= 1) {
            glUniform1ui(glGetUniformLocation(shaderID, "step"), step);

            unsigned int numGroups = (m_paddedParticleCount + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE;
            m_bitonicSortShader.dispatch(numGroups);

            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        }
    }
}

void SPHSolverGPU::buildCellStartGPU() {
    m_cellStartShader.use();
    glUniform1ui(glGetUniformLocation(m_cellStartShader.getProgramId(), "numParticles"), m_particleCount);
    glUniform1ui(glGetUniformLocation(m_cellStartShader.getProgramId(), "numCells"), m_numCells);
    
    unsigned int numGroups = (m_particleCount + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE;
    m_cellStartShader.dispatch(numGroups);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}