#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <mutex>
#include "texture.h"
#include "shader.h"

/**
 * Centralized resource management system
 * 
 * Benefits:
 * - No duplicate loading of assets
 * - Automatic memory management
 * - Thread-safe resource sharing
 * - Cache management
 * 
 * Example usage:
 *   auto texture = ResourceManager::GetTexture(\"space.jpg\");
 *   auto shader = ResourceManager::GetShader(\"ship.vert\", \"ship.frag\");
 */
class ResourceManager {
private:
    // Private constructor - this is a singleton
    ResourceManager() = default;
    
    // Resource caches
    std::unordered_map<std::string, std::shared_ptr<Texture>> textureCache;
    std::unordered_map<std::string, std::shared_ptr<Shader>> shaderCache;
    
    // Thread safety
    mutable std::mutex textureMutex;
    mutable std::mutex shaderMutex;
    
    // Asset directories
    std::string textureDir = TEXTURES_DIR;
    std::string shaderDir = SHADER_DIR;
    std::string resourceDir = RESSOURCES_DIR;

public:
    // Singleton access
    static ResourceManager& Instance() {
        static ResourceManager instance;
        return instance;
    }
    
    // Delete copy/move operations (singleton)
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    ResourceManager(ResourceManager&&) = delete;
    ResourceManager& operator=(ResourceManager&&) = delete;
    
    /**
     * Load or get cached texture
     * Returns shared_ptr - automatically managed memory!
     */
    std::shared_ptr<Texture> GetTexture(const std::string& filename);
    
    /**
     * Load or get cached shader
     * Key is generated from vertex + fragment filenames
     */
    std::shared_ptr<Shader> GetShader(const std::string& vertexFile, const std::string& fragmentFile);
    
    /**
     * Preload common assets at game startup
     */
    void PreloadCommonAssets();
    
    /**
     * Clear unused resources (garbage collection)
     */
    void CleanupUnusedResources();
    
    /**
     * Get resource statistics
     */
    struct ResourceStats {
        size_t texturesLoaded;
        size_t shadersLoaded;
        size_t texturesInUse;
        size_t shadersInUse;
    };
    ResourceStats GetStats() const;
    
    /**
     * Check if texture exists in cache
     */
    bool HasTexture(const std::string& filename) const;
    
    /**
     * Check if shader exists in cache  
     */
    bool HasShader(const std::string& vertexFile, const std::string& fragmentFile) const;
    
    /**
     * Force reload a texture (useful for hot-reloading during development)
     */
    void ReloadTexture(const std::string& filename);
    
    /**
     * Force reload a shader
     */
    void ReloadShader(const std::string& vertexFile, const std::string& fragmentFile);

private:
    std::string GenerateShaderKey(const std::string& vertexFile, const std::string& fragmentFile) const;
    std::string GetFullTexturePath(const std::string& filename) const;
    std::string GetFullShaderPath(const std::string& filename) const;
};

// Convenience functions for easy access
namespace Resources {
    inline std::shared_ptr<Texture> GetTexture(const std::string& filename) {
        return ResourceManager::Instance().GetTexture(filename);
    }
    
    inline std::shared_ptr<Shader> GetShader(const std::string& vertexFile, const std::string& fragmentFile) {
        return ResourceManager::Instance().GetShader(vertexFile, fragmentFile);
    }
}