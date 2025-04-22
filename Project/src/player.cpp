#include "player.h"

Player::Player(Shader* shader_program)
{
    model = new ShapeModel(ship_dir, shader_program);
    
    // Initialize the transformation matrix
    playerMat = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), scale);
    
    // Create and initialize the node with the transformation matrix
    node = new Node(playerMat);
    
    // Add the model to the node
    node->add(model);
}

void Player::updatePosition(){
    node->transform_ = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), scale) 
    * glm::rotate(glm::mat4(1.0f), glm::radians(xAngle), glm::vec3(1.0f, 0.0f, 0.0f))
    * glm::rotate(glm::mat4(1.0f), glm::radians(yAngle), glm::vec3(0.0f, 1.0f, 0.0f))
    * glm::rotate(glm::mat4(1.0f), glm::radians(zAngle), glm::vec3(0.0f, 0.0f, 1.0f));
}