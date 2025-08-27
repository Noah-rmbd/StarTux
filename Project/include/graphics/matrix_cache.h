#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <unordered_map>
#include <string>
#include <memory>

class MatrixCache {
private:
    struct MatrixEntry {
        glm::mat4 matrix;
        bool dirty;
        
        MatrixEntry() : matrix(1.0f), dirty(true) {}
        MatrixEntry(const glm::mat4& mat) : matrix(mat), dirty(false) {}
    };
    
    std::unordered_map<std::string, MatrixEntry> cached_matrices_;
    
    // Common transformation components
    std::unordered_map<std::string, glm::vec3> cached_translations_;
    std::unordered_map<std::string, glm::vec3> cached_scales_;
    std::unordered_map<std::string, std::pair<float, glm::vec3>> cached_rotations_;
    
public:
    MatrixCache() = default;
    ~MatrixCache() = default;
    
    // Get cached matrix or compute and store if not exists/dirty
    const glm::mat4& getMatrix(const std::string& key);
    
    // Pre-compute and cache static matrices
    void cacheStaticMatrix(const std::string& key, const glm::mat4& matrix);
    
    // Cache transformation components for dynamic matrices
    void cacheTranslation(const std::string& key, const glm::vec3& translation);
    void cacheScale(const std::string& key, const glm::vec3& scale);
    void cacheRotation(const std::string& key, float angle, const glm::vec3& axis);
    
    // Build matrix from cached components
    const glm::mat4& buildMatrix(const std::string& key, 
                                  const std::string& translation_key = "",
                                  const std::string& scale_key = "",
                                  const std::string& rotation_key = "");
    
    // Update specific transformation component and mark matrix as dirty
    void updateTranslation(const std::string& key, const glm::vec3& translation);
    void updateScale(const std::string& key, const glm::vec3& scale);
    void updateRotation(const std::string& key, float angle, const glm::vec3& axis);
    
    // Force rebuild of specific matrix
    void invalidateMatrix(const std::string& key);
    
    // Clear cache (useful for scene changes)
    void clear();
    
    // Get cache statistics
    size_t getCacheSize() const { return cached_matrices_.size(); }
    void printCacheStats() const;
    
    // Singleton access
    static MatrixCache& getInstance() {
        static MatrixCache instance;
        return instance;
    }
};