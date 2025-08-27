#include "dynamic_transform_cache.h"
#include <iostream>
#include <algorithm>

const glm::mat4& DynamicTransformCache::getTransform(const std::string& object_id,
                                                     const glm::vec3& position,
                                                     const glm::vec3& rotation,
                                                     const glm::vec3& scale) {
    auto& state = transforms_[object_id];
    state.update_frame = current_frame_;
    
    // Check if transform has changed
    bool position_changed = state.position != position;
    bool rotation_changed = state.rotation != rotation;
    bool scale_changed = state.scale != scale;
    
    if (position_changed || rotation_changed || scale_changed || state.dirty) {
        // Update state
        state.position = position;
        state.rotation = rotation;
        state.scale = scale;
        state.dirty = false;
        
        // Recalculate matrix
        glm::mat4 matrix(1.0f);
        
        // Apply transformations in TRS order (Translation, Rotation, Scale)
        matrix = glm::translate(matrix, position);
        
        // Apply rotations (assuming XYZ order)
        if (rotation.x != 0.0f) matrix = glm::rotate(matrix, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        if (rotation.y != 0.0f) matrix = glm::rotate(matrix, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        if (rotation.z != 0.0f) matrix = glm::rotate(matrix, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        
        if (scale != glm::vec3(1.0f)) matrix = glm::scale(matrix, scale);
        
        state.cached_matrix = matrix;
        matrix_calculations_++;
        cache_misses_++;
    } else {
        cache_hits_++;
    }
    
    return state.cached_matrix;
}

void DynamicTransformCache::updateTransform(const std::string& object_id,
                                           const glm::vec3& position,
                                           const glm::vec3& rotation,
                                           const glm::vec3& scale) {
    auto& state = transforms_[object_id];
    state.update_frame = current_frame_;
    
    // Check if any component changed
    if (state.position != position || state.rotation != rotation || state.scale != scale) {
        state.position = position;
        state.rotation = rotation;
        state.scale = scale;
        state.dirty = true; // Mark for recalculation
    }
}

void DynamicTransformCache::batchCalculateMatrices() {
    size_t calculated = 0;
    for (auto& [id, state] : transforms_) {
        if (state.dirty) {
            glm::mat4 matrix(1.0f);
            matrix = glm::translate(matrix, state.position);
            
            if (state.rotation.x != 0.0f) matrix = glm::rotate(matrix, state.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
            if (state.rotation.y != 0.0f) matrix = glm::rotate(matrix, state.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
            if (state.rotation.z != 0.0f) matrix = glm::rotate(matrix, state.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
            
            if (state.scale != glm::vec3(1.0f)) matrix = glm::scale(matrix, state.scale);
            
            state.cached_matrix = matrix;
            state.dirty = false;
            calculated++;
        }
    }
    matrix_calculations_ += calculated;
}

void DynamicTransformCache::endFrame() {
    // Optional: batch calculate all dirty matrices at end of frame
    // This can be more cache-friendly than calculating on-demand
    batchCalculateMatrices();
}

void DynamicTransformCache::invalidateTransform(const std::string& object_id) {
    auto it = transforms_.find(object_id);
    if (it != transforms_.end()) {
        it->second.dirty = true;
    }
}

void DynamicTransformCache::cleanupUnusedTransforms(uint32_t frames_threshold) {
    auto it = transforms_.begin();
    size_t removed = 0;
    
    while (it != transforms_.end()) {
        if (current_frame_ - it->second.update_frame > frames_threshold) {
            it = transforms_.erase(it);
            removed++;
        } else {
            ++it;
        }
    }
    
    if (removed > 0) {
        std::cout << "Cleaned up " << removed << " unused transforms" << std::endl;
    }
}

size_t DynamicTransformCache::getMatrixCalculationsThisFrame() const {
    static size_t last_calculations = 0;
    size_t current_calculations = matrix_calculations_;
    size_t this_frame = current_calculations - last_calculations;
    last_calculations = current_calculations;
    return this_frame;
}

void DynamicTransformCache::printStatistics() const {
    std::cout << "Dynamic Transform Cache Statistics:" << std::endl;
    std::cout << "  Active transforms: " << transforms_.size() << std::endl;
    std::cout << "  Cache hit rate: " << (getCacheHitRate() * 100.0f) << "%" << std::endl;
    std::cout << "  Matrix calculations: " << matrix_calculations_ << std::endl;
    std::cout << "  Cache hits: " << cache_hits_ << std::endl;
    std::cout << "  Cache misses: " << cache_misses_ << std::endl;
    
    size_t dirty_count = 0;
    for (const auto& [id, state] : transforms_) {
        if (state.dirty) dirty_count++;
    }
    std::cout << "  Dirty transforms: " << dirty_count << std::endl;
}