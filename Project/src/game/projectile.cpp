// projectile.cpp
#include "projectile.h"
#include "shape.h"
//#include <cmath>

Projectile::Projectile(Shader* shader, glm::vec3 position, glm::vec3 direction, glm::vec3 cursor) {
    this->position = position;
    this->direction = glm::normalize(direction);
    speed = 3.0f; // Projectile speed
    active = true;
    cursorPosition = cursor;
    
    // Create a simple shape for the projectile
    Shape* projectile_shape = new Sphere(shader); // Assuming Sphere is a class that derives from Shape
    
    // Create node for the projectile
    glm::mat4 projectile_mat = 
        glm::translate(glm::mat4(1.0f), position) *
        glm::scale(glm::mat4(1.0f), 0.05f * glm::vec3(1.0f, 1.0f, 1.0f));
    
    node = new Node(projectile_mat);
    node->add(projectile_shape);
    
    // Set velocity of projectile
    node->velocity_ = direction * speed;
}

Projectile::~Projectile() {
    // Shape will be deleted by the Node destructor
}

void Projectile::update(double currentTime) {
    // Update position based on direction and speed
    position += direction * speed * 0.01f;
    
    // Update the node's transformation matrix
    node->transform_ = glm::translate(glm::mat4(1.0f), position) *
                      glm::scale(glm::mat4(1.0f), 0.05f * glm::vec3(1.0f, 1.0f, 1.0f));
    
    // Deactivate the projectile if it's gone too far
    if (position.z > 4.0f) {
        active = false;
    }
}

void Projectile::draw(glm::mat4 &model, glm::mat4 &view, glm::mat4 &projection) {
    node->draw(model, view, projection);
}

bool Projectile::checkCollision(glm::vec3 asteroid_pos) {
    // Simple collision detection - distance based
    double x = (position.x - asteroid_pos.x);
    double y = (position.y - asteroid_pos.y);
    double z = (position.z - asteroid_pos.z);
    
    // Use an appropriate collision radius
    if (sqrt(x*x + y*y + z*z) < 0.2) {
        return true;
    }
    return false;
}