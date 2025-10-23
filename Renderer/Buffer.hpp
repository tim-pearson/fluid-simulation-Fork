#pragma once

#include "glad/glad.h"
#include "glm/glm.hpp"
#include <vector>

struct MeshBuffer {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    size_t index_count = UINT32_MAX;
    size_t vertex_count = 0;
    GLenum draw_mode = GL_TRIANGLES;

    void bind() const {glBindVertexArray(vao);}
    void bindVBO() const {glBindBuffer(GL_ARRAY_BUFFER, vbo);}
    void bindEBO() const {glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);}
    void unbind() const {glBindVertexArray(0);}

    void draw() const {
        if (index_count == UINT32_MAX) glDrawArrays(draw_mode, 0, vertex_count);
        else glDrawElements(draw_mode, index_count, GL_UNSIGNED_INT, 0);
    }

    void drawInstanced(size_t instanceCount) const {
        if (index_count == UINT32_MAX) glDrawArraysInstanced(draw_mode, 0, vertex_count, instanceCount);
        else glDrawElementsInstanced(draw_mode, index_count, GL_UNSIGNED_INT, 0, instanceCount);
    }

    void cleanup() {
        glDeleteVertexArrays(1, &vao);
        if (vbo) glDeleteBuffers(1, &vbo);
        if (ebo) glDeleteBuffers(1, &ebo);
        vao = 0;
        vbo = 0;
        ebo = 0;
    }
};

struct AttribInfo {
    uint32_t index;
    size_t size;
    GLenum type;
    bool normalized = false;
    GLsizei stride;
    size_t offset;
};

struct VAOConfig {
    std::vector<AttribInfo> attributes;
    size_t size_vertex;
    size_t num_vertices;
    size_t index_count = UINT32_MAX;
    GLenum draw_mode = GL_TRIANGLES;
    GLenum usage = GL_DYNAMIC_DRAW;
};

class Buffer {
private:

public:
    static MeshBuffer createMeshBuffer(const VAOConfig& config, const void* vertex_data, const void* index_data = nullptr);
    static MeshBuffer createInstanceBuffer(size_t maxInstances, uint32_t attribIndex);
    static MeshBuffer createColorBuffer(size_t maxInstances, uint32_t attribIndex);
    static void createOrResizeSSBO(GLuint& bufferId, size_t size, GLenum usage = GL_DYNAMIC_DRAW);

    static void deleteMeshBuffer(MeshBuffer& vao);
};