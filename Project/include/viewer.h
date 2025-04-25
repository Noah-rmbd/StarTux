#ifndef VIEWER_H
#define VIEWER_H

#include <map>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <chrono>
#include <thread>

// #include <ft2build.h>
// #include FT_FREETYPE_H

#include "camera.h"
#include "game.h"
#include "node.h"
#include "shader.h"

// Structure to hold character information
/*struct Character {
    unsigned int TextureID;  // ID handle of the glyph texture
    glm::ivec2   Size;       // Size of glyph
    glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
    unsigned int Advance;    // Offset to advance to next glyph
};*/

class Viewer {
public:
  Viewer(int width = 1280, int height = 960);

  void run();
  void on_key(int key, int action);

  Node *scene_root;
  Node *world_node;
  Camera camera;
  Game *game;
  void spawn_rectangle();

private:
  GLFWwindow *win;

  // void renderText(std::string text, float x, float y, float scale, glm::vec3
  // color); void initFreeType();

  // UI-related members
  /*FT_Library ft;
  FT_Face face;
  std::map<char, Character> Characters;
  unsigned int VAO, VBO;
  Shader textShader;*/

  // Store key states
  std::unordered_map<int, std::pair<bool, double>> keyStates;
  static void key_callback_static(GLFWwindow *window, int key, int scancode,
                                  int action, int mods);
  static void mouse_callback_static(GLFWwindow *window, double xpos,
                                    double ypos);
  void keyboard_events();
  std::vector<Node *> asteorides_;
  const size_t max_asteorides = 30;

  ShapeModel *asteroid_template = nullptr;
};

#endif // VIEWER_H
