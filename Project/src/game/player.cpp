#include "player.h"
#ifndef RESSOURCES_DIR
#error "RESSOURCES_DIR not defined"
#endif

Player::Player(Shader* shader_program)
{
    ship_dir = RESSOURCES_DIR;
    // Create texture shader
    std::string shader_dir = SHADER_DIR;
    texture_shader = new Shader(shader_dir + "ship.vert", shader_dir + "ship.frag");
    
    // Load texture
    ship_texture = new Texture(ship_dir + "Material.001_Base_color.jpg");
    
    // Create model with texture shader
    model = new ShapeModel(ship_dir + "ship.obj", texture_shader);
    static_cast<ShapeModel*>(model)->setTexture(ship_texture);
    
    // Initialize the transformation matrix
    playerMat = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), scale);
    
    // Create and initialize the node with the transformation matrix
    node = new Node(playerMat);
    
    // Add the model to the node
    node->add(model);
    
    // Initialize collision points
    setupCollisionPoints();
    
    // Set up debug visualization
    debugShader = shader_program;  // Reuse the main shader for now
    setupDebugSpheres();
}

Player::~Player() {
    // Clean up debug spheres
    for (auto sphere : debugSpheres) {
        delete sphere;
    }
    for (auto debugNode : debugNodes) {
        delete debugNode;
    }
    
    delete node;
    delete model;
    delete ship_texture;
    delete texture_shader;
}

void Player::updatePosition(){
    node->transform_ = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), scale) 
    * glm::rotate(glm::mat4(1.0f), glm::radians(xAngle), glm::vec3(1.0f, 0.0f, 0.0f))
    * glm::rotate(glm::mat4(1.0f), glm::radians(yAngle), glm::vec3(0.0f, 1.0f, 0.0f))
    * glm::rotate(glm::mat4(1.0f), glm::radians(zAngle), glm::vec3(0.0f, 0.0f, 1.0f));
    
    // Update debug collision spheres
    updateDebugSpheres();
}

void Player::increaseLife(){
    if (life < 3){
        life += 1;
    }
}

void Player::increaseBullets(){
    if (bullets < 10) {
        bullets += 1;
    } else {
        score += 100;
    }
}

void Player::updateShield(double time) {
    if (time - shieldStart >= shieldDuration) {
        shieldIsActive = false;
    }
}
void Player::createShield(double start, float duration) {
    shieldIsActive = true;
    shieldStart = start;
    shieldDuration = duration;
}

void Player::damage(float time) {
    if (shieldIsActive == false) {
        life -= 1;
        createShield(time, 5.0);
    }
}

bool Player::isDead() {
    return life<=0;
}

// Multi-point collision system implementation
void Player::setupCollisionPoints() {
    collisionPoints.clear();
    
    // Front center collision point (nose of ship)
    collisionPoints.push_back({
        glm::vec3(0.0f, 0.0f, 0.06f),  // Offset from center (forward)
        0.03f,                         // Collision radius  
        DAMAGED_TOP,                   // Damage type (front impact)
        "Front Center"                 // Debug name
    });
    
    // Back center collision point (rear of ship)
    collisionPoints.push_back({
        glm::vec3(0.0f, 0.0f, -0.06f), // Offset from center (backward)
        0.03f,                         // Collision radius
        DAMAGED_BOTTOM,                // Damage type (rear impact)
        "Back Center"                  // Debug name
    });
    
    // Left wing collision point
    collisionPoints.push_back({
        glm::vec3(-0.06f, 0.0f, 0.02f), // Offset from center (left side)
        0.03f,                         // Smaller radius for wing
        DAMAGED_LEFT,                  // Damage type (left impact)
        "Left Wing"                    // Debug name
    });
    
    // Right wing collision point
    collisionPoints.push_back({
        glm::vec3(0.06f, 0.0f, 0.02f),  // Offset from center (right side)
        0.03f,                         // Smaller radius for wing
        DAMAGED_RIGHT,                 // Damage type (right impact)
        "Right Wing"                   // Debug name
    });
}

void Player::updateCollisionPoints() {
    // This method can be used to dynamically adjust collision points
    // based on ship rotation, scaling, or other factors if needed
    // For now, collision points are relative to ship center and scale with the ship
}

glm::vec3 Player::getWorldCollisionPoint(int index) const {
    if (index < 0 || index >= collisionPoints.size()) {
        return position; // Return ship center if invalid index
    }
    
    // Transform local collision point to world coordinates
    // Apply ship rotation and position
    glm::mat4 rotationMatrix = 
        glm::rotate(glm::mat4(1.0f), glm::radians(xAngle), glm::vec3(1.0f, 0.0f, 0.0f)) *
        glm::rotate(glm::mat4(1.0f), glm::radians(yAngle), glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::rotate(glm::mat4(1.0f), glm::radians(zAngle), glm::vec3(0.0f, 0.0f, 1.0f));
    
    // Scale the local offset and transform it
    glm::vec3 scaledOffset = collisionPoints[index].localOffset;
    glm::vec4 rotatedOffset = rotationMatrix * glm::vec4(scaledOffset, 1.0f);
    
    return position + glm::vec3(rotatedOffset);
}

int Player::checkCollisionPoint(glm::vec3 objectPos, float objectRadius) const {
    for (int i = 0; i < collisionPoints.size(); i++) {
        glm::vec3 collisionWorldPos = getWorldCollisionPoint(i);
        float distance = glm::distance(collisionWorldPos, objectPos);
        
        // Check if collision occurs (sum of radii)
        if (distance < (collisionPoints[i].radius + objectRadius)) {
            return i; // Return index of colliding point
        }
    }
    
    return -1; // No collision
}

void Player::setupDebugSpheres() {
    debugSpheres.clear();
    debugNodes.clear();
    
    // Colors for different collision points
    std::vector<glm::vec3> colors = {
        glm::vec3(1.0f, 0.0f, 0.0f),  // Red for front center
        glm::vec3(0.0f, 0.0f, 1.0f),  // Blue for back center  
        glm::vec3(1.0f, 1.0f, 0.0f),  // Yellow for left wing
        glm::vec3(0.0f, 1.0f, 0.0f)   // Green for right wing
    };
    
    glm::vec3 lightPos = glm::vec3(0.0f, 2.0f, 2.0f);
    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    
    for (int i = 0; i < collisionPoints.size(); i++) {
        // Create debug sphere for this collision point
        LightingSphere* debugSphere = new LightingSphere(debugShader, lightPos, lightColor, colors[i]);
        debugSpheres.push_back(debugSphere);
        
        // Create node for the debug sphere
        glm::mat4 sphereTransform = glm::scale(glm::mat4(1.0f), glm::vec3(collisionPoints[i].radius));
        Node* sphereNode = new Node(sphereTransform);
        sphereNode->add(debugSphere);
        debugNodes.push_back(sphereNode);
    }
}

void Player::updateDebugSpheres() {
    if (!showCollisionDebug) return;
    
    for (int i = 0; i < debugNodes.size() && i < collisionPoints.size(); i++) {
        // Get world position for this collision point
        glm::vec3 worldPos = getWorldCollisionPoint(i);
        
        // Update debug sphere transform
        glm::mat4 sphereTransform = glm::translate(glm::mat4(1.0f), worldPos) * 
                                   glm::scale(glm::mat4(1.0f), glm::vec3(collisionPoints[i].radius));
        debugNodes[i]->transform_ = sphereTransform;
    }
}

void Player::addDebugSpheresToScene(Node* sceneRoot) {
    if (!showCollisionDebug) return;
    
    for (auto debugNode : debugNodes) {
        sceneRoot->add(debugNode);
    }
}

void Player::removeDebugSpheresFromScene(Node* sceneRoot) {
    for (auto debugNode : debugNodes) {
        sceneRoot->remove(debugNode);
    }
}