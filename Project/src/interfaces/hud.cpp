#include "hud.h"
#include "interface.h"
#include <cstddef>
#include <string>
#include <utility>

Hud::Hud(int width, int height) : windowWidth(width), windowHeight(height){
    game_interface = new Interface(windowWidth, windowHeight);
}

void Hud::update(int life, double score) {
    game_interface->renderText("Life : " + std::to_string(life), 25.0f, windowHeight-60, 0.5f, glm::vec3(1.0f));
    game_interface->renderText("Score : " + std::to_string(score), 25.0f, windowHeight-100, 0.5f, glm::vec3(1.0f));
    
    if (currentDialog != NULL) {
        game_interface->renderText(currentDialog->first, 25.0, windowHeight-500, 0.5f, glm::vec3(1.0f));
        if(currentDialog->second > 10.0) {
            currentDialog = NULL;
        }
    }
}

void Hud::newDialog(int number, double time) {
    currentDialog = new std::pair<std::string, double>(dialogs[number], time);
}