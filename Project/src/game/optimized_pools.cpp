#include "optimized_pools.h"
#include <iostream>

OptimizedPools::OptimizedPools() : total_nodes_created_(0), total_transforms_cached_(0) {
}

void OptimizedPools::initialize() {
    // Create dynamic transform cache
    transform_cache_ = std::make_unique<DynamicTransformCache>();
    
    // Setup node factory and resetter
    auto node_factory = []() -> std::unique_ptr<Node> {
        return std::make_unique<Node>(glm::mat4(1.0f));
    };
    
    auto node_resetter = [](Node& node) {
        // Reset node state
        node.transform_ = glm::mat4(1.0f);
        node.velocity_ = glm::vec3(0.0f);
        // Clear children (but don't delete them - they should be managed separately)
        node.children_.clear();
        node.children_shape_.clear();
    };
    
    // Create node pool
    node_pool_ = std::make_unique<ObjectPool<Node>>(
        config_.node_initial_size,
        config_.node_max_size,
        node_factory,
        node_resetter
    );
    
    std::cout << "OptimizedPools initialized:" << std::endl;
    std::cout << "  Node pool: " << config_.node_initial_size << "/" << config_.node_max_size << std::endl;
    std::cout << "  Dynamic transform cache: ENABLED" << std::endl;
}

Node* OptimizedPools::createNode(const glm::mat4& transform, const std::string& cache_id) {
    Node* node = node_pool_->acquire();
    if (node) {
        node->transform_ = transform;
        total_nodes_created_++;
        
        // If cache_id provided, store transform components for dynamic updates
        if (!cache_id.empty()) {
            // Extract position from transform matrix (simplified)
            glm::vec3 position = glm::vec3(transform[3]);
            transform_cache_->updateTransform(cache_id, position);
            total_transforms_cached_++;
        }
    }
    return node;
}

Node* OptimizedPools::createNode(const glm::vec3& position,
                                const glm::vec3& rotation,
                                const glm::vec3& scale,
                                const std::string& cache_id) {
    Node* node = node_pool_->acquire();
    if (node) {
        // Use cached transform if available
        if (!cache_id.empty()) {
            const glm::mat4& cached_transform = transform_cache_->getTransform(cache_id, position, rotation, scale);
            node->transform_ = cached_transform;
            total_transforms_cached_++;
        } else {
            // Calculate transform directly (fallback)
            glm::mat4 transform(1.0f);
            transform = glm::translate(transform, position);
            if (rotation.x != 0.0f) transform = glm::rotate(transform, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
            if (rotation.y != 0.0f) transform = glm::rotate(transform, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
            if (rotation.z != 0.0f) transform = glm::rotate(transform, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
            if (scale != glm::vec3(1.0f)) transform = glm::scale(transform, scale);
            node->transform_ = transform;
        }
        total_nodes_created_++;
    }
    return node;
}

void OptimizedPools::releaseNode(Node* node) {
    if (node && node_pool_) {
        node_pool_->release(node);
    }
}

void OptimizedPools::updateNodeTransform(Node* node,
                                        const std::string& cache_id,
                                        const glm::vec3& position,
                                        const glm::vec3& rotation,
                                        const glm::vec3& scale) {
    if (!node || cache_id.empty()) return;
    
    // Get cached transform
    const glm::mat4& cached_transform = transform_cache_->getTransform(cache_id, position, rotation, scale);
    node->transform_ = cached_transform;
}

void OptimizedPools::warmPools() {
    if (!node_pool_) return;
    
    std::cout << "Warming optimized pools..." << std::endl;
    
    // Pre-create nodes
    std::vector<Node*> temp_nodes;
    for (size_t i = 0; i < config_.node_initial_size / 2; ++i) {
        Node* node = node_pool_->acquire();
        if (node) temp_nodes.push_back(node);
    }
    
    // Return nodes to pool
    for (Node* node : temp_nodes) {
        node_pool_->release(node);
    }
    
    std::cout << "Pool warming complete - pre-created " << temp_nodes.size() << " nodes" << std::endl;
}

void OptimizedPools::printStatistics() const {
    std::cout << "\\n=== OptimizedPools Statistics ===" << std::endl;
    
    if (node_pool_) {
        std::cout << "Node Pool:" << std::endl;
        std::cout << "  Active: " << node_pool_->getActiveCount() << "/" << node_pool_->getTotalCount() << std::endl;
        std::cout << "  Total created: " << total_nodes_created_ << std::endl;
    }
    
    if (transform_cache_) {
        std::cout << "Transform Cache:" << std::endl;
        transform_cache_->printStatistics();
        std::cout << "  Total transforms cached: " << total_transforms_cached_ << std::endl;
    }
}

size_t OptimizedPools::getTransformCacheSize() const {
    // This would need to be implemented in DynamicTransformCache
    return total_transforms_cached_;
}