#ifndef VIEWER_H
#define VIEWER_H

#include <map>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <chrono>
#include <thread>


#include "shader.h"
#include "node.h"
#include "camera.h"
#include "game.h"
#include "texture.h"
#include "interface.h"
#include "startup_screen.h"
#include "hud.h"



class Viewer {
public:
    Viewer(int width=1280, int height=960);
    ~Viewer();
    void run();
    void on_key(int key, int action);

    float windowWidth;
    float windowHeight;

    Game *game;
    StartupScreen *startup_screen;

private:
    GLFWwindow *win;
    Texture* startScreenImage;
    
    bool startGame = false;
    int target_FPS = 30;

    // Store key states
    std::unordered_map<int, std::pair<bool, double>> keyStates;
    // Input handlers
    static void key_callback_static(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouse_callback_static(GLFWwindow* window, double xpos, double ypos);
    static void mouse_button_callback_static(GLFWwindow* window, int button, int action, int mods);
    void on_mouse_button(int button, int action, double xpos, double ypos);
    void keyboard_events();
};

#endif // VIEWER_H
