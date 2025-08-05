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

Hud::Hud(int width, int height) : windowWidth(width), windowHeight(height){
    std::string textures_dir = TEXTURES_DIR;
    
    // Load textures
    aim_image = new Texture(textures_dir + "aim.png");
    dialogBoxTexture = new Texture(textures_dir + "text_box.png");
    leftPanelTexture = nullptr;   // Will be loaded when you create the panel backgrounds
    rightPanelTexture = nullptr;
    cockpitFrameTexture = nullptr;
    
    game_interface = new Interface(windowWidth, windowHeight);
    
    // Initialize 3D HUD settings
    is3DHudEnabled = true;  // Enable 3D HUD
    panelDistance = 2.0f;   // Distance from camera
    panelAngle = 25.0f;     // Angle in degrees
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
                glm::mat4 view, glm::mat4 projection) {
    if (is3DHudEnabled) {
        // Use new 3D HUD system
        game_interface->beginFrame(view, projection);
        
        // Update panel positions based on camera
        update3DPanels();
        
        // Render different sections of the HUD (cursor last since it uses CURSOR layer)
        renderLeftPanelContent(time);
        renderRightPanelContent(life, score, fps);
        renderCenterContent(bullets, speed, time);
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
        render2DHUD(life, score, bullets, time, speed, fps);
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
        leftPanelTexture
    );
    
    // Create right panel (for main stats)
    rightPanelId = game_interface->createPanel(
        rightPanelPos, rightPanelRot, 
        glm::vec2(panelWidth, panelHeight), 
        rightPanelTexture
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

void Hud::renderLeftPanelContent(double time) {
    // Calculate left panel screen position (simulate 3D positioning)
    float leftPanelScreenX = windowWidth * 0.10f;  // Left side of screen
    float leftPanelScreenY = windowHeight * 0.6f;  // Middle height
    
    // Render dialogs on the left panel area
    if (currentDialog != nullptr) {
        // Calculate dialog box dimensions based on text length
        //float dialogWidth = std::min(400.0f, std::max(200.0f, currentDialog->first.length() * 12.0f));
        //float dialogHeight = 80.0f;
        float dialogWidth = 763.0f;
        float dialogHeight = 139.0f;
        
        // Render dialog box background first (behind text) - use deeper Z value
        game_interface->addImageElement(
            dialogBoxTexture,
            glm::vec3(leftPanelScreenX - 180.0f, leftPanelScreenY - 280.0f - dialogHeight/2, -0.1f), // Deeper Z for background
            glm::vec2(763.0f, 139.0f),                          // Size based on text
            glm::vec4(1.0f, 1.0f, 1.0f, 0.8f),                                                             // White with transparency
            false,                                                                                           // 2D image (screen space)
            -1                                                                                               // No panel attachment
        );
        
        // Render dialog text on top of the box (centered) - use closer Z value
        game_interface->addTextOverlay(
            currentDialog->first,
            glm::vec3(leftPanelScreenX - 120.0f, leftPanelScreenY - 270.0f, 0.1f), // Center text over dialog box, closer Z
            0.4f,                                                                   // Scale
            glm::vec3(1.0f, 1.0f, 1.0f),                                          // White text for readability on dark box
            false,                                                                  // 2D text (screen space)
            -1                                                                      // No panel attachment
        );
    }
}

void Hud::renderRightPanelContent(int life, double score, int fps) {
    // Format score string
    std::string scoreString = std::to_string(score);
    if (scoreString.length() > 5) {
        scoreString = scoreString.substr(0, scoreString.length()-5);
    }
    
    // Calculate right panel screen position (simulate 3D positioning)
    float rightPanelScreenX = windowWidth * 0.7f;  // Right side of screen
    float rightPanelScreenY = windowHeight * 0.8f; // Upper area
    
    // Life display on right panel
    game_interface->addTextOverlay(
        "LIFE: " + std::to_string(life),
        glm::vec3(rightPanelScreenX, rightPanelScreenY, 0.0f),       // Screen space position
        0.5f,                                                         // Scale
        glm::vec3(1.0f, 0.3f, 0.3f),                                // Red color
        false,                                                        // 2D text (screen space)
        -1                                                            // No panel attachment
    );
    
    // Score display on right panel
    game_interface->addTextOverlay(
        "SCORE: " + scoreString,
        glm::vec3(rightPanelScreenX, rightPanelScreenY - 50.0f, 0.0f), // Screen space position
        0.4f,                                                           // Scale
        scoreColor,                                                     // Dynamic color
        false,                                                          // 2D text (screen space)
        -1                                                              // No panel attachment
    );
    
    // FPS display on right panel
    game_interface->addTextOverlay(
        "FPS: " + std::to_string(fps),
        glm::vec3(rightPanelScreenX, rightPanelScreenY - 100.0f, 0.0f), // Screen space position
        0.3f,                                                            // Smaller scale
        glm::vec3(0.7f, 0.7f, 0.7f),                                   // Gray color
        false,                                                           // 2D text (screen space)
        -1                                                               // No panel attachment
    );
}

void Hud::renderCenterContent(int bullets, int speed, double time) {
    // Convert to proper screen coordinates for 2D text
    float centerX = windowWidth * 0.5f;
    float bottomY = 100.0f;  // Distance from bottom
    
    // Render ammunition counter at bottom center
    game_interface->addTextOverlay(
        std::to_string(bullets) + "/10",
        glm::vec3(centerX - 50.0f, bottomY + 40.0f, 0.0f),  // Screen space position
        0.8f,                                                 // Scale
        glm::vec3(1.0f, 1.0f, 0.0f),                        // Yellow color
        false,                                                // 2D text (screen space)
        -1                                                    // No panel attachment
    );
    
    // Render speed indicator at bottom center (slightly lower)
    game_interface->addTextOverlay(
        std::to_string(speed) + " KM/H",
        glm::vec3(centerX - 80.0f, bottomY, 0.0f),          // Screen space position
        0.6f,                                                 // Scale
        glm::vec3(0.0f, 1.0f, 0.0f),                        // Green color
        false,                                                // 2D text (screen space)
        -1                                                    // No panel attachment
    );

    // Render score feedback on left panel if active
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
void Hud::render2DHUD(int life, double score, int bullets, double time, int speed, int fps) {
    // Format score string
    std::string scoreString = std::to_string(score);
    if (scoreString.length() > 5) {
        scoreString = scoreString.substr(0, scoreString.length()-5);
    }
    
    // Render main stats with proper spacing
    game_interface->renderText("Life : " + std::to_string(life), 25.0f, windowHeight-60, 0.5f, glm::vec3(1.0f, 0.3f, 0.3f));
    game_interface->renderText("FPS : " + std::to_string(fps), windowWidth-150, windowHeight-60, 0.5f, glm::vec3(0.7f, 0.7f, 0.7f));
    game_interface->renderText("Score : " + scoreString, 25.0f, windowHeight-100, 0.5f, scoreColor);
    
    // Render bottom info with better spacing
    game_interface->renderText(std::to_string(bullets) + " /10", 25.0f, windowHeight-750, 0.5f, glm::vec3(1.0f, 1.0f, 0.0f));
    game_interface->renderText(std::to_string(speed) + " Km/h", 25.0f, windowHeight-800, 0.8f, glm::vec3(0.0f, 1.0f, 0.0f));
    
    // Render cursor
    game_interface->renderImage(aim_image, xPos, windowHeight-yPos, 50.0f, 50.0f);
    
    // Render dialogs
    if (currentDialog != nullptr) {
        game_interface->renderText(currentDialog->first, 25.0f, windowHeight-500, 0.5f, glm::vec3(0.0f, 1.0f, 1.0f));
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