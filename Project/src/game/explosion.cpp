#include "explosion.h"
#include "lighting_sphere.h"

Explosion::Explosion(Shader* shader, glm::vec3 position, double start_time) 
    : shader(shader), start_time(start_time) {
    // Create a lighting sphere for the explosion with orange/red colors
    glm::vec3 light_position = position;  // Light at explosion position
    glm::vec3 light_color = glm::vec3(1.0f, 0.5f, 0.0f);  // Orange light
    glm::vec3 object_color = glm::vec3(1.0f, 1.0f, 1.0f);  // White object color
    
    explosion_shape = new LightingSphere(shader, light_position, light_color, object_color);
    
    // Create the node with initial position and scale
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * 
                         glm::scale(glm::mat4(1.0f), initial_scale);
    explosion_node = new Node(transform);
    explosion_node->add(explosion_shape);
}

Explosion::~Explosion() {
    delete explosion_node;
    delete explosion_shape;
}

void Explosion::update(double current_time) {
    if (!active) return;
    
    double elapsed = current_time - start_time;
    if (elapsed >= duration) {
        active = false;
        return;
    }
    
    // Calculate progress (0 to 1)
    float progress = elapsed / duration;
    
    // Interpolate scale from initial to max
    glm::vec3 current_scale = initial_scale + (max_scale - initial_scale) * progress;
    
    // Update the node's transform
    glm::vec3 position = glm::vec3(explosion_node->transform_[3]);
    explosion_node->transform_ = glm::translate(glm::mat4(1.0f), position) * 
                               glm::scale(glm::mat4(1.0f), current_scale);

    // Update colors based on progress
    LightingSphere* sphere = static_cast<LightingSphere*>(explosion_shape);
    glm::vec3 light_color = glm::vec3(1.0f, 0.5f, 0.0f) * (1.0f - progress);  // Fade out orange
    glm::vec3 object_color = glm::vec3(1.0f, 1.0f, 1.0f) * (1.0f - progress);  // Fade out white
    sphere->setColors(light_color, object_color);
}

void Explosion::draw(glm::mat4& model, glm::mat4& view, glm::mat4& projection) {
    if (!active) return;
    explosion_node->draw(model, view, projection);
} 