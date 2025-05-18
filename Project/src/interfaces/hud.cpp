#include "hud.h"
#include "glm/fwd.hpp"
#include "interface.h"
#include <cstddef>
#include <iostream>
#include <string>
#include <utility>
#ifndef TEXTURES_DIR
#error "TEXTURES_DIR not defined"
#endif

Hud::Hud(int width, int height) : windowWidth(width), windowHeight(height){
    std::string textures_dir = TEXTURES_DIR;
    aim_image = new Texture(textures_dir + "aim.png");
    game_interface = new Interface(windowWidth, windowHeight);
}

void Hud::update(int life, double score, double time, int speed, int fps) {
    // Render info and cursor
    std::string scoreString = std::to_string(score);
    scoreString = scoreString.substr(0, scoreString.length()-5);
    
    game_interface->renderText("Life : " + std::to_string(life), 25.0f, windowHeight-60, 0.5f, glm::vec3(1.0f));
    game_interface->renderText("FPS : " + std::to_string(fps), windowWidth-150, windowHeight-60, 0.5f, glm::vec3(1.0f));
    game_interface->renderText("Score : " + scoreString, 25.0f, windowHeight-100, 0.5f, scoreColor);
    game_interface->renderImage(aim_image, xPos, windowHeight-yPos, 50.0f, 50.0f);
    game_interface->renderText(std::to_string(speed) + " Km/h", 25.0, windowHeight-700, 0.8f, glm::vec3(1.0f));
    // Render dialogs
    if (currentDialog != NULL) {
        game_interface->renderText(currentDialog->first, 25.0, windowHeight-500, 0.5f, glm::vec3(1.0f));
        if(time - currentDialog->second > 10.0) {
            currentDialog = NULL;
        }
    }
    // Render score
    if (scoreFeedback != NULL) {
        game_interface->renderText(scoreFeedback->first.first, scoreFeedback->second.first, windowHeight-scoreFeedback->second.second, 0.5f, glm::vec3(1.0f));
        scoreColor = glm::vec3(0.0f, 0.0f, 255.0f);
        if(time - scoreFeedback->first.second > 1.0) {
            scoreFeedback = NULL;
        } else if (time - scoreFeedback->first.second > 0.25) {
            scoreColor = glm::vec3(1.0f);
        }
    } else {
        scoreColor = glm::vec3(1.0f);
    }
}

void Hud::newDialog(int number, double time) {
    currentDialog = new std::pair<std::string, double>(dialogs[number], time);
}

void Hud::mouse(double xpos, double ypos){
    xPos = xpos;
    yPos = ypos;
}

void Hud::scoreIncrement(int xpos, int ypos, double time){
    scoreFeedback = new std::pair<std::pair<std::string, double>, std::pair<int, int>>(
        std::make_pair("+50p", time),
        std::make_pair(xpos, ypos)
    );
}