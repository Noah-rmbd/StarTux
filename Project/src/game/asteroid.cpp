#include "asteroid.h"
#include "global.h" // Pour accéder au particleSystem

Asteroid::Asteroid(Node *node) { asteroid_node = node; }

void Asteroid::onHit() {
  life -= 1;
  if (life <= 0 && !isDestroyed) {
    explode();
  }
}

void Asteroid::explode() {
  isDestroyed = true;

  glm::vec3 pos = getPosition();

  for (int i = 0; i < 20; ++i) {
    glm::vec3 dir = glm::sphericalRand(1.0f); // direction aléatoire
    particleSystem.spawn(pos, dir, 2.0f); // particule avec durée de vie de 2s
  }
}

glm::vec3 Asteroid::getPosition() const {
  return glm::vec3(
      asteroid_node->transform_[3]); // extrait la position depuis la matrice
}
