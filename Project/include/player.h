#ifndef PLAYER_H
#define PLAYER_H

#include "node.h"
#include "shader.h"
#include "shape_model.h"
#include <glm/glm.hpp>

class Player{
    public :
        Player(Shader* shader_program);
        void updatePosition();

        Shape* model;

        glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 scale = glm::vec3(0.005f, 0.005f, 0.005f);
        glm::mat4 playerMat;
        Node* node;

        float xAngle = 0.0f;
        float yAngle = 0.0f;
        float zAngle = 0.0f;

    private:
        std::string ship_dir;
}; 

#endif