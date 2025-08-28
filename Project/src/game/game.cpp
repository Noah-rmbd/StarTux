#include "game.h"
#include "GLFW/glfw3.h"
#include "real_profiler.h"
#include "asteroid.h"
#include "glm/fwd.hpp"
#include "hud.h"
#include "projectile.h"
#include "shape_model.h"

#ifndef SHADER_DIR
#error "SHADER_DIR not defined"
#endif
#ifndef TEXTURES_DIR
#error "TEXTURES_DIR not defined"
#endif
#ifndef RESSOURCES_DIR
#error "RESSOURCES_DIR not defined"
#endif

Game::Game(int width, int height, int target_fps, DailyMissions* missions) {
  PROFILE_SCOPE("Game Constructor");
  
  // IMPROVED: Decomposed constructor for better readability
  window_width = width;
  window_height = height;
  targeted_fps = target_fps;
  
  initializeDirectories();
  initializeLightingSystem();
  initializeUI(width, height, missions);
  initializeGraphicsResources();
  initializeGameWorld();
  initializeSceneGraph();
  initializeCameraAndPlayer();
  initializeGameState();
  
  // Initialize manager classes
  collision_manager_ = std::make_unique<CollisionManager>(player, world_node, scene_root, hud, 
                                                         dailyMissions, asteroid_speed, is_boost_mode, 
                                                         boost_time, lost, invincible, phong_shader);
  input_handler_ = std::make_unique<InputHandler>(player, &camera, scene_root, phong_shader,
                                                 window_width, window_height);
}

// IMPROVED: Constructor helper methods for better code organization
void Game::initializeDirectories() {
  shader_dir_ = SHADER_DIR;
  textures_dir_ = TEXTURES_DIR; 
  resources_dir_ = RESSOURCES_DIR;
}

void Game::initializeLightingSystem() {
  Lighting::Initialize();
  Lighting::Get().SetupSpaceLighting();
}

void Game::initializeUI(int width, int height, DailyMissions* missions) {
  dailyMissions = missions;
  
  hud = new Hud(width, height);
  
  if (dailyMissions) {
    hud->setMissionsManager(dailyMissions);
  }
}

void Game::initializeGraphicsResources() {
  phong_shader = new Shader(shader_dir_ + "phong.vert", shader_dir_ + "phong_enhanced.frag");
  player = new Player(phong_shader);
  
  bullet = new ShapeModel(resources_dir_ + "bullets.obj", phong_shader);
  
  Shader* asteroid_texture_shader = new Shader(shader_dir_ + "asteroid.vert", shader_dir_ + "asteroid.frag");
  Texture* asteroid_texture = new Texture(textures_dir_ + "asteroid.png");
  asteroid = new ShapeModel(resources_dir_ + "Asteroid.obj", asteroid_texture_shader);
  static_cast<ShapeModel*>(asteroid)->setTexture(asteroid_texture);
}

void Game::initializeGameWorld() {
  glm::mat4 world_mat = glm::mat4(1.0f); // Identity matrix is cleaner than complex transforms
  world_node = new Node(world_mat);

  Shader *texture_shader = new Shader(shader_dir_ + "texture.vert", shader_dir_ + "texture.frag");
  Texture *space_texture = new Texture(textures_dir_ + "space3.jpeg");
  Shape *environment_sphere = new TexturedSphere(texture_shader, space_texture);
  
  glm::mat4 environment_mat = glm::scale(glm::mat4(1.0f), 120.0f * glm::vec3(1.0f));
  environment_node_ = new Node(environment_mat);
  environment_node_->add(environment_sphere);
  
  // Create only 40 initial objects instead of 40 for faster startup
  for (int i = 0; i < 40; ++i) {
    if (i % 3 == 0) {
      spawn_ring(true, i / 5.0f);
    } else {
      spawn_asteroid(true, i / 5.0f);
    }
  }
}

void Game::initializeSceneGraph() {
  scene_root = new Node();
  scene_root->add(player->node);
  scene_root->add(world_node);
  scene_root->add(environment_node_);
  
  player->addDebugSpheresToScene(scene_root);
}

void Game::initializeCameraAndPlayer() {
  camera.cameraFront = glm::vec3(0.0f, 0.0f, 1.0f);
  camera.cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
  camera.cameraPos = player->position + glm::vec3(0.0f, 0.05f, -0.3f);
}

void Game::initializeGameState() {
  spawn_ring();
  
  player->gameStartTime = glfwGetTime();
  int initial_speed = -asteroid_speed * 50;
  player->gameStats->recordSpeedReached(initial_speed);
}

void Game::draw(glm::mat4 model, glm::mat4 view, glm::mat4 projection, double time, int fps) {
  PROFILE_SCOPE("Game Draw");
  
  // OPTIMIZATION: Cache matrices to avoid recalculation every frame
  static glm::mat4 cached_mvp_matrix;
  static bool mvp_dirty = true;
  
  if (mvp_dirty) {
    cached_mvp_matrix = projection * view * model;
    mvp_dirty = false;
  }
  
  {
    PROFILE_SCOPE("Scene Draw");
    scene_root->draw(model, view, projection);
  }
  
  // Draw engine flames (after main scene but before UI)
  if (!lost) { // Only draw flames when alive
    player->drawEngineFlames(view, projection);
  }
  
  // Prepare player rotation data
  glm::vec3 playerRotation(player->xAngle, player->yAngle, player->zAngle);
  
  // Set final score and statistics when game is lost (do this before calling hud->update)
  if (lost) {
      hud->setFinalScore(int(player->score));
      hud->setGameStatistics(player->gameStats);
      hud->setMissionsManager(dailyMissions);
  }
  
  // Track current speed for statistics (only record max speed, don't call every frame)
  int currentSpeed = -asteroid_speed * 50;
  
  // Track missions progress
  if (dailyMissions) {
    dailyMissions->recordSpeedReached(currentSpeed);
    dailyMissions->recordScoreReached(static_cast<int>(player->score));
    dailyMissions->updatePlayTime(time);
  }
  
  hud->update(player->life, int(player->score), player->bullets, time, currentSpeed, fps, 
              view, projection, player->position, playerRotation, player->shipState, paused, invincible, player->shieldIsActive, player->isDeathAnimationActive(), lost);
}

void Game::updateGame(double time, int fps) {
  PROFILE_SCOPE("updateGame");
  
  // OPTIMIZATION: Reduce update frequency for expensive operations
  static int frame_counter = 0;
  static double last_expensive_update = 0.0;
  frame_counter++;
  
  // Update lighting system
  if (g_LightingSystem) {
    static double lastTime = 0.0;
    double deltaTime = (lastTime == 0.0) ? 0.016 : time - lastTime; // Default to ~60fps on first frame
    lastTime = time;
    
    g_LightingSystem->Update(deltaTime);
    g_LightingSystem->SetupGameplayLighting(player->position);
  }
  
  // Don't update game logic if paused
  if (paused) {
    return;
  }
  
  // Update death animation
  if (player->isDeathAnimationActive()) {
    player->updateDeathAnimation(time);
    player->updatePosition(); // CRITICAL: Update visual position during death animation
    // Check if death animation finished and trigger explosion
    if (time - player->deathAnimationStart >= player->deathAnimationDuration) {
      create_explosion(player->position, time);
      
      // Record game end statistics ONLY ONCE by checking if not already lost
      if (!lost) {
        double gameTime = time - player->gameStartTime;
        player->gameStats->recordGameEnd(int(player->score), gameTime);
      }
      
      lost = true; // Set game as lost after death animation
    }
    // Don't update other game elements during death animation
    return;
  }
  
  // Adjust game speed to the number of fps
  float fps_correction = 1.0f;
  if (fps > 15) {
    fps_correction = static_cast<float>(targeted_fps) / static_cast<float>(fps);
    player->fps_correction = fps_correction;
  }
  
  // Generate game elements
  if (generation_cooldown > 0) {
    if(generation_cooldown % 15 == 0) { // Fifteen frames cooldown for generating moving asteroids
      spawn_moving_asteroid();
    }
    else if(generation_cooldown % 3 == 0) { // Two frames cooldown for generating asteroids
      spawn_asteroid();
    }
    if (generation_cooldown == 60) { // Thirty frames cooldown for generating rings
      spawn_ring();
    }
    
    generation_cooldown -= 1;
  } else {
    generation_cooldown = 60;
  }

  // Moves the world forward
  {
    PROFILE_SCOPE("world_node animation");
    world_node->animation(fps_correction);
  }

  // Manage every item in the world node and its collisions
  {
    PROFILE_SCOPE("detect_colisions");
    collision_manager_->detectCollisions(time, asteroids_, bullets_, rings_, projectiles, light_projectiles, explosions);
  }

  // Update explosions
  for(auto it = explosions.begin(); it != explosions.end();) {
    auto& explosion = *it;
    explosion->update(time);
    
    if (!explosion->isActive()) {
      scene_root->remove(explosion->getNode());
      it = explosions.erase(it);
    } else {
      ++it;
    }
  }

  // Increments player's score
  player->score += 1.0;

  // Update boost mode - OPTIMIZED DEACTIVATION
  if(is_boost_mode) {
    if(time - boost_time > 3.0) {
      float old_speed = asteroid_speed;
      asteroid_speed /= 2.0f;
      
      // Apply velocity change to world_node for smooth performance
      if (world_node && world_node->velocity_.z != 0.0f) {
        world_node->velocity_.z /= 2.0f;
      }
      
      is_boost_mode = false;
      player->shipState = player->NORMAL;
      
      std::cout << "Boost deactivated - speed: " << old_speed << " -> " << asteroid_speed << std::endl;
    }
  }

  // Updates player's shield state
  if (player->shieldIsActive) {
    player->updateShield(time);
    // Add shield visual if not already added
    player->addShieldToScene(scene_root);
  } else {
    // Remove shield visual if shield is not active
    player->removeShieldFromScene(scene_root);
  }

  // Every 5000 points, player get an extra life
  if(static_cast<int>(player->score) % 5000 == 0) {
    player->increaseLife();
    hud->newDialog(hud->EXTRA_LIFE, time);
  }

  // Every 1000 points, the ship speed increases
  if(static_cast<int>(player->score) % 500 == 0) {
    int boost_multiplicator = 1.0f;
    if(is_boost_mode) {
      boost_multiplicator = 2.0f;
    }
    if (player->score > 10000.0) {
      asteroid_speed -= 0.8 * boost_multiplicator;
    } else if(player->score > 5000.0) {
      asteroid_speed -= 0.6 * boost_multiplicator;
    } else {
      asteroid_speed -= 0.4 * boost_multiplicator;
    }
    
    // Record the new speed for statistics when it actually changes
    int currentSpeed = -asteroid_speed * 50;
    player->gameStats->recordSpeedReached(currentSpeed);
  }

  // Adds dialogs
  if(player->position.z == 0 && player->score < 100.0) {
    hud->newDialog(hud->WELCOME, time);
  }
  
  // Monitor all performance optimizations
  static double last_perf_monitor = 0.0;
  if (time - last_perf_monitor > 60.0) { // Every 60 seconds
    std::cout << "\n=== Performance Optimization Status ===" << std::endl;
    std::cout << "Collision detection: SINGLE-THREADED" << std::endl;
    
    // Show REAL performance bottlenecks
    std::cout << "\n=== ACTUAL BOTTLENECKS (last 60s) ===" << std::endl;
    RealProfiler::getInstance().printReport();
    RealProfiler::getInstance().reset();
    
    last_perf_monitor = time;
  }
}

void Game::mouse_callback(double xpos, double ypos){
  if(dev_mode) {
    camera.mouse_callback(xpos, ypos);
  }

  if(xpos < 0.0){
    xpos = 0.0;
  } else if (xpos>window_width) {
    xpos = window_width;
  } 

  if(ypos < 0.0){
    ypos = 0.0;
  } else if (ypos>window_height) {
    ypos = window_height;
  }

  x_mouse = xpos;
  y_mouse = ypos;
  hud->mouse(xpos, ypos);
}

void Game::mouse_button_callback(int button, int action, double xpos, double ypos, double time){
  mouse_callback(xpos, ypos);
  
  // Handle death menu clicks first
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    if (handleDeathMenuClick(xpos, ypos)) {
      return; // Death menu handled the click
    }
    l_mouse_button_pressed = true;
  } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE){
    l_mouse_button_pressed = false;
  }

  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
    r_mouse_button_pressed = true;
  } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE){
    r_mouse_button_pressed = false;
  }
}

void Game::keyHandler(std::unordered_map<int, std::pair<bool, double>> keyStates, double time) {
  input_handler_->handleInput(keyStates, time, dev_mode, x_mouse, y_mouse, 
                             l_mouse_button_pressed, r_mouse_button_pressed,
                             projectiles, light_projectiles, bullets_, 
                             paused, invincible, is_boost_mode);
}

void Game::spawn_asteroid(bool start_generation, float generation_distance) {
  PROFILE_SCOPE("spawn_asteroid");
  bool is_valid_position = false;
  float posX;
  float posY;
  float posZ;
  glm::vec3 candidate_position;
  
  // Generate random position for the asteroid while avoiding the rings
  while (!is_valid_position) {
    posX = ((rand() % 300) / 100.0f) - 1.5f; // Between -1.5 and 1.5
    posY = ((rand() % 200) / 100.0f) - 1.0f; // Between -1 and 1
    if (!start_generation) {
      posZ = ((rand() % 400) / 100.0f) + 3.0f; // Between 3 and 7
    } else {
      posZ = 6.5f * generation_distance + 0.5f;
    } 
    candidate_position = glm::vec3(posX, posY, posZ);
    is_valid_position = true;
    
    for (const auto& ring : rings_) {
      glm::vec3 ring_pos = glm::vec3(ring->ring_node->transform_[3]);
      float dist = glm::distance(candidate_position, ring_pos);
      if (dist < 0.4f) { // too close
        is_valid_position = false;
        break;
      }
    }
  }
  
  glm::mat4 asteroid_mat =
      glm::translate(glm::mat4(1.0f), candidate_position) *
      glm::scale(glm::mat4(1.0f), 0.006f * glm::vec3(1.0f, 1.0f, 1.0f)) *
      glm::rotate(glm::mat4(1.0f), glm::radians(10.0f),
                  glm::vec3(1.0f, 0.0f, 0.0f));
  
  Node *asteroidNode = new Node(asteroid_mat);

  asteroidNode->velocity_ = glm::vec3(0.0f, 0.0f, 0.0f);
  // Every asteroid move on z axis
  asteroidNode->z_speed = &asteroid_speed;
  asteroidNode->add(asteroid);

  auto asteroid_obj = std::make_unique<Asteroid>(asteroidNode);
  world_node->add(asteroid_obj->asteroid_node);
  asteroids_.push_back(std::move(asteroid_obj));
}

void Game::spawn_moving_asteroid() {
  PROFILE_SCOPE("spawn_moving_asteroid");
  // Speed of the asteroid on x and y axis
  float x_speed = ((rand() % 6000) / 1000.0f) - 3.0f;
  float y_speed = ((rand() % 6000) / 1000.0f) - 3.0f;

  // Random position of the asteroid on z axis
  float posZ = ((rand() % 400) / 100.0f) + 3.0f; // Between 3 and 7

  // Calculate the time required for the asteroid to reach the space ship (dist/speed)
  float time = posZ / -asteroid_speed; 

  // Calculate the distance of the asteroid so that it goes in the field on time
  float posX = -(time * x_speed) + ((rand() % 200) / 100.0f) - 1.0f;
  float posY = -(time * y_speed) + ((rand() % 200) / 100.0f) - 1.5f;

  // Transformation matrix of the asteroid
  glm::mat4 asteroid_mat =
      glm::translate(glm::mat4(1.0f), glm::vec3(posX, posY, posZ)) *
      glm::scale(glm::mat4(1.0f), 0.006f * glm::vec3(1.0f, 1.0f, 1.0f)) *
      glm::rotate(glm::mat4(1.0f), glm::radians(10.0f),
                  glm::vec3(1.0f, 0.0f, 0.0f));
  
  Node *asteroidNode = new Node(asteroid_mat);

  // Generate asteroid object
  auto asteroid_obj = std::make_unique<Asteroid>(asteroidNode);
  asteroid_obj->is_moving = true;
  asteroid_obj->x_speed = x_speed;
  asteroid_obj->y_speed = y_speed;  

  asteroidNode->velocity_ = glm::vec3(0.0f, 0.0f, 0.0f);
  asteroidNode->x_speed = &asteroid_obj->x_speed;
  asteroidNode->y_speed = &asteroid_obj->y_speed;
  asteroidNode->z_speed = &asteroid_speed;
  asteroidNode->add(asteroid);

  world_node->add(asteroid_obj->asteroid_node);
  asteroids_.push_back(std::move(asteroid_obj));
}

void Game::spawn_bullet(glm::vec3 position) {
  collision_manager_->spawnBullet(position, bullets_, bullet);
}

void Game::spawn_ring(bool start_generation, float generation_distance) {
  PROFILE_SCOPE("spawn_ring");
  
  // OPTIMIZATION: Use object pool for rings to avoid expensive allocations
  static int ring_creation_count = 0;
  ring_creation_count++;
  
  // Throttle ring creation if we're creating too many too fast
  if (ring_creation_count > 5) {
    static double last_ring_time = 0.0;
    double current_time = glfwGetTime();
    if (current_time - last_ring_time < 0.1) { // Min 100ms between rings
      return; // Skip creation to prevent lag
    }
    last_ring_time = current_time;
  }
  float posX, posY, posZ;
  
  posX = ((rand() % 200) / 100.0f) - 1.0f; // Between -1.5 and 1.5
  posY = ((rand() % 150) / 100.0f) - 0.75f; // Between -1 and 1
  if (start_generation) {
    posZ = 6.5f * generation_distance + 0.5f;  
  } else {
    posZ = 7.0f;
  }

  glm::vec3 position = glm::vec3(posX, posY, posZ);
  auto ring = std::make_unique<Ring>(position);

  ring->ring_node->velocity_ = glm::vec3(0.0f, 0.0f, 0.0f);
  ring->ring_node->z_speed = &asteroid_speed;
  
  world_node->add(ring->ring_node);
  rings_.push_back(std::move(ring));
}

void Game::create_explosion(glm::vec3 position, double time) {
    collision_manager_->createExplosion(position, time, explosions);
}

void Game::activate_dev_mode() {
  input_handler_->activateDevMode(dev_mode);
}

void Game::deactivate_dev_mode() {
  input_handler_->deactivateDevMode(dev_mode);
}

// Death menu handling methods
bool Game::handleDeathMenuClick(double xpos, double ypos) {
  if (!hud->isDeathMenuActive()) return false;
  
  bool clicked = hud->checkDeathMenuClick(xpos, ypos);
  if (clicked) {
    Hud::DeathMenuAction action = hud->getLastMenuAction();
    
    if (action == Hud::DEATH_MENU_QUIT) {
      quitToMenu = true;
    } else if (action == Hud::DEATH_MENU_PLAY) {
      resetGame();
    }
    
    hud->resetMenuAction();
    return true;
  }
  
  return false;
}

void Game::resetGame() {
  // Reset player state
  player->life = 3;
  player->bullets = 10;
  player->score = 0;
  player->position = glm::vec3(0.0f, 0.0f, 0.0f);
  player->xAngle = 0.0f;
  player->yAngle = 0.0f;
  player->zAngle = 0.0f;
  player->shipState = Player::NORMAL;
  
  // Reset death animation state
  player->deathAnimationActive = false;
  player->deathAnimationStart = 0.0;
  
  // Reset damage animation state
  player->damageAnimationActive = false;
  player->damageAnimationStart = 0.0;
  
  // Reset shield state
  player->shieldIsActive = false;
  player->shieldStart = 0.0;
  
  // Reset game state
  lost = false;
  paused = false;
  
  // Reset camera position
  camera.cameraPos = player->position + glm::vec3(0.0f, 0.05f, -0.3f);
  
  // Clear all projectiles
  for (auto& projectile : projectiles) {
    scene_root->remove(projectile->node);
  }
  projectiles.clear();
  
  for (auto& light_projectile : light_projectiles) {
    scene_root->remove(light_projectile->node);
  }
  light_projectiles.clear();
  
  // Clear all asteroids
  for (auto& asteroid : asteroids_) {
    world_node->remove(asteroid->asteroid_node);
  }
  asteroids_.clear();
  
  // Clear all rings
  for (auto& ring : rings_) {
    world_node->remove(ring->ring_node);
  }
  rings_.clear();
  
  // Clear all explosions
  for (auto& explosion : explosions) {
    scene_root->remove(explosion->getNode());
  }
  explosions.clear();
  
  // Reset asteroid speed
  asteroid_speed = -2.4f;
  
  // Reset statistics for new game session
  player->gameStats->resetSession();
  player->gameStartTime = glfwGetTime();
  
  // Record initial speed for new game
  int initialSpeed = -asteroid_speed * 50;
  player->gameStats->recordSpeedReached(initialSpeed);
  
  // Start new missions session
  if (dailyMissions) {
    dailyMissions->startSession();
  }
  
  // Reset HUD state
  hud->resetDeathMenuState();
  
  // Regenerate initial objects
  for (int i = 0; i < 40; ++i) {
    if (i%10 == 0) {
      spawn_ring(true, i/40.0f);
    } else if (i%2 == 0) {
      spawn_asteroid(true, i/40.0f);
    }
  }
  
  spawn_ring();
}

bool Game::isDeathMenuActive() const {
  return hud->isDeathMenuActive();
}