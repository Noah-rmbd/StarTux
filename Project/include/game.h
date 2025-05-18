#ifndef GAME_H
#define GAME_H

#include "hud.h"
#include "node.h"
#include "player.h"
#include "shader.h"
#include "camera.h"
#include "projectile.h"
#include "texture.h"
#include "textured_sphere.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <vector>

class Game {
public:
  Game(int width, int height, int target_fps);
  void updateGame(double time, int fps);
  void draw(glm::mat4 model, glm::mat4 view, glm::mat4 projection, double time, int fps);
  void keyHandler(std::unordered_map<int, std::pair<bool, double>> keyStates,
                  double time);
  void mouse_callback(double xpos, double ypos);
  void mouse_button_callback(int button, int action, double xpos, double ypos, double time);
  
  Player *player;
  Camera camera;
  Node *scene_root;
  Node *world_node;
  bool lost = false;
  

private:
  void spawn_rectangle();
  Shader *phong_shader;
  bool dev_mode = false;
  bool is_rotating = false;
  bool idle_rot = false;
  bool idle_lr =
      false; // stores if the idle animation for left/right is running
  bool idle_ud = false; // stores if the idle animation for up/down is running
  double idle_start_lr =
      0.0; // stores the timestamp of the beginning of the idle animation
  double idle_start_ud = 0.0;
  double idle_start_rot = 0.0;

  double x_mouse;
  double y_mouse;
  bool l_mouse_button_pressed = false;

  Hud *hud;
  int window_width;
  int window_height;
  int targeted_fps;

  // List of active projectiles in the game
  std::vector<Projectile *> projectiles;
  Node *projectile_node;
  double last_shoot_time;

  // Asteroids elements
  Shape *asteroid;
  std::vector<Node *> asteorides_;
  const size_t max_asteorides_ = 30;
  float asteroid_speed = -2.4f;
  int latence = 0;
};

#endif
