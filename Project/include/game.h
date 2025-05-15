#ifndef GAME_H
#define GAME_H

#include "hud.h"
#include "node.h"
#include "player.h"
#include "shader.h"
#include "camera.h"
#include "texture.h"
#include "textured_sphere.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <unordered_map>

class Game {
public:
  Game(int width, int height);
  void updateGame(double time);
  
  void draw(glm::mat4 model, glm::mat4 view, glm::mat4 projection);
  void keyHandler(std::unordered_map<int, std::pair<bool, double>> keyStates,
                  double time);
  void mouse_callback(double xpos, double ypos);
  
  Player *player;
  Camera camera;
  Node *scene_root;
  Node *world_node;
  bool lost = false;
  

private:
  void spawn_rectangle();
  void updateHud();
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

  Hud *hud;

  std::vector<Node *> asteorides_;
  const size_t max_asteorides_ = 30;
  int latence = 0;
};

#endif
