#pragma once

#include "object_pool.h"
#include "dynamic_transform_cache.h"
#include "node.h"
#include <memory>
#include <string>

class OptimizedPools {
private:
    // Node pool for frequent allocations
    std::unique_ptr<ObjectPool<Node>> node_pool_;
    
    // Dynamic transform cache for moving objects
    std::unique_ptr<DynamicTransformCache> transform_cache_;
    
    // Pool configuration
    struct PoolConfig {
        size_t node_initial_size = 100;
        size_t node_max_size = 500;
    } config_;
    
    // Performance tracking
    size_t total_nodes_created_;
    size_t total_transforms_cached_;
    
public:
    OptimizedPools();
    ~OptimizedPools() = default;
    
    // Initialize pools
    void initialize();
    
    // Node management with automatic transform caching
    Node* createNode(const glm::mat4& transform = glm::mat4(1.0f), const std::string& cache_id = "");
    Node* createNode(const glm::vec3& position, 
                    const glm::vec3& rotation = glm::vec3(0.0f),
                    const glm::vec3& scale = glm::vec3(1.0f),
                    const std::string& cache_id = "");
    void releaseNode(Node* node);
    
    // Transform management
    void updateNodeTransform(Node* node, 
                           const std::string& cache_id,
                           const glm::vec3& position,
                           const glm::vec3& rotation = glm::vec3(0.0f),
                           const glm::vec3& scale = glm::vec3(1.0f));
    
    // Frame management for caching
    void beginFrame() { if (transform_cache_) transform_cache_->beginFrame(); }
    void endFrame() { if (transform_cache_) transform_cache_->endFrame(); }
    
    // Performance optimization
    void warmPools();
    void cleanupUnusedTransforms() { if (transform_cache_) transform_cache_->cleanupUnusedTransforms(); }
    
    // Statistics
    void printStatistics() const;
    size_t getTotalNodesActive() const { return node_pool_ ? node_pool_->getActiveCount() : 0; }
    size_t getTransformCacheSize() const;
    
    // Singleton access
    static OptimizedPools& getInstance() {
        static OptimizedPools instance;
        return instance;
    }
    
private:
    void setupNodeResetter();
};