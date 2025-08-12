#include "startup_screen.h"
#ifndef SHADER_DIR
#error "SHADER_DIR not defined"
#endif
#ifndef TEXTURES_DIR
#error "TEXTURES_DIR not defined"
#endif
#ifndef RESSOURCES_DIR
#error "RESSOURCES_DIR not defined"
#endif

StartupScreen::StartupScreen(int width, int height) : windowWidth(width), windowHeight(height) {
    std::string shader_dir = SHADER_DIR;
    std::string textures_dir = TEXTURES_DIR;
    std::string ressources_dir = RESSOURCES_DIR;

    start_interface = new Interface(width, height);
    
    // Initialize missions and statistics systems
    dailyMissions = new DailyMissions();
    gameStatistics = new GameStatistics();
    logo_image = new Texture(textures_dir + "start_banner.png");

    Shader *space_shader = new Shader(shader_dir + "texture.vert", shader_dir + "texture.frag");
    Texture *texture = new Texture(textures_dir + "space3.jpeg");
    Shape* space_sphere = new TexturedSphere(space_shader, texture);
    glm::mat4 space_mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f))
        * glm::scale(glm::mat4(1.0f), 120.0f * glm::vec3(1.0f, 1.0f, 1.0f))
        * glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    Node* space_node = new Node(space_mat);
    space_node->add(space_sphere);

    // Create ship's texture and shader
    Shader* ship_shader = new Shader(shader_dir + "ship.vert", shader_dir + "ship.frag");
    Texture* ship_texture = new Texture(ressources_dir + "Material.001_Base_color.jpg");
    // Create model with texture shader
    Shape* ship = new ShapeModel(ressources_dir + "ship.obj", ship_shader);
    static_cast<ShapeModel*>(ship)->setTexture(ship_texture);
    
    glm::mat4 ship_mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f))
        * glm::scale(glm::mat4(1.0f), 0.1f * glm::vec3(1.0f, 1.0f, 1.0f))
        * glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    Node* ship_node = new Node(ship_mat);
    ship_node->add(ship);
    
    glm::mat4 environment_mat = glm::translate(glm::mat4(1.0f), glm::vec3(-0.7f, -0.5f, 0.0f))
        * glm::scale(glm::mat4(1.0f), 1.0f * glm::vec3(1.0f, 1.0f, 1.0f))
        * glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    background_space = new Node(environment_mat);
    background_space->add(space_node);
    background_space->add(ship_node);
}

void StartupScreen::update(){
    // Update background animation
    angle += 0.01f;
    background_space->transform_ = glm::translate(glm::mat4(1.0f), glm::vec3(-0.7f, -0.5f, 0.0f))
        * glm::scale(glm::mat4(1.0f), 1.0f * glm::vec3(1.0f, 1.0f, 1.0f))
        * glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));

    // Render 3D background
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 5.0f) + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    background_space->draw(model, view, projection);
    
    // Render appropriate screen
    if (showStatistics) {
        renderStatisticsScreen();
    } else {
        renderMainMenu();
    }
}

void StartupScreen::mouse(int button, int action, double xpos, double ypos) {
    checkButtonClicks(button, action, xpos, ypos);
}

void StartupScreen::keyHandler(int key, int action) {
    // Handle back button in statistics screen
    if (showStatistics && action == GLFW_PRESS && key == GLFW_KEY_B) {
        showStatistics = false;
    }
}

bool StartupScreen::isLaunched() {
    return start_game;
}

StartupScreen::~StartupScreen(){
    delete start_interface;
    delete dailyMissions;
    delete gameStatistics;
}

void StartupScreen::renderMainMenu() {
    float imageWidth = 373.0f;
    float imageHeight = 960.0f;
    float centerX = 1110.0f;
    float centerY = 0.0f;
    
    // Render logo
    start_interface->renderImage(logo_image, centerX, centerY, imageWidth, imageHeight);
    
    // Render "PLAY" text over the clickable area (1154-1440 x, 622-692 y)
    start_interface->renderText("PLAY", 1270.0f, 650.0f, 1.0f, glm::vec3(0.0f, 1.0f, 0.0f), FontType::ROBOTO);
    
    // Render "STATISTICS" button below the logo area
    start_interface->renderText("STATISTICS", 1200.0f, 500.0f, 0.8f, glm::vec3(0.0f, 0.5f, 1.0f), FontType::ROBOTO);
    
    // Render missions
    renderMissions();
}

void StartupScreen::renderStatisticsScreen() {
    // Title
    start_interface->renderText("LIFETIME STATISTICS", windowWidth/2 - 250.0f, windowHeight - 100.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f), FontType::ROBOTO);
    
    // Statistics content - simple vertical list
    float leftColumnX = 200.0f;
    float statsStartY = windowHeight - 200.0f;
    float lineHeight = 40.0f;
    
    // Get lifetime stats
    GameStatistics::LifetimeStats lifetimeStats = gameStatistics->lifetime;
    
    start_interface->renderText("Games Played: " + std::to_string(lifetimeStats.gamesPlayed), leftColumnX, statsStartY, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f), FontType::ROBOTO);
    start_interface->renderText("High Score: " + std::to_string(lifetimeStats.highScore), leftColumnX, statsStartY - lineHeight, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f), FontType::ROBOTO);
    start_interface->renderText("Total Asteroids Destroyed: " + std::to_string(lifetimeStats.totalAsteroidsDestroyed), leftColumnX, statsStartY - 2*lineHeight, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f), FontType::ROBOTO);
    start_interface->renderText("Total Rings Collected: " + std::to_string(lifetimeStats.totalRingsTaken), leftColumnX, statsStartY - 3*lineHeight, 0.5f, glm::vec3(1.0f, 1.0f, 0.0f), FontType::ROBOTO);
    start_interface->renderText("Max Speed Ever: " + std::to_string(lifetimeStats.maxSpeedEverReached) + " Km/h", leftColumnX, statsStartY - 4*lineHeight, 0.5f, glm::vec3(0.0f, 1.0f, 0.0f), FontType::ROBOTO);
    start_interface->renderText("Best Ring Streak: " + std::to_string(lifetimeStats.bestConsecutiveRings), leftColumnX, statsStartY - 5*lineHeight, 0.5f, glm::vec3(1.0f, 0.5f, 0.0f), FontType::ROBOTO);
    
    // Play time in minutes
    int totalMinutes = static_cast<int>(lifetimeStats.totalPlayTime / 60.0);
    start_interface->renderText("Total Play Time: " + std::to_string(totalMinutes) + " minutes", leftColumnX, statsStartY - 6*lineHeight, 0.5f, glm::vec3(0.8f, 0.8f, 0.8f), FontType::ROBOTO);
    
    // Back button text
    start_interface->renderText("BACK (Press B)", windowWidth/2 - 100.0f, 100.0f, 0.6f, glm::vec3(0.8f, 0.2f, 0.2f), FontType::ROBOTO);
}

void StartupScreen::renderMissions() {
    // Title
    start_interface->renderText("DAILY MISSIONS", 50.0f, windowHeight - 100.0f, 0.6f, glm::vec3(1.0f, 1.0f, 0.0f), FontType::ROBOTO);
    
    // Mission list
    auto missions = dailyMissions->getTodaysMissions();
    float missionY = windowHeight - 150.0f;
    
    for (size_t i = 0; i < missions.size() && i < 3; i++) {
        const auto& mission = missions[i];
        
        // Mission status color
        glm::vec3 statusColor = glm::vec3(0.7f, 0.7f, 0.7f); // Default gray
        if (mission.status == MissionStatus::IN_PROGRESS) {
            statusColor = glm::vec3(1.0f, 1.0f, 0.0f); // Yellow
        } else if (mission.status == MissionStatus::COMPLETED) {
            statusColor = glm::vec3(0.0f, 1.0f, 0.0f); // Green
        }
        
        // Mission title and progress
        std::string missionText = mission.title + ": " + mission.description + 
                                 " (" + std::to_string(mission.currentProgress) + "/" + std::to_string(mission.targetValue) + ")";
        
        start_interface->renderText(missionText, 70.0f, missionY, 0.35f, statusColor, FontType::ROBOTO);
        
        missionY -= 50.0f;
    }
}

void StartupScreen::checkButtonClicks(int button, int action, double xpos, double ypos) {
    if (!showStatistics) {
        // Main menu buttons
        // Play button (existing coordinates)
        if (action == GLFW_PRESS && button == 0 && xpos <= 1440.0 && xpos >= 1154.0 && ypos <= 692.0 && ypos >= 622.0) {
            click_valid = true;
        }
        if (click_valid && action == GLFW_RELEASE && button == 0 && xpos <= 1440.0 && xpos >= 1154.0 && ypos <= 692.0 && ypos >= 622.0) {
            click_valid = false;
            start_game = true;
        }
        else if(click_valid && action == GLFW_RELEASE) {
            click_valid = false;
        }
        
        // Statistics button (text area around 1200, 500)
        if (action == GLFW_PRESS && button == 0 && xpos <= 1450.0 && xpos >= 1150.0 && ypos <= 520.0 && ypos >= 480.0) {
            statsClick_valid = true;
        }
        if (statsClick_valid && action == GLFW_RELEASE && button == 0 && xpos <= 1450.0 && xpos >= 1150.0 && ypos <= 520.0 && ypos >= 480.0) {
            statsClick_valid = false;
            showStatistics = true;
        }
        else if(statsClick_valid && action == GLFW_RELEASE) {
            statsClick_valid = false;
        }
    } else {
        // Statistics screen - Back button
        float panelWidth = 800.0f;
        float panelHeight = 600.0f;
        float panelX = (windowWidth - panelWidth) / 2.0f;
        float panelY = (windowHeight - panelHeight) / 2.0f;
        
        if (action == GLFW_PRESS && button == 0 && 
            xpos >= panelX + 50.0f && xpos <= panelX + 150.0f && 
            ypos >= panelY + 50.0f && ypos <= panelY + 90.0f) {
            backClick_valid = true;
        }
        if (backClick_valid && action == GLFW_RELEASE && button == 0 && 
            xpos >= panelX + 50.0f && xpos <= panelX + 150.0f && 
            ypos >= panelY + 50.0f && ypos <= panelY + 90.0f) {
            backClick_valid = false;
            showStatistics = false;
        }
        else if(backClick_valid && action == GLFW_RELEASE) {
            backClick_valid = false;
        }
    }
}