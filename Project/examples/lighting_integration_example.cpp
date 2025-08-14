/**
 * Example integration of the new LightingSystem with existing game code
 * This demonstrates how to upgrade from single-light to multi-light rendering
 */

#include "lighting.h"
#include "resource_manager.h"

// Example of how to integrate the lighting system into Game class constructor:
void GameConstructorExample() {
    // Initialize lighting system early in Game constructor
    Lighting::Initialize();
    
    // Set up the lighting for the game scene
    Lighting::Get().SetupSpaceLighting();
    
    // Load enhanced shaders using ResourceManager
    auto enhancedPhongShader = Resources::GetShader("phong.vert", "phong_enhanced.frag");
    auto enhancedShipShader = Resources::GetShader("ship.vert", "ship_enhanced.frag");
    auto pbrShader = Resources::GetShader("pbr.vert", "pbr.frag");
}

// Example of how to update lighting in the game loop:
void GameUpdateExample(float deltaTime, const glm::vec3& playerPosition) {
    // Update dynamic lighting (animations, flickering, etc.)
    Lighting::Get().Update(deltaTime);
    
    // Update gameplay-specific lighting based on player position
    Lighting::Get().SetupGameplayLighting(playerPosition);
}

// Example of how to render objects with the new lighting system:
void RenderObjectExample(Shader& shader, const glm::vec3& cameraPosition) {
    shader.use();
    
    // Apply all lights to the shader
    Lighting::Get().ApplyToShader(shader, cameraPosition);
    
    // Set material properties for different object types:
    
    // For asteroids (rocky, rough surface):
    shader.setFloat("materialRoughness", 0.8f);
    shader.setFloat("materialMetallic", 0.05f);
    shader.setFloat("materialSpecular", 0.2f);
    
    // For ship (metallic, smooth):
    // shader.setFloat("shipRoughness", 0.3f);
    // shader.setFloat("shipMetallic", 0.7f);
    // shader.setFloat("shipSpecular", 0.8f);
    
    // For rings (glowing, special material):
    // shader.setFloat("materialRoughness", 0.1f);
    // shader.setFloat("materialMetallic", 0.9f);
    // shader.setFloat("materialSpecular", 1.0f);
    
    // Continue with normal rendering...
    // Draw the object as usual
}

// Example of adding dynamic lighting effects:
void AddExplosionEffectExample(const glm::vec3& explosionPosition) {
    // This creates a bright orange light that fades over 2 seconds
    Lighting::Get().AddExplosionLight(explosionPosition, 8.0f, 2.0f);
}

void AddEngineGlowExample(const glm::vec3& playerPosition, const glm::vec3& playerDirection) {
    // Add blue engine glow behind the player ship
    glm::vec3 enginePos = playerPosition - playerDirection * 2.0f;
    Lighting::Get().AddEngineGlow(enginePos, -playerDirection, 2.5f);
}

// Example of switching lighting modes:
void SetMenuLighting() {
    Lighting::Get().SetupMenuLighting(); // Bright, clear lighting for UI
}

void SetGameLighting() {
    Lighting::Get().SetupSpaceLighting(); // Atmospheric space lighting
}

/**
 * STEP-BY-STEP INTEGRATION GUIDE:
 * 
 * 1. Add to Game constructor:
 *    - Initialize lighting system
 *    - Load enhanced shaders
 *    - Set initial lighting mode
 * 
 * 2. Modify render loop:
 *    - Update lighting system each frame
 *    - Apply lights to shaders before rendering
 *    - Set appropriate material properties per object type
 * 
 * 3. Add dynamic effects:
 *    - Call AddExplosionLight() when asteroids are destroyed
 *    - Call AddEngineGlow() to show ship engines
 *    - Update lighting based on game events
 * 
 * 4. Replace old shaders:
 *    - Use "phong_enhanced.frag" instead of "phong.frag"
 *    - Use "ship_enhanced.frag" for the player ship
 *    - Use "pbr.frag" for high-quality objects
 * 
 * 5. Material properties:
 *    - Asteroids: High roughness, low metallic (rocky)
 *    - Ship: Medium roughness, high metallic (metal hull)
 *    - Rings: Low roughness, high metallic (shiny collectibles)
 *    - Projectiles: Vary by type (energy vs physical)
 */