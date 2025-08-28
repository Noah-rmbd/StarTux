#include "input_handler.h"
#include <iostream>
#include <cmath>

InputHandler::InputHandler(Player* player, Camera* camera, Node* scene_root, Shader* phong_shader,
                          int window_width, int window_height)
    : player_(player), camera_(camera), scene_root_(scene_root), phong_shader_(phong_shader),
      window_width_(window_width), window_height_(window_height) {
}

void InputHandler::handleInput(std::unordered_map<int, std::pair<bool, double>> keyStates, double time,
                              bool dev_mode, double x_mouse, double y_mouse, bool l_mouse_pressed, 
                              bool r_mouse_pressed, std::vector<std::unique_ptr<Projectile>>& projectiles,
                              std::vector<std::unique_ptr<LightProjectile>>& light_projectiles,
                              std::vector<std::unique_ptr<Node>>& bullets, bool& paused, bool& invincible,
                              bool& is_boost_mode) {
    if (player_->isDeathAnimationActive()) {
        return;
    }
    
    if (dev_mode) {
        camera_->keyboard_events(keyStates);
    }
    
    handleMouseShooting(time, x_mouse, y_mouse, l_mouse_pressed, r_mouse_pressed, 
                       projectiles, light_projectiles);
    handleDeveloperModeKeys(keyStates, const_cast<bool&>(dev_mode));
    handleDebugToggleKeys(keyStates, paused, invincible);
    handleBulletMovementKeys(keyStates, bullets);
    handlePlayerMovement(keyStates, time, dev_mode);
    handlePlayerRotation(keyStates, time);
    handleIdleAnimations(keyStates, time, is_boost_mode);
}

void InputHandler::handleMouseShooting(double time, double x_mouse, double y_mouse, 
                                     bool l_mouse_pressed, bool r_mouse_pressed,
                                     std::vector<std::unique_ptr<Projectile>>& projectiles,
                                     std::vector<std::unique_ptr<LightProjectile>>& light_projectiles) {
    // Right mouse button - Heavy projectiles
    if (r_mouse_pressed) {
        if ((last_shoot_time_ == 0.0 || time - last_shoot_time_ > 1.0) && (player_->bullets > 0)) {
            player_->bullets -= 1;
            
            glm::vec3 shoot_position = glm::vec3(player_->position.x, player_->position.y, player_->position.z + 0.15f);
            glm::vec3 shoot_direction = calculateShootDirection(x_mouse, y_mouse);
            
            auto new_shot = std::make_unique<Projectile>(phong_shader_, shoot_position, shoot_direction, 
                                                        glm::vec3(x_mouse, y_mouse, 0.0f));
            scene_root_->add(new_shot->node);
            projectiles.push_back(std::move(new_shot));
            last_shoot_time_ = time;
        }
    }
    
    // Left mouse button - Light projectiles
    if (l_mouse_pressed) {
        if (last_shoot_time_l_ == 0.0 || time - last_shoot_time_l_ > 0.1) {
            glm::vec3 shoot_position = glm::vec3(player_->position.x, player_->position.y, player_->position.z + 0.15f);
            glm::vec3 shoot_direction = calculateShootDirection(x_mouse, y_mouse);
            
            auto new_shot = std::make_unique<LightProjectile>(phong_shader_, shoot_position, shoot_direction, 
                                                             glm::vec3(x_mouse, y_mouse, 0.0f));
            scene_root_->add(new_shot->node);
            light_projectiles.push_back(std::move(new_shot));
            last_shoot_time_l_ = time;
        }
    }
}

void InputHandler::handleDeveloperModeKeys(const std::unordered_map<int, std::pair<bool, double>>& keyStates,
                                         bool& dev_mode) {
    if (keyStates.find(GLFW_KEY_T) != keyStates.end() && keyStates.at(GLFW_KEY_T).first) {
        activateDevMode(dev_mode);
    } else if (keyStates.find(GLFW_KEY_G) != keyStates.end() && keyStates.at(GLFW_KEY_G).first) {
        deactivateDevMode(dev_mode);
    }
}

void InputHandler::handleDebugToggleKeys(const std::unordered_map<int, std::pair<bool, double>>& keyStates,
                                       bool& paused, bool& invincible) {
    // Pause toggle (V key)
    if (keyStates.find(GLFW_KEY_V) != keyStates.end() && keyStates.at(GLFW_KEY_V).first && !pause_key_pressed_) {
        paused = !paused;
        pause_key_pressed_ = true;
    } else if (keyStates.find(GLFW_KEY_V) == keyStates.end() || !keyStates.at(GLFW_KEY_V).first) {
        pause_key_pressed_ = false;
    }
    
    // Invincibility toggle (B key)
    if (keyStates.find(GLFW_KEY_B) != keyStates.end() && keyStates.at(GLFW_KEY_B).first && !invincible_key_pressed_) {
        invincible = !invincible;
        invincible_key_pressed_ = true;
    } else if (keyStates.find(GLFW_KEY_B) == keyStates.end() || !keyStates.at(GLFW_KEY_B).first) {
        invincible_key_pressed_ = false;
    }
    
    // Debug collision spheres toggle (D key)
    if (keyStates.find(GLFW_KEY_D) != keyStates.end() && keyStates.at(GLFW_KEY_D).first && !debug_key_pressed_) {
        player_->showCollisionDebug = !player_->showCollisionDebug;
        if (player_->showCollisionDebug) {
            player_->addDebugSpheresToScene(scene_root_);
        } else {
            player_->removeDebugSpheresFromScene(scene_root_);
        }
        debug_key_pressed_ = true;
    } else if (keyStates.find(GLFW_KEY_D) == keyStates.end() || !keyStates.at(GLFW_KEY_D).first) {
        debug_key_pressed_ = false;
    }
}

void InputHandler::handleBulletMovementKeys(const std::unordered_map<int, std::pair<bool, double>>& keyStates,
                                          std::vector<std::unique_ptr<Node>>& bullets) {
    // W key - Move bullets forward
    if (keyStates.find(GLFW_KEY_W) != keyStates.end() && keyStates.at(GLFW_KEY_W).first) {
        for (auto it = bullets.begin(); it != bullets.end(); ++it) {
            auto& bulletNode = *it;
            bulletNode->transform_[3].z += 0.01f;
        }
    }
    
    // S key - Move bullets backward
    if (keyStates.find(GLFW_KEY_S) != keyStates.end() && keyStates.at(GLFW_KEY_S).first) {
        for (auto it = bullets.begin(); it != bullets.end(); ++it) {
            auto& bulletNode = *it;
            bulletNode->transform_[3].z -= 0.01f;
        }
    }
}

void InputHandler::handlePlayerMovement(const std::unordered_map<int, std::pair<bool, double>>& keyStates, 
                                      double time, bool dev_mode) {
    float speed = player_->movement_speed * player_->fps_correction;
    
    // U key - Move Forward
    if (keyStates.find(GLFW_KEY_U) != keyStates.end() && keyStates.at(GLFW_KEY_U).first) {
        idle_ud_ = false;
        float smoother = calculateMovementSmoother(keyStates.at(GLFW_KEY_U).second, time);
        
        if (player_->position.y + smoother * speed >= -1.0) {
            player_->position -= smoother * glm::vec3(0.0f, speed, 0.0f);
        }
        
        if (!dev_mode && camera_->cameraPos.y - player_->position.y >= 0.04) {
            camera_->cameraPos.y = player_->position.y + 0.04f;
        }
        
        if (player_->xAngle < 15.0f) {
            player_->xAngle += smoother * 1.0f;
        }
    }
    
    // J key - Move Backward  
    if (keyStates.find(GLFW_KEY_J) != keyStates.end() && keyStates.at(GLFW_KEY_J).first) {
        idle_ud_ = false;
        float smoother = calculateMovementSmoother(keyStates.at(GLFW_KEY_J).second, time);
        
        if (player_->position.y + smoother * speed <= 1.0) {
            player_->position += smoother * glm::vec3(0.0f, speed, 0.0f);
        }
        
        if (!dev_mode && camera_->cameraPos.y - player_->position.y <= -0.04) {
            camera_->cameraPos.y = player_->position.y + -0.04f;
        }
        
        if (player_->xAngle > -15.0f) {
            player_->xAngle -= smoother * 1.0f;
        }
    }
    
    // H key - Move Left
    if (keyStates.find(GLFW_KEY_H) != keyStates.end() && keyStates.at(GLFW_KEY_H).first) {
        idle_lr_ = false;
        float smoother = calculateMovementSmoother(keyStates.at(GLFW_KEY_H).second, time);
        
        if (player_->position.x + smoother * speed <= 1.5) {
            player_->position += smoother * glm::vec3(speed, 0.0f, 0.0f);
        }
        
        if (!dev_mode && camera_->cameraPos.x - player_->position.x <= -0.06) {
            camera_->cameraPos.x = player_->position.x + -0.06f;
        }
        
        if (player_->zAngle > -15.0f && !is_rotating_) {
            player_->zAngle -= smoother * 1.0f;
        }
        if (player_->yAngle < 15.0f) {
            player_->yAngle += smoother * 1.0f;
        }
    }
    
    // K key - Move Right
    if (keyStates.find(GLFW_KEY_K) != keyStates.end() && keyStates.at(GLFW_KEY_K).first) {
        idle_lr_ = false;
        float smoother = calculateMovementSmoother(keyStates.at(GLFW_KEY_K).second, time);
        
        if (player_->position.x - smoother * speed >= -1.5) {
            player_->position -= smoother * glm::vec3(speed, 0.0f, 0.0f);
        }
        
        if (!dev_mode && camera_->cameraPos.x - player_->position.x >= 0.06) {
            camera_->cameraPos.x = player_->position.x + 0.06f;
        }
        
        if (player_->zAngle < 15.0f && !is_rotating_) {
            player_->zAngle += smoother * 1.0f;
        }
        if (player_->yAngle > -15.0f) {
            player_->yAngle -= smoother * 1.0f;
        }
    }
}

void InputHandler::handlePlayerRotation(const std::unordered_map<int, std::pair<bool, double>>& keyStates, 
                                      double time) {
    // O key - Rotate left
    if (keyStates.find(GLFW_KEY_O) != keyStates.end() && keyStates.at(GLFW_KEY_O).first) {
        is_rotating_ = true;
        idle_rot_ = false;
        
        float smoother = calculateMovementSmoother(keyStates.at(GLFW_KEY_O).second, time);
        if (player_->zAngle > -90.0f) {
            player_->zAngle -= smoother * 1.8f;
        }
    }
    
    // P key - Rotate right
    if (keyStates.find(GLFW_KEY_P) != keyStates.end() && keyStates.at(GLFW_KEY_P).first) {
        is_rotating_ = true;
        idle_rot_ = false;
        
        float smoother = calculateMovementSmoother(keyStates.at(GLFW_KEY_P).second, time);
        if (player_->zAngle < 90.0f) {
            player_->zAngle += smoother * 1.8f;
        }
    }
}

void InputHandler::handleIdleAnimations(const std::unordered_map<int, std::pair<bool, double>>& keyStates, 
                                      double time, bool is_boost_mode) {
    // Handle idle animations when no movement keys are pressed
    bool up_down_pressed = (keyStates.find(GLFW_KEY_U) != keyStates.end() && keyStates.at(GLFW_KEY_U).first) ||
                          (keyStates.find(GLFW_KEY_J) != keyStates.end() && keyStates.at(GLFW_KEY_J).first);
    bool left_right_pressed = (keyStates.find(GLFW_KEY_H) != keyStates.end() && keyStates.at(GLFW_KEY_H).first) ||
                             (keyStates.find(GLFW_KEY_K) != keyStates.end() && keyStates.at(GLFW_KEY_K).first);
    bool rotation_pressed = (keyStates.find(GLFW_KEY_O) != keyStates.end() && keyStates.at(GLFW_KEY_O).first) ||
                           (keyStates.find(GLFW_KEY_P) != keyStates.end() && keyStates.at(GLFW_KEY_P).first);
    
    // Left/Right idle animation
    if (!left_right_pressed) {
        if (!idle_lr_ && player_->yAngle != 0.0f) {
            idle_lr_ = true;
            idle_start_lr_ = time;
        }
        
        if (idle_lr_ && (time - idle_start_lr_) <= 1.0) {
            if (!is_rotating_) {
                player_->zAngle *= cos(glm::radians(90.0f * (time - idle_start_lr_)));
            }
            player_->yAngle *= cos(glm::radians(90.0f * (time - idle_start_lr_)));
        } else if (idle_lr_) {
            idle_lr_ = false;
            if (!is_rotating_) {
                player_->zAngle = 0.0f;
            }
            player_->yAngle = 0.0f;
        }
    }
    
    // Up/Down idle animation
    if (!up_down_pressed) {
        if (!idle_ud_ && player_->xAngle != 0.0f) {
            idle_ud_ = true;
            idle_start_ud_ = time;
        }
        
        if (idle_ud_ && (time - idle_start_ud_) <= 1.0) {
            player_->xAngle *= cos(glm::radians(90.0f * (time - idle_start_ud_)));
        } else if (idle_ud_) {
            idle_ud_ = false;
            player_->xAngle = 0.0f;
        }
    }
    
    // Rotation idle animation
    if (!rotation_pressed) {
        if (is_rotating_ && player_->zAngle != 0.0f) {
            idle_rot_ = true;
            is_rotating_ = false;
            idle_start_rot_ = time;
        }
        
        // Complex rotation idle logic based on movement keys
        if (keyStates.find(GLFW_KEY_H) != keyStates.end() && keyStates.at(GLFW_KEY_H).first &&
            (keyStates.find(GLFW_KEY_K) == keyStates.end() || !keyStates.at(GLFW_KEY_K).first)) {
            if (idle_rot_ && player_->zAngle <= -16.8f) {
                player_->zAngle += 1.8f;
            } else if (idle_rot_ && player_->zAngle >= -13.2f) {
                player_->zAngle -= 1.8f;
            } else if (idle_rot_) {
                is_rotating_ = false;
                idle_rot_ = false;
                player_->zAngle = -15.0f;
            }
        } else if ((keyStates.find(GLFW_KEY_H) == keyStates.end() || !keyStates.at(GLFW_KEY_H).first) &&
                   keyStates.find(GLFW_KEY_K) != keyStates.end() && keyStates.at(GLFW_KEY_K).first) {
            if (idle_rot_ && player_->zAngle <= 13.2f) {
                player_->zAngle += 1.8f;
            } else if (idle_rot_ && player_->zAngle >= 16.8f) {
                player_->zAngle -= 1.8f;
            } else if (idle_rot_) {
                is_rotating_ = false;
                idle_rot_ = false;
                player_->zAngle = 15.0f;
            }
        } else {
            if (idle_rot_ && (time - idle_start_rot_) <= 1.0) {
                player_->zAngle *= cos(glm::radians(90.0f * (time - idle_start_rot_)));
            } else if (idle_rot_) {
                is_rotating_ = false;
                idle_rot_ = false;
                player_->zAngle = 0.0f;
            }
        }
    }
    
    // Update damage animation
    player_->updateDamageAnimation(time);
    player_->updatePosition();
    
    // Update engine flames
    static double lastFlameTime = 0.0;
    float deltaTime = (lastFlameTime == 0.0) ? 0.016f : (float)(time - lastFlameTime);
    lastFlameTime = time;
    player_->updateEngineFlames(deltaTime, is_boost_mode);
}

float InputHandler::calculateMovementSmoother(double key_press_time, double current_time) {
    if (current_time - key_press_time < 1.0) {
        return sin(glm::radians(90 * (current_time - key_press_time)));
    }
    return 1.0f;
}

glm::vec3 InputHandler::calculateShootDirection(double mouse_x, double mouse_y) {
    double xpos = (mouse_x / window_width_) - 0.5;
    double ypos = (mouse_y / window_height_) - 0.5;
    
    return glm::vec3(
        -xpos + camera_->cameraPos.x - player_->position.x,
        -ypos + camera_->cameraPos.y - player_->position.y,
        1.0f
    );
}

void InputHandler::activateDevMode(bool& dev_mode) {
    dev_mode = true;
}

void InputHandler::deactivateDevMode(bool& dev_mode) {
    dev_mode = false;

    camera_->cameraFront = glm::vec3(0.0f, 0.0f, 1.0f);
    camera_->cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    camera_->cameraPos = player_->position + glm::vec3(0.0f, 0.05f, -0.3f);
}