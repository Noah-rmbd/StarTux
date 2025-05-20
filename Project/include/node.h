#pragma once

#include "shape.h"
#include <glm/glm.hpp>
#include <vector>

class Shape;

class Node {
public:
  Node(const glm::mat4 &transform = glm::mat4(1.0f));
  void add(Node *node);
  void add(Shape *shape);
  void draw(glm::mat4 &model, glm::mat4 &view, glm::mat4 &projection);
  void key_handler(int key) const;
  void remove(Node *node);
  void animation(float fps_correction);

  float *z_speed;
  glm::vec3 velocity_ = glm::vec3(0.0f);
  glm::mat4 transform_;
  std::vector<Node *> children_;

private:
  std::vector<Shape *> children_shape_;
};
