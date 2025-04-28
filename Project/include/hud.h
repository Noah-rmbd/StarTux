#ifndef HUD_H
#define HUD_H

#include "interface.h"

class Hud {
    public:
        Hud(int width, int height);
        //~Hud();
        void update(int life, double score);

    private:
        Interface *game_interface;
        int windowWidth;
        int windowHeight;

};

#endif