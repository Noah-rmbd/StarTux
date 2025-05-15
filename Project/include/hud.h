#ifndef HUD_H
#define HUD_H

#include "interface.h"
#include <string>
#include <vector>

class Hud {
    public:
        Hud(int width, int height);
        //~Hud();
        void update(int life, double score);
        void newDialog(int number, double time);

    private:
        Interface *game_interface;
        int windowWidth;
        int windowHeight;

        std::vector<std::string> dialogs = {"Welcum Tux, it's time to defeat MicroShip", "Nice cock"};
        std::pair<std::string, double>* currentDialog;

};

#endif