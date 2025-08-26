#pragma once

#include <glm/glm.hpp>
#include "shader.h"
#include "lighting_sphere.h"

class TransparentSphere : public LightingSphere {
public:
    TransparentSphere(Shader *shader_program, glm::vec3 light_position, glm::vec3 light_color, glm::vec3 object_color, float alpha = 0.3f);

    virtual void draw(glm::mat4& model, glm::mat4& view, glm::mat4& projection) override;
    void setAlpha(float new_alpha);
    float getAlpha() const { return alpha; }

private:
    float alpha;
    
    // Store our own values since parent's are private
    glm::vec3 light_position;
    glm::vec3 light_color;
    glm::vec3 object_color;
    
    // Our own uniform locations for the transparent shader
    GLint light_pos_loc;
    GLint light_color_loc;
    GLint object_color_loc;
    GLint alpha_loc;
};