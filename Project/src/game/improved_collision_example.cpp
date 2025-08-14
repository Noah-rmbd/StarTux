// EXAMPLE: How to modernize your collision detection code
// This shows the BEFORE and AFTER comparison

#include "game_utils.h"
#include <vector>
#include <memory>

// ===========================================
// BEFORE: Your current dangerous code
// ===========================================
void Game::colisions_player_ring_OLD_WAY(double time) {
    // Dangerous manual iterator management
    for(auto it = rings_.begin(); it != rings_.end();) {
        Ring* ring = *it;  // Raw pointer - dangerous!
        
        // Collision check...
        if (collision && !ring->animating && !ring->collected) {
            // Process collision...
            
            // Manual cleanup - error prone
            world_node->remove(ringNode);
            delete ring;  // Easy to forget or double-delete!
            it = rings_.erase(it);
        } else {
            ++it;  // Easy to forget and cause infinite loop!
        }
    }
}

// ===========================================
// AFTER: Modern, safe approach
// ===========================================
void Game::colisions_player_ring_NEW_WAY(double time) {
    // Step 1: Update all rings safely
    GameUtils::ProcessObjects(rings_, [time](auto& ring) {
        ring->update(time);
    });
    
    // Step 2: Process collisions safely
    for (auto& ring : rings_) {  // Range-based loop - much cleaner!
        if (!ring->collected && !ring->animating) {
            glm::vec3 ring_pos = getRingPosition(*ring);
            
            if (checkCollisionWithPlayer(ring_pos)) {
                processRingCollection(*ring, time);
            }
        }
    }
    
    // Step 3: Remove dead rings safely (no manual iterator management!)
    GameUtils::RemoveDeadObjects(rings_, [](const auto& ring) {
        return ring->to_delete || ring->shouldRemove();
    });
    // Dead rings are automatically cleaned up! No memory leaks possible!
}

// ===========================================
// Helper functions - much cleaner separation
// ===========================================
glm::vec3 Game::getRingPosition(const Ring& ring) {
    const auto& transform = ring.ring_node->transform_;
    return glm::vec3(transform[3].x, transform[3].y, transform[3].z);
}

bool Game::checkCollisionWithPlayer(const glm::vec3& ring_pos) {
    double x = player->position.x - ring_pos.x;
    double y = player->position.y - ring_pos.y;
    double z = player->position.z - ring_pos.z;
    
    return (sqrt(x * x + y * y) < 0.055 && z > 0.03 && z < 0.05);
}

void Game::processRingCollection(Ring& ring, double time) {
    ring.collected = true;
    
    if (!is_boost_mode) {
        is_boost_mode = true;
        boost_time = time;
        asteroid_speed *= 2.0f;
        player->shipState = Player::ACCELERATING;
        hud->newDialog(hud->ACCELERATION_1, time);
        
        int currentSpeed = -asteroid_speed * 50;
        player->gameStats->recordSpeedReached(currentSpeed);
    } else {
        boost_time = time;
    }
    
    ring.startAnimation(time);
    player->gameStats->recordRingTaken();
    
    if (dailyMissions) {
        dailyMissions->recordRingCollected();
        dailyMissions->recordConsecutiveRings(player->gameStats->getCurrentConsecutiveRings());
    }
}

// ===========================================
// Even more advanced: Generic collision system
// ===========================================
template<typename EntityA, typename EntityB>
void Game::processCollisions(
    std::vector<std::unique_ptr<EntityA>>& entitiesA,
    std::vector<std::unique_ptr<EntityB>>& entitiesB,
    std::function<bool(const EntityA&, const EntityB&)> collisionCheck,
    std::function<void(EntityA&, EntityB&)> onCollision) {
    
    for (auto& entityA : entitiesA) {
        for (auto& entityB : entitiesB) {
            if (collisionCheck(*entityA, *entityB)) {
                onCollision(*entityA, *entityB);
            }
        }
    }
}

// Usage example:
void Game::checkProjectileAsteroidCollisions() {
    processCollisions(projectiles, asteroids_,
        // Collision check
        [](const Projectile& proj, const Asteroid& ast) {
            return proj.checkCollision(ast.getPosition());
        },
        // What to do on collision
        [this](Projectile& proj, Asteroid& ast) {
            ast.life -= 1;
            if (ast.life <= 0) {
                createExplosion(ast.getPosition());
                player->gameStats->recordAsteroidDestroyed(ast.is_moving);
                ast.markForDeletion();
            }
            proj.setInactive();
        }
    );
}