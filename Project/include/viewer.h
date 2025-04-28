#ifndef VIEWER_H
#define VIEWER_H

#include <map>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <chrono>
#include <thread>

//#include <ft2build.h>
//#include FT_FREETYPE_H

#include "camera.h"
#include "game.h"

// Structure to hold character information
/*struct Character {
    unsigned int TextureID;  // ID handle of the glyph texture
    glm::ivec2   Size;       // Size of glyph
    glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
    unsigned int Advance;    // Offset to advance to next glyph
};*/


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
    StartupScreen *startup_screen;

private:
  GLFWwindow *win;

    bool startGame = false;
    Texture* startScreenImage;
    std::vector<Node *> asteorides_;

    // Store key states
    std::unordered_map<int, std::pair<bool, double>> keyStates;
    static void key_callback_static(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouse_callback_static(GLFWwindow* window, double xpos, double ypos);
    static void mouse_button_callback_static(GLFWwindow* window, int button, int action, int mods);
    void on_mouse_button(int button, int action, double xpos, double ypos);
    void keyboard_events();
};

#endif // VIEWER_H
