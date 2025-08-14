#include "engine_flames.h"
#include <GL/glew.h>
#include <algorithm>
#include <iostream>
#include <cstdlib>

EngineFlames::EngineFlames(Shader* shader) : shader(shader) {
    // Default engine positions (behind the ship)
    leftEngineOffset = glm::vec3(-0.3f, -0.2f, -0.8f);   // Left engine
    rightEngineOffset = glm::vec3(0.3f, -0.2f, -0.8f);   // Right engine
    
    flames.reserve(maxFlames);
    setupBuffers();
    
    std::cout << "EngineFlames: Initialized with max " << maxFlames << " flames" << std::endl;
}

EngineFlames::~EngineFlames() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void EngineFlames::setEnginePositions(const glm::vec3& leftEngine, const glm::vec3& rightEngine) {
    leftEngineOffset = leftEngine;
    rightEngineOffset = rightEngine;
}

void EngineFlames::update(float deltaTime, const glm::vec3& shipDirection, bool boosting) {
    if (!active) return;
    
    spawnTimer += deltaTime;
    
    // Spawn new flames
    if (spawnTimer >= spawnRate) {
        spawnTimer = 0.0f;
        
        float intensity = boosting ? boostIntensity : baseIntensity;
        int flameCount = boosting ? 8 : 4; // More flames when boosting
        
        // Spawn flames from both engines (positions are relative to ship center)
        spawnFlames(leftEngineOffset, -shipDirection, flameCount, intensity);
        spawnFlames(rightEngineOffset, -shipDirection, flameCount, intensity);
    }
    
    // Update existing flames
    updateFlames(deltaTime);
}

void EngineFlames::spawnFlames(const glm::vec3& enginePos, const glm::vec3& direction, int count, float intensity) {
    for (int i = 0; i < count && flames.size() < maxFlames; ++i) {
        // Add some randomness to flame direction and position
        glm::vec3 randomOffset = glm::vec3(
            (rand() % 200 - 100) * 0.00004f,  // -0.2 to 0.2
            (rand() % 200 - 100) * 0.00004f,
            -(rand() % 100) * 0.0002f         // -0.5 to -0.6 (backward)
        );
        
        glm::vec3 randomDir = direction + glm::vec3(
            (rand() % 200 - 100) * 0.005f,  // Add spread to flame direction
            (rand() % 200 - 100) * 0.005f,
            (rand() % 100 - 50) * 0.002f
        );
        
        glm::vec3 velocity = randomDir * (2.0f + (rand() % 100) * 0.2f) * intensity * 0.0005f;
        float lifetime = 0.2f + (rand() % 100) * 0.003f; // 0.2 to 0.5 seconds
        
        flames.emplace_back(enginePos + randomOffset, velocity, lifetime);
    }
}

void EngineFlames::updateFlames(float deltaTime) {
    // Update and remove dead flames
    flames.erase(
        std::remove_if(flames.begin(), flames.end(), [deltaTime](Flame& flame) {
            flame.life -= deltaTime / flame.maxLife;
            flame.position += flame.velocity * deltaTime;
            
            // Flames slow down and cool over time
            flame.velocity *= 0.98f;
            flame.size *= 0.995f;
            
            // Color changes as flame dies (orange -> red -> dark)
            float lifeRatio = flame.life;
            flame.color = glm::vec3(
                1.0f,                                    // Always full red
                0.3f + lifeRatio * 0.5f,                // Green fades
                lifeRatio * 0.2f                        // Blue fades quickly
            );
            
            return flame.life <= 0.0f;
        }),
        flames.end()
    );
}

void EngineFlames::setupBuffers() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    // Reserve space for max flames * 6 vertices per flame (2 triangles)
    glBufferData(GL_ARRAY_BUFFER, maxFlames * 6 * 6 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    
    // Position attribute (location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Color attribute (location 1)  
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void EngineFlames::updateBuffers() {
    if (flames.empty()) return;
    
    std::vector<float> vertices;
    vertices.reserve(flames.size() * 6 * 6); // 6 vertices per flame, 6 floats per vertex
    
    for (const auto& flame : flames) {
        // Create a simple billboard quad for each flame
        float halfSize = flame.size * 0.5f;
        
        // Triangle 1
        vertices.insert(vertices.end(), {
            flame.position.x - halfSize, flame.position.y - halfSize, flame.position.z, flame.color.r, flame.color.g, flame.color.b,
            flame.position.x + halfSize, flame.position.y - halfSize, flame.position.z, flame.color.r, flame.color.g, flame.color.b,
            flame.position.x - halfSize, flame.position.y + halfSize, flame.position.z, flame.color.r, flame.color.g, flame.color.b,
        });
        
        // Triangle 2  
        vertices.insert(vertices.end(), {
            flame.position.x + halfSize, flame.position.y - halfSize, flame.position.z, flame.color.r, flame.color.g, flame.color.b,
            flame.position.x + halfSize, flame.position.y + halfSize, flame.position.z, flame.color.r, flame.color.g, flame.color.b,
            flame.position.x - halfSize, flame.position.y + halfSize, flame.position.z, flame.color.r, flame.color.g, flame.color.b,
        });
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void EngineFlames::draw(const glm::mat4& view, const glm::mat4& projection, const glm::mat4& model) {
    if (!active || flames.empty()) return;
    
    updateBuffers();
    
    shader->use();
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);
    
    // Extract the ship's scale to compensate for it in flame size
    // The ship scale is 0.005, so we need to scale flames up by ~200x to compensate
    glm::mat4 flameModel = model * glm::scale(glm::mat4(1.0f), glm::vec3(200.0f, 200.0f, 200.0f));
    shader->setMat4("model", flameModel);
    
    // Enable blending for flame transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending for flames
    
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, flames.size() * 6);
    glBindVertexArray(0);
    
    glDisable(GL_BLEND);
}