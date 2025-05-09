#include "game.h"
#include <cmath>
#ifndef SHADER_DIR
#error "SHADER_DIR not defined"
#endif
#ifndef TEXTURES_DIR
#error "TEXTURES_DIR not defined"
#endif
#ifndef RESSOURCES_DIR
#error "RESSOURCES_DIR not defined"
#endif

Game::Game() {
  std::string shader_dir = SHADER_DIR;
  std::string textures_dir = TEXTURES_DIR;
  std::string ressources_dir = RESSOURCES_DIR;

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

  glm::mat4 environment_mat =
      glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)) *
      glm::scale(glm::mat4(1.0f), 120.0f * glm::vec3(1.0f, 1.0f, 1.0f)) *
      glm::rotate(glm::mat4(1.0f), glm::radians(0.0f),
                  glm::vec3(1.0f, 0.0f, 0.0f));
  Node *environment_node = new Node(environment_mat);
  environment_node->add(environment_sphere);
  // Ajout partie de banzai oui je parle français merde
  Shape *asteroid =
      new ShapeModel(ressources_dir + "Asteroid.obj", phong_shader);
  // Créer plusieurs asteroid
  for (int i = 0; i < 20; ++i) {
    // Position aléatoire
    float x = ((rand() % 200) / 100.0f) - 1.0f; // entre -1 et 1
    float y = ((rand() % 200) / 100.0f) - 1.0f;

    float z = -(((rand() % 300) / 100.0f) - 1.0f); // entre -1 et -4

    glm::mat4 asteroid_mat1 =
        glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z)) *
        glm::scale(glm::mat4(1.0f), 0.006f * glm::vec3(1.0f, 1.0f, 1.0f)) *
        glm::rotate(glm::mat4(1.0f), glm::radians(10.0f),
                    glm::vec3(1.0f, 0.0f, 0.0f)); // asteroid

    Node *a_node1 = new Node(asteroid_mat1);
    a_node1->velocity_ = glm::vec3(0.0f, 0.0f, -0.2f);
    a_node1->add(asteroid);

    world_node->add(a_node1);
  }

  // Add the player and the world to the scene root
  scene_root = new Node();
  scene_root->add(player->node);
  scene_root->add(world_node);
  scene_root->add(environment_node);
  distance = 45.0f;
}

void Game::updateGame() {
  if (latence == 0) {
    spawn_rectangle();
    latence = 80;
  } else {
    latence -= 1;
  }
  world_node->animation();
  // Colisiont
  for (Node *child : world_node->children_) {
    double x = (player->position.x - child->transform_[3].x);
    double y = (player->position.y - child->transform_[3].y);
    double z = (player->position.z - child->transform_[3].z);
    // Suprime la sphére si elle s'aproche trop
    if (sqrt(x * x + y * y + z * z) < 0.3) {
      world_node->remove(child);
      player->life -= 1;
      if (player->life <= 0) {
        lost = true;
      }
    };
  }
}

void Game::keyHandler(
    std::unordered_map<int, std::pair<bool, double>> keyStates, double time) {
  float smoother = 0.0f;
  float speed = player->movement_speed;

  if (keyStates[GLFW_KEY_U].first) { // Move Forward
    idle_ud = false;

    if (time - keyStates[GLFW_KEY_U].second < 1.0) {
      smoother = sin(glm::radians(90 * (time - keyStates[GLFW_KEY_U].second)));
    } else {
      smoother = 1.0f;
    }

    player->position -= smoother * glm::vec3(0.0f, speed, 0.0f); // Move

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

    player->position += smoother * glm::vec3(0.0f, speed, 0.0f); // Move
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

    player->position += smoother * glm::vec3(speed, 0.0f, 0.0f); // Move
    if (player->zAngle > -15.0f && !is_rotating) {
      player->zAngle -= smoother * 1.0f;
    }
    if (player->yAngle < 15.0f) {
      player->yAngle += smoother * 1.0f;
    }
  }

  if (keyStates[GLFW_KEY_K].first) { // Move Right
    idle_lr = false;

    if (time - keyStates[GLFW_KEY_K].second < 1.0) {
      smoother = sin(glm::radians(90 * (time - keyStates[GLFW_KEY_K].second)));
    } else {
      smoother = 1.0f;
    }

    player->position -= smoother * glm::vec3(speed, 0.0f, 0.0f); // Move
    if (player->zAngle < 15.0f && !is_rotating) {
      player->zAngle += smoother * 1.0f;
    }
    if (player->yAngle > -15.0f) {
      player->yAngle -= smoother * 1.0f;
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
      }
      player->yAngle =
          player->yAngle * cos(glm::radians(90.0f * (time - idle_start_lr)));
    } else if (idle_lr) { // end of the animation
      idle_lr = false;
      if (!is_rotating) {
        player->zAngle = 0.0f;
      }
      player->yAngle = 0.0f;
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
  std::string ressources_dir = RESSOURCES_DIR;
  // Clone de l'astéroïde
  Shape *asteroid =
      new ShapeModel(ressources_dir + "Asteroid.obj",
                     phong_shader); // appel du constructeur de copie

  // Position aléatoire
  float posX = ((rand() % 200) / 100.0f) - 1.0f;
  float posY = ((rand() % 200) / 100.0f) - 1.0f;

  glm::mat4 asteroid_mat =
      glm::translate(glm::mat4(1.0f), glm::vec3(posX, posY, +4.0f)) *
      glm::scale(glm::mat4(1.0f), 0.006f * glm::vec3(1.0f, 1.0f, 1.0f)) *
      glm::rotate(glm::mat4(1.0f), glm::radians(10.0f),
                  glm::vec3(1.0f, 0.0f, 0.0f));
  // Teste
  // std::cout << "Spawning at position: (" << posX << ", " << posY << ",
  // -2.0)"<< std::endl;
  Node *rectNode = new Node(asteroid_mat);
  rectNode->velocity_ = glm::vec3(0.0f, 0.0f, -0.2f);
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
