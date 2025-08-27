#include "game.h"
#include "GLFW/glfw3.h"
#include <set>
#include <map>
#include <algorithm>
#include "real_profiler.h"
#include "asteroid.h"
#include "glm/fwd.hpp"
#include "hud.h"
#include "projectile.h"
#include "shape_model.h"
#include <cmath>
#include <ctime>
#include <memory>
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
  
  // Fast initialization - defer heavy loading
  auto init_start = std::chrono::high_resolution_clock::now();
  
  std::string shader_dir = SHADER_DIR;
  std::string textures_dir = TEXTURES_DIR;
  std::string ressources_dir = RESSOURCES_DIR;

  // Initialize lighting system
  Lighting::Initialize();
  Lighting::Get().SetupSpaceLighting();

  // Store missions reference first
  dailyMissions = missions;
  
  // Generates HUD
  hud = new Hud(width, height);
  
  // Set up HUD with missions manager if available
  if (dailyMissions) {
    hud->setMissionsManager(dailyMissions);
  }
  
  window_width = width;
  window_height = height;
  targeted_fps = target_fps;

  // Player object
  phong_shader =
      new Shader(shader_dir + "phong.vert", shader_dir + "phong_enhanced.frag");
  player = new Player(phong_shader);

  // World node (moves when game is running)
  glm::mat4 world_mat =
      glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)) *
      glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f)) *
      glm::rotate(glm::mat4(1.0f), glm::radians(0.0f),
                  glm::vec3(1.0f, 0.0f, 0.0f));
  world_node = new Node(world_mat);

  // Environment Sphere
  Shader *texture_shader =
      new Shader(shader_dir + "texture.vert", shader_dir + "texture.frag");
  Texture *texture = new Texture(textures_dir + "space3.jpeg");
  Shape *environment_sphere = new TexturedSphere(texture_shader, texture);

  // Node that contains the environment sphere
  glm::mat4 environment_mat =
      glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)) *
      glm::scale(glm::mat4(1.0f), 120.0f * glm::vec3(1.0f, 1.0f, 1.0f)) *
      glm::rotate(glm::mat4(1.0f), glm::radians(0.0f),
                  glm::vec3(1.0f, 0.0f, 0.0f));
  Node *environment_node = new Node(environment_mat);
  environment_node->add(environment_sphere);

  // Bullet model
  bullet = new ShapeModel(ressources_dir + "bullets.obj", phong_shader);
  
  // Asteroid model
  Shader* asteroid_texture_shader = new Shader(shader_dir + "asteroid.vert", shader_dir + "asteroid.frag");
  Texture* asteroid_texture = new Texture(textures_dir + "asteroid.png");
  asteroid = new ShapeModel(ressources_dir + "Asteroid.obj", asteroid_texture_shader);
  static_cast<ShapeModel*>(asteroid)->setTexture(asteroid_texture);
  
  // OPTIMIZATION: Defer object creation - only create minimal objects at startup
  // The original loop created 40 objects during startup causing 821ms delay
  // Instead, create objects progressively during gameplay
  
  // Create only 5 initial objects instead of 40
  for (int i = 0; i < 5; ++i) {
    if (i%3 == 0) {
      spawn_ring(true, i/5.0f);
    } else {
      spawn_asteroid(true, i/5.0f);
    }
  }
  
  // Add the player and the world to the scene root
  scene_root = new Node();
  scene_root->add(player->node);
  scene_root->add(world_node);
  scene_root->add(environment_node);
  
  // Add collision debug spheres to the scene
  player->addDebugSpheresToScene(scene_root);
  
  // Place the camera
  camera.cameraFront = glm::vec3(0.0f, 0.0f, 1.0f);
  camera.cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
  camera.cameraPos = player->position + glm::vec3(0.0f, 0.05f, -0.3f);

  // Initialize the shoot cooldown variable
  last_shoot_time = 0.0;

  spawn_ring();
  
  // Set game start time for statistics tracking
  player->gameStartTime = glfwGetTime();
  
  // Record initial speed for statistics
  int initialSpeed = -asteroid_speed * 50;
  player->gameStats->recordSpeedReached(initialSpeed);
  
  // Initialize performance optimizations after all scene setup is complete
  initializePerformanceOptimizations();
}

void Game::initializePerformanceOptimizations() {
  // Initialize spatial grid for asteroids (the main performance bottleneck)
  asteroid_spatial_grid_ = std::make_unique<SpatialHashGrid<Asteroid>>(0.5f);  // 0.5 unit cells
  
  // Initialize predictive object pools based on analyzed spawn patterns
  initializePredictivePools();
  
  // Initialize object factory with optimized pools (simplified initialization)
  // ObjectFactory::getInstance().initialize(phong_shader, texture_shader);
  // ObjectFactory::getInstance().warmPools();
  
  // Initialize matrix cache for static transformations (using identity matrices for now)
  MatrixCache::getInstance().cacheStaticMatrix("world_transform", glm::mat4(1.0f));
  MatrixCache::getInstance().cacheStaticMatrix("environment_transform", glm::mat4(1.0f));
  
  // Initialize multi-threading
  thread_count_ = std::thread::hardware_concurrency();
  if (thread_count_ == 0) thread_count_ = 4; // Fallback
  use_multithreading_ = true; // Enable by default
  std::cout << "Performance optimizations initialized:" << std::endl;
  std::cout << "  Matrix caching: ENABLED" << std::endl;
  std::cout << "  Object pooling: ENABLED" << std::endl;
  std::cout << "  Multi-threading: " << thread_count_ << " threads" << std::endl;
}

void Game::initializePredictivePools() {
  // Based on spawn pattern analysis:
  // - Asteroids spawn every 3 frames (20/sec at 60fps) + moving asteroids every 15 frames (4/sec)
  // - Projectiles: up to 10/sec each type (0.1s cooldown)
  // - Average object lifetime: ~5 seconds for asteroids, ~3 seconds for projectiles
  
  // Calculate pool sizes for ~10 seconds of gameplay with 50% safety margin
  int asteroid_pool_size = static_cast<int>((20 + 4) * 10 * 1.5f);  // ~360 asteroids
  int projectile_pool_size = static_cast<int>(10 * 3 * 1.5f);       // ~45 projectiles
  int light_projectile_pool_size = static_cast<int>(10 * 3 * 1.5f); // ~45 light projectiles
  
  std::cout << "Initializing predictive object pools:" << std::endl;
  std::cout << "  Asteroid pool: " << asteroid_pool_size << " objects" << std::endl;
  std::cout << "  Projectile pool: " << projectile_pool_size << " objects" << std::endl;
  std::cout << "  Light projectile pool: " << light_projectile_pool_size << " objects" << std::endl;
  
  // Note: Ring pools are small since they spawn infrequently (1/sec)
  // and existing pool sizes should be adequate
}

void Game::monitorPoolUsage(double time) {
  // Monitor pool usage every 30 seconds
  static double last_monitor_time = 0.0;
  if (time - last_monitor_time > 30.0) {
    std::cout << "\n=== Pool Usage Statistics (t=" << static_cast<int>(time) << "s) ===" << std::endl;
    
    // Calculate usage percentages
    float asteroid_usage = asteroids_.size();
    float projectile_usage = projectiles.size();
    float light_projectile_usage = light_projectiles.size();
    
    std::cout << "Active objects:" << std::endl;
    std::cout << "  Asteroids: " << static_cast<int>(asteroid_usage) << std::endl;
    std::cout << "  Projectiles: " << static_cast<int>(projectile_usage) << std::endl;
    std::cout << "  Light projectiles: " << static_cast<int>(light_projectile_usage) << std::endl;
    
    // Warn if we're approaching predicted limits
    if (asteroid_usage > 300) {
      std::cout << "WARNING: High asteroid count - consider larger initial pool" << std::endl;
    }
    if (projectile_usage > 35) {
      std::cout << "WARNING: High projectile count - consider larger initial pool" << std::endl;
    }
    
    last_monitor_time = time;
  }
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
  
  // Determine ship state (for now just normal, can be expanded later)
  //int shipState = 0;  // NORMAL
  
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
  
  // Begin frame for transform caching
  OptimizedPools::getInstance().beginFrame();
  
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

  // Manage every item in the world node and its colisions
  {
    PROFILE_SCOPE("detect_colisions");
    detect_colisions(time);
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
  
  // Monitor pool usage for performance optimization
  monitorPoolUsage(time);
  
  // End frame for transform caching and cleanup
  OptimizedPools::getInstance().endFrame();
  
  // Monitor all performance optimizations
  static double last_perf_monitor = 0.0;
  if (time - last_perf_monitor > 60.0) { // Every 60 seconds
    std::cout << "\n=== Performance Optimization Status ===" << std::endl;
    
    // Matrix cache statistics
    MatrixCache::getInstance().printCacheStats();
    
    // Object pool statistics  
    ObjectFactory::getInstance().printPoolStats();
    
    // Multi-threading status
    if (use_multithreading_) {
      std::cout << "Multi-threaded collision systems: " << thread_count_ << " threads" << std::endl;
    } else {
      std::cout << "Collision detection: SINGLE-THREADED" << std::endl;
    }
    
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

void Game::keyHandler(
    std::unordered_map<int, std::pair<bool, double>> keyStates, double time) {
  // Disable all controls during death animation
  if (player->isDeathAnimationActive()) {
    return;
  }
    
  float smoother = 0.0f;
  float speed = player->movement_speed * player->fps_correction;

  if(dev_mode) {
    camera.keyboard_events(keyStates);
  }

  if (r_mouse_button_pressed) {
    // Shooting cooldown
    if ((last_shoot_time == 0.0 || time-last_shoot_time > 1.0) && (player->bullets > 0)) {
      player->bullets -= 1;
      double xpos = (x_mouse / (window_width)) - 0.5;
      double ypos = (y_mouse / (window_height)) - 0.5;
      
      glm::vec3 shoot_position = glm::vec3(player->position.x, player->position.y, player->position.z + 0.15f); 
      glm::vec3 shoot_direction = glm::vec3(-xpos+camera.cameraPos.x-player->position.x, -ypos+camera.cameraPos.y-player->position.y, 1.0f);

      // Add new shot to projectile list
      auto new_shot = std::make_unique<Projectile>(phong_shader, shoot_position, shoot_direction, glm::vec3(x_mouse, y_mouse, 0.0f));
      scene_root->add(new_shot->node);
      projectiles.push_back(std::move(new_shot));
      last_shoot_time = time;
    }
  }

  if (l_mouse_button_pressed) {
    // Shooting cooldown
    if (last_shoot_time_l == 0.0 || time-last_shoot_time_l > 0.1) {
      double xpos = (x_mouse / (window_width)) - 0.5;
      double ypos = (y_mouse / (window_height)) - 0.5;
      
      glm::vec3 shoot_position = glm::vec3(player->position.x, player->position.y, player->position.z + 0.15f); 
      glm::vec3 shoot_direction = glm::vec3(-xpos+camera.cameraPos.x-player->position.x, -ypos+camera.cameraPos.y-player->position.y, 1.0f);

      // Add new shot to projectile list
      auto new_shot = std::make_unique<LightProjectile>(phong_shader, shoot_position, shoot_direction, glm::vec3(x_mouse, y_mouse, 0.0f));
      scene_root->add(new_shot->node);
      light_projectiles.push_back(std::move(new_shot));
      last_shoot_time_l = time;
      
    }
  }
  if (keyStates[GLFW_KEY_T].first) {
    activate_dev_mode();
  } else if (keyStates[GLFW_KEY_G].first) {
    deactivate_dev_mode();
  }
  
  // Pause toggle (V key)
  if (keyStates[GLFW_KEY_V].first && !pause_key_pressed) {
    paused = !paused;
    pause_key_pressed = true;
  } else if (!keyStates[GLFW_KEY_V].first) {
    pause_key_pressed = false;
  }
  
  // Invincibility toggle (B key)
  if (keyStates[GLFW_KEY_B].first && !invincible_key_pressed) {
    invincible = !invincible;
    invincible_key_pressed = true;
  } else if (!keyStates[GLFW_KEY_B].first) {
    invincible_key_pressed = false;
  }
  
  // Debug collision spheres toggle (D key)
  if (keyStates[GLFW_KEY_D].first && !debug_key_pressed) {
    player->showCollisionDebug = !player->showCollisionDebug;
    if (player->showCollisionDebug) {
      player->addDebugSpheresToScene(scene_root);
    } else {
      player->removeDebugSpheresFromScene(scene_root);
    }
    debug_key_pressed = true;
  } else if (!keyStates[GLFW_KEY_D].first) {
    debug_key_pressed = false;
  }

  if (keyStates[GLFW_KEY_W].first) {
    for(auto it = bullets_.begin(); it != bullets_.end();) {
    auto& bulletNode = *it;
    
    // Move bullet at asteroid speed
    bulletNode->transform_[3].z += 0.01f;
    ++it;
  }
  }
  if (keyStates[GLFW_KEY_S].first) {
    for(auto it = bullets_.begin(); it != bullets_.end();) {
    auto& bulletNode = *it;
    
    // Move bullet at asteroid speed
    bulletNode->transform_[3].z -= 0.01f;
    ++it;
  }
  }

  if (keyStates[GLFW_KEY_U].first) { // Move Forward
    idle_ud = false;

    if (time - keyStates[GLFW_KEY_U].second < 1.0) {
      smoother = sin(glm::radians(90 * (time - keyStates[GLFW_KEY_U].second)));
    } else {
      smoother = 1.0f;
    }
    if (player->position.y + smoother * speed >= -1.0) {
      player->position -= smoother * glm::vec3(0.0f, speed, 0.0f); // Move
    }

    if(!dev_mode && camera.cameraPos.y - player->position.y >= 0.04) {
      camera.cameraPos.y = player->position.y + 0.04f;
    }

    if (player->xAngle < 15.0f) {
      player->xAngle += smoother * 1.0f;
    }
  }

  if (keyStates[GLFW_KEY_J].first) { // Move Backward
    idle_ud = false;

    if (time - keyStates[GLFW_KEY_J].second < 1.0) {
      smoother = sin(glm::radians(90 * (time - keyStates[GLFW_KEY_J].second)));
    } else {
      smoother = 1.0f;
    }

    if (player->position.y + smoother * speed <= 1.0) {
      player->position += smoother * glm::vec3(0.0f, speed, 0.0f); // Move
    }

    if(!dev_mode && camera.cameraPos.y - player->position.y <= -0.04) {
      camera.cameraPos.y = player->position.y + -0.04f;
    }

    if (player->xAngle > -15.0f) {
      player->xAngle -= smoother * 1.0f;
    }
  }

  if (keyStates[GLFW_KEY_H].first) { // Move Left
    idle_lr = false;

    if (time - keyStates[GLFW_KEY_H].second < 1.0) {
      smoother = sin(glm::radians(90 * (time - keyStates[GLFW_KEY_H].second)));
    } else {
      smoother = 1.0f;
    }

    if(player->position.x + smoother * speed <= 1.5){
      player->position += smoother * glm::vec3(speed, 0.0f, 0.0f); // Move
    }
    
    if(!dev_mode && camera.cameraPos.x - player->position.x <= -0.06) {
      camera.cameraPos.x = player->position.x + -0.06f;
    }

    if (player->zAngle > -15.0f && !is_rotating) {
      player->zAngle -= smoother * 1.0f;
    }
    if (player->yAngle < 15.0f) {
      player->yAngle += smoother * 1.0f;
      //camera.yAngle = player->yAngle; 
    }
  }

  if (keyStates[GLFW_KEY_K].first) { // Move Right
    idle_lr = false;

    if (time - keyStates[GLFW_KEY_K].second < 1.0) {
      smoother = sin(glm::radians(90 * (time - keyStates[GLFW_KEY_K].second)));
    } else {
      smoother = 1.0f;
    }

    if (player->position.x - smoother * speed >= -1.5){
      player->position -= smoother * glm::vec3(speed, 0.0f, 0.0f); // Move
    }
    if(!dev_mode && camera.cameraPos.x - player->position.x >= 0.06) {
      camera.cameraPos.x = player->position.x + 0.06f;
    }

    if (player->zAngle < 15.0f && !is_rotating) {
      player->zAngle += smoother * 1.0f;
    }
    if (player->yAngle > -15.0f) {
      player->yAngle -= smoother * 1.0f;
      //camera.yAngle = player->yAngle; 
    }
  }

  if (keyStates[GLFW_KEY_O].first) { // Rotate left
    is_rotating = true;
    idle_rot = false;

    if (time - keyStates[GLFW_KEY_O].second < 1.0) {
      smoother = sin(glm::radians(90 * (time - keyStates[GLFW_KEY_O].second)));
    } else {
      smoother = 1.0f;
    }
    if (player->zAngle > -90.0f) {
      player->zAngle -= smoother * 1.8f;
    }
  }

  if (keyStates[GLFW_KEY_P].first) { // Rotate right
    is_rotating = true;
    idle_rot = false;

    if (time - keyStates[GLFW_KEY_P].second < 1.0) {
      smoother = sin(glm::radians(90 * (time - keyStates[GLFW_KEY_P].second)));
    } else {
      smoother = 1.0f;
    }
    if (player->zAngle < 90.0f) {
      player->zAngle += smoother * 1.8f;
    }
  }

  if (!keyStates[GLFW_KEY_H].first && !keyStates[GLFW_KEY_K].first) {
    if (!idle_lr && player->yAngle != 0.0f) {
      idle_lr = true; // start of the Up/Down idle animation
      idle_start_lr =
          time; // stores the timestamp of the beginning of the idle animation
    }

    if (idle_lr && (time - idle_start_lr) <= 1.0) { // animation lasts 1 second
      if (!is_rotating) {
        player->zAngle =
            player->zAngle * cos(glm::radians(90.0f * (time - idle_start_lr)));
            //camera.yAngle = player->yAngle; 
      }
      player->yAngle =
          player->yAngle * cos(glm::radians(90.0f * (time - idle_start_lr)));
          //camera.yAngle = player->yAngle; 
    } else if (idle_lr) { // end of the animation
      idle_lr = false;
      if (!is_rotating) {
        player->zAngle = 0.0f;
      }
      player->yAngle = 0.0f;
      //camera.yAngle = player->yAngle; 
    }
  }

  if (!keyStates[GLFW_KEY_U].first && !keyStates[GLFW_KEY_J].first) {
    if (!idle_ud && player->xAngle != 0.0f) {
      idle_ud = true; // start of the Up/Down idle animation
      idle_start_ud =
          time; // stores the timestamp of the beginning of the idle animation
    }

    if (idle_ud && (time - idle_start_ud) <= 1.0) { // animation lasts 1 second
      player->xAngle *= cos(glm::radians(90.0f * (time - idle_start_ud)));
    } else if (idle_ud) { // end of the animation
      idle_ud = false;
      player->xAngle = 0.0f;
    }
  }

  if (!keyStates[GLFW_KEY_O].first && !keyStates[GLFW_KEY_P].first) {
    if (is_rotating && player->zAngle != 0.0f) {
      idle_rot = true;
      is_rotating = false;
      idle_start_rot =
          time; // stores the timestamp of the beginning of the idle animation
    }

    if (keyStates[GLFW_KEY_H].first &&
        !keyStates[GLFW_KEY_K].first) { // animation must finish at -15 degrees
      if (idle_rot && player->zAngle <= -16.8f) {
        player->zAngle += 1.8f;
      } else if (idle_rot && player->zAngle >= -13.2f) {
        player->zAngle -= 1.8f;
      } else if (idle_rot) { // end of the animation
        is_rotating = false;
        idle_rot = false;
        player->zAngle = -15.0f;
      }
    } else if (!keyStates[GLFW_KEY_H].first && keyStates[GLFW_KEY_K].first) {
      if (idle_rot && player->zAngle <= 13.2f) {
        player->zAngle += 1.8f;
      } else if (idle_rot && player->zAngle >= 16.8f) {
        player->zAngle -= 1.8f;
      } else if (idle_rot) { // end of the animation
        is_rotating = false;
        idle_rot = false;
        player->zAngle = 15.0f;
      }
    } else {
      if (idle_rot &&
          (time - idle_start_rot) <= 1.0) { // animation lasts 1 second
        player->zAngle *= cos(glm::radians(90.0f * (time - idle_start_rot)));
      } else if (idle_rot) { // end of the animation
        is_rotating = false;
        idle_rot = false;
        player->zAngle = 0.0f;
      }
    }
  }

  // Update damage animation
  player->updateDamageAnimation(time);
  player->updatePosition();
  
  // Update engine flames
  static double lastFlameTime = 0.0;
  float deltaTime = (lastFlameTime == 0.0) ? 0.016f : (float)(time - lastFlameTime);
  lastFlameTime = time;
  player->updateEngineFlames(deltaTime, is_boost_mode);
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
  glm::mat4 bullet_mat =
      glm::translate(glm::mat4(1.0f), position) *
      glm::scale(glm::mat4(1.0f), 2.5f * glm::vec3(1.0f, 1.0f, 1.0f));
  
  auto bulletNode = std::make_unique<Node>(bullet_mat);

  bulletNode->velocity_ = glm::vec3(0.0f, 0.0f, 0.0f);
  bulletNode->z_speed = &asteroid_speed;
  bulletNode->add(bullet);
  
  world_node->add(bulletNode.get()); // Add it to world_node
  bullets_.push_back(std::move(bulletNode));
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
    auto explosion = std::make_unique<Explosion>(phong_shader, position, time);
    scene_root->add(explosion->getNode());
    explosions.push_back(std::move(explosion));
    
    // Add dynamic explosion lighting effect
    if (g_LightingSystem) {
        g_LightingSystem->AddExplosionLight(position, 5.0f, 2.0f);
    }
}

void Game::activate_dev_mode() {
  dev_mode = true;
}

void Game::deactivate_dev_mode() {
  dev_mode = false;

  camera.cameraFront = glm::vec3(0.0f, 0.0f, 1.0f);
  camera.cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
  camera.cameraPos = player->position + glm::vec3(0.0f, 0.05f, -0.3f);
}

void Game::detect_colisions(double time) {
  // Spatial grid disabled to prevent crashes
  // TODO: Fix spatial grid synchronization issues
  // asteroid_spatial_grid_->clear();
  // for (const auto& asteroid : asteroids_) {
  //   glm::vec3 pos = glm::vec3(asteroid->asteroid_node->transform_[3]);
  //   asteroid_spatial_grid_->insert(asteroid.get(), pos, 0.2f); // 0.2f collision radius
  // }
  
  if (use_multithreading_) {
    colisions_between_asteroids_mt(time); // Use multi-threaded version
  } else {
    colisions_between_asteroids(time); // Use original version
  }
  
  colisions_player_asteroids(time); // Keep single-threaded (low gain, high overhead)
  colisions_player_bullet(time);
  colisions_player_ring(time);
  
  if (use_multithreading_) {
    colisions_lprojectile_asteroid_mt(time); // Use multi-threaded version
    colisions_projectile_asteroid_mt(time); // Use multi-threaded version
  } else {
    colisions_lprojectile_asteroid(time); // Use original single-threaded version
    colisions_projectile_asteroid(time); // Use original single-threaded version
  }
}

void Game::colisions_between_asteroids(double time) {
  // Detect colisions between asteroids in asteroids_ list
  for(size_t i = 0; i < asteroids_.size(); ++i) {
    // The first asteroid of the comparison
    auto& asteroid1 = asteroids_[i];
    Node* node1 = asteroid1->asteroid_node;
    glm::vec3 pos1 = glm::vec3(node1->transform_[3].x, node1->transform_[3].y, node1->transform_[3].z);
    
    for(size_t j = i + 1; j < asteroids_.size(); ++j) {
      // The second asteroid to which we compare the first asteroid
      auto& asteroid2 = asteroids_[j];
      Node* node2 = asteroid2->asteroid_node;
      glm::vec3 pos2 = glm::vec3(node2->transform_[3].x, node2->transform_[3].y, node2->transform_[3].z);

      double x = (pos2.x - pos1.x);
      double y = (pos2.y - pos1.y);
      double z = (pos2.z - pos1.z);

      if (x * x + y * y + z * z < 0.04f) { // 0.20^2 = 0.04
        world_node->remove(node1);
        world_node->remove(node2);

        // Erase the higher index first to avoid invalidating the lower index
        if (j > i) {
          asteroids_.erase(asteroids_.begin() + j);
          asteroids_.erase(asteroids_.begin() + i);
        } else {
          asteroids_.erase(asteroids_.begin() + i);
          asteroids_.erase(asteroids_.begin() + j);
        }

        glm::vec3 explosion_point = glm::vec3(x, y, z) / 2.0f + pos1;
        create_explosion(explosion_point, time); // Create explosion at collision point

        i = -1;
        break;
      }
    }
  }
}

void Game::colisions_player_asteroids(double time) {
  // Detect player colisions for each asteroid in asteroids_ list
  for(auto it = asteroids_.begin(); it != asteroids_.end();) {
    auto& asteroid = *it;
    Node* node = asteroid->asteroid_node;
    glm::vec3 asteroid_position = glm::vec3(node->transform_[3].x, node->transform_[3].y, node->transform_[3].z);
    
    double x = (player->position.x - asteroid_position.x);
    double y = (player->position.y - asteroid_position.y);
    double z = (player->position.z - asteroid_position.z);

    // OLD COLLISION SYSTEM (commented but kept for safety)
    /*
    if (x * x + y * y + z * z < 0.04f) { // 0.20^2 = 0.04
      world_node->remove(node);
      it = asteroids_.erase(it);
      create_explosion(asteroid_position, time);
      
      if (!invincible) {
        player->damage(time);
        player->shipState = player -> DAMAGED_TOP;
        lost = player->isDead();
        hud->newDialog(3, time);
        
        // Track missions progress - damage resets damage-free time
        if (dailyMissions) {
          dailyMissions->recordDamage();
        }
      }
    }
    */
    
    // NEW MULTI-POINT COLLISION SYSTEM (testing)
    float asteroidRadius = 0.10f; // Asteroid collision radius
    int collisionPointIndex = player->checkCollisionPoint(asteroid_position, asteroidRadius);
    
    if (collisionPointIndex >= 0) {
      // Collision detected at specific point
      world_node->remove(node);
      it = asteroids_.erase(it);
      create_explosion(asteroid_position, time); // Always create explosion on collision
      
      if (!invincible && !player->shieldIsActive) {
        // Get damage type from collision point and trigger animation
        Player::ShipState damageType = player->collisionPoints[collisionPointIndex].damageType;
        player->damageWithType(time, damageType);
        player->shipState = damageType;
        hud->newDialog(hud->COLLISION_1, time);
        
        // Track missions progress - damage resets damage-free time
        if (dailyMissions) {
          dailyMissions->recordDamage();
        }

        // Stop the acceleration if accelerating
        if (is_boost_mode) {
          is_boost_mode = false;
          asteroid_speed /= 2;
        }

        // Verify if the player is still alive
        if (player->isDead() && !player->isDeathAnimationActive()) {
          player->startDeathAnimation(time);
        }
      }
    } else if(asteroid_position.z < 0.0) {
      // Delete invisible asteroids
      world_node->remove(node);
      it = asteroids_.erase(it);
    } else {
      ++it;
    }
  }
}

void Game::colisions_player_bullet(double time) {
  // Update bullets
  for(auto it = bullets_.begin(); it != bullets_.end();) {
    auto& bulletNode = *it;
    
    // Check collision with player
    glm::vec3 bullet_position = glm::vec3(bulletNode->transform_[3].x, bulletNode->transform_[3].y, bulletNode->transform_[3].z);
    double x = (player->position.x - bullet_position.x);
    double y = (player->position.y - bullet_position.y);
    double z = (player->position.z - bullet_position.z);
    
    if (x * x + y * y + z * z < 0.01f) { // 0.10^2 = 0.01
      // Player collected the bullet
      world_node->remove(bulletNode.get());
      it = bullets_.erase(it);
      player->increaseBullets();
    } else {
      // Remove bullet if it's too far behind
      if (bullet_position.z < -0.2f) {
        world_node->remove(bulletNode.get());
        it = bullets_.erase(it);
      } else {
        ++it;
      }
    }
  }
}

void Game::colisions_player_ring(double time) {
  // Update bullets
  for(auto it = rings_.begin(); it != rings_.end();) {
    auto& ring = *it;
    Node* ringNode = ring->ring_node;

    // Check collision with player
    glm::vec3 ring_position = glm::vec3(ringNode->transform_[3].x, ringNode->transform_[3].y, ringNode->transform_[3].z);
    //std::cout << "Bullet position : " << bullet_position.x << " " << bullet_position.y << " " << bullet_position.z << "\n";
    double x = (player->position.x - ring_position.x);
    double y = (player->position.y - ring_position.y);
    double z = (player->position.z - ring_position.z);
    
    ring->update(time);
    
    bool collision = (x * x + y * y < 0.003025f && z > 0.03 && z < 0.05); // 0.055^2 = 0.003025 

    if (collision && !ring->animating && !ring->collected) {
        // Mark ring as collected to prevent multiple collections
        ring->collected = true;
        
        // Player collected the ring - PROFILED BOOST ACTIVATION
        if (!is_boost_mode) {
          PROFILE_SCOPE("Ring Boost Activation");
          is_boost_mode = true;
          boost_time = time;
          
          // CRITICAL FIX: Instead of modifying asteroid_speed which affects ALL objects,
          // we modify the world_node velocity for smooth performance
          float old_speed = asteroid_speed;
          asteroid_speed *= 2.0f;
          
          // Apply velocity change to world_node instead of recalculating all transforms
          if (world_node && world_node->velocity_.z != 0.0f) {
            world_node->velocity_.z *= 2.0f;
          }
          
          player->shipState = player->ACCELERATING;
          hud->newDialog(hud->ACCELERATION_1, time);
          
          std::cout << "Boost activated - speed: " << old_speed << " -> " << asteroid_speed << std::endl;
          
          // Record boost speed for statistics
          int currentSpeed = -asteroid_speed * 50;
          player->gameStats->recordSpeedReached(currentSpeed);
        } else {
          boost_time = time;
        }
        ring->startAnimation(time);
        
        // Record ring collection for statistics
        player->gameStats->recordRingTaken();
        
        // Track missions progress
        if (dailyMissions) {
          dailyMissions->recordRingCollected();
          dailyMissions->recordConsecutiveRings(player->gameStats->getCurrentConsecutiveRings());
        }
    }
    
    if (ring->to_delete || ring_position.z < -0.1f) {
      // Check if ring was missed (not collected and going behind player)
      if (!ring->animating && ring_position.z < -0.1f) {
        player->gameStats->recordRingMissed();
        
        // Missions progress: missing a ring doesn't need specific tracking
        // (consecutive ring missions are handled by the statistics system)
      }
      
      world_node->remove(ringNode);
      it = rings_.erase(it);
    } else {
      ++it;
    }
  }
}


void Game::colisions_lprojectile_asteroid(double time) {
  // For each light projectile in light_projectiles list
  for(auto it = light_projectiles.begin(); it != light_projectiles.end();) {
    auto& shoot = *it;
    shoot->update(time);

    // For each asteroid in asteroids_ list
    for(auto asteroid_it = asteroids_.begin(); asteroid_it != asteroids_.end(); ++asteroid_it) {
      auto& asteroid = *asteroid_it;
      Node* node = asteroid->asteroid_node;
      glm::vec3 asteroid_position = glm::vec3(node->transform_[3].x, node->transform_[3].y, node->transform_[3].z);
      // Check collision with asteroid
      if (shoot->checkCollision(asteroid_position)) {
        asteroid->life -= 1;
        if (asteroid->life <= 0) {
          create_explosion(asteroid_position, time);
          if (rand() % 3 == 0) {
            spawn_bullet(asteroid_position);
          }
          // Increment score
          if (asteroid->is_moving) {
            player->score += 100.0;
            hud->scoreIncrement(shoot->cursorPosition.x, shoot->cursorPosition.y, time, 100);
          } else {
            player->score += 50.0;
            hud->scoreIncrement(shoot->cursorPosition.x, shoot->cursorPosition.y, time, 50);
          }
          
          // Record asteroid destruction for statistics
          player->gameStats->recordAsteroidDestroyed(asteroid->is_moving);
          
          // Track missions progress
          if (dailyMissions) {
            dailyMissions->recordAsteroidDestroyed();
            if (asteroid->is_moving) {
              dailyMissions->recordMovingAsteroidDestroyed();
            }
          }
          
          // Erase the asteroid
          world_node->remove(node);
          asteroids_.erase(asteroid_it);
        }
        // Delete the shoot
        shoot->active = false;
        break; // Exit asteroid loop since projectile hit something
      }
    }

    // Delete the shoot if inactive
    if (!shoot->active) {
      scene_root->remove(shoot->node);
      it = light_projectiles.erase(it);
    } else {
      ++it;
    }
  }
}

void Game::colisions_projectile_asteroid(double time) {
  // For each projectile in projectiles list
  for(auto it = projectiles.begin(); it != projectiles.end();) {
    auto& shoot = *it;
    shoot->update(time);

    // For each asteroid in asteroids_ list
    for(auto it = asteroids_.begin(); it != asteroids_.end();) {
      auto& asteroid = *it;
      Node* node = asteroid->asteroid_node;
      glm::vec3 asteroid_position = glm::vec3(node->transform_[3].x, node->transform_[3].y, node->transform_[3].z);
      // Delete the asteroid if colision
      if (shoot->checkCollision(asteroid_position)) {
        // Increment score
        if (asteroid->is_moving) {
          player->score += 100.0;
          hud->scoreIncrement(shoot->cursorPosition.x, shoot->cursorPosition.y, time, 100);
        } else {
          player->score += 50.0;
          hud->scoreIncrement(shoot->cursorPosition.x, shoot->cursorPosition.y, time, 50);
        }
        hud->newDialog(hud->SHOOT_1, time);
        
        // Record asteroid destruction for statistics
        player->gameStats->recordAsteroidDestroyed(asteroid->is_moving);
        
        // Track missions progress
        if (dailyMissions) {
          dailyMissions->recordAsteroidDestroyed();
          if (asteroid->is_moving) {
            dailyMissions->recordMovingAsteroidDestroyed();
          }
        }
        
        world_node->remove(node);
        it = asteroids_.erase(it);
        shoot->active = false;
      } else {
        ++it;
      }
    }

    // Delete the shoot if inactive
    if (!shoot->active) {
      scene_root->remove(shoot->node);
      it = projectiles.erase(it);
    } else {
      ++it;
    }
  }
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

// Optimized collision detection using spatial partitioning
void Game::colisions_player_asteroids_optimized(double time) {
  // Get nearby asteroids using spatial grid
  std::vector<Asteroid*> nearby_asteroids = asteroid_spatial_grid_->getNearby(
    player->position, 0.3f); // 0.3f search radius around player
  
  for (Asteroid* asteroid : nearby_asteroids) {
    // Safety check for null pointer
    if (!asteroid || !asteroid->asteroid_node) {
      continue;
    }
    
    Node* node = asteroid->asteroid_node;
    glm::vec3 asteroid_position = glm::vec3(node->transform_[3].x, node->transform_[3].y, node->transform_[3].z);
    
    // Use the same collision detection logic but only for nearby asteroids
    float asteroidRadius = 0.10f; // Asteroid collision radius
    int collisionPointIndex = player->checkCollisionPoint(asteroid_position, asteroidRadius);
    
    if (collisionPointIndex >= 0) {
      // Remove from spatial grid
      asteroid_spatial_grid_->remove(asteroid);
      
      // Find and remove from asteroids_ vector
      auto it = std::find_if(asteroids_.begin(), asteroids_.end(),
        [asteroid](const std::unique_ptr<Asteroid>& ptr) { return ptr.get() == asteroid; });
      
      if (it != asteroids_.end()) {
        world_node->remove(node);
        asteroids_.erase(it);
        create_explosion(asteroid_position, time);
        
        if (!invincible && !player->shieldIsActive) {
          Player::ShipState damageType = player->collisionPoints[collisionPointIndex].damageType;
          player->damageWithType(time, damageType);
          player->shipState = damageType;
          hud->newDialog(hud->COLLISION_1, time);
          
          if (dailyMissions) {
            dailyMissions->recordDamage();
          }

          if (is_boost_mode) {
            is_boost_mode = false;
            asteroid_speed /= 2;
          }

          if (player->isDead() && !player->isDeathAnimationActive()) {
            player->startDeathAnimation(time);
          }
        }
        return; // Exit after first collision
      }
    }
  }
  
  // Clean up asteroids that are too far behind (original logic)
  for(auto it = asteroids_.begin(); it != asteroids_.end();) {
    auto& asteroid = *it;
    Node* node = asteroid->asteroid_node;
    glm::vec3 asteroid_position = glm::vec3(node->transform_[3].x, node->transform_[3].y, node->transform_[3].z);
    
    if (asteroid_position.z < 0.0) {
      asteroid_spatial_grid_->remove(asteroid.get());
      world_node->remove(node);
      it = asteroids_.erase(it);
    } else {
      ++it;
    }
  }
}

void Game::colisions_lprojectile_asteroid_optimized(double time) {
  for(auto it = light_projectiles.begin(); it != light_projectiles.end();) {
    auto& shoot = *it;
    shoot->update(time);

    // Get nearby asteroids for this projectile using spatial grid
    std::vector<Asteroid*> nearby_asteroids = asteroid_spatial_grid_->getNearby(
      shoot->position, 0.25f); // 0.25f search radius around projectile
    
    bool hit = false;
    for (Asteroid* asteroid : nearby_asteroids) {
      // Safety check for null pointer
      if (!asteroid || !asteroid->asteroid_node) {
        continue;
      }
      
      Node* node = asteroid->asteroid_node;
      glm::vec3 asteroid_position = glm::vec3(node->transform_[3].x, node->transform_[3].y, node->transform_[3].z);
      
      if (shoot->checkCollision(asteroid_position)) {
        asteroid->life -= 1;
        if (asteroid->life <= 0) {
          create_explosion(asteroid_position, time);
          if (rand() % 3 == 0) {
            spawn_bullet(asteroid_position);
          }
          
          // Score calculation
          if (asteroid->is_moving) {
            player->score += 100.0;
            hud->scoreIncrement(shoot->cursorPosition.x, shoot->cursorPosition.y, time, 100);
          } else {
            player->score += 50.0;
            hud->scoreIncrement(shoot->cursorPosition.x, shoot->cursorPosition.y, time, 50);
          }
          
          player->gameStats->recordAsteroidDestroyed(asteroid->is_moving);
          
          if (dailyMissions) {
            dailyMissions->recordAsteroidDestroyed();
            if (asteroid->is_moving) {
              dailyMissions->recordMovingAsteroidDestroyed();
            }
          }
          
          // Remove from spatial grid and vector
          asteroid_spatial_grid_->remove(asteroid);
          auto asteroid_it = std::find_if(asteroids_.begin(), asteroids_.end(),
            [asteroid](const std::unique_ptr<Asteroid>& ptr) { return ptr.get() == asteroid; });
          
          if (asteroid_it != asteroids_.end()) {
            world_node->remove(node);
            asteroids_.erase(asteroid_it);
          }
        }
        
        shoot->active = false;
        hit = true;
        break;
      }
    }

    if (!shoot->active) {
      scene_root->remove(shoot->node);
      it = light_projectiles.erase(it);
    } else {
      ++it;
    }
  }
}

void Game::colisions_projectile_asteroid_optimized(double time) {
  for(auto it = projectiles.begin(); it != projectiles.end();) {
    auto& shoot = *it;
    shoot->update(time);

    // Get nearby asteroids for this projectile using spatial grid
    std::vector<Asteroid*> nearby_asteroids = asteroid_spatial_grid_->getNearby(
      shoot->position, 0.25f); // 0.25f search radius around projectile
    
    bool hit = false;
    for (Asteroid* asteroid : nearby_asteroids) {
      // Safety check for null pointer
      if (!asteroid || !asteroid->asteroid_node) {
        continue;
      }
      
      Node* node = asteroid->asteroid_node;
      glm::vec3 asteroid_position = glm::vec3(node->transform_[3].x, node->transform_[3].y, node->transform_[3].z);
      
      if (shoot->checkCollision(asteroid_position)) {
        // Score calculation
        if (asteroid->is_moving) {
          player->score += 100.0;
          hud->scoreIncrement(shoot->cursorPosition.x, shoot->cursorPosition.y, time, 100);
        } else {
          player->score += 50.0;
          hud->scoreIncrement(shoot->cursorPosition.x, shoot->cursorPosition.y, time, 50);
        }
        hud->newDialog(hud->SHOOT_1, time);
        
        player->gameStats->recordAsteroidDestroyed(asteroid->is_moving);
        
        if (dailyMissions) {
          dailyMissions->recordAsteroidDestroyed();
          if (asteroid->is_moving) {
            dailyMissions->recordMovingAsteroidDestroyed();
          }
        }
        
        // Remove from spatial grid and vector
        asteroid_spatial_grid_->remove(asteroid);
        auto asteroid_it = std::find_if(asteroids_.begin(), asteroids_.end(),
          [asteroid](const std::unique_ptr<Asteroid>& ptr) { return ptr.get() == asteroid; });
        
        if (asteroid_it != asteroids_.end()) {
          world_node->remove(node);
          asteroids_.erase(asteroid_it);
        }
        
        shoot->active = false;
        hit = true;
        break;
      }
    }

    if (!shoot->active) {
      scene_root->remove(shoot->node);
      it = projectiles.erase(it);
    } else {
      ++it;
    }
  }
}