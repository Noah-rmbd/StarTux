#include "collision_manager.h"
#include "real_profiler.h"
#include "lighting.h"
#include "shape_model.h"
#include <iostream>
#include <algorithm>

CollisionManager::CollisionManager(Player* player, Node* world_node, Node* scene_root, 
                                  Hud* hud, DailyMissions* missions, float& asteroid_speed, 
                                  bool& is_boost_mode, double& boost_time, bool& lost, 
                                  bool invincible, Shader* phong_shader)
    : player_(player), world_node_(world_node), scene_root_(scene_root), hud_(hud),
      dailyMissions_(missions), asteroid_speed_(asteroid_speed), is_boost_mode_(is_boost_mode),
      boost_time_(boost_time), lost_(lost), invincible_(invincible), phong_shader_(phong_shader) {
}

void CollisionManager::detectCollisions(double time,
                                       std::vector<std::unique_ptr<Asteroid>>& asteroids,
                                       std::vector<std::unique_ptr<Node>>& bullets,
                                       std::vector<std::unique_ptr<Ring>>& rings,
                                       std::vector<std::unique_ptr<Projectile>>& projectiles,
                                       std::vector<std::unique_ptr<LightProjectile>>& light_projectiles,
                                       std::vector<std::unique_ptr<Explosion>>& explosions) {
    PROFILE_SCOPE("detect_colisions");
    
    collisionsBetweenAsteroids(time, asteroids, explosions);
    collisionsPlayerAsteroids(time, asteroids, explosions);
    collisionsPlayerBullet(time, bullets);
    collisionsPlayerRing(time, rings);
    collisionsLprojectileAsteroid(time, asteroids, light_projectiles, bullets, nullptr, explosions);
    collisionsProjectileAsteroid(time, asteroids, projectiles, explosions);
}

void CollisionManager::collisionsBetweenAsteroids(double time, 
                                                 std::vector<std::unique_ptr<Asteroid>>& asteroids,
                                                 std::vector<std::unique_ptr<Explosion>>& explosions) {
    for(size_t i = 0; i < asteroids.size(); ++i) {
        auto& asteroid1 = asteroids[i];
        Node* node1 = asteroid1->asteroid_node;
        glm::vec3 pos1 = glm::vec3(node1->transform_[3].x, node1->transform_[3].y, node1->transform_[3].z);
        
        for(size_t j = i + 1; j < asteroids.size(); ++j) {
            auto& asteroid2 = asteroids[j];
            Node* node2 = asteroid2->asteroid_node;
            glm::vec3 pos2 = glm::vec3(node2->transform_[3].x, node2->transform_[3].y, node2->transform_[3].z);

            double x = (pos2.x - pos1.x);
            double y = (pos2.y - pos1.y);
            double z = (pos2.z - pos1.z);

            if (x * x + y * y + z * z < 0.04f) {
                world_node_->remove(node1);
                world_node_->remove(node2);

                if (j > i) {
                    asteroids.erase(asteroids.begin() + j);
                    asteroids.erase(asteroids.begin() + i);
                } else {
                    asteroids.erase(asteroids.begin() + i);
                    asteroids.erase(asteroids.begin() + j);
                }

                glm::vec3 explosion_point = glm::vec3(x, y, z) / 2.0f + pos1;
                createExplosion(explosion_point, time, explosions);

                i = -1;
                break;
            }
        }
    }
}

void CollisionManager::collisionsPlayerAsteroids(double time, 
                                                std::vector<std::unique_ptr<Asteroid>>& asteroids,
                                                std::vector<std::unique_ptr<Explosion>>& explosions) {
    for(auto it = asteroids.begin(); it != asteroids.end();) {
        auto& asteroid = *it;
        Node* node = asteroid->asteroid_node;
        glm::vec3 asteroid_position = glm::vec3(node->transform_[3].x, node->transform_[3].y, node->transform_[3].z);
        
        float asteroidRadius = 0.10f;
        int collisionPointIndex = player_->checkCollisionPoint(asteroid_position, asteroidRadius);
        
        if (collisionPointIndex >= 0) {
            world_node_->remove(node);
            it = asteroids.erase(it);
            createExplosion(asteroid_position, time, explosions);
            
            if (!invincible_ && !player_->shieldIsActive) {
                Player::ShipState damageType = player_->collisionPoints[collisionPointIndex].damageType;
                player_->damageWithType(time, damageType);
                player_->shipState = damageType;
                hud_->newDialog(hud_->COLLISION_1, time);
                
                if (dailyMissions_) {
                    dailyMissions_->recordDamage();
                }

                if (is_boost_mode_) {
                    is_boost_mode_ = false;
                    asteroid_speed_ /= 2;
                }

                if (player_->isDead() && !player_->isDeathAnimationActive()) {
                    player_->startDeathAnimation(time);
                }
            }
        } else if(asteroid_position.z < 0.0) {
            world_node_->remove(node);
            it = asteroids.erase(it);
        } else {
            ++it;
        }
    }
}

void CollisionManager::collisionsPlayerBullet(double time, std::vector<std::unique_ptr<Node>>& bullets) {
    for(auto it = bullets.begin(); it != bullets.end();) {
        auto& bulletNode = *it;
        
        glm::vec3 bullet_position = glm::vec3(bulletNode->transform_[3].x, bulletNode->transform_[3].y, bulletNode->transform_[3].z);
        double x = (player_->position.x - bullet_position.x);
        double y = (player_->position.y - bullet_position.y);
        double z = (player_->position.z - bullet_position.z);
        
        if (x * x + y * y + z * z < 0.01f) {
            world_node_->remove(bulletNode.get());
            it = bullets.erase(it);
            player_->increaseBullets();
        } else {
            if (bullet_position.z < -0.2f) {
                world_node_->remove(bulletNode.get());
                it = bullets.erase(it);
            } else {
                ++it;
            }
        }
    }
}

void CollisionManager::collisionsPlayerRing(double time, std::vector<std::unique_ptr<Ring>>& rings) {
    for(auto it = rings.begin(); it != rings.end();) {
        auto& ring = *it;
        Node* ringNode = ring->ring_node;

        glm::vec3 ring_position = glm::vec3(ringNode->transform_[3].x, ringNode->transform_[3].y, ringNode->transform_[3].z);
        double x = (player_->position.x - ring_position.x);
        double y = (player_->position.y - ring_position.y);
        double z = (player_->position.z - ring_position.z);
        
        ring->update(time);
        
        bool collision = (x * x + y * y < 0.003025f && z > 0.03 && z < 0.05);

        if (collision && !ring->animating && !ring->collected) {
            ring->collected = true;
            
            if (!is_boost_mode_) {
                PROFILE_SCOPE("Ring Boost Activation");
                is_boost_mode_ = true;
                boost_time_ = time;
                
                float old_speed = asteroid_speed_;
                asteroid_speed_ *= 2.0f;
                
                if (world_node_ && world_node_->velocity_.z != 0.0f) {
                    world_node_->velocity_.z *= 2.0f;
                }
                
                player_->shipState = player_->ACCELERATING;
                hud_->newDialog(hud_->ACCELERATION_1, time);
                
                std::cout << "Boost activated - speed: " << old_speed << " -> " << asteroid_speed_ << std::endl;
                
                int currentSpeed = -asteroid_speed_ * 50;
                player_->gameStats->recordSpeedReached(currentSpeed);
            } else {
                boost_time_ = time;
            }
            ring->startAnimation(time);
            
            player_->gameStats->recordRingTaken();
            
            if (dailyMissions_) {
                dailyMissions_->recordRingCollected();
                dailyMissions_->recordConsecutiveRings(player_->gameStats->getCurrentConsecutiveRings());
            }
        }
        
        if (ring->to_delete || ring_position.z < -0.1f) {
            if (!ring->animating && ring_position.z < -0.1f) {
                player_->gameStats->recordRingMissed();
            }
            
            world_node_->remove(ringNode);
            it = rings.erase(it);
        } else {
            ++it;
        }
    }
}

void CollisionManager::collisionsLprojectileAsteroid(double time,
                                                    std::vector<std::unique_ptr<Asteroid>>& asteroids,
                                                    std::vector<std::unique_ptr<LightProjectile>>& light_projectiles,
                                                    std::vector<std::unique_ptr<Node>>& bullets,
                                                    Shape* bullet,
                                                    std::vector<std::unique_ptr<Explosion>>& explosions) {
    for(auto it = light_projectiles.begin(); it != light_projectiles.end();) {
        auto& shoot = *it;
        shoot->update(time);

        for(auto asteroid_it = asteroids.begin(); asteroid_it != asteroids.end(); ++asteroid_it) {
            auto& asteroid = *asteroid_it;
            Node* node = asteroid->asteroid_node;
            glm::vec3 asteroid_position = glm::vec3(node->transform_[3].x, node->transform_[3].y, node->transform_[3].z);
            
            if (shoot->checkCollision(asteroid_position)) {
                asteroid->life -= 1;
                if (asteroid->life <= 0) {
                    createExplosion(asteroid_position, time, explosions);
                    if (rand() % 3 == 0) {
                        spawnBullet(asteroid_position, bullets, bullet);
                    }
                    
                    if (asteroid->is_moving) {
                        player_->score += 100.0;
                        hud_->scoreIncrement(shoot->cursorPosition.x, shoot->cursorPosition.y, time, 100);
                    } else {
                        player_->score += 50.0;
                        hud_->scoreIncrement(shoot->cursorPosition.x, shoot->cursorPosition.y, time, 50);
                    }
                    
                    player_->gameStats->recordAsteroidDestroyed(asteroid->is_moving);
                    
                    if (dailyMissions_) {
                        dailyMissions_->recordAsteroidDestroyed();
                        if (asteroid->is_moving) {
                            dailyMissions_->recordMovingAsteroidDestroyed();
                        }
                    }
                    
                    world_node_->remove(node);
                    asteroid_it = asteroids.erase(asteroid_it);
                }
                shoot->active = false;
                break;
            }
        }

        if (!shoot->active) {
            scene_root_->remove(shoot->node);
            it = light_projectiles.erase(it);
        } else {
            ++it;
        }
    }
}

void CollisionManager::collisionsProjectileAsteroid(double time,
                                                   std::vector<std::unique_ptr<Asteroid>>& asteroids,
                                                   std::vector<std::unique_ptr<Projectile>>& projectiles,
                                                   std::vector<std::unique_ptr<Explosion>>& explosions) {
    for(auto it = projectiles.begin(); it != projectiles.end();) {
        auto& shoot = *it;
        shoot->update(time);

        for(auto it2 = asteroids.begin(); it2 != asteroids.end();) {
            auto& asteroid = *it2;
            Node* node = asteroid->asteroid_node;
            glm::vec3 asteroid_position = glm::vec3(node->transform_[3].x, node->transform_[3].y, node->transform_[3].z);
            
            if (shoot->checkCollision(asteroid_position)) {
                if (asteroid->is_moving) {
                    player_->score += 100.0;
                    hud_->scoreIncrement(shoot->cursorPosition.x, shoot->cursorPosition.y, time, 100);
                } else {
                    player_->score += 50.0;
                    hud_->scoreIncrement(shoot->cursorPosition.x, shoot->cursorPosition.y, time, 50);
                }
                hud_->newDialog(hud_->SHOOT_1, time);
                
                player_->gameStats->recordAsteroidDestroyed(asteroid->is_moving);
                
                if (dailyMissions_) {
                    dailyMissions_->recordAsteroidDestroyed();
                    if (asteroid->is_moving) {
                        dailyMissions_->recordMovingAsteroidDestroyed();
                    }
                }
                
                createExplosion(asteroid_position, time, explosions);
                std::cout << "DEBUG: Removing asteroid node from world_node" << std::endl;
                world_node_->remove(node);
                std::cout << "DEBUG: Erasing asteroid from vector" << std::endl;
                it2 = asteroids.erase(it2);
                shoot->active = false;
                std::cout << "DEBUG: Asteroid collision processed, breaking" << std::endl;
                break;
            } else {
                ++it2;
            }
        }

        if (!shoot->active) {
            scene_root_->remove(shoot->node);
            it = projectiles.erase(it);
        } else {
            ++it;
        }
    }
}

void CollisionManager::createExplosion(glm::vec3 position, double time, 
                                     std::vector<std::unique_ptr<Explosion>>& explosions) {
    auto explosion = std::make_unique<Explosion>(phong_shader_, position, time);
    scene_root_->add(explosion->getNode());
    explosions.push_back(std::move(explosion));
    
    if (g_LightingSystem) {
        g_LightingSystem->AddExplosionLight(position, 5.0f, 2.0f);
    }
}

void CollisionManager::spawnBullet(glm::vec3 position, std::vector<std::unique_ptr<Node>>& bullets, 
                                  Shape* bullet) {
    glm::mat4 bullet_mat =
        glm::translate(glm::mat4(1.0f), position) *
        glm::scale(glm::mat4(1.0f), 2.5f * glm::vec3(1.0f, 1.0f, 1.0f));
    
    auto bulletNode = std::make_unique<Node>(bullet_mat);

    bulletNode->velocity_ = glm::vec3(0.0f, 0.0f, 0.0f);
    bulletNode->z_speed = &asteroid_speed_;
    bulletNode->add(bullet);
    
    world_node_->add(bulletNode.get());
    bullets.push_back(std::move(bulletNode));
}