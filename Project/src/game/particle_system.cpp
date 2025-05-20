#include "particle_system.h"
#include <GL/gl.h>
#include <algorithm>

void ParticleSystem::spawn(glm::vec3 pos, glm::vec3 velocity, float lifetime) {
  Particle p;
  p.position = pos;
  p.velocity = velocity;
  p.life = lifetime;
  particles.push_back(p);
}

void ParticleSystem::update(float dt) {
  for (auto &p : particles) {
    p.position += p.velocity * dt;
    p.life -= dt;
  }

  // Supprimer les particules mortes
  particles.erase(
      std::remove_if(particles.begin(), particles.end(),
                     [](const Particle &p) { return p.life <= 0.0f; }),
      particles.end());
}

void ParticleSystem::render() {
  glPointSize(5.0f);
  glBegin(GL_POINTS);
  for (const auto &p : particles) {
    glVertex3f(p.position.x, p.position.y, p.position.z);
  }
  glEnd();
}
