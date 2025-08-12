#ifndef STARTUPSCREEN_H
#define STARTUPSCREEN_H

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

class StartupScreen {
    public:
        StartupScreen(int width, int height);
        ~StartupScreen();
        void update();
        void mouse(int button, int action, double xpos, double ypos);
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
        bool click_valid = false;
        bool start_game = false;
        
        // New features
        bool showStatistics = false;
        bool statsClick_valid = false;
        bool backClick_valid = false;
        
        // Mission and statistics systems
        DailyMissions* dailyMissions;
        GameStatistics* gameStatistics;
        
        // UI helper methods
        void renderMainMenu();
        void renderStatisticsScreen();
        void renderMissions();
        void checkButtonClicks(int button, int action, double xpos, double ypos);
};

#endif