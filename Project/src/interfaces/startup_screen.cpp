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
    float imageWidth = 373.0f;
    float imageHeight = 960.0f;
    
    float centerX = 1094.0f;
    float centerY = 480.0f;
    angle += 0.01f;
    background_space->transform_ = glm::translate(glm::mat4(1.0f), glm::vec3(-0.7f, -0.5f, 0.0f))
        * glm::scale(glm::mat4(1.0f), 1.0f * glm::vec3(1.0f, 1.0f, 1.0f))
        * glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 5.0f) + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    start_interface->renderImage(logo_image, centerX, centerY, imageWidth, imageHeight);
    background_space->draw(model, view, projection);
}

void StartupScreen::mouse(int button, int action, double xpos, double ypos) {
    if (action == GLFW_PRESS && button == 0 && xpos <= 1240.0 && xpos>= 954.0 && ypos <= 692.0 && ypos >= 622.0) {
        click_valid = true; // means the click started inhe button
    }
    if (click_valid && action == GLFW_RELEASE && button == 0 && xpos <= 1240.0 && xpos>= 954.0 && ypos <= 692.0 && ypos >= 622.0) {
        click_valid = false;
        start_game = true;
    }
    else if(click_valid && action == GLFW_RELEASE){
        click_valid = false;
    }
}

bool StartupScreen::isLaunched() {
    return start_game;
}

StartupScreen::~StartupScreen(){
    delete start_interface;
}