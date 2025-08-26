#pragma once

#include "node.h"
#include "shader.h"
#include "shape.h"
#include <glm/glm.hpp>

class Explosion {
public:
    Explosion(Shader* shader, glm::vec3 position, double start_time);
    ~Explosion();
    
    void update(double current_time);
    void draw(glm::mat4& model, glm::mat4& view, glm::mat4& projection);
    bool isActive() const { return active; }
    Node* getNode() const { return explosion_node; }

private:
    Node* explosion_node;
    Shape* explosion_shape;
    Shader* shader;
    double start_time;
    double duration = 0.2; // Duration of explosion in seconds
    bool active = true;
    glm::vec3 initial_scale = glm::vec3(0.01f);
    glm::vec3 max_scale = glm::vec3(0.2f);
};