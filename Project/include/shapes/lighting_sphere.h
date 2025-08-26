#pragma once

#include <glm/glm.hpp>
#include "shader.h"
#include "sphere.h"

class LightingSphere : public Sphere {
public:
    LightingSphere(Shader *shader_program, glm::vec3 light_position, glm::vec3 light_color, glm::vec3 object_color);

    virtual void draw(glm::mat4& model, glm::mat4& view, glm::mat4& projection) override;
    void setColors(glm::vec3 new_light_color, glm::vec3 new_object_color);

private:
    glm::vec3 light_position;
    glm::vec3 light_color;
    glm::vec3 object_color;

    // uniform locations
    GLint light_pos_loc;
    GLint light_color_loc;
    GLint object_color_loc;
};