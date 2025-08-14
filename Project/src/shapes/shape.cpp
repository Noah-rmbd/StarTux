// shape.cpp

#include "shape.h"
#include "lighting.h"

Shape::Shape(Shader *shader_program)
    : shader_program_(shader_program->get_id()) {
    // Get uniform locations for phong shader
    light_pos_loc = glGetUniformLocation(this->shader_program_, "lightPos");
    light_color_loc = glGetUniformLocation(this->shader_program_, "lightColor");
    object_color_loc = glGetUniformLocation(this->shader_program_, "objectColor");

    // Set default values for phong lighting
    light_position = glm::vec3(10.0f, 10.0f, 5.0f); // Closer, more directional
    light_color = glm::vec3(1.0f, 0.95f, 0.9f);     // Slightly warm white
    object_color = glm::vec3(1.0f, 0.816f, 0.0f);   // Yellow color
}

void Shape::draw(glm::mat4 &model, glm::mat4 &view, glm::mat4 &projection) {
  glUseProgram(this->shader_program_);
  
  GLint loc = glGetUniformLocation(this->shader_program_, "model");
  glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(model));

  loc = glGetUniformLocation(this->shader_program_, "view");
  glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(view));

  loc = glGetUniformLocation(this->shader_program_, "projection");
  glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(projection));

  // Simple enhanced lighting (minimal performance impact)
  if (g_LightingSystem) {
    // Just set better ambient lighting - no expensive calculations
    glUniform3fv(glGetUniformLocation(this->shader_program_, "ambientLight"), 1, 
                 glm::value_ptr(g_LightingSystem->GetAmbientLight()));
  }
  
  // Always use the standard phong lighting (keep performance)
  glUniform3fv(light_pos_loc, 1, glm::value_ptr(light_position));
  glUniform3fv(light_color_loc, 1, glm::value_ptr(light_color));
  glUniform3fv(object_color_loc, 1, glm::value_ptr(object_color));
}
