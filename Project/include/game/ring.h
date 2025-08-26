#pragma once

#ifndef RESSOURCES_DIR
#error "RESSOURCES_DIR not defined"
#endif
#ifndef SHADER_DIR
#error "SHADER_DIR not defined"
#endif

#include "shape_model.h"
#include "node.h"
#include "shader.h"

class Ring {
    public:
    Ring(glm::vec3 position);
    ~Ring();

    void startAnimation(double current_time);
    void update(float current_time);

    ShapeModel* ring_model;
    Node* ring_node;

    // Animation state
    float animation_time = 0.0f;
    bool animating = false;
    bool collected = false;  // Flag to prevent multiple collections
    bool to_delete = false;
    float animation_duration = 2.0f; // seconds
    float start_scale = 0.0005f;
    float end_scale = 0.006f;
    glm::vec3 position;
};
