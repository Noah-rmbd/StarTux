#pragma once

#include "node.h"
#include <string>
#include <memory>

// Enhanced Node class with transform caching support
class OptimizedNode : public Node {
private:
    std::string cache_id_;
    bool use_dynamic_cache_;
    glm::vec3 local_position_;
    glm::vec3 local_rotation_;
    glm::vec3 local_scale_;
    
    // Hierarchy optimization
    mutable glm::mat4 world_transform_; // Cached world-space transform
    mutable bool world_transform_dirty_;
    mutable uint32_t last_update_frame_;
    
    // Performance tracking
    mutable size_t transform_calculations_;
    mutable size_t cache_hits_;
    
public:
    OptimizedNode(const glm::mat4& transform = glm::mat4(1.0f), 
                  const std::string& cache_id = "",
                  bool use_dynamic_cache = false);
    
    // Enhanced transform management
    void setLocalTransform(const glm::vec3& position,
                          const glm::vec3& rotation = glm::vec3(0.0f),
                          const glm::vec3& scale = glm::vec3(1.0f));
    
    void setLocalPosition(const glm::vec3& position);
    void setLocalRotation(const glm::vec3& rotation);
    void setLocalScale(const glm::vec3& scale);
    
    // Cached world transform calculation
    const glm::mat4& getWorldTransform() const;
    void invalidateWorldTransform();
    
    // Override draw to use cached transforms
    void draw(glm::mat4& model, glm::mat4& view, glm::mat4& projection) override;
    
    // Batch update for multiple nodes (more efficient)
    static void batchUpdateTransforms(std::vector<OptimizedNode*>& nodes);
    
    // Statistics
    size_t getTransformCalculations() const { return transform_calculations_; }
    size_t getCacheHits() const { return cache_hits_; }
    float getCacheHitRate() const { 
        return (transform_calculations_ > 0) ? 
               static_cast<float>(cache_hits_) / transform_calculations_ : 0.0f; 
    }
    
    // Frame management
    static void setCurrentFrame(uint32_t frame) { current_frame_ = frame; }
    
private:
    void updateLocalTransform();
    void markHierarchyDirty();
    
    static uint32_t current_frame_;
};