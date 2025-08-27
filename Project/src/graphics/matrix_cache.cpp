#include "matrix_cache.h"
#include <iostream>

const glm::mat4& MatrixCache::getMatrix(const std::string& key) {
    auto it = cached_matrices_.find(key);
    if (it != cached_matrices_.end() && !it->second.dirty) {
        return it->second.matrix;
    }
    
    // Matrix not found or dirty - need to rebuild
    // This should not happen if properly managed, but provide fallback
    if (it == cached_matrices_.end()) {
        cached_matrices_[key] = MatrixEntry(glm::mat4(1.0f));
    }
    
    return cached_matrices_[key].matrix;
}

void MatrixCache::cacheStaticMatrix(const std::string& key, const glm::mat4& matrix) {
    cached_matrices_[key] = MatrixEntry(matrix);
}

void MatrixCache::cacheTranslation(const std::string& key, const glm::vec3& translation) {
    cached_translations_[key] = translation;
}

void MatrixCache::cacheScale(const std::string& key, const glm::vec3& scale) {
    cached_scales_[key] = scale;
}

void MatrixCache::cacheRotation(const std::string& key, float angle, const glm::vec3& axis) {
    cached_rotations_[key] = std::make_pair(angle, axis);
}

const glm::mat4& MatrixCache::buildMatrix(const std::string& key, 
                                          const std::string& translation_key,
                                          const std::string& scale_key,
                                          const std::string& rotation_key) {
    auto& entry = cached_matrices_[key];
    
    if (!entry.dirty) {
        return entry.matrix;
    }
    
    // Rebuild matrix from components
    glm::mat4 matrix(1.0f);
    
    // Apply translation
    if (!translation_key.empty()) {
        auto trans_it = cached_translations_.find(translation_key);
        if (trans_it != cached_translations_.end()) {
            matrix = glm::translate(matrix, trans_it->second);
        }
    }
    
    // Apply rotation
    if (!rotation_key.empty()) {
        auto rot_it = cached_rotations_.find(rotation_key);
        if (rot_it != cached_rotations_.end()) {
            matrix = glm::rotate(matrix, rot_it->second.first, rot_it->second.second);
        }
    }
    
    // Apply scale
    if (!scale_key.empty()) {
        auto scale_it = cached_scales_.find(scale_key);
        if (scale_it != cached_scales_.end()) {
            matrix = glm::scale(matrix, scale_it->second);
        }
    }
    
    entry.matrix = matrix;
    entry.dirty = false;
    
    return entry.matrix;
}

void MatrixCache::updateTranslation(const std::string& key, const glm::vec3& translation) {
    cached_translations_[key] = translation;
    // Mark associated matrix as dirty
    auto it = cached_matrices_.find(key);
    if (it != cached_matrices_.end()) {
        it->second.dirty = true;
    }
}

void MatrixCache::updateScale(const std::string& key, const glm::vec3& scale) {
    cached_scales_[key] = scale;
    // Mark associated matrix as dirty
    auto it = cached_matrices_.find(key);
    if (it != cached_matrices_.end()) {
        it->second.dirty = true;
    }
}

void MatrixCache::updateRotation(const std::string& key, float angle, const glm::vec3& axis) {
    cached_rotations_[key] = std::make_pair(angle, axis);
    // Mark associated matrix as dirty
    auto it = cached_matrices_.find(key);
    if (it != cached_matrices_.end()) {
        it->second.dirty = true;
    }
}

void MatrixCache::invalidateMatrix(const std::string& key) {
    auto it = cached_matrices_.find(key);
    if (it != cached_matrices_.end()) {
        it->second.dirty = true;
    }
}

void MatrixCache::clear() {
    cached_matrices_.clear();
    cached_translations_.clear();
    cached_scales_.clear();
    cached_rotations_.clear();
}

void MatrixCache::printCacheStats() const {
    std::cout << "Matrix Cache Statistics:" << std::endl;
    std::cout << "  Cached matrices: " << cached_matrices_.size() << std::endl;
    std::cout << "  Cached translations: " << cached_translations_.size() << std::endl;
    std::cout << "  Cached scales: " << cached_scales_.size() << std::endl;
    std::cout << "  Cached rotations: " << cached_rotations_.size() << std::endl;
    
    size_t dirty_count = 0;
    for (const auto& [key, entry] : cached_matrices_) {
        if (entry.dirty) dirty_count++;
    }
    std::cout << "  Dirty matrices: " << dirty_count << std::endl;
}