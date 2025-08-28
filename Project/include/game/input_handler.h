#pragma once

#include "player.h"
#include "camera.h"
#include "projectile.h"
#include "light_projectile.h"
#include "node.h"
#include "shader.h"
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <vector>
#include <memory>

class InputHandler {
public:
    InputHandler(Player* player, Camera* camera, Node* scene_root, Shader* phong_shader,
                int window_width, int window_height);

    void handleInput(std::unordered_map<int, std::pair<bool, double>> keyStates, double time,
                    bool dev_mode, double x_mouse, double y_mouse, bool l_mouse_pressed, 
                    bool r_mouse_pressed, std::vector<std::unique_ptr<Projectile>>& projectiles,
                    std::vector<std::unique_ptr<LightProjectile>>& light_projectiles,
                    std::vector<std::unique_ptr<Node>>& bullets, bool& paused, bool& invincible,
                    bool& is_boost_mode);

    void handleMouseShooting(double time, double x_mouse, double y_mouse, bool l_mouse_pressed,
                           bool r_mouse_pressed, std::vector<std::unique_ptr<Projectile>>& projectiles,
                           std::vector<std::unique_ptr<LightProjectile>>& light_projectiles);

    void activateDevMode(bool& dev_mode);
    void deactivateDevMode(bool& dev_mode);

private:
    void handleDeveloperModeKeys(const std::unordered_map<int, std::pair<bool, double>>& keyStates,
                               bool& dev_mode);
    void handleDebugToggleKeys(const std::unordered_map<int, std::pair<bool, double>>& keyStates,
                             bool& paused, bool& invincible);
    void handleBulletMovementKeys(const std::unordered_map<int, std::pair<bool, double>>& keyStates,
                                std::vector<std::unique_ptr<Node>>& bullets);
    void handlePlayerMovement(const std::unordered_map<int, std::pair<bool, double>>& keyStates, 
                            double time, bool dev_mode);
    void handlePlayerRotation(const std::unordered_map<int, std::pair<bool, double>>& keyStates, 
                            double time);
    void handleIdleAnimations(const std::unordered_map<int, std::pair<bool, double>>& keyStates, 
                            double time, bool is_boost_mode);

    float calculateMovementSmoother(double key_press_time, double current_time);
    glm::vec3 calculateShootDirection(double mouse_x, double mouse_y);

    Player* player_;
    Camera* camera_;
    Node* scene_root_;
    Shader* phong_shader_;
    int window_width_;
    int window_height_;

    // Shooting timing
    double last_shoot_time_ = 0.0;
    double last_shoot_time_l_ = 0.0;

    // Rotation and idle state
    bool is_rotating_ = false;
    bool idle_rot_ = false;
    bool idle_lr_ = false;
    bool idle_ud_ = false;
    double idle_start_lr_ = 0.0;
    double idle_start_ud_ = 0.0;
    double idle_start_rot_ = 0.0;

    // Key press states for toggles
    bool pause_key_pressed_ = false;
    bool invincible_key_pressed_ = false;
    bool debug_key_pressed_ = false;
};