#pragma once

#include "shader.h"
#include "player.h"
#include "asteroid.h"
#include "projectile.h"
#include "light_projectile.h"
#include "ring.h"
#include "explosion.h"
#include "node.h"
#include "hud.h"
#include "missions.h"
#include <vector>
#include <memory>

class CollisionManager {
public:
    CollisionManager(Player* player, Node* world_node, Node* scene_root, Hud* hud, 
                    DailyMissions* missions, float& asteroid_speed, bool& is_boost_mode, 
                    double& boost_time, bool& lost, bool invincible, Shader* phong_shader);

    void detectCollisions(double time, 
                         std::vector<std::unique_ptr<Asteroid>>& asteroids,
                         std::vector<std::unique_ptr<Node>>& bullets,
                         std::vector<std::unique_ptr<Ring>>& rings,
                         std::vector<std::unique_ptr<Projectile>>& projectiles,
                         std::vector<std::unique_ptr<LightProjectile>>& light_projectiles,
                         std::vector<std::unique_ptr<Explosion>>& explosions);

    void createExplosion(glm::vec3 position, double time, 
                        std::vector<std::unique_ptr<Explosion>>& explosions);
    void spawnBullet(glm::vec3 position, std::vector<std::unique_ptr<Node>>& bullets, 
                    Shape* bullet);

private:
    void collisionsBetweenAsteroids(double time, std::vector<std::unique_ptr<Asteroid>>& asteroids,
                                   std::vector<std::unique_ptr<Explosion>>& explosions);
    void collisionsPlayerAsteroids(double time, std::vector<std::unique_ptr<Asteroid>>& asteroids,
                                  std::vector<std::unique_ptr<Explosion>>& explosions);
    void collisionsPlayerBullet(double time, std::vector<std::unique_ptr<Node>>& bullets);
    void collisionsPlayerRing(double time, std::vector<std::unique_ptr<Ring>>& rings);
    void collisionsLprojectileAsteroid(double time, 
                                      std::vector<std::unique_ptr<Asteroid>>& asteroids,
                                      std::vector<std::unique_ptr<LightProjectile>>& light_projectiles,
                                      std::vector<std::unique_ptr<Node>>& bullets,
                                      Shape* bullet,
                                      std::vector<std::unique_ptr<Explosion>>& explosions);
    void collisionsProjectileAsteroid(double time, 
                                     std::vector<std::unique_ptr<Asteroid>>& asteroids,
                                     std::vector<std::unique_ptr<Projectile>>& projectiles,
                                     std::vector<std::unique_ptr<Explosion>>& explosions);

    Player* player_;
    Node* world_node_;
    Node* scene_root_;
    Hud* hud_;
    DailyMissions* dailyMissions_;
    float& asteroid_speed_;
    bool& is_boost_mode_;
    double& boost_time_;
    bool& lost_;
    bool invincible_;
    Shader* phong_shader_;
};