#include "Shader.hpp"

#include "utils/files.h"

Shader::Shader(const std::string& vertPath, const std::string& fragPath) :
    m_vertPath(vertPath), m_fragPath(fragPath) {}

Shader::Shader(const std::string& compPath) :
    m_computePath(compPath) {}

Shader::~Shader() {}

void Shader::cleanup() {
    glDeleteProgram(m_id);
    m_id = 0;
}

void Shader::reload() {
    glDeleteProgram(m_id);
    init();
}

void Shader::init() {
    if (m_computePath != "") initComp();
    else initNormal();
}

void Shader::initComp() {
    std::string compCodeStr;
    readFile(m_computePath, compCodeStr);

    compileShader(m_computeId, GL_COMPUTE_SHADER, compCodeStr);

    m_id = glCreateProgram();
    glAttachShader(m_id, m_computeId);
    glLinkProgram(m_id);
    checkCompileErrors(m_id, GL_PROGRAM);

    glDeleteShader(m_computeId);
}

void Shader::initNormal() {
    std::string vertCodeStr;
    std::string fragCodeStr;
    readFile(m_vertPath, vertCodeStr);
    readFile(m_fragPath, fragCodeStr);

    compileShader(m_vertId, GL_VERTEX_SHADER, vertCodeStr);
    compileShader(m_fragId, GL_FRAGMENT_SHADER, fragCodeStr);

    createProg();

    glDeleteShader(m_vertId);
    m_vertId = 0;
    glDeleteShader(m_fragId);
    m_fragId = 0;
}

void Shader::use() {
    glUseProgram(m_id);
}

void Shader::dispatch(unsigned int workGroupsX, unsigned int workGroupsY, unsigned int workGroupsZ) {
    glDispatchCompute(workGroupsX, workGroupsY, workGroupsZ);
}

void Shader::createProg() {
    m_id = glCreateProgram();
    glAttachShader(m_id, m_vertId);
    glAttachShader(m_id, m_fragId);
    glLinkProgram(m_id);
    checkCompileErrors(m_id, GL_PROGRAM);
}

void Shader::compileShader(GLuint& id, GLuint type, const std::string& code) {
    id = glCreateShader(type);
    const char* codeCstr = code.c_str();
    glShaderSource(id, 1, &codeCstr, nullptr);
    glCompileShader(id);
    checkCompileErrors(id, type);
}

void Shader::checkCompileErrors(GLuint id, GLuint type) {
    int success;
    char infoLog[512];
    std::string message;
    if (type == GL_PROGRAM) glGetProgramiv(id, GL_LINK_STATUS, &success);
    else glGetShaderiv(id, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(id, 512, nullptr, infoLog);
        if (type == GL_PROGRAM) message = "Shader Program Linking Failed: ";
        else message = "Shader Compilation Failed: ";
        message += infoLog;
        throw std::runtime_error(message);
    }

}