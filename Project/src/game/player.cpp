#include "player.h"
#include <cmath>
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
    
    // Initialize engine flames
    initializeEngineFlames();
    
    // Initialize collision points
    setupCollisionPoints();
    
    // Set up debug visualization
    debugShader = shader_program;  // Reuse the main shader for now
    setupDebugSpheres();
    
    // Set up shield visual with transparent shader
    std::string transparent_shader_dir = SHADER_DIR;
    transparentShader = new Shader(transparent_shader_dir + "phong.vert", transparent_shader_dir + "transparent_phong.frag");
    
    glm::vec3 lightPos = glm::vec3(0.0f, 2.0f, 2.0f);
    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 shieldColor = glm::vec3(0.0f, 0.8f, 1.0f); // Blue/cyan shield
    shieldSphere = new TransparentSphere(transparentShader, lightPos, lightColor, shieldColor, shieldOpacity);
    
    // Create shield node with larger scale to surround the ship
    glm::mat4 shieldTransform = glm::scale(glm::mat4(1.0f), glm::vec3(0.08f)); // Scale up the shield
    shieldNode = new Node(shieldTransform);
    shieldNode->add(shieldSphere);
    
    // Initialize statistics tracking
    gameStats = new GameStatistics();
    gameStats->resetSession(); // Start fresh session
    gameStartTime = 0.0; // Will be set when game actually starts
}

Player::~Player() {
    // Clean up engine flames
    if (engineFlames) {
        delete engineFlames;
    }
    if (flameShader) {
        delete flameShader;
    }
    
    // Clean up debug spheres
    for (auto sphere : debugSpheres) {
        delete sphere;
    }
    for (auto debugNode : debugNodes) {
        delete debugNode;
    }
    
    // Clean up shield
    delete shieldSphere;
    delete shieldNode;
    delete transparentShader;
    
    delete node;
    delete model;
    delete ship_texture;
    delete texture_shader;
    
    // Clean up statistics (will automatically save to file)
    delete gameStats;
}

void Player::updatePosition(){
    // Combine normal rotation with damage animation rotation
    float totalXAngle = xAngle + damageRotationX;
    float totalYAngle = yAngle + damageRotationY;
    float totalZAngle = zAngle + damageRotationZ;
    
    node->transform_ = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), scale) 
    * glm::rotate(glm::mat4(1.0f), glm::radians(totalXAngle), glm::vec3(1.0f, 0.0f, 0.0f))
    * glm::rotate(glm::mat4(1.0f), glm::radians(totalYAngle), glm::vec3(0.0f, 1.0f, 0.0f))
    * glm::rotate(glm::mat4(1.0f), glm::radians(totalZAngle), glm::vec3(0.0f, 0.0f, 1.0f));
    
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
        if (!damageAnimationActive) {
            shipState = NORMAL;
        }
    } else {
        // Ensure ship state is PROTECTED while shield is active
        if (!damageAnimationActive && shipState != PROTECTED) {
            shipState = PROTECTED;
        }
    }
    
    // Update shield visual
    updateShieldVisual(time);
}
void Player::createShield(double start, float duration) {
    shieldIsActive = true;
    shieldStart = start;
    shieldDuration = duration;
    if (!damageAnimationActive) {
        shipState = PROTECTED;
    }
}

void Player::damage(float time) {
    if (shieldIsActive == false) {
        life -= 1;
        createShield(time, 5.0);
    }
}

void Player::damageWithType(float time, ShipState damageType) {
    if (shieldIsActive == false) {
        life -= 1;
        createShield(time, 5.0);
        startDamageAnimation(time, damageType);
    }
}

void Player::startDamageAnimation(double startTime, ShipState damageType) {
    damageAnimationActive = true;
    damageAnimationStart = startTime;
    currentDamageType = damageType;
    
    // Reset damage rotation offsets
    damageRotationX = 0.0f;
    damageRotationY = 0.0f;
    damageRotationZ = 0.0f;
}

void Player::updateDamageAnimation(double currentTime) {
    if (!damageAnimationActive) return;
    
    double elapsedTime = currentTime - damageAnimationStart;
    
    // Check if animation is finished
    if (elapsedTime >= damageAnimationDuration) {
        damageAnimationActive = false;
        damageRotationX = 0.0f;
        damageRotationY = 0.0f;
        damageRotationZ = 0.0f;
        currentDamageType = PROTECTED;
        // Update ship state to PROTECTED if shield is still active
        if (shieldIsActive) {
            shipState = PROTECTED;
        }
        return;
    }
    
    // Calculate animation progress (0.0 to 1.0)
    double progress = elapsedTime / damageAnimationDuration;
    
    // Use sine wave for smooth animation that peaks in the middle and returns to normal
    double animationStrength = sin(progress * M_PI);
    
    // Maximum rotation angles for different damage types
    const float maxWingRotation = 25.0f;    // degrees for wing lean
    const float maxCenterRotation = 10.0f;  // degrees for nose dip
    
    // Apply different animations based on damage type
    switch (currentDamageType) {
        case DAMAGED_LEFT:
            // Ship leans to the left (rotate around Z-axis)
            damageRotationZ = maxWingRotation * animationStrength;
            break;
            
        case DAMAGED_RIGHT:
            // Ship leans to the right (rotate around Z-axis)
            damageRotationZ = -maxWingRotation * animationStrength;
            break;
            
        case DAMAGED_TOP:
        case DAMAGED_BOTTOM:
            // Nose dips down (rotate around X-axis)
            damageRotationX = -maxCenterRotation * animationStrength;
            break;
            
        default:
            // No animation for normal state
            break;
    }
}

bool Player::isDead() {
    return life<=0;
}

void Player::startDeathAnimation(double startTime) {
    deathAnimationActive = true;
    deathAnimationStart = startTime;
    shipState = DYING;
    
    // Set random death velocity (falling down and slightly sideways)
    deathVelocity.x = ((rand() % 200) / 100.0f) - 1.0f; // Random -1.0 to 1.0
    deathVelocity.y = -1.5f; // Fall downward
    deathVelocity.z = -0.5f; // Move slightly backward
    
    // Set random rotation speed
    deathRotationSpeed = ((rand() % 600) / 100.0f) + 2.0f; // Random 2.0 to 8.0 degrees per frame
}

void Player::updateDeathAnimation(double currentTime) {
    if (!deathAnimationActive) return;
    
    double elapsedTime = currentTime - deathAnimationStart;
    
    // Update position with death velocity
    position += deathVelocity * 0.02f; // Apply velocity with time scaling
    
    // Add gravity effect (accelerate downward)
    deathVelocity.y -= 0.04f; // Gravity acceleration
    
    // Add spinning rotation
    zAngle += deathRotationSpeed;
    yAngle += deathRotationSpeed * 0.5f;
    xAngle += deathRotationSpeed * 0.3f;
    
    // Keep angles in reasonable range
    if (zAngle > 360.0f) zAngle -= 360.0f;
    if (yAngle > 360.0f) yAngle -= 360.0f;
    if (xAngle > 360.0f) xAngle -= 360.0f;
}

bool Player::isDeathAnimationActive() const {
    return deathAnimationActive;
}

void Player::updateShieldVisual(double time) {
    if (shieldIsActive && shieldVisualActive) {
        // Update shield position to follow player
        glm::mat4 shieldTransform = glm::translate(glm::mat4(1.0f), position) * 
                                   glm::scale(glm::mat4(1.0f), glm::vec3(0.08f));
        shieldNode->transform_ = shieldTransform;
        
        // Calculate flashing as shield expires (last 1 second)
        double remainingTime = shieldDuration - (time - shieldStart);
        if (remainingTime <= 1.0 && remainingTime > 0.0) {
            // Flash faster as time runs out
            float flashFreq = 5.0f + (1.0f - remainingTime) * 10.0f; // 5-15 Hz
            float flash = (sin(time * flashFreq) + 1.0f) * 0.5f; // 0.0 to 1.0
            shieldOpacity = 0.1f + flash * 0.4f; // 0.1 to 0.5 opacity
        } else {
            shieldOpacity = 0.4f; // Normal shield opacity - more visible
        }
        
        // Update the shield sphere's alpha
        shieldSphere->setAlpha(shieldOpacity);
    }
}

void Player::addShieldToScene(Node* sceneRoot) {
    if (shieldIsActive && !shieldVisualActive) {
        sceneRoot->add(shieldNode);
        shieldVisualActive = true;
    }
}

void Player::removeShieldFromScene(Node* sceneRoot) {
    if (shieldVisualActive) {
        sceneRoot->remove(shieldNode);
        shieldVisualActive = false;
    }
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

void Player::initializeEngineFlames() {
    std::string shader_dir = SHADER_DIR;
    flameShader = new Shader(shader_dir + "flame.vert", shader_dir + "flame.frag");
    engineFlames = new EngineFlames(flameShader);
    
    // Set engine positions relative to ship center (scaled to match smaller flame size)
    glm::vec3 leftEngine = glm::vec3(-0.02f, 0.0055f, -0.045f);   // Scaled down 10x for smaller flames
    glm::vec3 rightEngine = glm::vec3(0.02f, 0.0055f, -0.045f);   // Scaled down 10x for smaller flames
    engineFlames->setEnginePositions(leftEngine, rightEngine);
}

void Player::updateEngineFlames(float deltaTime, bool boosting) {
    if (engineFlames) {
        // Get ship direction from current rotation
        glm::vec3 direction = glm::vec3(0.0f, 0.0f, 1.0f); // Forward direction
        engineFlames->update(deltaTime, direction, boosting);
    }
}

void Player::drawEngineFlames(const glm::mat4& view, const glm::mat4& projection) {
    if (engineFlames) {
        // Use the ship's transformation matrix so flames follow ship rotation and position
        engineFlames->draw(view, projection, node->transform_);
    }
}