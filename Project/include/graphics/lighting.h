#ifndef LIGHTING_H
#define LIGHTING_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <memory>
#include "shader.h"

/**
 * Modern lighting system for StarTux
 * Supports multiple light types and dynamic lighting
 */

enum class LightType {
    DIRECTIONAL,   // Sun-like light (infinite distance)
    POINT,         // Point light (like a star or explosion)
    SPOT          // Spotlight (like ship headlights)
};

struct Light {
    LightType type = LightType::POINT;
    
    // Position (for point/spot lights)
    glm::vec3 position = glm::vec3(0.0f);
    
    // Direction (for directional/spot lights)
    glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);
    
    // Light properties
    glm::vec3 color = glm::vec3(1.0f);
    float intensity = 1.0f;
    
    // Attenuation (for point/spot lights)
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;
    
    // Spot light properties
    float cutOff = glm::cos(glm::radians(12.5f));
    float outerCutOff = glm::cos(glm::radians(17.5f));
    
    // Animation properties
    bool animated = false;
    float animationSpeed = 1.0f;
    float animationTime = 0.0f;
    glm::vec3 originalPosition;
};

class LightingSystem {
private:
    std::vector<Light> lights;
    glm::vec3 ambientLight = glm::vec3(0.05f, 0.05f, 0.1f);  // Subtle blue ambient for space
    
public:
    LightingSystem();
    
    // Light management
    int AddLight(const Light& light);
    void RemoveLight(int lightId);
    void UpdateLight(int lightId, const Light& light);
    Light& GetLight(int lightId) { return lights[lightId]; }
    
    // Update all lights (for animations)
    void Update(float deltaTime);
    
    // Apply lights to shader
    void ApplyToShader(Shader& shader, const glm::vec3& viewPos);
    
    // Preset lighting setups for different scenes
    void SetupSpaceLighting();
    void SetupMenuLighting();
    void SetupGameplayLighting(const glm::vec3& playerPos);
    
    // Dynamic effects
    void AddExplosionLight(const glm::vec3& position, float intensity = 5.0f, float duration = 2.0f);
    void AddEngineGlow(const glm::vec3& position, const glm::vec3& direction, float intensity = 2.0f);
    
    // Ambient light control
    void SetAmbientLight(const glm::vec3& ambient) { ambientLight = ambient; }
    glm::vec3 GetAmbientLight() const { return ambientLight; }
    
    // Get light count for shader
    int GetActiveLightCount() const { return static_cast<int>(lights.size()); }
};

// Global lighting instance
extern std::unique_ptr<LightingSystem> g_LightingSystem;

// Convenience functions
namespace Lighting {
    inline void Initialize() {
        if (!g_LightingSystem) {
            g_LightingSystem = std::unique_ptr<LightingSystem>(new LightingSystem());
        }
    }
    
    inline LightingSystem& Get() {
        if (!g_LightingSystem) Initialize();
        return *g_LightingSystem;
    }
}

#endif // LIGHTING_H