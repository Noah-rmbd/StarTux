#include "hud.h"
#include "glm/fwd.hpp"
#include "interface.h"
#include <cstddef>
#include <iostream>
#include <string>
#include <utility>
#include <cmath>
#include <algorithm>
#ifndef TEXTURES_DIR
#error "TEXTURES_DIR not defined"
#endif

// Define static constants
const float Hud::BAR_THICKNESS = 20.0f;
const float Hud::TOP_BAR_WIDTH = 400.0f;
const float Hud::SIDE_BAR_HEIGHT = 300.0f;
const float Hud::X_POSITION_RANGE = 1.5f;  // X: -1.5 to 1.5
const float Hud::Y_POSITION_RANGE = 1.0f;  // Y: -1.0 to 1.0  
const float Hud::Z_POSITION_RANGE = 2.0f;  // Z: estimated range
const float Hud::ROTATION_RANGE = 45.0f;

Hud::Hud(int width, int height) : windowWidth(width), windowHeight(height){
    std::string textures_dir = TEXTURES_DIR;
    
    // Load textures
    aim_image = new Texture(textures_dir + "aim.png");
    dialogBoxTexture = new Texture(textures_dir + "text_box.png");
    leftPanelTexture = new Texture(textures_dir + "HUD_container_L.png");   // Will be loaded when you create the panel backgrounds
    rightPanelTexture = new Texture(textures_dir + "HUD_container_R.png");
    shieldIconTexture = new Texture(textures_dir + "shield_icon.png");
    cockpitFrameTexture = nullptr;
    
    game_interface = new Interface(windowWidth, windowHeight);
    
    // Initialize 3D HUD settings
    is3DHudEnabled = true;  // Enable 3D HUD
    panelDistance = 2.0f;   // Distance from camera
    panelAngle = 45.0f;     // Increased angle for more distinctive cockpit feel
    panelWidth = 1.5f;
    panelHeight = 1.0f;
    
    // Initialize panel IDs
    leftPanelId = -1;
    rightPanelId = -1;
    cockpitFrameId = -1;
    
    // Initialize dialog and feedback pointers
    currentDialog = nullptr;
    scoreFeedback = nullptr;
    
    // Set up 3D HUD panels
    setup3DHUD();
}

void Hud::update(int life, double score, int bullets, double time, int speed, int fps, 
                glm::mat4 view, glm::mat4 projection, glm::vec3 playerPos, 
                glm::vec3 playerRotation, int shipState, bool paused, bool invincible, bool shieldActive, bool playerDying) {
    
    // Update fade state
    updateFade(time, playerDying);
    
    // Don't render HUD if nearly invisible
    if (currentAlpha < 0.01f) {
        return;
    }
    
    if (is3DHudEnabled) {
        // Use new 3D HUD system
        game_interface->beginFrame(view, projection);
        
        // Update panel positions based on camera
        update3DPanels();
        
        // Render different sections of the HUD (cursor last since it uses CURSOR layer)
        renderLeftPanelContent(life, bullets, time, shieldActive);
        renderRightPanelContent(score, speed, fps);
        renderCenterContent(time, paused, invincible);
        renderPositionBars(playerPos, playerRotation, shipState);
        renderCursor();
        
        // Handle score feedback color changes
        if (scoreFeedback != nullptr) {
            scoreColor = glm::vec3(0.0f, 1.0f, 0.0f);  // Green for score feedback
            if(time - scoreFeedback->first.second > 1.0) {
                delete scoreFeedback;
                scoreFeedback = nullptr;
            } else if (time - scoreFeedback->first.second > 0.25) {
                scoreColor = glm::vec3(1.0f);
            }
        } else {
            scoreColor = glm::vec3(1.0f);
        }
        
        // Clean up expired dialogs
        if (currentDialog != nullptr && time - currentDialog->second > 10.0) {
            delete currentDialog;
            currentDialog = nullptr;
        }
        
        game_interface->endFrame();
    } else {
        // Fallback to 2D HUD for compatibility
        render2DHUD(life, score, bullets, time, speed, fps, shieldActive, playerDying);
    }
}

void Hud::newDialog(int number, double time) {
    currentDialog = new std::pair<std::string, double>(dialogs[number], time);
}

void Hud::mouse(double xpos, double ypos){
    xPos = xpos;
    yPos = ypos;
}

void Hud::scoreIncrement(int xpos, int ypos, double time, int score){
    scoreFeedback = new std::pair<std::pair<std::string, double>, std::pair<int, int>>(
        std::make_pair("+" + std::to_string(score) + "p", time),
        std::make_pair(xpos, ypos)
    );
}

// 3D HUD setup and management
void Hud::setup3DHUD() {
    calculatePanelPositions();
    
    // Create left panel (for dialogs and secondary info)
    leftPanelId = game_interface->createPanel(
        leftPanelPos, leftPanelRot, 
        glm::vec2(panelWidth, panelHeight), 
        nullptr
    );
    
    // Create right panel (for main stats)
    rightPanelId = game_interface->createPanel(
        rightPanelPos, rightPanelRot, 
        glm::vec2(panelWidth, panelHeight), 
        nullptr
    );
}

void Hud::update3DPanels() {
    // Recalculate panel positions (in case camera has moved)
    calculatePanelPositions();
    
    // Update panel positions
    if (leftPanelId >= 0) {
        game_interface->updatePanel(leftPanelId, leftPanelPos, leftPanelRot);
    }
    if (rightPanelId >= 0) {
        game_interface->updatePanel(rightPanelId, rightPanelPos, rightPanelRot);
    }
}

void Hud::calculatePanelPositions() {
    // Calculate positions for cockpit-style panels
    float angleRad = glm::radians(panelAngle);
    
    // Left panel - angled inward
    leftPanelPos = glm::vec3(
        -panelDistance * sin(angleRad),  // X position (left side)
        0.0f,                            // Y position (center height)
        panelDistance * cos(angleRad)    // Z position (forward)
    );
    leftPanelRot = glm::vec3(0.0f, panelAngle, 0.0f);  // Rotate to face camera
    
    // Right panel - angled inward
    rightPanelPos = glm::vec3(
        panelDistance * sin(angleRad),   // X position (right side)
        0.0f,                            // Y position (center height)
        panelDistance * cos(angleRad)    // Z position (forward)
    );
    rightPanelRot = glm::vec3(0.0f, -panelAngle, 0.0f); // Rotate to face camera
}

void Hud::renderLeftPanelContent(int life, int bullets, double time, bool shieldActive) {
    // Calculate left panel screen position (simulate 3D positioning with increased angle)
    // Create a more dramatic perspective effect
    float baseX = windowWidth * 0.02f;   // Start very close to left edge
    float baseY = windowHeight * 0.7f;   // Higher up
    
    // Create perspective distortion by varying Y positions based on panel angle
    float angleOffset = sin(glm::radians(panelAngle)) * 100.0f; // More dramatic offset

    // Dimensions of the left panel container
    float containerWidth = 204.0f;
    float containerHeight = 123.0f;

    // Render left panel container (with fade effect)
    game_interface->addImageElement(
        leftPanelTexture,
        glm::vec3(baseX + 10.0f, baseY + angleOffset - containerHeight/2 - 15.0f, -0.1f), // Angled position
        glm::vec2(containerWidth, containerHeight),                          // Size based on text
        glm::vec4(1.0f, 1.0f, 1.0f, 0.8f * currentAlpha),                                                             // White with transparency
        false,                                                                                           // 2D image (screen space)
        -1                                                                                               // No panel attachment
    );

    // Life display on left panel with perspective (with fade effect)
    game_interface->addTextOverlay(
        "LIFE: " + std::to_string(life),
        glm::vec3(baseX + 45.0f, baseY + angleOffset, 0.0f),                // Angled position
        0.5f,                                                         // Scale
        glm::vec3(1.0f, 0.3f, 0.3f) * currentAlpha,                                // Red color with fade
        false,                                                        // 2D text (screen space)
        -1                                                            // No panel attachment
    );

    float iconWidth = 26.0f;
    float iconHeight = 28.0f;

    // Shield icon next to life display (with fade effect)
    if (shieldActive) {
        game_interface->addImageElement(
            shieldIconTexture,  // Simple shield icon
            glm::vec3(baseX + 130.0f, baseY + angleOffset - 5.0f, 0.0f),           // Next to life display
            glm::vec2(iconWidth, iconHeight),                               // Size of the icon
            glm::vec4(1.0f, 1.0f, 1.0f, 0.8f * currentAlpha),                            
            false,                                                    // 2D image (screen space)
            -1                                                        // No panel attachment
        );
    }

    // Render ammunition counter with perspective (with fade effect)
    game_interface->addTextOverlay(
        std::to_string(bullets) + "/10",
        glm::vec3(baseX + 100.0f, baseY + angleOffset - 50.0f, 0.0f),  // Slightly offset and angled
        0.7f,                                                 // Scale
        glm::vec3(1.0f, 1.0f, 0.0f) * currentAlpha,                        // Yellow color with fade
        false,                                                // 2D text (screen space)
        -1                                                    // No panel attachment
    ); // 65.0f and 0.8f
    
    // Render dialogs on the left panel area
    if (currentDialog != nullptr) {
        // Calculate dialog box image dimensions
        float dialogWidth = 763.0f;
        float dialogHeight = 139.0f;
        
        // Render dialog box background with perspective
        game_interface->addImageElement(
            dialogBoxTexture,
            glm::vec3(baseX + 10.0f, baseY + angleOffset - 580.0f - dialogHeight/2, -0.1f), // Angled position
            glm::vec2(dialogWidth, dialogHeight),                          // Size based on text
            glm::vec4(1.0f, 1.0f, 1.0f, 0.8f),                                                             // White with transparency
            false,                                                                                           // 2D image (screen space)
            -1                                                                                               // No panel attachment
        ); // - 80
        
        // Render dialog text with perspective
        game_interface->addTextOverlay(
            currentDialog->first,
            glm::vec3(baseX + 220.0f, baseY + angleOffset - 570.0f, 0.1f), // Angled position
            0.5f,                                                                   // Scale
            glm::vec3(1.0f, 1.0f, 1.0f),                                          // White text for readability on dark box
            false,                                                                  // 2D text (screen space)
            -1                                                                      // No panel attachment
        ); // + 130

        // Render captain text with perspective
        game_interface->addTextOverlay(
            "Captain",
            glm::vec3(baseX + 200.0f, baseY + angleOffset - 620.0f, 0.1f), // Angled position
            0.4f,                                                                   // Scale
            glm::vec3(0.8f, 0.8f, 0.8f),                                          // White text for readability on dark box
            false,                                                                  // 2D text (screen space)
            -1                                                                      // No panel attachment
        ); // +110
    }
}

void Hud::renderRightPanelContent(double score, int speed, int fps) {
    // Format score string
    std::string scoreString = std::to_string(score);
    if (scoreString.length() > 5) {
        scoreString = scoreString.substr(0, scoreString.length()-5);
    }
    
    // Calculate right panel screen position (simulate 3D positioning with increased angle)
    float baseX = windowWidth * 0.98f;   // Very close to right edge
    float baseY = windowHeight * 0.85f;  // Higher up for better cockpit feel
    
    // Create perspective distortion by varying positions based on panel angle (opposite direction)
    float angleOffset = -sin(glm::radians(panelAngle)) * 100.0f; // Negative for right panel
    
    // Dimensions of the right panel container
    float containerWidth = 204.0f;
    float containerHeight = 123.0f;

    // Render left panel container
    game_interface->addImageElement(
        rightPanelTexture,
        glm::vec3(baseX - 10.0f - containerWidth, baseY + angleOffset - containerHeight/2 - 15.0f, -0.1f), // Angled position
        glm::vec2(containerWidth, containerHeight),                          // Size based on text
        glm::vec4(1.0f, 1.0f, 1.0f, 0.8f),                                                             // White with transparency
        false,                                                                                           // 2D image (screen space)
        -1                                                                                               // No panel attachment
    );


    // Score display on right panel with perspective
    game_interface->addTextOverlay(
        "SCORE: " + scoreString,
        glm::vec3(baseX - 60.0f - containerWidth/2, baseY + angleOffset, 0.0f), // Angled position
        0.4f,                                                           // Scale
        scoreColor,                                                     // Dynamic color
        false,                                                          // 2D text (screen space)
        -1                                                              // No panel attachment
    );

    // Render speed indicator with perspective
    game_interface->addTextOverlay(
        std::to_string(speed) + " KM/h",
        glm::vec3(baseX - 85.0f - containerWidth/2, baseY + angleOffset - 50.0f, 0.0f),          // Angled position
        0.6f,                                                 // Scale
        glm::vec3(0.0f, 1.0f, 0.0f),                        // Green color
        false,                                                // 2D text (screen space)
        -1                                                    // No panel attachment
    );
    
    // FPS display on right panel with perspective
    game_interface->addTextOverlay(
        "FPS: " + std::to_string(fps),
        glm::vec3(baseX - 100.0f - containerWidth/2, baseY + angleOffset - 100.0f, 0.0f), // Angled position
        0.3f,                                                            // Smaller scale
        glm::vec3(0.7f, 0.7f, 0.7f),                                   // Gray color
        false,                                                           // 2D text (screen space)
        -1                                                               // No panel attachment
    );
}

void Hud::renderCenterContent(double time, bool paused, bool invincible) {
    // Render score feedback at the given cursor position
    if (scoreFeedback != nullptr) {
        game_interface->addTextOverlay(
            scoreFeedback->first.first,
            glm::vec3(scoreFeedback->second.first, windowHeight-scoreFeedback->second.second, 0.0f), // Screen space position
            0.6f,                                                         // Larger scale for emphasis
            glm::vec3(0.0f, 1.0f, 0.0f),                                // Green color
            false,                                                        // 2D text (screen space)
            -1                                                            // No panel attachment
        );
    }
    
    // Render pause indicator
    if (paused) {
        game_interface->addTextOverlay(
            "PAUSED",
            glm::vec3(windowWidth * 0.5f - 60.0f, windowHeight * 0.5f, 0.0f), // Center of screen
            1.2f,                                                             // Large scale
            glm::vec3(1.0f, 0.0f, 0.0f),                                    // Red color
            false,                                                            // 2D text (screen space)
            -1                                                                // No panel attachment
        );
        
        game_interface->addTextOverlay(
            "Press V to resume",
            glm::vec3(windowWidth * 0.5f - 90.0f, windowHeight * 0.5f - 50.0f, 0.0f), // Below pause text
            0.6f,                                                             // Medium scale
            glm::vec3(1.0f, 1.0f, 1.0f),                                    // White color
            false,                                                            // 2D text (screen space)
            -1                                                                // No panel attachment
        );
    }
    
    // Render invincibility indicator
    if (invincible) {
        game_interface->addTextOverlay(
            "INVINCIBLE",
            glm::vec3(windowWidth * 0.5f - 80.0f, windowHeight - 100.0f, 0.0f), // Top center
            0.8f,                                                             // Medium-large scale
            glm::vec3(0.0f, 1.0f, 1.0f),                                    // Cyan color
            false,                                                            // 2D text (screen space)
            -1                                                                // No panel attachment
        );
    }
}

void Hud::renderCursor() {
    // Render crosshair cursor at mouse position in CURSOR layer - deepest Z so it's behind everything
    game_interface->addCursorElement(
        aim_image,
        glm::vec3(xPos, windowHeight - yPos, -0.5f),  // Screen space position with deep Z
        glm::vec2(50.0f, 50.0f),                      // Size
        glm::vec4(1.0f, 1.0f, 1.0f, 0.8f)            // White with transparency
    );
}

// Legacy 2D HUD for backward compatibility
void Hud::updateFade(double time, bool playerDying) {
    if (playerDying && !isFading) {
        // Start fading
        isFading = true;
        fadeStartTime = time;
    }
    
    if (isFading) {
        double elapsedTime = time - fadeStartTime;
        float progress = std::min(1.0f, static_cast<float>(elapsedTime / fadeDuration));
        currentAlpha = 1.0f - progress; // Fade from 1.0 to 0.0
    } else {
        currentAlpha = 1.0f; // Full opacity when not fading
    }
}

void Hud::render2DHUD(int life, double score, int bullets, double time, int speed, int fps, bool shieldActive, bool playerDying) {
    // Format score string
    std::string scoreString = std::to_string(score);
    if (scoreString.length() > 5) {
        scoreString = scoreString.substr(0, scoreString.length()-5);
    }
    
    // Apply fade to all HUD elements by reducing their opacity
    if (currentAlpha < 0.01f) {
        // Don't render HUD if nearly invisible
        return;
    }
    
    // Render main stats with proper spacing (with fade effect)
    std::string lifeText = "Life : " + std::to_string(life);
    if (shieldActive) {
        lifeText += " [SHIELD]";
    }
    game_interface->renderText(lifeText, 25.0f, windowHeight-60, 0.5f, glm::vec3(1.0f, 0.3f, 0.3f) * currentAlpha);
    if (shieldActive) {
        // Render shield indicator in cyan color
        game_interface->renderText("[SHIELD]", 25.0f + lifeText.length() * 12.0f - 70.0f, windowHeight-60, 0.5f, glm::vec3(0.0f, 0.8f, 1.0f) * currentAlpha);
    }
    game_interface->renderText("FPS : " + std::to_string(fps), windowWidth-150, windowHeight-60, 0.5f, glm::vec3(0.7f, 0.7f, 0.7f) * currentAlpha);
    game_interface->renderText("Score : " + scoreString, 25.0f, windowHeight-100, 0.5f, scoreColor * currentAlpha);
    
    // Render bottom info with better spacing
    game_interface->renderText(std::to_string(bullets) + " /10", 25.0f, windowHeight-750, 0.5f, glm::vec3(1.0f, 1.0f, 0.0f) * currentAlpha);
    game_interface->renderText(std::to_string(speed) + " Km/h", 25.0f, windowHeight-800, 0.8f, glm::vec3(0.0f, 1.0f, 0.0f) * currentAlpha);
    
    // Render cursor (with fade effect)
    game_interface->renderImage(aim_image, xPos, windowHeight-yPos, 50.0f, 50.0f, glm::vec4(1.0f, 1.0f, 1.0f, currentAlpha));
    
    // Render dialogs (with fade effect)
    if (currentDialog != nullptr) {
        game_interface->renderText(currentDialog->first, 25.0f, windowHeight-500, 0.5f, glm::vec3(0.0f, 1.0f, 1.0f) * currentAlpha);
        if(time - currentDialog->second > 10.0) {
            delete currentDialog;
            currentDialog = nullptr;
        }
    }
    
    // Render score feedback
    if (scoreFeedback != nullptr) {
        game_interface->renderText(scoreFeedback->first.first, scoreFeedback->second.first, windowHeight-scoreFeedback->second.second, 0.5f, glm::vec3(0.0f, 1.0f, 0.0f));
        scoreColor = glm::vec3(0.0f, 1.0f, 0.0f);
        if(time - scoreFeedback->first.second > 1.0) {
            delete scoreFeedback;
            scoreFeedback = nullptr;
        } else if (time - scoreFeedback->first.second > 0.25) {
            scoreColor = glm::vec3(1.0f);
        }
    } else {
        scoreColor = glm::vec3(1.0f);
    }
}

// Position and rotation bar implementation
void Hud::renderPositionBars(glm::vec3 playerPos, glm::vec3 playerRotation, int shipState) {
    // Render top horizontal position bar (X position)
    renderTopPositionBar(playerPos.x, shipState);
    
    // Render left vertical position bar (Z position + rotation)
    renderLeftPositionBar(playerPos.z, playerRotation.y, playerRotation.z, 
                         (shipState == DAMAGED_RIGHT) ? DAMAGED_RIGHT : ((shipState == ACCELERATING) ? ACCELERATING : NORMAL));
    
    // Render right vertical position bar (Y position + rotation)
    renderRightPositionBar(playerPos.y, playerRotation.y, playerRotation.x, 
                          (shipState == DAMAGED_LEFT) ? DAMAGED_LEFT : ((shipState == ACCELERATING) ? ACCELERATING : NORMAL));
}

void Hud::renderTopPositionBar(float xPosition, int shipState) {
    float centerX = windowWidth * 0.5f;
    float topY = windowHeight * 0.05f;
    
    glm::vec3 barColor = getBarColor((shipState == DAMAGED_TOP) ? DAMAGED_TOP : ((shipState == ACCELERATING) ? ACCELERATING : NORMAL));
    
    // Normalize X position from [-1.5, 1.5] to [0, 1] range
    float normalizedPos = (xPosition + X_POSITION_RANGE) / (2.0f * X_POSITION_RANGE);
    normalizedPos = std::max(0.0f, std::min(1.0f, normalizedPos));
    
    drawGraduatedBar(
        glm::vec3(centerX - TOP_BAR_WIDTH/2, topY, 0.0f),
        glm::vec2(TOP_BAR_WIDTH, BAR_THICKNESS),
        true,  // horizontal
        normalizedPos,
        barColor
    );
}

void Hud::renderLeftPositionBar(float zPosition, float yRotation, float zRotation, int shipState) {
    float leftX = windowWidth * 0.05f;
    float centerY = windowHeight * 0.5f;
    
    glm::vec3 barColor = getBarColor(shipState);
    
    // Normalize Z position from [-Z_POSITION_RANGE, Z_POSITION_RANGE] to [0, 1] range
    float normalizedPos = (zPosition + Z_POSITION_RANGE) / (2.0f * Z_POSITION_RANGE);
    normalizedPos = std::max(0.0f, std::min(1.0f, normalizedPos));
    
    // Draw left bar without cursor (since no Z-axis movement)
    drawGraduatedBar(
        glm::vec3(leftX, centerY - SIDE_BAR_HEIGHT/2, 0.0f),
        glm::vec2(BAR_THICKNESS, SIDE_BAR_HEIGHT),
        false,  // vertical
        normalizedPos,
        barColor,
        false   // showCursor = false
    );
    
    // Add rotation gauge
    float combinedRotation = yRotation + zRotation;  // Combine Y and Z rotation for left wing
    drawRotationGauge(
        glm::vec3(leftX + BAR_THICKNESS + 5.0f, centerY, 0.0f),
        glm::vec2(15.0f, SIDE_BAR_HEIGHT),
        combinedRotation,
        barColor,
        true  // left side
    );
}

void Hud::renderRightPositionBar(float yPosition, float yRotation, float xRotation, int shipState) {
    float rightX = windowWidth * 0.95f;
    float centerY = windowHeight * 0.5f;
    
    glm::vec3 barColor = getBarColor(shipState);
    
    // Normalize Y position from [-1.0, 1.0] to [0, 1] range
    float normalizedPos = (yPosition + Y_POSITION_RANGE) / (2.0f * Y_POSITION_RANGE);
    normalizedPos = std::max(0.0f, std::min(1.0f, normalizedPos));
    
    drawGraduatedBar(
        glm::vec3(rightX - BAR_THICKNESS, centerY - SIDE_BAR_HEIGHT/2, 0.0f),
        glm::vec2(BAR_THICKNESS, SIDE_BAR_HEIGHT),
        false,  // vertical
        normalizedPos,
        barColor
    );
    
    // Add rotation gauge
    float combinedRotation = yRotation + xRotation;  // Combine Y and X rotation for right wing
    drawRotationGauge(
        glm::vec3(rightX - BAR_THICKNESS - 20.0f, centerY, 0.0f),
        glm::vec2(15.0f, SIDE_BAR_HEIGHT),
        combinedRotation,
        barColor,
        false  // right side
    );
}

glm::vec3 Hud::getBarColor(int shipState) {
    switch(shipState) {
        case ACCELERATING:
            return glm::vec3(0.0f, 0.5f, 1.0f);  // Blue
        case DAMAGED_LEFT:
        case DAMAGED_RIGHT:
        case DAMAGED_TOP:
        case DAMAGED_BOTTOM:
            return glm::vec3(1.0f, 0.0f, 0.0f);  // Red
        case PROTECTED:
            return glm::vec3(0.0f, 1.0f, 0.0f);  // Green
        case DYING:
            return glm::vec3(0.5f, 0.0f, 0.5f);  // Purple
        default:
            return glm::vec3(0.8f, 0.8f, 0.8f);  // Light gray
    }
}

void Hud::drawGraduatedBar(glm::vec3 position, glm::vec2 size, bool horizontal, float cursorPos, glm::vec3 color, bool showCursor) {
    // Draw main bar background (darker) using colored rectangle
    game_interface->addColoredRectangle(
        position,
        size,
        glm::vec4(color * 0.3f, 0.8f),  // Darker version with transparency
        false,  // 2D
        -1      // No panel
    );
    
    // Draw graduation marks
    int numGrads = horizontal ? 4 : 2;  // Different number of graduations
    for (int i = 0; i <= numGrads; i++) {
        float gradPos = (float)i / (float)numGrads;
        
        if (horizontal) {
            // Horizontal graduations (vertical lines)
            float gradX = position.x + gradPos * size.x;
            game_interface->addColoredRectangle(
                glm::vec3(gradX - 1.0f, position.y - 5.0f, position.z + 0.1f),
                glm::vec2(2.0f, size.y + 10.0f),
                glm::vec4(color, 0.9f),
                false, -1
            );
        } else {
            // Vertical graduations (horizontal lines)
            float gradY = position.y + gradPos * size.y;
            game_interface->addColoredRectangle(
                glm::vec3(position.x - 5.0f, gradY - 1.0f, position.z + 0.1f),
                glm::vec2(size.x + 10.0f, 2.0f),
                glm::vec4(color, 0.9f),
                false, -1
            );
        }
    }
    
    // Draw position cursor (only if showCursor is true)
    if (showCursor) {
        if (horizontal) {
            float cursorX = position.x + cursorPos * size.x;
            game_interface->addColoredRectangle(
                glm::vec3(cursorX - 3.0f, position.y - 8.0f, position.z + 0.2f),
                glm::vec2(6.0f, size.y + 16.0f),
                glm::vec4(1.0f, 1.0f, 0.0f, 1.0f),  // Yellow cursor
                false, -1
            );
        } else {
            float cursorY = position.y + cursorPos * size.y;
            game_interface->addColoredRectangle(
                glm::vec3(position.x - 8.0f, cursorY - 3.0f, position.z + 0.2f),
                glm::vec2(size.x + 16.0f, 6.0f),
                glm::vec4(1.0f, 1.0f, 0.0f, 1.0f),  // Yellow cursor
                false, -1
            );
        }
    }
}

void Hud::drawRotationGauge(glm::vec3 position, glm::vec2 barSize, float rotation, glm::vec3 color, bool leftSide) {
    float centerY = position.y;
    float normalizedRotation = rotation / ROTATION_RANGE;  // Normalize to -1 to 1
    normalizedRotation = std::max(-1.0f, std::min(1.0f, normalizedRotation));
    
    // Calculate gauge height based on rotation
    float gaugeHeight = std::abs(normalizedRotation) * (barSize.y * 0.4f);  // Max 40% of bar height
    
    // Determine gauge position (above or below center based on rotation direction)
    float gaugeY = centerY + (normalizedRotation > 0 ? gaugeHeight/2 : -gaugeHeight/2);
    
    if (gaugeHeight > 1.0f) {  // Only draw if there's significant rotation
        game_interface->addColoredRectangle(
            glm::vec3(position.x, gaugeY - gaugeHeight/2, position.z + 0.1f),
            glm::vec2(barSize.x, gaugeHeight),
            glm::vec4(color, 0.9f),
            false, -1
        );
        
        // Draw center line for reference
        game_interface->addColoredRectangle(
            glm::vec3(position.x - 2.0f, centerY - 1.0f, position.z + 0.2f),
            glm::vec2(barSize.x + 4.0f, 2.0f),
            glm::vec4(1.0f, 1.0f, 1.0f, 0.7f),  // White reference line
            false, -1
        );
    }
}

Hud::~Hud() {
    delete game_interface;
    delete aim_image;
    delete dialogBoxTexture;
    if (leftPanelTexture) delete leftPanelTexture;
    if (rightPanelTexture) delete rightPanelTexture;
    if (cockpitFrameTexture) delete cockpitFrameTexture;
    if (currentDialog) delete currentDialog;
    if (scoreFeedback) delete scoreFeedback;
}