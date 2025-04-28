#include "shader.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <sstream>

using namespace std;

Shader::Shader(const std::string& vertex_path, const std::string& fragment_path) {
    GLuint vertex_shader = compile_shader(vertex_path, GL_VERTEX_SHADER);
    GLuint fragment_shader = compile_shader(fragment_path, GL_FRAGMENT_SHADER);

    glid = glCreateProgram();
    glAttachShader(glid, vertex_shader);
    glAttachShader(glid, fragment_shader);
    glLinkProgram(glid);

    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(glid, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(glid, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

Shader::~Shader() {
    glDeleteProgram(glid);
}

GLuint Shader::get_id() {
    return glid;
}

void Shader::use() {
    glUseProgram(glid);
}

void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const {
    glUniformMatrix4fv(glGetUniformLocation(glid, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setVec3(const std::string &name, const glm::vec3 &vec) const {
    glUniform3fv(glGetUniformLocation(glid, name.c_str()), 1, &vec[0]);
}

void Shader::setVec4(const std::string &name, const glm::vec4 &vec) const {
    glUniform4fv(glGetUniformLocation(glid, name.c_str()), 1, &vec[0]);
}

void Shader::setInt(const std::string &name, int value) const {
    glUniform1i(glGetUniformLocation(glid, name.c_str()), value);
}

GLuint Shader::compile_shader(const std::string& path, GLenum shader_type) {
    std::string shader_code;
    std::ifstream shader_file;
    shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        shader_file.open(path);
        std::stringstream shader_stream;
        shader_stream << shader_file.rdbuf();
        shader_file.close();
        shader_code = shader_stream.str();
    }
    catch (std::ifstream::failure& e) {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << path << std::endl;
    }

    const char* shader_code_cstr = shader_code.c_str();
    GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &shader_code_cstr, NULL);
    glCompileShader(shader);

    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    return shader;
}
