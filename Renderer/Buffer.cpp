#include "Buffer.hpp"

MeshBuffer Buffer::createMeshBuffer(const VAOConfig& config, const void* vertex_data, const void* index_data) {
    MeshBuffer mesh;
    mesh.vao = 0;
    mesh.index_count = config.index_count;
    mesh.draw_mode = config.draw_mode;
    mesh.vertex_count = config.num_vertices;
    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);
    if (config.index_count != UINT32_MAX) glGenBuffers(1, &mesh.ebo);

    mesh.bind();

    mesh.bindVBO();
    glBufferData(GL_ARRAY_BUFFER, config.size_vertex * config.num_vertices, vertex_data, config.usage);
    if (config.index_count != UINT32_MAX) {
        mesh.bindEBO();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, config.index_count * sizeof(GLuint), index_data, config.usage);
    }

    for (const auto& attrib : config.attributes) {
        glEnableVertexAttribArray(attrib.index);
        glVertexAttribPointer(attrib.index,
                                attrib.size,
                                attrib.type,
                                attrib.normalized,
                                attrib.stride,
                                reinterpret_cast<const void*>(attrib.offset)
                            );
    }

    return mesh;
}

MeshBuffer Buffer::createInstanceBuffer(size_t maxInstances, uint32_t attribIndex) {
    MeshBuffer instanceBuffer;
    glGenBuffers(1, &instanceBuffer.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * maxInstances, nullptr, GL_DYNAMIC_DRAW);
    
    glEnableVertexAttribArray(attribIndex);
    glVertexAttribPointer(attribIndex, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glVertexAttribDivisor(attribIndex, 1);
    
    return instanceBuffer;
}

MeshBuffer Buffer::createColorBuffer(size_t maxInstances, uint32_t attribIndex) {
    MeshBuffer colorBuffer;
    glGenBuffers(1, &colorBuffer.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * maxInstances, nullptr, GL_DYNAMIC_DRAW);
    
    glEnableVertexAttribArray(attribIndex);
    glVertexAttribPointer(attribIndex, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
    glVertexAttribDivisor(attribIndex, 1);
    
    return colorBuffer;
}

void Buffer::deleteMeshBuffer(MeshBuffer& mesh) {
    if (mesh.vao != 0) mesh.cleanup();
    mesh.vao = 0;
    mesh.vbo = 0;
    mesh.ebo = 0;
}

void Buffer::createOrResizeSSBO(GLuint& bufferId, size_t size, GLenum usage) {
    if (bufferId == 0) {
        glGenBuffers(1, &bufferId);
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferId);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, usage);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}