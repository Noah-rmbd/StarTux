#ifndef STARTUPSCREEN_H
#define STARTUPSCREEN_H

#include <glm/glm.hpp>
#include "interface.h"
#include "texture.h"
#include "shader.h"
#include "shape.h"
#include "textured_sphere.h"
#include "node.h"
#include "shape_model.h"

class StartupScreen {
    public:
        StartupScreen(int width, int height);
        ~StartupScreen();
        void update();
        void mouse(int button, int action, double xpos, double ypos);
        bool isLaunched();
    
    private:
        Interface *start_interface;
        Texture *logo_image;
        Node *background_space;
        int windowWidth;
        int windowHeight;
        float angle = 0.0f;
        bool click_valid = false;
        bool start_game = false;
};

#endif