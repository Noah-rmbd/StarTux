#ifndef ASTEROID_H
#define ASTEROID_H

#include "node.h"

class Asteroid{
public:
    Asteroid(Node* node);

    Node* asteroid_node;
    int life = 15;
};

#endif