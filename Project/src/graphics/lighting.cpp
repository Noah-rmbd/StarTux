#include "lighting.h"
#include <iostream>
#include <algorithm>
#include <cmath>

std::unique_ptr<LightingSystem> g_LightingSystem = nullptr;

LightingSystem::LightingSystem() {
    // Initialize with basic space lighting
    SetupSpaceLighting();
    std::cout << "LightingSystem: Initialized with " << lights.size() << " lights" << std::endl;
}

int LightingSystem::AddLight(const Light& light) {
    lights.push_back(light);
    return static_cast<int>(lights.size() - 1);
}

void LightingSystem::RemoveLight(int lightId) {
    if (lightId >= 0 && lightId < static_cast<int>(lights.size())) {
        lights.erase(lights.begin() + lightId);
    }
}

void LightingSystem::UpdateLight(int lightId, const Light& light) {
    if (lightId >= 0 && lightId < static_cast<int>(lights.size())) {
        lights[lightId] = light;
    }
}

void LightingSystem::Update(float deltaTime) {
    for (auto& light : lights) {
        if (light.animated) {
            light.animationTime += deltaTime * light.animationSpeed;
            
            // Create subtle movement for animated lights
            float wobble = sin(light.animationTime) * 0.5f;
            light.position = light.originalPosition + glm::vec3(wobble, wobble * 0.3f, wobble * 0.7f);
            
            // Vary intensity slightly for more dynamic feel
            light.intensity = std::max(0.1f, 1.0f + sin(light.animationTime * 2.0f) * 0.3f);
        }
    }
}

void LightingSystem::ApplyToShader(Shader& shader, const glm::vec3& viewPos) {
    shader.use();
    
    // Set ambient light
    shader.setVec3("ambientLight", ambientLight);
    shader.setVec3("viewPos", viewPos);
    
    // Set number of lights
    int activeLights = std::min(static_cast<int>(lights.size()), 8); // Max 8 lights for performance
    shader.setInt("numLights", activeLights);
    
    // Set individual lights
    for (int i = 0; i < activeLights; i++) {
        const Light& light = lights[i];
        std::string base = "lights[" + std::to_string(i) + "]";
        
        shader.setInt(base + ".type", static_cast<int>(light.type));
        shader.setVec3(base + ".position", light.position);
        shader.setVec3(base + ".direction", light.direction);
        shader.setVec3(base + ".color", light.color);
        shader.setFloat(base + ".intensity", light.intensity);
        shader.setFloat(base + ".constant", light.constant);
        shader.setFloat(base + ".linear", light.linear);
        shader.setFloat(base + ".quadratic", light.quadratic);
        shader.setFloat(base + ".cutOff", light.cutOff);
        shader.setFloat(base + ".outerCutOff", light.outerCutOff);
    }
}

void LightingSystem::SetupSpaceLighting() {
    lights.clear();
    
    // Single main light - keep it simple for performance
    Light mainLight;
    mainLight.type = LightType::POINT;
    mainLight.position = glm::vec3(10.0f, 10.0f, 5.0f);
    mainLight.color = glm::vec3(1.0f, 0.95f, 0.9f); // Slightly warm white
    mainLight.intensity = 1.2f;
    AddLight(mainLight);
    
    // Set nicer space ambient lighting
    ambientLight = glm::vec3(0.05f, 0.05f, 0.12f); // Blue space ambient
}

void LightingSystem::SetupMenuLighting() {
    lights.clear();
    
    // Bright frontal light for menu visibility
    Light menuLight;
    menuLight.type = LightType::DIRECTIONAL;
    menuLight.direction = glm::vec3(0.0f, 0.0f, -1.0f);
    menuLight.color = glm::vec3(1.0f, 1.0f, 1.0f);
    menuLight.intensity = 1.2f;
    AddLight(menuLight);
    
    // Warmer ambient for better readability
    ambientLight = glm::vec3(0.3f, 0.3f, 0.3f);
}

void LightingSystem::SetupGameplayLighting(const glm::vec3& playerPos) {
    // Keep existing space lighting but add player-centered effects
    if (lights.empty()) {
        SetupSpaceLighting();
    }
    
    // Add ship engine glow
    AddEngineGlow(playerPos - glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, -1.0f), 1.5f);
}

void LightingSystem::AddExplosionLight(const glm::vec3& position, float intensity, float duration) {
    Light explosionLight;
    explosionLight.type = LightType::POINT;
    explosionLight.position = position;
    explosionLight.color = glm::vec3(1.0f, 0.6f, 0.2f); // Orange explosion
    explosionLight.intensity = intensity;
    explosionLight.constant = 1.0f;
    explosionLight.linear = 0.7f;
    explosionLight.quadratic = 1.8f;
    explosionLight.animated = true;
    explosionLight.animationSpeed = 3.0f / duration; // Fade over duration
    explosionLight.originalPosition = position;
    
    int lightId = AddLight(explosionLight);
    
    // TODO: Add timer system to remove explosion lights after duration
    // For now, they'll fade naturally through animation
}

void LightingSystem::AddEngineGlow(const glm::vec3& position, const glm::vec3& direction, float intensity) {
    Light engineLight;
    engineLight.type = LightType::SPOT;
    engineLight.position = position;
    engineLight.direction = direction;
    engineLight.color = glm::vec3(0.3f, 0.7f, 1.0f); // Blue engine glow
    engineLight.intensity = intensity;
    engineLight.cutOff = glm::cos(glm::radians(15.0f));
    engineLight.outerCutOff = glm::cos(glm::radians(25.0f));
    engineLight.constant = 1.0f;
    engineLight.linear = 0.35f;
    engineLight.quadratic = 0.44f;
    engineLight.animated = true;
    engineLight.animationSpeed = 8.0f; // Fast flicker
    engineLight.originalPosition = position;
    
    AddLight(engineLight);
}