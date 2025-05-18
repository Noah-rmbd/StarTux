#ifndef HUD_H
#define HUD_H

#include "interface.h"
#include "texture.h"
#include <string>
#include <vector>

class Hud {
    public:
        Hud(int width, int height);
        //~Hud();
        void update(int life, double score, double time, int speed, int fps);
        void mouse(double xpos, double ypos);
        void newDialog(int number, double time);
        void scoreIncrement(int xpos, int ypos, double time);

    private:
        Interface *game_interface;
        Texture *aim_image;
        int windowWidth;
        int windowHeight;
        float xPos;
        float yPos;
        glm::vec3 scoreColor = glm::vec3(1.0f);

        std::vector<std::string> dialogs = {"Welcum Tux, it's time to defeat MicroShip", "Nice cock", "Good job Tux, you got an extra life"};
        std::pair<std::string, double>* currentDialog;
        std::pair<std::pair<std::string, double>, std::pair<int, int>>* scoreFeedback;

};

#endif