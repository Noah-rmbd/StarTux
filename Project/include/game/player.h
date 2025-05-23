#ifndef PLAYER_H
#define PLAYER_H

#include "node.h"
#include "shader.h"
#include "texture.h"
#include "shape_model.h"
#include <glm/glm.hpp>

class Player{
    public :
        Player(Shader* shader_program);
        ~Player();
        void updatePosition();
        void increaseLife();
        void increaseBullets();

        Shape* model;
        Shader* texture_shader;
        Texture* ship_texture;

        glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 scale = glm::vec3(0.005f, 0.005f, 0.005f);
        glm::mat4 playerMat;
        Node* node;

        float xAngle = 0.0f;
        float yAngle = 0.0f;
        float zAngle = 0.0f;
        float movement_speed = 0.04f;
        float fps_correction = 1.0f;

        int life = 3;
        int bullets = 10;
        double score = 0;

    private:
        std::string ship_dir;
}; 

#endif