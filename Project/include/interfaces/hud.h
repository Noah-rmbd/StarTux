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
                   glm::mat4 view, glm::mat4 projection);
        void mouse(double xpos, double ypos);
        void newDialog(int number, double time);
        void scoreIncrement(int xpos, int ypos, double time, int score);
        
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
        
        int windowWidth;
        int windowHeight;
        float xPos;
        float yPos;
        glm::vec3 scoreColor = glm::vec3(1.0f);

        std::vector<std::string> dialogs = {"Welcome Tux, it's time to defeat MicroShip", "Nice shot", "Good job Tux, you got an extra life", "Oh no, a colision"};
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
        
        // Helper methods for 3D HUD
        void calculatePanelPositions();
        void renderLeftPanelContent(double time);
        void renderRightPanelContent(int life, double score, int fps);
        void renderCenterContent(int bullets, int speed, double time);
        void renderCursor();
        
        // Legacy 2D methods (for backward compatibility)
        void render2DHUD(int life, double score, int bullets, double time, int speed, int fps);
};

#endif