#pragma once

#include <glm/glm.hpp>
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
#include "collision_manager.h"
#include "input_handler.h"
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

  Shader *phong_shader;
  bool dev_mode = false;

  double x_mouse;
  double y_mouse;
  bool l_mouse_button_pressed = false;
  bool r_mouse_button_pressed = false;
  
  // Manager classes
  std::unique_ptr<CollisionManager> collision_manager_;
  std::unique_ptr<InputHandler> input_handler_;

  Hud *hud;
  int window_width;
  int window_height;
  int targeted_fps;

  // Functions related to the dev mode
  void activate_dev_mode();
  void deactivate_dev_mode();
  
  // IMPROVED: Constructor helper methods (decomposed from monolithic constructor)
  void initializeDirectories();
  void initializeLightingSystem();
  void initializeUI(int width, int height, DailyMissions* missions);
  void initializeGraphicsResources();
  void initializeGameWorld();
  void initializeSceneGraph();
  void initializeCameraAndPlayer();
  void initializeGameState();
  

  // List of active projectiles in the game
  std::vector<std::unique_ptr<Projectile>> projectiles;
  Node *projectile_node;

  // List of light active projectiles in the game
  std::vector<std::unique_ptr<LightProjectile>> light_projectiles;
  Node *projectile_l_node;

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
  
  // Missions system
  DailyMissions* dailyMissions = nullptr;
  
  // Directory paths for resources
  std::string shader_dir_;
  std::string textures_dir_;
  std::string resources_dir_;
  
  // Environment node for scene graph
  Node* environment_node_;
};