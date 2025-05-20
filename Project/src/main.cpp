#include "global.h"
#include "lighting_sphere.h"
#include "node.h"
#include "pyramid.h"
#include "shader.h"
#include "shape_model.h"
#include "texture.h"
#include "textured_sphere.h"
#include "viewer.h"
#include <string>

ParticleSystem particleSystem;
#ifndef SHADER_DIR
#error "SHADER_DIR not defined"
#endif

int main() {
  // create window, add shaders & scene objects, then run rendering loop
  Viewer viewer;

  viewer.run();
}
