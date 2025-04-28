#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>

/** \brief A graphic program.*/
class Shader {
public:
    /** \brief the shader constructor. Should never be called alone (use loader functions)*/
    Shader(const std::string& vertex_path, const std::string& fragment_path);

    /* \brief Destructor. Destroy the shader component created */
    ~Shader();

    GLuint get_id();
    
    // Use the shader
    void use();
    
    // Uniform setters
    void setMat4(const std::string &name, const glm::mat4 &mat) const;
    void setVec3(const std::string &name, const glm::vec3 &vec) const;
    void setVec4(const std::string &name, const glm::vec4 &vec) const;
    void setInt(const std::string &name, int value) const;

private:
    GLuint glid;
    GLuint compile_shader(const std::string& path, GLenum shader_type);
};

#endif // SHADER_H
