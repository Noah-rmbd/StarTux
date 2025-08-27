#pragma once

#include "object_pool.h"
#include "projectile.h"
#include "light_projectile.h"
#include "asteroid.h"
#include "explosion.h"
#include "ring.h"
#include <memory>
#include <functional>

class ObjectFactory {
private:
    // Pre-configured object pools with optimized sizes
    std::unique_ptr<ObjectPool<Projectile>> projectile_pool_;
    std::unique_ptr<ObjectPool<LightProjectile>> light_projectile_pool_;
    std::unique_ptr<ObjectPool<Asteroid>> asteroid_pool_;
    std::unique_ptr<ObjectPool<Explosion>> explosion_pool_;
    std::unique_ptr<ObjectPool<Ring>> ring_pool_;
    
    // Factory functions that create objects with proper initialization
    std::function<std::unique_ptr<Projectile>()> projectile_factory_;
    std::function<std::unique_ptr<LightProjectile>()> light_projectile_factory_;
    std::function<std::unique_ptr<Asteroid>()> asteroid_factory_;
    std::function<std::unique_ptr<Explosion>()> explosion_factory_;
    std::function<std::unique_ptr<Ring>()> ring_factory_;
    
    // Reset functions to clean objects when returned to pool
    std::function<void(Projectile&)> projectile_resetter_;
    std::function<void(LightProjectile&)> light_projectile_resetter_;
    std::function<void(Asteroid&)> asteroid_resetter_;
    std::function<void(Explosion&)> explosion_resetter_;
    std::function<void(Ring&)> ring_resetter_;
    
public:
    ObjectFactory();
    ~ObjectFactory() = default;
    
    // Initialize pools with shaders and other dependencies
    void initialize(Shader* phong_shader, Shader* texture_shader);
    
    // Object creation methods (pool-based)
    Projectile* createProjectile(glm::vec3 position, glm::vec3 direction, glm::vec3 cursor);
    LightProjectile* createLightProjectile(glm::vec3 position, glm::vec3 direction, glm::vec3 cursor);
    Asteroid* createAsteroid(Node* parent_node, bool is_moving = false);
    Explosion* createExplosion(glm::vec3 position, double creation_time);
    Ring* createRing(glm::vec3 position);
    
    // Object release methods (return to pool)
    void releaseProjectile(Projectile* projectile);
    void releaseLightProjectile(LightProjectile* light_projectile);
    void releaseAsteroid(Asteroid* asteroid);
    void releaseExplosion(Explosion* explosion);
    void releaseRing(Ring* ring);
    
    // Pool management
    void warmPools(); // Pre-create objects
    void clearPools(); // Reset all pools
    
    // Statistics
    void printPoolStats() const;
    size_t getTotalActiveObjects() const;
    size_t getTotalPoolSize() const;
    
    // Singleton access
    static ObjectFactory& getInstance() {
        static ObjectFactory instance;
        return instance;
    }
    
private:
    void setupFactoryFunctions(Shader* phong_shader);
    void setupResetterFunctions();
};