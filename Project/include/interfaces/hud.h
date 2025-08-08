#ifndef HUD_H
#define HUD_H

#include "interface.h"
#include "texture.h"
#include <string>
#include <vector>
#include <glm/glm.hpp>

class Hud {
    public:
        Hud(int width, int height);
        ~Hud();
        
        // Main update method - now takes view and projection matrices for 3D HUD
        void update(int life, double score, int bullets, double time, int speed, int fps, 
                   glm::mat4 view, glm::mat4 projection, glm::vec3 playerPos = glm::vec3(0.0f), 
                   glm::vec3 playerRotation = glm::vec3(0.0f), int shipState = 0, bool paused = false, bool invincible = false, bool shieldActive = false, bool playerDying = false);
        void mouse(double xpos, double ypos);
        void newDialog(int number, double time);
        void scoreIncrement(int xpos, int ypos, double time, int score);
        
        enum DialogsIndex {
            WELCOME = 0,
            SHOOT_1 = 1,
            EXTRA_LIFE = 2,
            COLLISION_1 = 3,
            ACCELERATION_1 = 4
        };

        // 3D HUD configuration
        void setup3DHUD();
        void update3DPanels();

    private:
        Interface *game_interface;
        Texture *aim_image;
        Texture *leftPanelTexture;
        Texture *rightPanelTexture;
        Texture *cockpitFrameTexture;
        Texture *dialogBoxTexture;
        Texture *shieldIconTexture;
        
        int windowWidth;
        int windowHeight;
        float xPos;
        float yPos;
        glm::vec3 scoreColor = glm::vec3(1.0f);
        
        // Fade out system
        bool isFading = false;
        double fadeStartTime = 0.0;
        float fadeDuration = 0.5f; // 0.5 seconds to fade out
        float currentAlpha = 1.0f;
        
        std::vector<std::string> dialogs = {"Welcome Tux, it's time to defeat MicroShip", "Nice shot", "Good job Tux, you got an extra life", "Oh no, a colision", "Gotta go fast Son.. Hum Tux the hedgehog"};
        std::pair<std::string, double>* currentDialog;
        std::pair<std::pair<std::string, double>, std::pair<int, int>>* scoreFeedback;
        
        // 3D HUD panel IDs
        int leftPanelId;
        int rightPanelId;
        int cockpitFrameId;
        
        // 3D HUD configuration
        bool is3DHudEnabled;
        float panelDistance;      // Distance from camera
        float panelAngle;         // Angle of panels relative to forward direction
        float panelWidth;
        float panelHeight;
        
        // Panel positions and rotations
        glm::vec3 leftPanelPos;
        glm::vec3 rightPanelPos;
        glm::vec3 leftPanelRot;
        glm::vec3 rightPanelRot;
        
        // Position bar configuration
        static const float BAR_THICKNESS;
        static const float TOP_BAR_WIDTH;
        static const float SIDE_BAR_HEIGHT;
        static const float X_POSITION_RANGE;  // X position range: -1.5 to 1.5
        static const float Y_POSITION_RANGE;  // Y position range: -1.0 to 1.0
        static const float Z_POSITION_RANGE;  // Z position range (estimated)
        static const float ROTATION_RANGE;  // Maximum rotation angle in degrees
        
        enum ShipState {
            NORMAL = 0,
            ACCELERATING = 1,
            DAMAGED_LEFT = 2,
            DAMAGED_RIGHT = 3,
            DAMAGED_TOP = 4,
            DAMAGED_BOTTOM = 5,
            PROTECTED = 6,
            DYING = 7
        };
        
        // Helper methods for 3D HUD
        void calculatePanelPositions();
        void renderLeftPanelContent(int life, int bullets, double time, bool shieldActive = false);
        void renderRightPanelContent(double score, int speed, int fps);
        void renderCenterContent(double time, bool paused = false, bool invincible = false);
        void updateFade(double time, bool playerDying);
        void renderCursor();
        
        // Position and rotation bar methods
        void renderPositionBars(glm::vec3 playerPos, glm::vec3 playerRotation, int shipState);
        void renderTopPositionBar(float xPosition, int shipState);
        void renderLeftPositionBar(float zPosition, float yRotation, float zRotation, int shipState);
        void renderRightPositionBar(float yPosition, float yRotation, float xRotation, int shipState);
        glm::vec3 getBarColor(int shipState);
        void drawGraduatedBar(glm::vec3 position, glm::vec2 size, bool horizontal, float cursorPos, glm::vec3 color, bool showCursor = true);
        void drawRotationGauge(glm::vec3 position, glm::vec2 barSize, float rotation, glm::vec3 color, bool leftSide = true);
        
        // Legacy 2D methods (for backward compatibility)
        void render2DHUD(int life, double score, int bullets, double time, int speed, int fps, bool shieldActive = false, bool playerDying = false);
};

#endif