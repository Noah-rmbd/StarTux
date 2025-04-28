#ifndef GAME_H
#define GAME_H

#include "lighting_sphere.h"
#include "node.h"
#include "player.h"
#include "shader.h"
#include "shape_model.h"
#include "texture.h"
#include "textured_sphere.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <unordered_map>

class Game {
public:
  Game();
  void updateGame();
  void keyHandler(std::unordered_map<int, std::pair<bool, double>> keyStates,
                  double time);
  void spawn_rectangle();

  Player *player;
  Node *scene_root;
  Node *world_node;
  float distance;
  bool lost = false;
  

private:
  Shader *phong_shader;
  bool is_rotating = false;
  bool idle_rot = false;
  bool idle_lr =
      false; // stores if the idle animation for left/right is running
  bool idle_ud = false; // stores if the idle animation for up/down is running
  double idle_start_lr =
      0.0; // stores the timestamp of the beginning of the idle animation
  double idle_start_ud = 0.0;
  double idle_start_rot = 0.0;
  // float rot_start_angle = 0.0f;

  std::vector<Node *> asteorides_;
  const size_t max_asteorides_ = 30;
  int latence = 0;
};

#endif
