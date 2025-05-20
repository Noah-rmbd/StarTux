#pragma once

#include <glm/glm.hpp>
#include <vector>

struct Particle {
  glm::vec3 position;
  glm::vec3 velocity;
  float life;
};

class ParticleSystem {
public:
  void spawn(glm::vec3 pos, glm::vec3 velocity, float lifetime);
  void update(float dt);
  void render();

private:
  std::vector<Particle> particles;
};
