#include "game.h"
#include "GLFW/glfw3.h"
#include "asteroid.h"
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

void Game::draw(glm::mat4 model, glm::mat4 view, glm::mat4 projection,
                double time, int fps) {
  scene_root->draw(model, view, projection);
  hud->update(player->life, player->score, player->bullets, time,
              -asteroid_speed * 50, fps);
}

void Game::updateGame(double time, int fps) {
  // Adjust game speed to the number of fps
  float fps_correction = 1.0f;
  if (fps > 15) {
    fps_correction = static_cast<float>(targeted_fps) / static_cast<float>(fps);
    player->fps_correction = fps_correction;
  }

  // Generates new asteroids
  latence -= fps_correction;
  if (latence <= 0.0f) {
    spawn_rectangle(); // Crée un seul astéroïde
    latence =
        20.0f; // Temps d’attente avant le suivant (ajuste selon ton besoin)
  }
  // Moves the world forward
  world_node->animation(fps_correction);
  // Suprime les astéoride dérriere
  derriere();
  // Detect player colisions for each asteroid in asteroids_ list
  for (auto it = asteroids_.begin(); it != asteroids_.end();) {
    Asteroid *asteroid = *it;
    Node *node = asteroid->asteroid_node;
    glm::vec3 asteroid_position = glm::vec3(
        node->transform_[3].x, node->transform_[3].y, node->transform_[3].z);
    double x = (player->position.x - asteroid_position.x);
    double y = (player->position.y - asteroid_position.y);
    double z = (player->position.z - asteroid_position.z);
    // Supprime la sphére si elle s'aproche trop
    if (sqrt(x * x + y * y + z * z) < 0.20) {
      world_node->remove(node);
      it = asteroids_.erase(it);
      asteroid->explode();
      player->life -= 1;
      if (player->life <= 0) {
        lost = true;
      }
    } else {
      ++it;
    }
  }

  // Increments player's score
  player->score += 1.0;

  // Every 5000 points, player get an extra life
  if (static_cast<int>(player->score) % 5000 == 0) {
    player->increaseLife();
    hud->newDialog(2, time);
  }

  // Every 1000 points, the ship speed increases
  if (static_cast<int>(player->score) % 1000 == 0) {
    if (player->score > 20000.0) {
      asteroid_speed -= 0.8;
    } else if (player->score > 10000.0) {
      asteroid_speed -= 0.6;
    } else {
      asteroid_speed -= 0.4;
    }
  }

  // Adds dialogs
  if (player->position.z == 0 && player->score < 100.0) {
    hud->newDialog(0, time);
  }

  // For each projectile in projectiles list
  for (auto it = projectiles.begin(); it != projectiles.end();) {
    Projectile *shoot = *it;
    shoot->update(time);

    // For each asteroid in asteroids_ list
    for (auto it = asteroids_.begin(); it != asteroids_.end();) {
      Asteroid *asteroid = *it;
      Node *node = asteroid->asteroid_node;
      glm::vec3 asteroid_position = glm::vec3(
          node->transform_[3].x, node->transform_[3].y, node->transform_[3].z);
      // Delete the asteroid if colision
      if (shoot->checkCollision(glm::vec3(asteroid_position))) {
        world_node->remove(node);
        it = asteroids_.erase(it);
        asteroid->explode();
        shoot->active = false;
        player->score += 50.0;
        hud->scoreIncrement(shoot->cursorPosition.x, shoot->cursorPosition.y,
                            time);
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

  // For each light projectile in light_projectiles list
  for (auto it = light_projectiles.begin(); it != light_projectiles.end();) {
    LightProjectile *shoot = *it;
    shoot->update(time);

    // For each asteroid in asteroids_ list
    for (auto asteroid_it = asteroids_.begin(); asteroid_it != asteroids_.end();
         ++asteroid_it) {
      Asteroid *asteroid = *asteroid_it;
      Node *node = asteroid->asteroid_node;
      glm::vec3 asteroid_position = glm::vec3(
          node->transform_[3].x, node->transform_[3].y, node->transform_[3].z);
      // Check collision with asteroid
      if (shoot->checkCollision(glm::vec3(asteroid_position))) {
        asteroid->life -= 1;
        if (asteroid->life <= 0) {
          // Erase the asteroid
          world_node->remove(node);
          asteroids_.erase(asteroid_it);
          // Increment score
          player->score += 50.0;
          hud->scoreIncrement(shoot->cursorPosition.x, shoot->cursorPosition.y,
                              time);
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

void Game::mouse_callback(double xpos, double ypos) {
  if (xpos < 0.0) {
    xpos = 0.0;
  } else if (xpos > window_width) {
    xpos = window_width;
  }

  if (ypos < 0.0) {
    ypos = 0.0;
  } else if (ypos > window_height) {
    ypos = window_height;
  }

  x_mouse = xpos;
  y_mouse = ypos;
  hud->mouse(xpos, ypos);
}

void Game::mouse_button_callback(int button, int action, double xpos,
                                 double ypos, double time) {
  mouse_callback(xpos, ypos);
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    l_mouse_button_pressed = true;
  } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
    l_mouse_button_pressed = false;
  }

  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
    r_mouse_button_pressed = true;
  } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
    r_mouse_button_pressed = false;
  }
}

void Game::keyHandler(
    std::unordered_map<int, std::pair<bool, double>> keyStates, double time) {
  float smoother = 0.0f;
  float speed = player->movement_speed * player->fps_correction;

  if (r_mouse_button_pressed) {
    // Shooting cooldown
    if ((last_shoot_time == 0.0 || time - last_shoot_time > 1.0) &&
        (player->bullets > 0)) {
      player->bullets -= 1;
      double xpos = (x_mouse / (window_width)) - 0.5;
      double ypos = (y_mouse / (window_height)) - 0.5;

      glm::vec3 shoot_position = glm::vec3(
          player->position.x, player->position.y, player->position.z + 0.2f);
      glm::vec3 shoot_direction =
          glm::vec3(-xpos + camera.cameraPos.x - player->position.x,
                    -ypos + camera.cameraPos.y - player->position.y, 1.0f);

      // Add new shot to projectile list
      Projectile *new_shot =
          new Projectile(phong_shader, shoot_position, shoot_direction,
                         glm::vec3(x_mouse, y_mouse, 0.0f));
      projectiles.push_back(new_shot);
      scene_root->add(new_shot->node);
      last_shoot_time = time;
    }
  }

  if (l_mouse_button_pressed) {
    // Shooting cooldown
    if (last_shoot_time_l == 0.0 || time - last_shoot_time_l > 0.1) {
      double xpos = (x_mouse / (window_width)) - 0.5;
      double ypos = (y_mouse / (window_height)) - 0.5;

      glm::vec3 shoot_position = glm::vec3(
          player->position.x, player->position.y, player->position.z + 0.2f);
      glm::vec3 shoot_direction =
          glm::vec3(-xpos + camera.cameraPos.x - player->position.x,
                    -ypos + camera.cameraPos.y - player->position.y, 1.0f);

      // Add new shot to projectile list
      LightProjectile *new_shot =
          new LightProjectile(phong_shader, shoot_position, shoot_direction,
                              glm::vec3(x_mouse, y_mouse, 0.0f));
      light_projectiles.push_back(new_shot);
      scene_root->add(new_shot->node);
      last_shoot_time_l = time;
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

    if (!dev_mode && camera.cameraPos.y - player->position.y >= 0.04) {
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

    if (!dev_mode && camera.cameraPos.y - player->position.y <= -0.04) {
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

    if (player->position.x + smoother * speed <= 1.5) {
      player->position += smoother * glm::vec3(speed, 0.0f, 0.0f); // Move
    }

    if (!dev_mode && camera.cameraPos.x - player->position.x <= -0.06) {
      camera.cameraPos.x = player->position.x + -0.06f;
    }

    if (player->zAngle > -15.0f && !is_rotating) {
      player->zAngle -= smoother * 1.0f;
    }
    if (player->yAngle < 15.0f) {
      player->yAngle += smoother * 1.0f;
      // camera.yAngle = player->yAngle;
    }
  }

  if (keyStates[GLFW_KEY_K].first) { // Move Right
    idle_lr = false;

    if (time - keyStates[GLFW_KEY_K].second < 1.0) {
      smoother = sin(glm::radians(90 * (time - keyStates[GLFW_KEY_K].second)));
    } else {
      smoother = 1.0f;
    }

    if (player->position.x - smoother * speed >= -1.5) {
      player->position -= smoother * glm::vec3(speed, 0.0f, 0.0f); // Move
    }
    if (!dev_mode && camera.cameraPos.x - player->position.x >= 0.06) {
      camera.cameraPos.x = player->position.x + 0.06f;
    }

    if (player->zAngle < 15.0f && !is_rotating) {
      player->zAngle += smoother * 1.0f;
    }
    if (player->yAngle > -15.0f) {
      player->yAngle -= smoother * 1.0f;
      // camera.yAngle = player->yAngle;
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
        // camera.yAngle = player->yAngle;
      }
      player->yAngle =
          player->yAngle * cos(glm::radians(90.0f * (time - idle_start_lr)));
      // camera.yAngle = player->yAngle;
    } else if (idle_lr) { // end of the animation
      idle_lr = false;
      if (!is_rotating) {
        player->zAngle = 0.0f;
      }
      player->yAngle = 0.0f;
      // camera.yAngle = player->yAngle;
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

  player->updatePosition();
}
void Game::spawn_rectangle() {
  // Position aléatoire
  float posX = ((rand() % 200) / 100.0f) - 1.0f; // Entre -1 et 1
  float posY = ((rand() % 200) / 100.0f) - 1.0f; // Entre -1 et 1
  float posZ = 2.0;

  glm::mat4 asteroid_mat =
      glm::translate(glm::mat4(1.0f), glm::vec3(posX, posY, posZ)) *
      glm::scale(glm::mat4(1.0f), 0.006f * glm::vec3(1.0f, 1.0f, 1.0f)) *
      glm::rotate(glm::mat4(1.0f), glm::radians(10.0f),
                  glm::vec3(1.0f, 0.0f, 0.0f));

  Node *asteroidNode = new Node(asteroid_mat);

  asteroidNode->velocity_ = glm::vec3(0.0f, 0.0f, 0.0f);
  asteroidNode->z_speed = &asteroid_speed;
  asteroidNode->add(asteroid);

  Asteroid *asteroid = new Asteroid(asteroidNode);
  asteroids_.push_back(asteroid);
  world_node->add(asteroid->asteroid_node);
}

void Game::derriere() {
  for (auto asteroid = asteroids_.begin(); asteroid != asteroids_.end();) {
    Asteroid *as = *asteroid;

    if (as->getPosition().z < -1.0) {
      Node *node = as->asteroid_node;
      world_node->remove(node);
      delete as;
      asteroid = asteroids_.erase(asteroid); // efface, avance automatiquement
    } else {
      ++asteroid; // sinon on avance manuellement
    }
  }
}
