#ifndef VIEWER_H
#define VIEWER_H

#include <vector>
#include <string>
#include <map>

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

class Viewer {
public:
    Viewer(int width=1280, int height=960);
    ~Viewer();
    void run();
    void on_key(int key, int action);

    float windowWidth;
    float windowHeight;

    Node *scene_root;
    Camera camera;
    Game *game;
    //Interface *interface;
    StartupScreen *startup_screen;

private:
    GLFWwindow* win;

    bool startGame = false;
    Texture* startScreenImage;

    // Store key states
    std::unordered_map<int, std::pair<bool, double>> keyStates;
    static void key_callback_static(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouse_callback_static(GLFWwindow* window, double xpos, double ypos);
    static void mouse_button_callback_static(GLFWwindow* window, int button, int action, int mods);
    void keyboard_events();
    void on_mouse_button(int button, int action, double xpos, double ypos);
};

#endif // VIEWER_H
