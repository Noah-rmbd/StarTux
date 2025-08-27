#include "game.h"
#include <set>
#include <map>
#include <algorithm>

// Multi-threaded collision detection methods
void Game::colisions_projectile_asteroid_mt(double time) {
  if (projectiles.empty() || asteroids_.empty()) {
    return;
  }
  
  // Dynamic thread count adjustment based on workload
  size_t workload = projectiles.size() * asteroids_.size();
  size_t effective_threads = std::min(thread_count_, std::max(size_t(1), workload / 100));
  if (effective_threads != thread_count_ && workload > 1000) {
    // Only adjust for significant workloads to avoid overhead
    effective_threads = thread_count_;
  }
  
  // Struct to hold collision results
  struct CollisionHit {
    size_t projectile_idx;
    size_t asteroid_idx;
    float score;
    bool is_moving;
    glm::vec3 position;
    glm::vec3 cursor_pos;
  };
  
  std::vector<CollisionHit> hits;
  std::mutex hits_mutex;
  
  // Update all projectiles first (single-threaded to avoid issues)
  for (auto& proj : projectiles) {
    proj->update(time);
  }
  
  // Divide projectiles among threads
  std::vector<std::future<void>> futures;
  size_t projectiles_per_thread = std::max(size_t(1), projectiles.size() / effective_threads);
  
  for (size_t thread_id = 0; thread_id < effective_threads; ++thread_id) {
    size_t start_idx = thread_id * projectiles_per_thread;
    size_t end_idx = (thread_id == thread_count_ - 1) 
                     ? projectiles.size() 
                     : std::min(start_idx + projectiles_per_thread, projectiles.size());
                     
    if (start_idx >= projectiles.size()) break;
    
    auto future = std::async(std::launch::async, [&, start_idx, end_idx]() {
      std::vector<CollisionHit> thread_hits;
      
      for (size_t proj_idx = start_idx; proj_idx < end_idx; ++proj_idx) {
        const auto& projectile = projectiles[proj_idx];
        
        for (size_t ast_idx = 0; ast_idx < asteroids_.size(); ++ast_idx) {
          const auto& asteroid = asteroids_[ast_idx];
          Node* node = asteroid->asteroid_node;
          if (!node) continue;
          
          glm::vec3 asteroid_position = glm::vec3(
            node->transform_[3].x, node->transform_[3].y, node->transform_[3].z);
          
          if (projectile->checkCollision(asteroid_position)) {
            CollisionHit hit;
            hit.projectile_idx = proj_idx;
            hit.asteroid_idx = ast_idx;
            hit.position = asteroid_position;
            hit.cursor_pos = projectile->cursorPosition;
            hit.is_moving = asteroid->is_moving;
            hit.score = asteroid->is_moving ? 100.0f : 50.0f;
            thread_hits.push_back(hit);
            break; // Projectile can only hit one asteroid
          }
        }
      }
      
      // Add thread results to global hits (thread-safe)
      std::lock_guard<std::mutex> lock(hits_mutex);
      hits.insert(hits.end(), thread_hits.begin(), thread_hits.end());
    });
    
    futures.push_back(std::move(future));
  }
  
  // Wait for all threads to complete
  for (auto& future : futures) {
    future.get();
  }
  
  // Process hits (single-threaded to maintain game state consistency)
  std::set<size_t> projectiles_to_remove;
  std::set<size_t> asteroids_to_remove;
  
  for (const auto& hit : hits) {
    if (projectiles_to_remove.find(hit.projectile_idx) != projectiles_to_remove.end() ||
        asteroids_to_remove.find(hit.asteroid_idx) != asteroids_to_remove.end()) {
      continue; // Already processed
    }
    
    // Update score
    player->score += hit.score;
    hud->scoreIncrement(hit.cursor_pos.x, hit.cursor_pos.y, time, static_cast<int>(hit.score));
    hud->newDialog(hud->SHOOT_1, time);
    
    // Record statistics
    player->gameStats->recordAsteroidDestroyed(hit.is_moving);
    
    // Create explosion
    create_explosion(hit.position, time);
    
    // Spawn bullet chance
    if (rand() % 3 == 0) {
      spawn_bullet(hit.position);
    }
    
    projectiles_to_remove.insert(hit.projectile_idx);
    asteroids_to_remove.insert(hit.asteroid_idx);
  }
  
  // Remove hit projectiles and asteroids (in reverse order to maintain indices)
  for (auto it = projectiles_to_remove.rbegin(); it != projectiles_to_remove.rend(); ++it) {
    scene_root->remove(projectiles[*it]->node);
    projectiles.erase(projectiles.begin() + *it);
  }
  
  for (auto it = asteroids_to_remove.rbegin(); it != asteroids_to_remove.rend(); ++it) {
    world_node->remove(asteroids_[*it]->asteroid_node);
    asteroids_.erase(asteroids_.begin() + *it);
  }
  
  // Remove inactive projectiles
  for (auto it = projectiles.begin(); it != projectiles.end();) {
    if (!(*it)->active) {
      scene_root->remove((*it)->node);
      it = projectiles.erase(it);
    } else {
      ++it;
    }
  }
}

void Game::colisions_lprojectile_asteroid_mt(double time) {
  if (light_projectiles.empty() || asteroids_.empty()) {
    return;
  }
  
  struct LightCollisionHit {
    size_t projectile_idx;
    size_t asteroid_idx;
    glm::vec3 position;
    glm::vec3 cursor_pos;
    bool is_moving;
  };
  
  std::vector<LightCollisionHit> hits;
  std::mutex hits_mutex;
  
  // Update all light projectiles first
  for (auto& proj : light_projectiles) {
    proj->update(time);
  }
  
  // Divide light projectiles among threads
  std::vector<std::future<void>> futures;
  size_t projectiles_per_thread = std::max(size_t(1), light_projectiles.size() / thread_count_);
  
  for (size_t thread_id = 0; thread_id < thread_count_; ++thread_id) {
    size_t start_idx = thread_id * projectiles_per_thread;
    size_t end_idx = (thread_id == thread_count_ - 1) 
                     ? light_projectiles.size() 
                     : std::min(start_idx + projectiles_per_thread, light_projectiles.size());
                     
    if (start_idx >= light_projectiles.size()) break;
    
    auto future = std::async(std::launch::async, [&, start_idx, end_idx]() {
      std::vector<LightCollisionHit> thread_hits;
      
      for (size_t proj_idx = start_idx; proj_idx < end_idx; ++proj_idx) {
        const auto& projectile = light_projectiles[proj_idx];
        
        for (size_t ast_idx = 0; ast_idx < asteroids_.size(); ++ast_idx) {
          const auto& asteroid = asteroids_[ast_idx];
          Node* node = asteroid->asteroid_node;
          if (!node) continue;
          
          glm::vec3 asteroid_position = glm::vec3(
            node->transform_[3].x, node->transform_[3].y, node->transform_[3].z);
          
          if (projectile->checkCollision(asteroid_position)) {
            LightCollisionHit hit;
            hit.projectile_idx = proj_idx;
            hit.asteroid_idx = ast_idx;
            hit.position = asteroid_position;
            hit.cursor_pos = projectile->cursorPosition;
            hit.is_moving = asteroid->is_moving;
            thread_hits.push_back(hit);
            break; // Projectile can only hit one asteroid
          }
        }
      }
      
      std::lock_guard<std::mutex> lock(hits_mutex);
      hits.insert(hits.end(), thread_hits.begin(), thread_hits.end());
    });
    
    futures.push_back(std::move(future));
  }
  
  // Wait for all threads
  for (auto& future : futures) {
    future.get();
  }
  
  // Process hits
  std::set<size_t> projectiles_to_remove;
  std::map<size_t, int> asteroid_damage; // asteroid_idx -> damage
  
  for (const auto& hit : hits) {
    if (projectiles_to_remove.find(hit.projectile_idx) != projectiles_to_remove.end()) {
      continue;
    }
    
    asteroid_damage[hit.asteroid_idx]++;
    projectiles_to_remove.insert(hit.projectile_idx);
  }
  
  // Apply damage and remove destroyed asteroids
  std::set<size_t> asteroids_to_remove;
  for (const auto& damage_pair : asteroid_damage) {
    size_t ast_idx = damage_pair.first;
    int damage = damage_pair.second;
    
    asteroids_[ast_idx]->life -= damage;
    if (asteroids_[ast_idx]->life <= 0) {
      const auto& hit = *std::find_if(hits.begin(), hits.end(), 
        [ast_idx](const LightCollisionHit& h) { return h.asteroid_idx == ast_idx; });
      
      create_explosion(hit.position, time);
      if (rand() % 3 == 0) {
        spawn_bullet(hit.position);
      }
      
      float score = hit.is_moving ? 100.0f : 50.0f;
      player->score += score;
      hud->scoreIncrement(hit.cursor_pos.x, hit.cursor_pos.y, time, static_cast<int>(score));
      player->gameStats->recordAsteroidDestroyed(hit.is_moving);
      
      asteroids_to_remove.insert(ast_idx);
    }
  }
  
  // Remove projectiles and asteroids
  for (auto it = projectiles_to_remove.rbegin(); it != projectiles_to_remove.rend(); ++it) {
    scene_root->remove(light_projectiles[*it]->node);
    light_projectiles.erase(light_projectiles.begin() + *it);
  }
  
  for (auto it = asteroids_to_remove.rbegin(); it != asteroids_to_remove.rend(); ++it) {
    world_node->remove(asteroids_[*it]->asteroid_node);
    asteroids_.erase(asteroids_.begin() + *it);
  }
  
  // Remove inactive light projectiles
  for (auto it = light_projectiles.begin(); it != light_projectiles.end();) {
    if (!(*it)->active) {
      scene_root->remove((*it)->node);
      it = light_projectiles.erase(it);
    } else {
      ++it;
    }
  }
}