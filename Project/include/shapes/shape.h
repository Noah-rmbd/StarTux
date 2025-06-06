#pragma once

#include "node.h"
#include "shader.h"

#include "glm/ext.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Shape {
public:
  Shape(Shader *shader_program);

  virtual void draw(glm::mat4 &model, glm::mat4 &view, glm::mat4 &projection);

protected:
  GLuint shader_program_;
  
  // Phong shader uniforms
  GLint light_pos_loc;
  GLint light_color_loc;
  GLint object_color_loc;
  glm::vec3 light_position;
  glm::vec3 light_color;
  glm::vec3 object_color;
};
