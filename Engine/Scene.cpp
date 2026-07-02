#include "Scene.hpp"

Scene::~Scene() {}

void Scene::cleanup() {
    m_sphRenderable.meshBuffer.cleanup();
    m_sphRenderable.instanceBuffer.cleanup();
    m_sphRenderable.colorBuffer.cleanup();
}

void Scene::update() {
    if (!m_paused) {
        m_sphSolver.step();
    }
}

void Scene::initSPH(size_t particleCount) {
  std::cout << "initi sip called" << '\n';
    AddSkyBox();
    AddLight();
    createSphereModel();
    
    m_sphSolver.init(particleCount);
    
    m_sphRenderable.instanceCount = particleCount;
    
    // CRITICAL: Bind the VAO ONCE at the start
    m_sphRenderable.meshBuffer.bind();
    
    // Now set up both instance attributes while VAO is bound
    
    // Attribute 3: Position
    GLuint particleSSBO = m_sphSolver.getPositionBufferID();
    glBindBuffer(GL_ARRAY_BUFFER, particleSSBO);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)0);
    glVertexAttribDivisor(3, 1);

    // Attribute 4: Velocity
    GLuint velocitySSBO = m_sphSolver.getVelocityBufferID();
    glBindBuffer(GL_ARRAY_BUFFER, velocitySSBO);
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)0);
    glVertexAttribDivisor(4, 1);

    // Clean up: unbind array buffer and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Scene::createSphereModel() {
  std::cout <<"init spehe modele" << '\n';
    Model sphereModel;
    sphereModel.SphereModel();
    
    std::vector<Vertex>& vertices = sphereModel.getVertices();
    
    VAOConfig config;
    config.attributes.push_back({0, 3, GL_FLOAT, false, sizeof(Vertex), offsetof(Vertex, pos)});
    config.attributes.push_back({1, 3, GL_FLOAT, false, sizeof(Vertex), offsetof(Vertex, normal)});
    config.attributes.push_back({2, 2, GL_FLOAT, false, sizeof(Vertex), offsetof(Vertex, texCoords)});
    
    config.size_vertex = sizeof(Vertex);
    config.num_vertices = vertices.size();
    config.index_count = UINT32_MAX;
    config.draw_mode = GL_TRIANGLES;
    config.usage = GL_STATIC_DRAW;
    
    m_sphRenderable.meshBuffer = Buffer::createMeshBuffer(config, vertices.data(), nullptr);
    m_sphRenderable.material.color = glm::vec3(0.2f, 0.5f, 0.8f);
}

void Scene::AddSkyBox() {
    std::vector<float> vertData = Model::CubeVertices();
    
    m_skyBox.meshBuffer = Buffer::createMeshBuffer(
        VAOConfig{
            .attributes = {{0, 3, GL_FLOAT, false, sizeof(float) * 3, 0}},
            .size_vertex = sizeof(float) * 3,
            .num_vertices = vertData.size() / 3,
            .draw_mode = GL_TRIANGLES,
            .usage = GL_STATIC_DRAW
        },
        vertData.data()
    );
    m_skyBox.cubeMapTexture.loadCubeMap({
        std::string(TEXTURE_DIR) + "space/px.png",
        std::string(TEXTURE_DIR) + "space/nx.png",
        std::string(TEXTURE_DIR) + "space/py.png",
        std::string(TEXTURE_DIR) + "space/ny.png",
        std::string(TEXTURE_DIR) + "space/pz.png",
        std::string(TEXTURE_DIR) + "space/nz.png"
    }, 10);
}

void Scene::AddLight() {
    Light mainLight;
    mainLight.pos = {3.0f, 5.0f, 0.0f};
    mainLight.color = {1.0f, 1.0f, 1.0f};
    glGenVertexArrays(1, &mainLight.dummyVAO);
    m_renderInfo.lights.push_back(mainLight);
}
