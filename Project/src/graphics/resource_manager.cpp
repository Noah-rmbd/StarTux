#include "resource_manager.h"
#include <iostream>
#include <filesystem>

std::shared_ptr<Texture> ResourceManager::GetTexture(const std::string& filename) {
    std::lock_guard<std::mutex> lock(textureMutex);
    
    // Check if already loaded
    auto it = textureCache.find(filename);
    if (it != textureCache.end()) {
        // Return existing texture - no duplicate loading!
        std::cout << "ResourceManager: Using cached texture: " << filename << std::endl;
        return it->second;
    }
    
    try {
        // Load new texture
        std::string fullPath = GetFullTexturePath(filename);
        std::cout << "ResourceManager: Loading texture: " << fullPath << std::endl;
        
        auto texture = std::make_shared<Texture>(fullPath);
        
        // Cache it for future use
        textureCache[filename] = texture;
        
        return texture;
    } catch (const std::exception& e) {
        std::cerr << "ResourceManager: Failed to load texture " << filename << ": " << e.what() << std::endl;
        return nullptr;
    }
}

std::shared_ptr<Shader> ResourceManager::GetShader(const std::string& vertexFile, const std::string& fragmentFile) {
    std::lock_guard<std::mutex> lock(shaderMutex);
    
    std::string key = GenerateShaderKey(vertexFile, fragmentFile);
    
    // Check if already loaded
    auto it = shaderCache.find(key);
    if (it != shaderCache.end()) {
        std::cout << "ResourceManager: Using cached shader: " << key << std::endl;
        return it->second;
    }
    
    try {
        // Load new shader
        std::string vertexPath = GetFullShaderPath(vertexFile);
        std::string fragmentPath = GetFullShaderPath(fragmentFile);
        
        std::cout << "ResourceManager: Loading shader: " << vertexPath << " + " << fragmentPath << std::endl;
        
        auto shader = std::make_shared<Shader>(vertexPath, fragmentPath);
        
        // Cache it
        shaderCache[key] = shader;
        
        return shader;
    } catch (const std::exception& e) {
        std::cerr << "ResourceManager: Failed to load shader " << vertexFile << "/" << fragmentFile << ": " << e.what() << std::endl;
        return nullptr;
    }
}

void ResourceManager::PreloadCommonAssets() {
    std::cout << "ResourceManager: Preloading common assets..." << std::endl;
    
    // Preload common textures
    GetTexture("space3.jpeg");
    GetTexture("asteroid.png");
    GetTexture("start_banner.png");
    GetTexture("aim.png");
    GetTexture("text_box.png");
    GetTexture("HUD_container_L.png");
    GetTexture("HUD_container_R.png");
    GetTexture("shield_icon.png");
    
    // Preload common shaders
    GetShader("texture.vert", "texture.frag");
    GetShader("ship.vert", "ship.frag");
    GetShader("phong.vert", "phong.frag");
    GetShader("text.vs", "text.fs");
    GetShader("ui_color.vert", "ui_color.frag");
    
    std::cout << "ResourceManager: Preloading complete!" << std::endl;
}

void ResourceManager::CleanupUnusedResources() {
    {
        std::lock_guard<std::mutex> lock(textureMutex);
        auto it = textureCache.begin();
        while (it != textureCache.end()) {
            // If only the cache holds a reference (use_count == 1), remove it
            if (it->second.use_count() == 1) {
                std::cout << "ResourceManager: Cleaning up unused texture: " << it->first << std::endl;
                it = textureCache.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    {
        std::lock_guard<std::mutex> lock(shaderMutex);
        auto it = shaderCache.begin();
        while (it != shaderCache.end()) {
            if (it->second.use_count() == 1) {
                std::cout << "ResourceManager: Cleaning up unused shader: " << it->first << std::endl;
                it = shaderCache.erase(it);
            } else {
                ++it;
            }
        }
    }
}

ResourceManager::ResourceStats ResourceManager::GetStats() const {
    std::lock_guard<std::mutex> texLock(textureMutex);
    std::lock_guard<std::mutex> shaderLock(shaderMutex);
    
    ResourceStats stats;
    stats.texturesLoaded = textureCache.size();
    stats.shadersLoaded = shaderCache.size();
    
    // Count resources actually in use (reference count > 1)
    stats.texturesInUse = 0;
    for (const auto& pair : textureCache) {
        if (pair.second.use_count() > 1) {
            stats.texturesInUse++;
        }
    }
    
    stats.shadersInUse = 0;
    for (const auto& pair : shaderCache) {
        if (pair.second.use_count() > 1) {
            stats.shadersInUse++;
        }
    }
    
    return stats;
}

bool ResourceManager::HasTexture(const std::string& filename) const {
    std::lock_guard<std::mutex> lock(textureMutex);
    return textureCache.find(filename) != textureCache.end();
}

bool ResourceManager::HasShader(const std::string& vertexFile, const std::string& fragmentFile) const {
    std::lock_guard<std::mutex> lock(shaderMutex);
    std::string key = GenerateShaderKey(vertexFile, fragmentFile);
    return shaderCache.find(key) != shaderCache.end();
}

void ResourceManager::ReloadTexture(const std::string& filename) {
    std::lock_guard<std::mutex> lock(textureMutex);
    textureCache.erase(filename);
    // Next GetTexture() call will reload it
}

void ResourceManager::ReloadShader(const std::string& vertexFile, const std::string& fragmentFile) {
    std::lock_guard<std::mutex> lock(shaderMutex);
    std::string key = GenerateShaderKey(vertexFile, fragmentFile);
    shaderCache.erase(key);
    // Next GetShader() call will reload it
}

std::string ResourceManager::GenerateShaderKey(const std::string& vertexFile, const std::string& fragmentFile) const {
    return vertexFile + "|" + fragmentFile;
}

std::string ResourceManager::GetFullTexturePath(const std::string& filename) const {
    return textureDir + filename;
}

std::string ResourceManager::GetFullShaderPath(const std::string& filename) const {
    return shaderDir + filename;
}