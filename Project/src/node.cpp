#include "node.h"
#include "shape.h"
#include <algorithm>
#include <iostream>
// debug
#include <glm/gtx/string_cast.hpp>
Node::Node(const glm::mat4 &transform) : transform_(transform) {

  children_ = std::vector<Node *>();
}

void Node::add(Node *node) { children_.push_back(node); }

void Node::add(Shape *shape) { children_shape_.push_back(shape); }

void Node::draw(glm::mat4 &model, glm::mat4 &view, glm::mat4 &projection) {
  glm::mat4 updatedModel = model * transform_;
  // std::cout << "Node draw: updatedModel = " << glm::to_string(updatedModel)
  // << std::endl;

  for (auto child : children_) {
    child->draw(updatedModel, view, projection);
  }

  for (auto child : children_shape_) {
    child->draw(updatedModel, view, projection);
  }
}

void Node::key_handler(int key) const {
  for (const auto &child : children_) {
    child->key_handler(key);
  }
}

void Node::animation() {
  // Avancer selon la vélocité
  transform_ = glm::translate(transform_, velocity_);

  // Si le cube est trop proche de la caméra
  if (transform_[3].z > 5.0f) {
    // Nouvelle position aléatoire en X et Y
    float newX = ((rand() % 200) / 100.0f) - 1.0f; // entre -1 et 1
    float newY = ((rand() % 200) / 100.0f) - 1.0f; // entre -1 et 1

    // Nouvelle matrice : position random en X,Y et fond à Z = -4
    transform_ = glm::translate(glm::mat4(1.0f), glm::vec3(newX, newY, -4.0f)) *
                 glm::scale(glm::mat4(1.0f), glm::vec3(0.2f));
  }

  // Animer les enfants
  for (Node *child : children_) {
    child->animation();
  }
}

void Node::remove(Node *node) {
  auto it = std::find(children_.begin(), children_.end(), node);
  if (it != children_.end()) {
    children_.erase(it);
  }
}
