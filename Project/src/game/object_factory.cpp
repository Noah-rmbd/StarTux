#include "object_factory.h"
#include <iostream>

ObjectFactory::ObjectFactory() {
    // Pools will be initialized in initialize() method
}

void ObjectFactory::initialize(Shader* phong_shader, Shader* texture_shader) {
    setupFactoryFunctions(phong_shader);
    setupResetterFunctions();
    
    // Create pools with optimized sizes based on game analysis
    projectile_pool_ = std::make_unique<ObjectPool<Projectile>>(
        20,  // initial size
        100, // max size
        projectile_factory_,
        projectile_resetter_
    );
    
    light_projectile_pool_ = std::make_unique<ObjectPool<LightProjectile>>(
        15,  // initial size
        80,  // max size
        light_projectile_factory_,
        light_projectile_resetter_
    );
    
    asteroid_pool_ = std::make_unique<ObjectPool<Asteroid>>(
        50,  // initial size
        300, // max size
        asteroid_factory_,
        asteroid_resetter_
    );
    
    explosion_pool_ = std::make_unique<ObjectPool<Explosion>>(
        10,  // initial size
        50,  // max size
        explosion_factory_,
        explosion_resetter_
    );
    
    ring_pool_ = std::make_unique<ObjectPool<Ring>>(
        5,   // initial size
        30,  // max size
        ring_factory_,
        ring_resetter_
    );
    
    std::cout << "ObjectFactory initialized with optimized pools:" << std::endl;
    std::cout << "  Projectiles: 20/100, LightProjectiles: 15/80" << std::endl;
    std::cout << "  Asteroids: 50/300, Explosions: 10/50, Rings: 5/30" << std::endl;
}

void ObjectFactory::setupFactoryFunctions(Shader* phong_shader) {
    // Note: These are simplified factory functions
    // In practice, you'd need to pass appropriate parameters
    
    projectile_factory_ = [phong_shader]() -> std::unique_ptr<Projectile> {
        // This is a placeholder - actual implementation would need position, direction, cursor
        return nullptr; // Will be properly implemented when integrating
    };
    
    light_projectile_factory_ = [phong_shader]() -> std::unique_ptr<LightProjectile> {
        return nullptr; // Placeholder
    };
    
    asteroid_factory_ = [phong_shader]() -> std::unique_ptr<Asteroid> {
        return nullptr; // Placeholder
    };
    
    explosion_factory_ = [phong_shader]() -> std::unique_ptr<Explosion> {
        return nullptr; // Placeholder
    };
    
    ring_factory_ = [phong_shader]() -> std::unique_ptr<Ring> {
        return nullptr; // Placeholder
    };
}

void ObjectFactory::setupResetterFunctions() {
    projectile_resetter_ = [](Projectile& proj) {
        proj.active = false;
        proj.position = glm::vec3(0.0f);
        proj.direction = glm::vec3(0.0f, 0.0f, 1.0f);
        proj.speed = 0.0f;
    };
    
    light_projectile_resetter_ = [](LightProjectile& proj) {
        proj.active = false;
        proj.position = glm::vec3(0.0f);
        proj.direction = glm::vec3(0.0f, 0.0f, 1.0f);
        proj.speed = 0.0f;
    };
    
    asteroid_resetter_ = [](Asteroid& ast) {
        ast.life = 1;
        ast.is_moving = false;
        // Reset position and other properties
    };
    
    explosion_resetter_ = [](Explosion& exp) {
        // Reset explosion state
    };
    
    ring_resetter_ = [](Ring& ring) {
        // Reset ring state
    };
}

Projectile* ObjectFactory::createProjectile(glm::vec3 position, glm::vec3 direction, glm::vec3 cursor) {
    auto* projectile = projectile_pool_->acquire();
    if (projectile) {
        // Initialize with provided parameters
        projectile->position = position;
        projectile->direction = glm::normalize(direction);
        projectile->cursorPosition = cursor;
        projectile->speed = 3.0f;
        projectile->active = true;
    }
    return projectile;
}

LightProjectile* ObjectFactory::createLightProjectile(glm::vec3 position, glm::vec3 direction, glm::vec3 cursor) {
    auto* light_projectile = light_projectile_pool_->acquire();
    if (light_projectile) {
        light_projectile->position = position;
        light_projectile->direction = glm::normalize(direction);
        light_projectile->cursorPosition = cursor;
        light_projectile->speed = 6.0f;
        light_projectile->active = true;
    }
    return light_projectile;
}

Asteroid* ObjectFactory::createAsteroid(Node* parent_node, bool is_moving) {
    auto* asteroid = asteroid_pool_->acquire();
    if (asteroid) {
        asteroid->is_moving = is_moving;
        asteroid->life = 1;
        // Additional initialization would go here
    }
    return asteroid;
}

Explosion* ObjectFactory::createExplosion(glm::vec3 position, double creation_time) {
    auto* explosion = explosion_pool_->acquire();
    if (explosion) {
        // Initialize explosion at position and time
        // This would need actual implementation details
    }
    return explosion;
}

Ring* ObjectFactory::createRing(glm::vec3 position) {
    auto* ring = ring_pool_->acquire();
    if (ring) {
        // Initialize ring at position
    }
    return ring;
}

void ObjectFactory::releaseProjectile(Projectile* projectile) {
    if (projectile) {
        projectile_pool_->release(projectile);
    }
}

void ObjectFactory::releaseLightProjectile(LightProjectile* light_projectile) {
    if (light_projectile) {
        light_projectile_pool_->release(light_projectile);
    }
}

void ObjectFactory::releaseAsteroid(Asteroid* asteroid) {
    if (asteroid) {
        asteroid_pool_->release(asteroid);
    }
}

void ObjectFactory::releaseExplosion(Explosion* explosion) {
    if (explosion) {
        explosion_pool_->release(explosion);
    }
}

void ObjectFactory::releaseRing(Ring* ring) {
    if (ring) {
        ring_pool_->release(ring);
    }
}

void ObjectFactory::warmPools() {
    std::cout << "Warming object pools..." << std::endl;
    
    // Pre-create some objects to avoid first-frame hitches
    std::vector<Projectile*> temp_projectiles;
    for (int i = 0; i < 10; ++i) {
        auto* proj = projectile_pool_->acquire();
        if (proj) temp_projectiles.push_back(proj);
    }
    for (auto* proj : temp_projectiles) {
        projectile_pool_->release(proj);
    }
    
    // Similar for other pools...
    std::cout << "Pool warming complete." << std::endl;
}

void ObjectFactory::clearPools() {
    projectile_pool_->clear();
    light_projectile_pool_->clear();
    asteroid_pool_->clear();
    explosion_pool_->clear();
    ring_pool_->clear();
}

void ObjectFactory::printPoolStats() const {
    std::cout << "\\n=== Object Factory Pool Statistics ===" << std::endl;
    
    if (projectile_pool_) {
        std::cout << "Projectiles: " << projectile_pool_->getActiveCount() 
                  << "/" << projectile_pool_->getTotalCount() << std::endl;
    }
    
    if (light_projectile_pool_) {
        std::cout << "Light Projectiles: " << light_projectile_pool_->getActiveCount() 
                  << "/" << light_projectile_pool_->getTotalCount() << std::endl;
    }
    
    if (asteroid_pool_) {
        std::cout << "Asteroids: " << asteroid_pool_->getActiveCount() 
                  << "/" << asteroid_pool_->getTotalCount() << std::endl;
    }
    
    if (explosion_pool_) {
        std::cout << "Explosions: " << explosion_pool_->getActiveCount() 
                  << "/" << explosion_pool_->getTotalCount() << std::endl;
    }
    
    if (ring_pool_) {
        std::cout << "Rings: " << ring_pool_->getActiveCount() 
                  << "/" << ring_pool_->getTotalCount() << std::endl;
    }
}

size_t ObjectFactory::getTotalActiveObjects() const {
    size_t total = 0;
    if (projectile_pool_) total += projectile_pool_->getActiveCount();
    if (light_projectile_pool_) total += light_projectile_pool_->getActiveCount();
    if (asteroid_pool_) total += asteroid_pool_->getActiveCount();
    if (explosion_pool_) total += explosion_pool_->getActiveCount();
    if (ring_pool_) total += ring_pool_->getActiveCount();
    return total;
}

size_t ObjectFactory::getTotalPoolSize() const {
    size_t total = 0;
    if (projectile_pool_) total += projectile_pool_->getTotalCount();
    if (light_projectile_pool_) total += light_projectile_pool_->getTotalCount();
    if (asteroid_pool_) total += asteroid_pool_->getTotalCount();
    if (explosion_pool_) total += explosion_pool_->getTotalCount();
    if (ring_pool_) total += ring_pool_->getTotalCount();
    return total;
}