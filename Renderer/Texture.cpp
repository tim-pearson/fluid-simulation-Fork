#include "Texture.hpp"

#include <iostream>
#include <string>
#include <stdexcept>

#include "stb_image.h"

Texture::Texture() {}
Texture::~Texture() {}

void Texture::loadTexture(const std::string& path, GLint unit, bool genMipMaps) {
    m_unit = unit;
    m_path = path;
    int widthImg, heightImg, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &widthImg, &heightImg, &nrChannels, 0);
    if (!data) error("Failed to load texture at path: " + path);
    
    glGenTextures(1, &m_id);
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    GLenum format;
    switch (nrChannels) {
        case 1: format = GL_RED; break;
        case 2: format = GL_RG; break;
        case 3: format = GL_RGB; break;
        case 4: format = GL_RGBA; break;
        default:
            stbi_image_free(data);
            throw std::runtime_error("Unsupported number of channels in texture image");
    }
    glTexImage2D(GL_TEXTURE_2D, 0, format, widthImg, heightImg, 0, format, GL_UNSIGNED_BYTE, data);
    if (genMipMaps) glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::loadCubeMap(const std::vector<std::string>& paths, GLint unit) {
    if (paths.size() != 6) {
        throw std::runtime_error("Cube map requires exactly 6 texture paths");
    }

    m_unit = unit;
    glGenTextures(1, &m_id);
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);

    for (size_t i = 0; i < paths.size(); ++i) {
        int widthImg, heightImg, nrChannels;
        stbi_set_flip_vertically_on_load(false);
        unsigned char* data = stbi_load(paths[i].c_str(), &widthImg, &heightImg, &nrChannels, 0);
        if (!data) {
            error("Failed to load texture at path: " + paths[i]);
            continue;
        }

        GLenum format;
        switch (nrChannels) {
            case 1: format = GL_RED; break;
            case 2: format = GL_RG; break;
            case 3: format = GL_RGB; break;
            case 4: format = GL_RGBA; break;
            default:
                stbi_image_free(data);
                throw std::runtime_error("Unsupported number of channels in cube map image");
        }
        
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, widthImg, heightImg, 0, format, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

