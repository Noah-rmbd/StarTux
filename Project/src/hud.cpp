#include "hud.h"
#include "interface.h"
#include <string>

Hud::Hud(int width, int height) : windowWidth(width), windowHeight(height){
    game_interface = new Interface(windowWidth, windowHeight);
}

void Hud::update(int life, double score) {
    game_interface->renderText("Life : " + std::to_string(life), 25.0f, windowHeight-60, 0.5f, glm::vec3(1.0f));
    game_interface->renderText("Score : " + std::to_string(score), 25.0f, windowHeight-100, 0.5f, glm::vec3(1.0f));
}