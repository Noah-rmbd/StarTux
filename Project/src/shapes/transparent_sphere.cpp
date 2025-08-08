#include "transparent_sphere.h"
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

TransparentSphere::TransparentSphere(Shader *shader_program, glm::vec3 light_position, glm::vec3 light_color, glm::vec3 object_color, float alpha) :
    LightingSphere(shader_program, light_position, light_color, object_color),
    light_position(light_position),
    light_color(light_color), 
    object_color(object_color),
    alpha(alpha)
{
    // Get uniform locations for the transparent shader
    light_pos_loc = glGetUniformLocation(this->shader_program_, "lightPos");
    light_color_loc = glGetUniformLocation(this->shader_program_, "lightColor");
    object_color_loc = glGetUniformLocation(this->shader_program_, "objectColor");
    alpha_loc = glGetUniformLocation(this->shader_program_, "alpha");
}

void TransparentSphere::draw(glm::mat4& model, glm::mat4& view, glm::mat4& projection)
{
    // Enable transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Disable depth writing for transparent objects to avoid z-fighting
    glDepthMask(GL_FALSE);
    
    // Call Sphere::draw to do the basic rendering (skip LightingSphere to avoid wrong uniforms)
    Sphere::draw(model, view, projection);
    
    // Set our own uniform variables for light, color, and alpha AFTER draw
    glUniform3fv(light_pos_loc, 1, glm::value_ptr(light_position));
    glUniform3fv(light_color_loc, 1, glm::value_ptr(light_color));
    glUniform3fv(object_color_loc, 1, glm::value_ptr(object_color));
    
    if (alpha_loc != -1) {
        glUniform1f(alpha_loc, alpha);
    }
    
    // Re-enable depth writing
    glDepthMask(GL_TRUE);
    
    // Disable blending
    glDisable(GL_BLEND);
}

void TransparentSphere::setAlpha(float new_alpha) {
    alpha = glm::clamp(new_alpha, 0.0f, 1.0f);
}