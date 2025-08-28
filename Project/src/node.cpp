#include "node.h"
#include "shape.h"
#include <algorithm>
#include <iostream>
// debug
#include <glm/gtx/string_cast.hpp>
Node::Node(const glm::mat4 &transform) : transform_(transform), z_speed(nullptr) {

  children_ = std::vector<Node *>();
}

Node::~Node() {
  // Clear vectors but don't delete the objects (they may be shared or owned elsewhere)
  children_.clear();
  children_shape_.clear();
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
    if (child == nullptr) {
      continue;
    }
    child->draw(updatedModel, view, projection);
  }
}

void Node::key_handler(int key) const {
  for (const auto &child : children_) {
    child->key_handler(key);
  }
}

void Node::animation(float fps_correction) {
  // Avancer selon la vélocité
  if(z_speed != nullptr) {
    velocity_.z = *z_speed * 0.006; 
  } else {
    velocity_.z = 0.0f;
  }
  if(y_speed != nullptr) {
    velocity_.y = *y_speed * 0.006; 
  } else {
    velocity_.y = 0.0f;
  }
  if(x_speed != nullptr) {
    velocity_.x = *x_speed * 0.006; 
  } else {
    velocity_.x = 0.0f;
  }

  transform_ = glm::translate(glm::mat4(1.0f), velocity_ * fps_correction) * transform_;

  // Animer les enfants
  for (Node *child : children_) {
    child->animation(fps_correction);
  }
}

void Node::remove(Node *node) {
  auto it = std::find(children_.begin(), children_.end(), node);
  if (it != children_.end()) {
    children_.erase(it);
  }
}

void Node::remove(Shape *shape) {
  auto it = std::find(children_shape_.begin(), children_shape_.end(), shape);
  if (it != children_shape_.end()) {
    children_shape_.erase(it);
  }
}
