#pragma once

#include <glm/glm.hpp>
#include "interface.h"
#include "texture.h"
#include "shader.h"
#include "shape.h"
#include "textured_sphere.h"
#include "node.h"
#include "shape_model.h"
#include "missions.h"
#include "statistics.h"

// Button region structure for precise click detection
struct ButtonRegion {
    float x, y;          // Position (bottom-left origin)
    float width, height; // Dimensions
};

class StartupScreen {
    public:
        StartupScreen(int width, int height);
        ~StartupScreen();
        void update();
        void mouse(int button, int action, double xpos, double ypos);
        void mouseMove(double xpos, double ypos);
        void keyHandler(int key, int action);
        bool isLaunched();
        
        // New functionality
        bool isShowingStatistics() const { return showStatistics; }
        void setShowStatistics(bool show) { showStatistics = show; }
        DailyMissions* getMissionsManager() { return dailyMissions; }
    
    private:
        Interface *start_interface;
        Texture *logo_image;
        Node *background_space;
        int windowWidth;
        int windowHeight;
        float angle = 0.0f;
        bool start_game = false;
        
        // New features
        bool showStatistics = false;
        
        // Mission and statistics systems
        DailyMissions* dailyMissions;
        GameStatistics* gameStatistics;
        
        // Button regions for precise interaction
        ButtonRegion playButtonRegion;
        ButtonRegion statsButtonRegion;
        ButtonRegion backButtonRegion;
        
        // Button states
        bool playButtonHovered = false;
        bool statsButtonHovered = false;
        bool backButtonHovered = false;
        bool playButtonPressed = false;
        bool statsButtonPressed = false;
        bool backButtonPressed = false;
        
        // UI helper methods
        void initializeButtonRegions();
        void updateButtonStates(double xpos, double ypos);
        bool isPointInRegion(float x, float y, const ButtonRegion& region);
        void renderMainMenuLayered();
        void renderStatisticsScreenLayered();
        void renderMissionsLayered();
        void checkButtonClicks(int button, int action, double xpos, double ypos);
};