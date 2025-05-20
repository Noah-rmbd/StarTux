#ifndef ASTEROID_H
#define ASTEROID_H

#pragma once

#include "node.h"
#include <glm/glm.hpp>

class Asteroid {
public:
  Asteroid(Node *node);

  void onHit();   // Appelée quand l'astéroïde est touché
  void explode(); // Déclenche l'explosion
  glm::vec3 getPosition() const;

  int life = 15;
  Node *asteroid_node;

private:
  bool isDestroyed = false;
};

#endif
