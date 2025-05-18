#include "game.h"
#include "GLFW/glfw3.h"
#include "glm/fwd.hpp"
#include "hud.h"
#include "projectile.h"
#include <cmath>
#include <ctime>
#ifndef SHADER_DIR
#error "SHADER_DIR not defined"
#endif
#ifndef TEXTURES_DIR
#error "TEXTURES_DIR not defined"
#endif
#ifndef RESSOURCES_DIR
#error "RESSOURCES_DIR not defined"
#endif

Game::Game(int width, int height, int target_fps) {
  std::string shader_dir = SHADER_DIR;
  std::string textures_dir = TEXTURES_DIR;
  std::string ressources_dir = RESSOURCES_DIR;

  // Generates HUD
  hud = new Hud(width, height);
  window_width = width;
  window_height = height;
  targeted_fps = target_fps;

  // Player object
  phong_shader =
      new Shader(shader_dir + "phong.vert", shader_dir + "phong.frag");
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

  // Asteroid model
  asteroid = new ShapeModel(ressources_dir + "Asteroid.obj", phong_shader);
  
  // Generates 20 asteroids
  for (int i = 0; i < 20; ++i) {
    // Random position
    float x = ((rand() % 200) / 100.0f) - 1.0f; // entre -1 et 1
    float y = ((rand() % 200) / 100.0f) - 1.0f;
    float z = -(((rand() % 300) / 100.0f) - 1.0f); // entre -1 et -4

    glm::mat4 asteroid_mat1 =
        glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z)) *
        glm::scale(glm::mat4(1.0f), 0.006f * glm::vec3(1.0f, 1.0f, 1.0f)) *
        glm::rotate(glm::mat4(1.0f), glm::radians(10.0f),
                    glm::vec3(1.0f, 0.0f, 0.0f)); // asteroid

    Node *a_node1 = new Node(asteroid_mat1);
    a_node1->z_speed = &asteroid_speed;
    a_node1->velocity_ = glm::vec3(0.0f, 0.0f, 0.0f);
    a_node1->add(asteroid);
    // Add it to the world
    world_node->add(a_node1);
  }
  
  // Add the player and the world to the scene root
  scene_root = new Node();
  scene_root->add(player->node);
  scene_root->add(world_node);
  scene_root->add(environment_node);
  
  // Place the camera
  camera.cameraFront = glm::vec3(0.0f, 0.0f, 1.0f);
  camera.cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
  camera.cameraPos = player->position + glm::vec3(0.0f, 0.05f, -0.3f);

  // Initialize the shoot cooldown variable
  last_shoot_time = 0.0;
}

void Game::draw(glm::mat4 model, glm::mat4 view, glm::mat4 projection, double time, int fps) {
  scene_root->draw(model, view, projection);
  hud->update(player->life, player->score, time, -asteroid_speed*50, fps);
}

void Game::updateGame(double time, int fps) {
  // Adjust game speed to the number of fps
  float fps_correction = 1.0f;
  if (fps > 15) {
    fps_correction = static_cast<float>(targeted_fps) / static_cast<float>(fps);
    player->fps_correction = fps_correction;
  }
  std::cout << "FPS corr : " << fps_correction << " " << static_cast<float>(targeted_fps) << " " << static_cast<float>(fps) <<"\n";
  // Generates new asteroids
  if (latence == 0) {
    spawn_rectangle();
    latence = 80;
  } else {
    latence -= 1;
  }

  // Moves the world forward
  world_node->animation(fps_correction);

  // Verify collisions
  for (Node *child : world_node->children_) {
    double x = (player->position.x - child->transform_[3].x);
    double y = (player->position.y - child->transform_[3].y);
    double z = (player->position.z - child->transform_[3].z);
    // Supprime la sphére si elle s'aproche trop
    if (sqrt(x * x + y * y + z * z) < 0.20) {
      world_node->remove(child);
      player->life -= 1;
      if (player->life <= 0) {
        lost = true;
      }
    };
  }

  // Increments player's score
  player->score += 1.0;

  // Every 5000 points, player get an extra life
  if(static_cast<int>(player->score) % 5000 == 0) {
    player->life += 1;
    hud->newDialog(2, time);
  }

  // Every 1000 points, the ship speed increases
  if(static_cast<int>(player->score) % 1000 == 0) {
    if (player->score > 20000.0) {
      asteroid_speed -= 0.8;
    } else if(player->score > 10000.0) {
      asteroid_speed -= 0.6;
    } else {
      asteroid_speed -= 0.4;
    }
  }

  // Adds dialogs
  if(player->position.z == 0 && player->score < 100.0) {
    hud->newDialog(0, time);
  }

  for(auto it = projectiles.begin(); it != projectiles.end();) {
    Projectile *shoot = *it;
    shoot->update(time);
    
    // Verify collisions with asteroids
    for (Node *child : world_node->children_) {
      glm::vec3 asteroid_position = glm::vec3(child->transform_[3].x, child->transform_[3].y, child->transform_[3].z);
      // Delete the asteroid if colision
      if (shoot->checkCollision(glm::vec3(asteroid_position))) {
        world_node->remove(child);
        shoot->active = false;
        player->score += 50.0;
        hud->scoreIncrement(shoot->cursorPosition.x, shoot->cursorPosition.y, time);
      };
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

void Game::mouse_callback(double xpos, double ypos){
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
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    l_mouse_button_pressed = true;
  } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE){
    l_mouse_button_pressed = false;
  }
}

void Game::keyHandler(
    std::unordered_map<int, std::pair<bool, double>> keyStates, double time) {
  float smoother = 0.0f;
  float speed = player->movement_speed * player->fps_correction;

  
  if (l_mouse_button_pressed) {
    // Add new shoot to the projectile list
    if (projectiles.size() < 10) {
      double xpos = (x_mouse / (window_width)) - 0.5;
      double ypos = (y_mouse / (window_height)) - 0.5;
      
      glm::vec3 shoot_position = glm::vec3(player->position.x, player->position.y, player->position.z + 0.2f); 
      glm::vec3 shoot_direction = glm::vec3(-xpos+camera.cameraPos.x-player->position.x, -ypos+camera.cameraPos.y-player->position.y, 1.0f);
      // Shooting cooldown
      if (last_shoot_time == 0.0 || time-last_shoot_time > 1.0) {
        Projectile *new_shot = new Projectile(phong_shader, shoot_position, shoot_direction, glm::vec3(x_mouse, y_mouse, 0.0f));
        projectiles.push_back(new_shot);
        scene_root->add(new_shot->node);
        last_shoot_time = time;
      }
    }
  }

  if (keyStates[GLFW_KEY_W].first) {
    asteroid_speed -= 1.0f;
  }
  if (keyStates[GLFW_KEY_S].first) {
    asteroid_speed += 1.0f;
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
    std::cout << is_rotating << " " << player->zAngle << "\n";
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

  player->updatePosition();
}

void Game::spawn_rectangle() {
  // Position aléatoire
  float posX = ((rand() % 200) / 100.0f) - 1.0f; // Entre -1 et 1
  float posY = ((rand() % 200) / 100.0f) - 1.0f; // Entre -1 et 1
  float posZ = ((rand() % 200) / 100.0f) + 1.0f; // Entre 1 et 2

  glm::mat4 asteroid_mat =
      glm::translate(glm::mat4(1.0f), glm::vec3(posX, posY, posZ)) *
      glm::scale(glm::mat4(1.0f), 0.006f * glm::vec3(1.0f, 1.0f, 1.0f)) *
      glm::rotate(glm::mat4(1.0f), glm::radians(10.0f),
                  glm::vec3(1.0f, 0.0f, 0.0f));
  
  Node *rectNode = new Node(asteroid_mat);

  rectNode->velocity_ = glm::vec3(0.0f, 0.0f, 0.0f);
  rectNode->z_speed = &asteroid_speed;
  rectNode->add(asteroid);

  world_node->add(rectNode);
  asteorides_.push_back(rectNode);
  /*
  if (asteorides_.size() > max_asteorides_) {
    Node *oldRect = asteorides_.front();
    asteorides_.erase(asteorides_.begin());

    world_node->remove(oldRect); // retirer du world node
    delete oldRect;
  }
  */
}

