#ifndef ASTEROID_H
#define ASTEROID_H

#include "node.h"

class Asteroid{
public:
    Asteroid(Node* node);
    bool is_moving = false;

    float x_speed;
    float y_speed;

    Node* asteroid_node;
    int life = 15;
};

#endif