#pragma once

#include "hud.h"
#include "node.h"
#include "player.h"
#include "shader.h"
#include "camera.h"
#include "light_projectile.h"
#include "projectile.h"
#include "asteroid.h"
#include "ring.h"
#include "texture.h"
#include "textured_sphere.h"
#include "explosion.h"
#include "missions.h"
#include "lighting.h"
// Removed unused optimization includes
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <vector>
#include <memory>  // For smart pointers
// Removed unused threading includes

class Game {
public:
  Game(int width, int height, int target_fps, DailyMissions* missions = nullptr);
  void updateGame(double time, int fps);
  void draw(glm::mat4 model, glm::mat4 view, glm::mat4 projection, double time, int fps);
  void keyHandler(std::unordered_map<int, std::pair<bool, double>> keyStates,
                  double time);
  void mouse_callback(double xpos, double ypos);
  void mouse_button_callback(int button, int action, double xpos, double ypos, double time);
  
  // Death menu handling
  bool handleDeathMenuClick(double xpos, double ypos);
  void resetGame();
  bool shouldQuitToMenu() const { return quitToMenu; }
  void resetQuitFlag() { quitToMenu = false; }
  bool isDeathMenuActive() const;
  
  Player *player;
  Camera camera;
  Node *scene_root;
  Node *world_node;
  bool lost = false;
  bool paused = false;
  bool invincible = false;
  bool quitToMenu = false;

private:
  void spawn_asteroid(bool start_generation = false, float generation_distance = 0.0f);
  void spawn_moving_asteroid();
  void spawn_bullet(glm::vec3 position);
  void spawn_ring(bool start_generation = false, float generation_distance = 0.0f);
  void create_explosion(glm::vec3 position, double time);
  void detect_colisions(double time);
  void colisions_between_asteroids(double time);
  void colisions_player_asteroids(double time);
  void colisions_player_asteroids_optimized(double time);
  void colisions_player_bullet(double time);
  void colisions_player_ring(double time);
  void colisions_lprojectile_asteroid(double time);
  void colisions_lprojectile_asteroid_optimized(double time);
  void colisions_projectile_asteroid(double time);
  void colisions_projectile_asteroid_optimized(double time);

  Shader *phong_shader;
  bool dev_mode = false;
  bool is_rotating = false;
  bool idle_rot = false;
  bool idle_lr = false; // stores if the idle animation for left/right is running
  bool idle_ud = false; // stores if the idle animation for up/down is running
  double idle_start_lr = 0.0; // stores the timestamp of the beginning of the idle animation
  double idle_start_ud = 0.0;
  double idle_start_rot = 0.0;

  double x_mouse;
  double y_mouse;
  bool l_mouse_button_pressed = false;
  bool r_mouse_button_pressed = false;

  Hud *hud;
  int window_width;
  int window_height;
  int targeted_fps;

  // Functions related to the dev mode
  void activate_dev_mode();
  void deactivate_dev_mode();

  // List of active projectiles in the game
  std::vector<std::unique_ptr<Projectile>> projectiles;
  Node *projectile_node;
  double last_shoot_time;

  // List of light active projectiles in the game
  std::vector<std::unique_ptr<LightProjectile>> light_projectiles;
  Node *projectile_l_node;
  double last_shoot_time_l;

  // Asteroids elements
  Shape *asteroid;
  std::vector<std::unique_ptr<Asteroid>> asteroids_;
  const size_t max_asteroids_ = 30;
  float asteroid_speed = -2.4f;
  int generation_cooldown = 0;

  // Bullet elements
  Shape *bullet;
  std::vector<std::unique_ptr<Node>> bullets_;

  // Ring elements
  Ring *ring;
  std::vector<std::unique_ptr<Ring>> rings_;

  // Explosion effects
  std::vector<std::unique_ptr<Explosion>> explosions;

private:
  // Removed unused optimization systems

  // Boost mode
  bool is_boost_mode = false;
  double boost_time;
  
  // Pause and developer features
  bool pause_key_pressed = false;
  bool invincible_key_pressed = false;
  bool debug_key_pressed = false;
  
  // Missions system
  DailyMissions* dailyMissions = nullptr;
};