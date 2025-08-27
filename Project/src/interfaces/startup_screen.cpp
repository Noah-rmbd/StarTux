#include "startup_screen.h"
#include "matrix_cache.h"
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
    logo_image = new Texture(textures_dir + "startuxlogo4.png");

    Shader *space_shader = new Shader(shader_dir + "texture.vert", shader_dir + "texture.frag");
    Texture *texture = new Texture(textures_dir + "space3.jpeg");
    Shape* space_sphere = new TexturedSphere(space_shader, texture);
    // Cache static space transformation matrix (computed once)
    auto& matrix_cache = MatrixCache::getInstance();
    matrix_cache.cacheStaticMatrix("startup_space", 
        glm::scale(glm::mat4(1.0f), 120.0f * glm::vec3(1.0f, 1.0f, 1.0f)));
    Node* space_node = new Node(matrix_cache.getMatrix("startup_space"));
    space_node->add(space_sphere);

    // Create ship's texture and shader
    Shader* ship_shader = new Shader(shader_dir + "ship.vert", shader_dir + "ship.frag");
    Texture* ship_texture = new Texture(ressources_dir + "Material.001_Base_color.jpg");
    // Create model with texture shader
    Shape* ship = new ShapeModel(ressources_dir + "ship.obj", ship_shader);
    static_cast<ShapeModel*>(ship)->setTexture(ship_texture);
    
    // Cache static ship transformation matrix (computed once)
    matrix_cache.cacheStaticMatrix("startup_ship", 
        glm::scale(glm::mat4(1.0f), 0.1f * glm::vec3(1.0f, 1.0f, 1.0f)));
    Node* ship_node = new Node(matrix_cache.getMatrix("startup_ship"));
    ship_node->add(ship);
    
    // Cache static environment transformation matrix (computed once)
    matrix_cache.cacheStaticMatrix("startup_environment", 
        glm::translate(glm::mat4(1.0f), glm::vec3(-0.7f, -0.5f, 0.0f)));
    const glm::mat4& environment_mat = matrix_cache.getMatrix("startup_environment");
    
    background_space = new Node(environment_mat);
    background_space->add(space_node);
    background_space->add(ship_node);
    
    // Initialize button regions for layered interface
    initializeButtonRegions();
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
    
    // Setup layered rendering system
    glm::mat4 orthoProjection = glm::ortho(0.0f, static_cast<float>(windowWidth), 0.0f, static_cast<float>(windowHeight), -10.0f, 10.0f);
    glm::mat4 orthoView = glm::mat4(1.0f);
    
    start_interface->beginFrame(orthoView, orthoProjection);
    
    // Render appropriate screen with layered system
    if (showStatistics) {
        renderStatisticsScreenLayered();
    } else {
        renderMainMenuLayered();
    }
    
    start_interface->endFrame();
}

void StartupScreen::mouse(int button, int action, double xpos, double ypos) {
    checkButtonClicks(button, action, xpos, ypos);
}

void StartupScreen::mouseMove(double xpos, double ypos) {
    updateButtonStates(xpos, ypos);
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

void StartupScreen::renderMainMenuLayered() {
    // Background panel with logo
    float logoZ = 0.10f;
    float logoWidth = 300.0f;
    float logoHeight = 168.8f;
    float logoX = windowWidth * 0.8f - logoWidth / 2;
    float logoY = windowHeight * 0.8f - logoHeight / 2;
    
    start_interface->addImageElement(logo_image, 
                                   glm::vec3(logoX, logoY, logoZ), 
                                   glm::vec2(logoWidth, logoHeight),
                                   glm::vec4(1.0f), false, -1);
    
    // Play button background
    float playButtonZ = -1.0f;
    glm::vec3 playButtonColor = glm::vec3(0.0f, 0.8f, 0.0f);
    if (playButtonHovered) playButtonColor = glm::vec3(0.0f, 1.0f, 0.0f);
    
    start_interface->addColoredRectangle(glm::vec3(playButtonRegion.x, playButtonRegion.y, playButtonZ),
                                       glm::vec2(playButtonRegion.width, playButtonRegion.height),
                                       glm::vec4(playButtonColor.r, playButtonColor.g, playButtonColor.b, 0.3f),
                                       false, -1);
    
    start_interface->addTextOverlay("Play", 
                                  glm::vec3(playButtonRegion.x + playButtonRegion.width/2 - 52, 
                                           playButtonRegion.y + playButtonRegion.height/2 - 12, 0.0f), 
                                  1.0f, playButtonColor, false, -1, FontType::NASALIZATION);
    
    // Statistics button background
    glm::vec3 statsButtonColor = glm::vec3(1.0f, 1.0f, 0.0f);
    if (statsButtonHovered) statsButtonColor = glm::vec3(1.0f, 1.0f, 0.5f);
    
    start_interface->addColoredRectangle(glm::vec3(statsButtonRegion.x, statsButtonRegion.y, playButtonZ),
                                       glm::vec2(statsButtonRegion.width, statsButtonRegion.height),
                                       glm::vec4(statsButtonColor.r, statsButtonColor.g, statsButtonColor.b, 0.3f),
                                       false, -1);
    
    start_interface->addTextOverlay("Statistics", 
                                  glm::vec3(statsButtonRegion.x + statsButtonRegion.width/2 - 110, 
                                           statsButtonRegion.y + statsButtonRegion.height/2 - 12, 0.0f), 
                                  1.0f, statsButtonColor, false, -1, FontType::NASALIZATION);
    
    // Render missions in the left panel
    renderMissionsLayered();
}

void StartupScreen::renderStatisticsScreenLayered() {
    // Semi-transparent overlay panel
    float panelZ = -1.0f;
    float panelWidth = windowWidth * 0.8f;
    float panelHeight = windowHeight * 0.8f;
    float panelX = (windowWidth - panelWidth) / 2;
    float panelY = (windowHeight - panelHeight) / 2;
    
    start_interface->addColoredRectangle(glm::vec3(panelX, panelY, panelZ),
                                       glm::vec2(panelWidth, panelHeight),
                                       glm::vec4(0.0f, 0.0f, 0.2f, 0.9f),
                                       false, -1);
    
    // Title
    start_interface->addTextOverlay("LIFETIME STATISTICS", 
                                  glm::vec3(panelX + panelWidth/2 - 150, panelY + panelHeight - 60, 0.0f), 
                                  1.0f, glm::vec3(1.0f, 1.0f, 1.0f), false, -1, FontType::ROBOTO);
    
    // Statistics content
    float leftColumnX = panelX + 50.0f;
    float statsStartY = panelY + panelHeight - 120.0f;
    float lineHeight = 40.0f;
    
    // Get lifetime stats
    GameStatistics::LifetimeStats lifetimeStats = gameStatistics->lifetime;
    
    start_interface->addTextOverlay("Games Played: " + std::to_string(lifetimeStats.gamesPlayed), 
                                  glm::vec3(leftColumnX, statsStartY, 0.0f), 
                                  0.5f, glm::vec3(1.0f, 1.0f, 1.0f), false, -1, FontType::ROBOTO);
    
    start_interface->addTextOverlay("High Score: " + std::to_string(lifetimeStats.highScore), 
                                  glm::vec3(leftColumnX, statsStartY - lineHeight, 0.0f), 
                                  0.5f, glm::vec3(1.0f, 1.0f, 1.0f), false, -1, FontType::ROBOTO);
    
    start_interface->addTextOverlay("Total Asteroids Destroyed: " + std::to_string(lifetimeStats.totalAsteroidsDestroyed), 
                                  glm::vec3(leftColumnX, statsStartY - 2*lineHeight, 0.0f), 
                                  0.5f, glm::vec3(1.0f, 1.0f, 1.0f), false, -1, FontType::ROBOTO);
    
    start_interface->addTextOverlay("Total Rings Collected: " + std::to_string(lifetimeStats.totalRingsTaken), 
                                  glm::vec3(leftColumnX, statsStartY - 3*lineHeight, 0.0f), 
                                  0.5f, glm::vec3(1.0f, 1.0f, 0.0f), false, -1, FontType::ROBOTO);
    
    start_interface->addTextOverlay("Max Speed Ever: " + std::to_string(lifetimeStats.maxSpeedEverReached) + " Km/h", 
                                  glm::vec3(leftColumnX, statsStartY - 4*lineHeight, 0.0f), 
                                  0.5f, glm::vec3(0.0f, 1.0f, 0.0f), false, -1, FontType::ROBOTO);
    
    start_interface->addTextOverlay("Best Ring Streak: " + std::to_string(lifetimeStats.bestConsecutiveRings), 
                                  glm::vec3(leftColumnX, statsStartY - 5*lineHeight, 0.0f), 
                                  0.5f, glm::vec3(1.0f, 0.5f, 0.0f), false, -1, FontType::ROBOTO);
    
    // Play time in minutes
    int totalMinutes = static_cast<int>(lifetimeStats.totalPlayTime / 60.0);
    start_interface->addTextOverlay("Total Play Time: " + std::to_string(totalMinutes) + " minutes", 
                                  glm::vec3(leftColumnX, statsStartY - 6*lineHeight, 0.0f), 
                                  0.5f, glm::vec3(0.8f, 0.8f, 0.8f), false, -1, FontType::ROBOTO);
    
    // Back button
    glm::vec3 backButtonColor = glm::vec3(0.8f, 0.2f, 0.2f);
    if (backButtonHovered) backButtonColor = glm::vec3(1.0f, 0.4f, 0.4f);
    
    start_interface->addColoredRectangle(glm::vec3(backButtonRegion.x, backButtonRegion.y, 0.0f),
                                       glm::vec2(backButtonRegion.width, backButtonRegion.height),
                                       glm::vec4(backButtonColor.r, backButtonColor.g, backButtonColor.b, 0.4f),
                                       false, -1);
    
    start_interface->addTextOverlay("BACK", 
                                  glm::vec3(backButtonRegion.x + backButtonRegion.width/2 - 25, 
                                           backButtonRegion.y + backButtonRegion.height/2 - 12, 0.0f), 
                                  0.6f, backButtonColor, false, -1, FontType::ROBOTO);
}

void StartupScreen::renderMissionsLayered() {
    // Missions panel background
    float missionsPanelZ = -1.5f;
    float missionsPanelWidth = windowWidth * 0.45f;
    float missionsPanelHeight = windowHeight * 0.6f;
    float missionsPanelX = 30.0f;
    float missionsPanelY = (windowHeight - missionsPanelHeight) / 2;
    
    start_interface->addColoredRectangle(glm::vec3(missionsPanelX, missionsPanelY, missionsPanelZ),
                                       glm::vec2(missionsPanelWidth, missionsPanelHeight),
                                       glm::vec4(0.1f, 0.1f, 0.3f, 0.7f),
                                       false, -1);
    
    // Title
    start_interface->addTextOverlay("DAILY MISSIONS", 
                                  glm::vec3(missionsPanelX + 20, missionsPanelY + missionsPanelHeight - 50, 0.0f), 
                                  0.6f, glm::vec3(1.0f, 1.0f, 0.0f), false, -1, FontType::ROBOTO);
    
    // Mission list
    auto missions = dailyMissions->getTodaysMissions();
    float missionY = missionsPanelY + missionsPanelHeight - 100.0f;
    
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
        
        start_interface->addTextOverlay(missionText, 
                                      glm::vec3(missionsPanelX + 20, missionY, 0.0f), 
                                      0.35f, statusColor, false, -1, FontType::ROBOTO);
        
        missionY -= 50.0f;
    }
}

void StartupScreen::initializeButtonRegions() {
    // Play button region - center right area
    playButtonRegion = {
        windowWidth * 0.7f,           // x
        windowHeight * 0.4f,          // y  
        300.0f,                       // width
        80.0f                         // height
    };
    
    // Statistics button region - below play button
    statsButtonRegion = {
        windowWidth * 0.7f,           // x
        windowHeight * 0.2f,          // y
        300.0f,                       // width
        80.0f                         // height
    };
    
    // Back button region - bottom center of stats screen
    backButtonRegion = {
        windowWidth * 0.5f - 50.0f,   // x
        50.0f,                        // y
        100.0f,                       // width
        40.0f                         // height
    };
}

void StartupScreen::updateButtonStates(double xpos, double ypos) {
    // Convert GLFW coordinates (top-left origin) to OpenGL coordinates (bottom-left origin)
    float glY = windowHeight - ypos;
    
    // Update hover states
    playButtonHovered = isPointInRegion(xpos, glY, playButtonRegion);
    statsButtonHovered = isPointInRegion(xpos, glY, statsButtonRegion);
    backButtonHovered = isPointInRegion(xpos, glY, backButtonRegion);
}

bool StartupScreen::isPointInRegion(float x, float y, const ButtonRegion& region) {
    return x >= region.x && x <= region.x + region.width &&
           y >= region.y && y <= region.y + region.height;
}

void StartupScreen::checkButtonClicks(int button, int action, double xpos, double ypos) {
    // Convert GLFW coordinates to OpenGL coordinates
    float glY = windowHeight - ypos;
    
    if (!showStatistics) {
        // Play button
        if (action == GLFW_PRESS && button == 0 && isPointInRegion(xpos, glY, playButtonRegion)) {
            playButtonPressed = true;
        }
        if (playButtonPressed && action == GLFW_RELEASE && button == 0) {
            if (isPointInRegion(xpos, glY, playButtonRegion)) {
                start_game = true;
            }
            playButtonPressed = false;
        }
        
        // Statistics button
        if (action == GLFW_PRESS && button == 0 && isPointInRegion(xpos, glY, statsButtonRegion)) {
            statsButtonPressed = true;
        }
        if (statsButtonPressed && action == GLFW_RELEASE && button == 0) {
            if (isPointInRegion(xpos, glY, statsButtonRegion)) {
                showStatistics = true;
            }
            statsButtonPressed = false;
        }
    } else {
        // Back button in statistics screen
        if (action == GLFW_PRESS && button == 0 && isPointInRegion(xpos, glY, backButtonRegion)) {
            backButtonPressed = true;
        }
        if (backButtonPressed && action == GLFW_RELEASE && button == 0) {
            if (isPointInRegion(xpos, glY, backButtonRegion)) {
                showStatistics = false;
            }
            backButtonPressed = false;
        }
    }
    
    // Update hover states
    updateButtonStates(xpos, ypos);
}