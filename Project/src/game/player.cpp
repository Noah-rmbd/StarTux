#include "player.h"
#ifndef RESSOURCES_DIR
#error "RESSOURCES_DIR not defined"
#endif

Player::Player(Shader* shader_program)
{
    ship_dir = RESSOURCES_DIR;
    // Create texture shader
    std::string shader_dir = SHADER_DIR;
    texture_shader = new Shader(shader_dir + "ship.vert", shader_dir + "ship.frag");
    
    // Load texture
    ship_texture = new Texture(ship_dir + "Material.001_Base_color.jpg");
    
    // Create model with texture shader
    model = new ShapeModel(ship_dir + "ship.obj", texture_shader);
    static_cast<ShapeModel*>(model)->setTexture(ship_texture);
    
    // Initialize the transformation matrix
    playerMat = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), scale);
    
    // Create and initialize the node with the transformation matrix
    node = new Node(playerMat);
    
    // Add the model to the node
    node->add(model);
}

Player::~Player() {
    delete node;
    delete model;
    delete ship_texture;
    delete texture_shader;
}

void Player::updatePosition(){
    node->transform_ = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), scale) 
    * glm::rotate(glm::mat4(1.0f), glm::radians(xAngle), glm::vec3(1.0f, 0.0f, 0.0f))
    * glm::rotate(glm::mat4(1.0f), glm::radians(yAngle), glm::vec3(0.0f, 1.0f, 0.0f))
    * glm::rotate(glm::mat4(1.0f), glm::radians(zAngle), glm::vec3(0.0f, 0.0f, 1.0f));
}