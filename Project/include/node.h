#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "shape.h"

class Shape;

class Node {
public:
  Node(const glm::mat4 &transform = glm::mat4(1.0f));
  void add(Node *node);
  void add(Shape *shape);
  void draw(glm::mat4 &model, glm::mat4 &view, glm::mat4 &projection);
  void key_handler(int key) const;
  glm::mat4 transform_;
  void animation();
  glm::vec3 velocity_ = glm::vec3(0.0f);
  void remove(Node *node);

private:
  std::vector<Node *> children_;
  std::vector<Shape *> children_shape_;
};
