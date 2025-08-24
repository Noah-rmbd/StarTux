#include "ring.h"

Ring::Ring(glm::vec3 position) : position(position) {
    std::string shader_dir = SHADER_DIR;
    std::string ressources_dir = RESSOURCES_DIR;

    Shader* rainbow_shader = new Shader(shader_dir + "phong.vert", shader_dir + "phong.frag");
    ring_model = new ShapeModel(ressources_dir + "ring2.obj", rainbow_shader);

    glm::mat4 node_mat =
      glm::translate(glm::mat4(1.0f), position) *
      glm::scale(glm::mat4(1.0f), start_scale * glm::vec3(1.0f)) *
      glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1,0,0));
  
    ring_node = new Node(node_mat);
    ring_node->add(ring_model);
}

Ring::~Ring() {
    delete ring_node;
    delete ring_model;
}

void Ring::startAnimation(double current_time) {
    animating = true;
    animation_time = current_time;
    std::cout << "Animation started at position: " << this
    << position.x << ", " << position.y << ", " << position.z << std::endl;
}

void Ring::update(float current_time) {
    if (animating) {
        double delta = current_time - animation_time;
        float t = delta / animation_duration;
        if (t > 1.0f) t = 1.0f;
        
        float scale = start_scale + (end_scale - start_scale) * t;
        
        glm::vec3 current_position = glm::vec3(ring_node->transform_[3]);
        ring_node->transform_ = glm::translate(glm::mat4(1.0f), current_position) *
                                glm::scale(glm::mat4(1.0f), scale * glm::vec3(1.0f)) *
                                glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1,0,0));
        
                                std::cout << "Animating ring at position: " << this
                                << position.x << ", " << position.y << ", " << position.z
                                << " | scale: " << scale << std::endl; 
        if (delta >= animation_duration) {
            animating = false;
            to_delete = true;
            // Mark for removal from scene (handle in your game logic)
        }
    }
}