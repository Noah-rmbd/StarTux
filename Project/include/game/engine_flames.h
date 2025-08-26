#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "shader.h"
#include "node.h"

struct Flame {
    glm::vec3 position;  // Position relative to ship center
    glm::vec3 velocity;  // Velocity relative to ship
    glm::vec3 color;
    float life;          // 0.0 to 1.0, 1.0 = just born
    float size;
    float maxLife;       // How long this flame lives
    
    Flame(glm::vec3 pos, glm::vec3 vel, float lifetime = 0.3f) 
        : position(pos), velocity(vel), life(1.0f), maxLife(lifetime) {
        // Orange to red gradient for flames
        color = glm::vec3(1.0f, 0.6f + (rand() % 100) * 0.004f, 0.1f + (rand() % 100) * 0.002f);
        size = 0.00002f + (rand() % 100) * 0.00002f; // Much smaller flames
    }
};

class EngineFlames {
public:
    EngineFlames(Shader* shader);
    ~EngineFlames();
    
    void update(float deltaTime, const glm::vec3& shipDirection, bool boosting = false);
    void draw(const glm::mat4& view, const glm::mat4& projection, const glm::mat4& model);
    
    // Set engine positions relative to ship center
    void setEnginePositions(const glm::vec3& leftEngine, const glm::vec3& rightEngine);
    
    void setActive(bool active) { this->active = active; }
    bool isActive() const { return active; }

private:
    void spawnFlames(const glm::vec3& enginePos, const glm::vec3& direction, int count, float intensity);
    void updateFlames(float deltaTime);
    void setupBuffers();
    void updateBuffers();
    
    std::vector<Flame> flames;
    Shader* shader;
    
    // Engine positions relative to ship
    glm::vec3 leftEngineOffset;
    glm::vec3 rightEngineOffset;
    
    // OpenGL buffers for efficient rendering
    GLuint VAO, VBO;
    
    bool active = true;
    float spawnTimer = 0.0f;
    float spawnRate = 0.02f; // Spawn every 20ms
    
    // Flame properties
    int maxFlames = 100;
    float baseIntensity = 1.0f;
    float boostIntensity = 2.5f;
};