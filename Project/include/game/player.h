#ifndef PLAYER_H
#define PLAYER_H

#include "node.h"
#include "shader.h"
#include "texture.h"
#include "shape_model.h"
#include "lighting_sphere.h"
#include <glm/glm.hpp>

class Player{
    public :
        Player(Shader* shader_program);
        ~Player();
        void updatePosition();
        void updateShield(double time);
        void createShield(double start, float duration);
        void increaseLife();
        void increaseBullets();
        void damage(float time);
        bool isDead();

        enum ShipState {
            NORMAL = 0,
            ACCELERATING = 1,
            DAMAGED_LEFT = 2,
            DAMAGED_RIGHT = 3,
            DAMAGED_TOP = 4,
            DAMAGED_BOTTOM = 5
        };
        int shipState = NORMAL;

        Shape* model;
        Shader* texture_shader;
        Texture* ship_texture;

        glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 scale = glm::vec3(0.005f, 0.005f, 0.005f);
        glm::mat4 playerMat;
        Node* node;
        
        // Multi-point collision system
        struct CollisionPoint {
            glm::vec3 localOffset;    // Position relative to player center
            float radius;             // Collision radius
            ShipState damageType;     // What damage this collision causes
            std::string name;         // For debugging
        };
        
        std::vector<CollisionPoint> collisionPoints;
        bool showCollisionDebug = true;  // Toggle for debug spheres
        
        // Debug visualization
        std::vector<LightingSphere*> debugSpheres;
        std::vector<Node*> debugNodes;
        Shader* debugShader;
        
        // Collision point management
        void setupCollisionPoints();
        void setupDebugSpheres();
        void updateCollisionPoints();
        void updateDebugSpheres();
        void addDebugSpheresToScene(Node* sceneRoot);
        void removeDebugSpheresFromScene(Node* sceneRoot);
        glm::vec3 getWorldCollisionPoint(int index) const;
        int checkCollisionPoint(glm::vec3 objectPos, float objectRadius) const;

        float xAngle = 0.0f;
        float yAngle = 0.0f;
        float zAngle = 0.0f;
        float movement_speed = 0.04f;
        float fps_correction = 1.0f;

        double shieldStart = 0.0;
        double shieldDuration = 0.0;
        bool shieldIsActive = false;

        int life = 3;
        int bullets = 10;
        double score = 0;

    private:
        std::string ship_dir;
}; 

#endif