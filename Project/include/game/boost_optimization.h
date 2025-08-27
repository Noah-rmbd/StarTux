#pragma once

#include <vector>
#include <unordered_set>
#include <glm/glm.hpp>

// Optimized boost system to prevent lag spikes during ring collection
class BoostOptimization {
private:
    // Track objects affected by boost
    std::unordered_set<void*> boost_affected_objects_;
    
    // Batch transform updates
    struct BatchedTransformUpdate {
        void* object_ptr;
        glm::vec3 velocity_change;
        float speed_multiplier;
    };
    std::vector<BatchedTransformUpdate> pending_updates_;
    
    // Performance metrics
    size_t total_boost_activations_;
    double last_boost_time_;
    double boost_processing_time_;
    
public:
    BoostOptimization();
    
    // Boost management
    void activateBoost(double current_time, float speed_multiplier = 2.0f);
    void deactivateBoost(double current_time, float speed_multiplier = 0.5f);
    
    // Batch update system (prevents lag spikes)
    void addObjectToBatch(void* object, const glm::vec3& velocity_change, float multiplier);
    void processBatchedUpdates(); // Call once per frame
    
    // Performance optimization
    void precomputeBoostEffects(); // Pre-calculate common boost scenarios
    
    // Statistics
    void printStatistics() const;
    double getLastBoostProcessingTime() const { return boost_processing_time_; }
    size_t getTotalBoostActivations() const { return total_boost_activations_; }
    
    // Singleton access
    static BoostOptimization& getInstance() {
        static BoostOptimization instance;
        return instance;
    }
    
private:
    void measureProcessingTime(std::function<void()> operation);
};