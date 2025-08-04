#ifndef RING_H
#define RING_H
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
    bool to_delete = false;
    float animation_duration = 2.0f; // seconds
    float start_scale = 0.05f;
    float end_scale = 0.6f;
    glm::vec3 position;
};

#endif
