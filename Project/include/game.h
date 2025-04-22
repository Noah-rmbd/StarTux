#ifndef GAME_H
#define GAME_H

#include <unordered_map>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "player.h"
#include "shader.h"
#include "node.h"
#include "textured_sphere.h"
#include "lighting_sphere.h"
#include "texture.h"
#include "shape_model.h"

class Game{
    public:
        Game();
        void updateGame();
        void keyHandler(std::unordered_map<int, std::pair<bool, double>> keyStates, double time);
        Player* player;
        Node *scene_root;
        Node *world_node;
        float distance;
        
    private:
        std::string shader_dir = "/Users/noah-r/Coding/TP4/TP4_material/shaders/";
        Shader *phong_shader;
        bool is_rotating = false;
        bool idle_rot = false;
        bool idle_lr = false;       // stores if the idle animation for left/right is running
        bool idle_ud = false;       // stores if the idle animation for up/down is running
        double idle_start_lr = 0.0; // stores the timestamp of the beginning of the idle animation
        double idle_start_ud = 0.0;
        double idle_start_rot = 0.0;
        //float rot_start_angle = 0.0f;
};

#endif