#pragma once

#include "matrix_cache.h"
#include <unordered_map>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class DynamicTransformCache {
private:
    struct TransformState {
        glm::vec3 position;
        glm::vec3 rotation; // Euler angles  
        glm::vec3 scale;
        glm::mat4 cached_matrix;
        bool dirty;
        uint32_t update_frame;
        
        TransformState() : position(0.0f), rotation(0.0f), scale(1.0f), 
                          cached_matrix(1.0f), dirty(true), update_frame(0) {}
    };
    
    std::unordered_map<std::string, TransformState> transforms_;
    uint32_t current_frame_;
    
    // Performance tracking
    size_t cache_hits_;
    size_t cache_misses_;
    size_t matrix_calculations_;
    
public:
    DynamicTransformCache() : current_frame_(0), cache_hits_(0), cache_misses_(0), matrix_calculations_(0) {}
    
    // Frame management
    void beginFrame() { current_frame_++; }
    void endFrame();
    
    // Transform management
    const glm::mat4& getTransform(const std::string& object_id,
                                  const glm::vec3& position,
                                  const glm::vec3& rotation = glm::vec3(0.0f),
                                  const glm::vec3& scale = glm::vec3(1.0f));
    
    // Batch update for multiple objects (more efficient)
    void updateTransform(const std::string& object_id,
                        const glm::vec3& position,
                        const glm::vec3& rotation = glm::vec3(0.0f),
                        const glm::vec3& scale = glm::vec3(1.0f));
    
    void batchCalculateMatrices(); // Calculate all dirty matrices at once
    
    // Invalidate specific transform
    void invalidateTransform(const std::string& object_id);
    
    // Remove unused transforms (garbage collection)
    void cleanupUnusedTransforms(uint32_t frames_threshold = 60);
    
    // Statistics
    float getCacheHitRate() const { 
        return cache_hits_ + cache_misses_ > 0 ? 
               static_cast<float>(cache_hits_) / (cache_hits_ + cache_misses_) : 0.0f; 
    }
    size_t getMatrixCalculationsThisFrame() const;
    void printStatistics() const;
    void resetStatistics() { cache_hits_ = cache_misses_ = matrix_calculations_ = 0; }
    
    // Clear all cached transforms
    void clear() { transforms_.clear(); }
};